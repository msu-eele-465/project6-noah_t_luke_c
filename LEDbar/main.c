#include <msp430.h>
#include "LEDbar.h"

unsigned char RXData = 0;

int startRight = 0;
int startLeft = 0;
unsigned char Pattern = 0; // Left = 'L' Right = 'R' Off = 'O'

int current = 0;

void timer_setup() {
    // Setup Timer A0
    TB0CTL |= TBCLR;                 // Clear timer
    TB0CTL |= TBSSEL__ACLK;         // ACLK = 32.768kHz
    TB0CTL |= MC__UP;               // Up mode
    TB0CCR0 = 32768;                // ~1 second delay
    TB0CCTL0 |= CCIE;               // Enable interrupt

    TB0CCR1 = 16384;                // ~1 second delay
    TB0CCTL1 |= CCIE;               // Enable interrupt
}

void i2c_setup() {
    // Configure Pins for I2C (P1.2 = SDA, P1.3 = SCL)
    P1SEL1 &= ~(BIT2 | BIT3);
    P1SEL0 |= BIT2 | BIT3;

    // Disable high-impedance mode
    PM5CTL0 &= ~LOCKLPM5;

    // Put eUSCI_B0 in reset and configure I2C slave
    UCB0CTLW0 |= UCSWRST;
    UCB0CTLW0 |= UCMODE_3 | UCSYNC;           // I2C slave, sync mode
    UCB0I2COA0 = 0x0B | UCOAEN;               // Own address 0x0A, enable

    UCB0CTLW0 &= ~UCSWRST;                    // Exit reset

    UCB0IE |= UCRXIE0;                        // Enable receive interrupt
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;                 // Stop watchdog

    P1OUT &= ~BIT4;                           
    P1DIR |= BIT4;                            


    i2c_setup();                              // Setup I2C
    LEDbarInit();                             // Initialize LED bar
    timer_setup();

    PM5CTL0 &= ~LOCKLPM5;       

    __enable_interrupt();                     // Enable global interrupts

    clear();

    while (1) {

    }
}

#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void){
    RXData = UCB0RXBUF;                        // Read received byte
    
    clear();

    switch (RXData){
        case 0x00: 
            break;
        case 0x01:
            Pattern = 'R';
            break;
        case 0x02: 
            Pattern = 'L';
            break;
        case 0x88:
            Pattern = 'R';
            break;
        case 0x89: 
            Pattern = 'L';
            break;
        case 0x04:
            Pattern = 'O';
            break;
        default: break;
    }
    
    return;
}


#pragma vector = TIMER0_B0_VECTOR
__interrupt void Timer_TB0_CCR0 (void)
{
    P1OUT ^= BIT4;
    TB0CCTL0 &= ~CCIFG;  // Clear interrupt flag
}


#pragma vector = TIMER0_B1_VECTOR
__interrupt void Timer_TB0_CCR1 (void)
{
    if (Pattern == 'L'){
        startLeft = fillLeft(startLeft);  // Fill one more LED to the left
        if (startLeft > 9) {               // Reset if full
            clear();
            startLeft = 1;
        }
    }
    else if (Pattern == 'R') {
        startRight = fillRight(startRight);  // Fill one more LED to the right
        if (startRight > 9) {               // Reset if full
            clear();
            startRight = 1;
        }
    }
    else if (Pattern == 'O') {
        clear();
    }
    
    
    TB0CCTL1 &= ~CCIFG;  // Clear interrupt flag
}


