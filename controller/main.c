#include "intrinsics.h"
#include <msp430.h>
#include "keypad.h"
#include "msp430fr2355.h"
#include <math.h>

#define unlock_code "1738"

unsigned char data = 0x00;
int plant[4];

int lock_status = 1;

volatile char key_pressed = 0;
volatile int key_flag = 0;

float ADC_Result;
float ambient_temp;
float calc;
int avg_ambient;
int ambient[4];
int n = 0;
int real_temp;
int thousands;
int hundreds;
int tens;

unsigned char msb_bank;
int msb_status;
short plant_out;
float plant_temp;
int real_plant;
int avg_plant;

char temp_to_send = 0;

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
    UCB0IE |= UCTXIE0 | UCRXIE0;
}

void timer_setup(){
    // Setup Timer B0
    TB0CTL |= TBCLR;  // Clear timer and dividers
    TB0CTL |= TBSSEL__ACLK;  // Use ACLK
    TB0CTL |= MC__UP;  // Up counting mode
    TB0CCR0 = 32768;    // Compare value
    //TB0CCR1 = 16384;    // CCR1 value

    // Set up timer compare IRQs
    TB0CCTL0 &= ~CCIFG;  // Clear CCR0 flag
    TB0CCTL0 |= CCIE;  // Enable flag

    // Set up timer compare IRQs
    //TB0CCTL1 &= ~CCIFG;  // Clear CCR1 flag
    //TB0CCTL1 |= CCIE;  // Enable flag
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
    ADCMCTL0 |= ADCINCH_1;                                     // A1 ADC input select
    ADCIE |= ADCIE0; 
}

// Send 3 digits of the ambient temperature to the LCD in 5 transmissions
void send_ambient()
{
    n = 0;
    UCB0I2CSA = 0x0B;
    thousands = (avg_ambient/1000) + 48;
    avg_ambient %= 1000;
    hundreds = (avg_ambient/100) + 48;
    avg_ambient %= 100;
    tens = (avg_ambient/10) + 48; 
    data = 0xAD;
    UCB0CTLW0 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTP);
    __delay_cycles(2000);
    data = thousands;
    UCB0CTLW0 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTP);
    __delay_cycles(2000);
    data = hundreds;
    UCB0CTLW0 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTP);
    __delay_cycles(2000);
    data = 0b00101110;
    UCB0CTLW0 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTP);
    __delay_cycles(2000);
    data = tens;
    UCB0CTLW0 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTP);
    __delay_cycles(2000); 
    P2OUT ^= BIT5;    
}

void recieve_plant()
{
    msb_status = 1;
    UCB0CTLW0 &= ~UCTR;
    UCB0I2CSA = 0x48;
    UCB0TBCNT = 0x02;
    while (UCB0CTL1 & UCTXSTP);
    UCB0CTL1 |= UCTXSTT;
    __delay_cycles(1000);
    UCB0I2CSA = 0x0B;
    UCB0TBCNT = 0x01;
    UCB0CTLW0 |= UCTR;
}

void send_plant()
{
    n = 0;
    UCB0I2CSA = 0x0B;
    thousands = (avg_plant/1000) + 48;
    avg_plant %= 1000;
    hundreds = (avg_plant/100) + 48;
    avg_plant %= 100;
    tens = (avg_plant/10) + 48; 
    data = 0xAC;
    UCB0CTLW0 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTP);
    __delay_cycles(2000);
    data = thousands;
    UCB0CTLW0 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTP);
    __delay_cycles(2000);
    data = hundreds;
    UCB0CTLW0 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTP);
    __delay_cycles(2000);
    data = 0b00101110;
    UCB0CTLW0 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTP);
    __delay_cycles(2000);
    data = tens;
    UCB0CTLW0 |= UCTXSTT;
    while (UCB0CTL1 & UCTXSTP);
    __delay_cycles(2000);
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

    //lock_keypad(unlock_code);
    
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
        if(temp_to_send == 'A')
        {
            //send_ambient();
            recieve_plant();
            __delay_cycles(20000);
            send_plant();
            temp_to_send = 0;
        }
    }
}

