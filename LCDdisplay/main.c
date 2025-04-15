#include "intrinsics.h"
#include <msp430.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "LCD.h"

unsigned char RXdata = 0;
unsigned char current = 0;
unsigned char write = 'rando starting';

void i2c_setup() {
    // Configure Pins for I2C (P1.2 = SDA, P1.3 = SCL)
    P1SEL1 &= ~(BIT2 | BIT3);
    P1SEL0 |= BIT2 | BIT3;

    // Disable high-impedance mode
    PM5CTL0 &= ~LOCKLPM5;

    // Put eUSCI_B0 in reset and configure I2C slave
    UCB0CTLW0 |= UCSWRST;
    UCB0CTLW0 |= UCMODE_3 | UCSYNC;           // I2C slave, sync mode
    UCB0I2COA0 = 0x0A | UCOAEN;               // Own address 0x0A, enable

    UCB0CTLW0 &= ~UCSWRST;                    // Exit reset

    UCB0IE |= UCRXIE0;                        // Enable receive interrupt
}


int main(void) { 
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer

    i2c_setup();
    lcd_init();
    clear_cgram();
    return_home();

    PM5CTL0 &= ~LOCKLPM5;  // Enable GPIO

    __enable_interrupt();  // Enable global interrupts
    
    __delay_cycles(500);
    
    
    while(1)
    {       
        switch(write){
            case 'H':
                lcd_write(0b01001000);
                lcd_write(0b01000101);
                lcd_write(0b01000001);
                lcd_write(0b01010100);
                //clear_cgram();
                break;
            default: break;
        }
    }
}

#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void){
    RXdata = UCB0RXBUF;                        // Read received byte
    
    switch (RXdata){
        case 0x01:
            write = 'H';
            break;
        default: break;
    }

    return;
}