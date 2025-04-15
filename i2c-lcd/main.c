#include "intrinsics.h"
#include <msp430.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <lcd.h>

unsigned char RXData = 0;
unsigned char current = 0;


int main(void) { 
    WDTCTL = WDTPW | WDTHOLD;  // Stop watchdog timer

    // Configure Pins for I2C
    P1SEL0 |= BIT2 | BIT3;                                // I2C pins

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure USCI_B0 for I2C mode
    UCB0CTLW0 |= UCSWRST;                                 //Software reset enabled
    UCB0CTLW0 |= UCMODE_3;                                //I2C slave mode, SMCLK
    UCB0I2COA0 = 0x0B | UCOAEN;                           //SLAVE0 own address is 0x0B| enable
    UCB0CTLW0 &=~UCSWRST;                                 //clear reset register

    UCB0IE |=  UCRXIE0;      //receive interrupt enable

    //__bis_SR_register(LPM0_bits | GIE);                   // Enter LPM0 w/ interrupts
    __no_operation();

    PM5CTL0 &= ~LOCKLPM5;  // Enable GPIO


    __enable_interrupt();  // Enable global interrupts
    lcd_init();
    __delay_cycles(500);
    clear_cgram();
    return_home();
    
    while(1)
    {       

    }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B0_VECTOR
__interrupt void USCIB0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B0_VECTOR))) USCIB0_ISR (void)
#else
#error Compiler not supported!
#endif
{
                                       // SLAVE0
        RXData = UCB0RXBUF;                              // Get RX data
        lcd_write(RXData);
        __delay_cycles(50000);
        __bic_SR_register_on_exit(LPM0_bits);                       // Vector 24: RXIFG0 break;
    

}