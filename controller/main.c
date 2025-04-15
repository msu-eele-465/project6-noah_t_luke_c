#include "intrinsics.h"
#include <msp430.h>
#include "keypad.h"
#include "msp430fr2355.h"
#include <math.h>

#define unlock_code "1738"

unsigned char data = 0x00;

int lock_status = 1;

volatile char key_pressed = 0;
volatile int key_flag = 0;

float ADC_Result;
float temp;
float calc;

void i2c_config(){
    // Configure USCI_B0 for I2C mode
    
    UCB0CTLW0 |= UCSWRST;                   // Software reset enabled

    UCB0CTLW0 |= UCSSEL__SMCLK;
    UCB0BRW = 10;
    
    UCB0CTLW0 |= UCMODE_3 | UCMST | UCTR;   // I2C mode, Master mode, TX
    UCB0CTLW1 |= UCASTP_2;                  // Automatic stop generated
                                            // after UCB0TBCNT is reached

    UCB0TBCNT = 0x0001;                     // number of bytes to be sent
    UCB0I2CSA = 0x0A;                       // Slave address
                                            // Two slaves are being used, 0x0A is the LEDbar, 0x0B is the LCD
                                            // When one of those keys is pressed, update slave address, send data, set back to 0x00  
    // I2C pins, 1.2 SDA, 1.3 SCL
    P1SEL1 &= ~(BIT2 & BIT3);
    P1SEL0 |= BIT2 | BIT3;     

    // Disable reset mode
    UCB0CTLW0 &= ~UCSWRST;

    // I2C interrupt
    UCB0IE |= UCTXIE0; 
}

void timer_setup(){
    // Setup Timer B0
    TB0CTL |= TBCLR;  // Clear timer and dividers
    TB0CTL |= TBSSEL__ACLK;  // Use ACLK
    TB0CTL |= MC__UP;  // Up counting mode
    TB0CCR0 = 32768;    // Compare value
    TB0CCR1 = 16384;    // CCR1 value

    // Set up timer compare IRQs
    TB0CCTL0 &= ~CCIFG;  // Clear CCR0 flag
    TB0CCTL0 |= CCIE;  // Enable flag

    // Set up timer compare IRQs
    TB0CCTL1 &= ~CCIFG;  // Clear CCR1 flag
    TB0CCTL1 |= CCIE;  // Enable flag
}

void io_pins_config(){
    // P1.0 Heartbeat LED
    P1DIR |= BIT0;
    P1OUT &= ~BIT0;

    // P4.3 Cool
    P4DIR |= BIT3;
    P4OUT &= ~BIT3;

    // P4.2 Heat
    P4DIR |= BIT2;
    P4OUT &= ~BIT2;

    // Configure ADC A1 pin
    P1SEL0 |= BIT1;
    P1SEL1 |= BIT1;
}

void adc_config(){
    // Configure ADC12
    ADCCTL0 |= ADCSHT_2 | ADCON;                             // ADCON, S&H=16 ADC clks
    ADCCTL1 |= ADCSHP;                                       // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~ADCRES;                                      // clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;                                     // 12-bit conversion results
    ADCMCTL0 |= ADCINCH_1 | ADCSREF_1;                       // A1 ADC input select; Vref=AVCC
    ADCIE |= ADCIE0; 
}

int main(void)
{
    // Stop WDT
    WDTCTL = WDTPW | WDTHOLD;

    // Setup I2C
    i2c_config();

    // Setup keypad
    keypad_config();

    // Setup timer
    timer_setup();

    // Other pins
    io_pins_config();

    // Config the ADC
    adc_config();

    // Disable low power mode
    PM5CTL0 &= ~LOCKLPM5; 

    // Enable interrupts
    __enable_interrupt();

    lock_keypad(unlock_code);
    
    while (1) {
        if(key_flag == 1){
            switch (key_pressed) {
                case 'D' : 
                    data = 0x00;
                    UCB0CTLW0 |= UCTXSTT;
                    P4OUT &= ~(BIT2 | BIT3);
                    lock_keypad(unlock_code);
                    lock_status = 0;
                    break;

                case 'A' : 
                    // Heat
                    // P4.2 is heat
                    P4OUT |= BIT2;
                    P4OUT &= ~BIT3;
                    data = 0x01;
                    UCB0CTLW0 |= UCTXSTT;
                    break;

                case 'B' : 
                    // Cool
                    // P4.3 is cool
                    P4OUT |= BIT3;
                    P4OUT &= ~BIT2;
                    data = 0x02;
                    UCB0CTLW0 |= UCTXSTT;
                    break;

                case 'C' : 
                    // Match Ambient

                    data = 0x03;
                    UCB0CTLW0 |= UCTXSTT;
                    break;
                    
                default: break;                
            }
            
            key_flag = 0;
        }
    }
}

#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){
    UCB0TXBUF = data;
    return;
}

#pragma vector = PORT3_VECTOR
__interrupt void Port_3(void) {

    key_pressed = scanPad();

    key_flag = 1;

    P3IFG &= ~(BIT0 | BIT1 | BIT2 | BIT3); // Clear flags
}


#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_TB0_CCR0(void){
    P1OUT ^= BIT0;
    TB0CCTL0 &= ~CCIFG;
}

#pragma vector = TIMER0_B1_VECTOR
__interrupt void Timer_TB0_CCR1(void){
    
    TB0CCTL1 &= ~CCIFG;
}