#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){
  switch(__even_in_range(UCB0IV, USCI_I2C_UCBIT9IFG))
  {
    case USCI_NONE: break;                  // Vector 0: No interrupts
    case USCI_I2C_UCALIFG: break;           // Vector 2: ALIFG
    case USCI_I2C_UCNACKIFG:                // Vector 4: NACKIFG
                            UCB0CTL1 |= UCTXSTT;                  // I2C start condition
                            break;
    case USCI_I2C_UCSTTIFG: break;          // Vector 6: STTIFG
    case USCI_I2C_UCSTPIFG: break;          // Vector 8: STPIFG
    case USCI_I2C_UCRXIFG3: break;          // Vector 10: RXIFG3
    case USCI_I2C_UCTXIFG3: break;          // Vector 14: TXIFG3
    case USCI_I2C_UCRXIFG2: break;          // Vector 16: RXIFG2
    case USCI_I2C_UCTXIFG2: break;          // Vector 18: TXIFG2
    case USCI_I2C_UCRXIFG1: break;          // Vector 20: RXIFG1
    case USCI_I2C_UCTXIFG1: break;          // Vector 22: TXIFG1
    case USCI_I2C_UCRXIFG0:                 // Vector 24: RXIFG0
                            if(msb_status == 1)
                            {
                                msb_bank = UCB0RXBUF;
                                msb_status = 0;
                            }
                            else
                            {
                                plant_out = (msb_bank << 8) | UCB0RXBUF;
                                plant_out = plant_out >> 3;
                                plant_temp = plant_out * .0625;
                                real_plant = 100*plant_temp;
                                if(n != 4)
                                {
                                    plant[n] = real_plant;
                                }
                                else if(n == 4){
                                    n = 0;
                                    plant[0] = real_plant;
                                }
                                n++;
                                avg_plant = (plant[0] + plant[1] + plant[2])/3;
                            }
                            break;
    case USCI_I2C_UCTXIFG0:                 // Vector 26: TXIFG0
                            UCB0TXBUF = data;
                            break;
    case USCI_I2C_UCBCNTIFG: break;                // Vector 28: BCNTIFG
    case USCI_I2C_UCCLTOIFG: break;         // Vector 30: clock low timeout
    case USCI_I2C_UCBIT9IFG: break;         // Vector 32: 9th bit
    default: break;
  }
}

#pragma vector = PORT3_VECTOR
__interrupt void ISR_PORT3_S2(void) {

    key_pressed = scanPad();

    key_flag = 1;

    P3IFG &= ~(BIT0 | BIT1 | BIT2 | BIT3); // Clear flags
}

// ADC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC_VECTOR))) ADC_ISR (void)
#else
#error Compiler not supported!
#endif
{
    calc = 0;
    ADC_Result = 0;
    switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG:
            break;
        case ADCIV_ADCTOVIFG:
            break;
        case ADCIV_ADCHIIFG:
            break;
        case ADCIV_ADCLOIFG:
            break;
        case ADCIV_ADCINIFG:
            break;
        case ADCIV_ADCIFG:
            ADC_Result = ADCMEM0;
            calc = (ADC_Result*3.3)/4096;
            ambient_temp = (calc-1.8663)/(-0.01169);
            real_temp = 100*ambient_temp;
            if(n != 4)
            {
                ambient[n] = real_temp;
            }
            else if(n == 4){
                n = 0;
                ambient[0] = real_temp;
            }
            n++;
            avg_ambient = (ambient[0] + ambient[1] + ambient[2])/3;
            __bic_SR_register_on_exit(LPM0_bits);            // Clear CPUOFF bit from LPM0          
            break;
        default:
            break;
    }
}
// Timer B0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_B (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_B0_VECTOR))) Timer_B (void)
#else
#error Compiler not supported!
#endif
{
    temp_to_send = 'A';
    ADCCTL0 |= ADCENC | ADCSC;                                    // Sampling and conversion start
}