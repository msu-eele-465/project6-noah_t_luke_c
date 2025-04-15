#include <msp430.h>
#include <ledbar.h>

unsigned char RXData=0;
int start1 = 0;
int start2 = 0;
int start3 = 0;
int repeat = 0;
int pattern = 100;
int up_down = 0;
int base_period = 32768;
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                             // Stop watchdog timer

    P1DIR |= BIT1;
    P1OUT &= ~BIT1;
    // Configure Pins for I2C
    P1SEL0 |= BIT2 | BIT3;                                // I2C pins

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    PM5CTL0 &= ~LOCKLPM5;

    // Configure USCI_B0 for I2C mode
    UCB0CTLW0 |= UCSWRST;                                 //Software reset enabled
    UCB0CTLW0 |= UCMODE_3;                                //I2C slave mode, SMCLK
    UCB0I2COA0 = 0x0A | UCOAEN;                           //SLAVE0 own address is 0x0A| enable
    UCB0CTLW0 &=~UCSWRST;                                 //clear reset register

    UCB0IE |=  UCRXIE0;     //receive interrupt enable

    __bis_SR_register(LPM0_bits | GIE);                   // Enter LPM0 w/ interrupts
    __no_operation();
    LEDbarInit();
    while(1)
    {
        if(up_down == 2)
        {
            base_period = base_period - 8192;
            TB0CCTL0 &= ~CCIE;  // Disable flag
            TB0CTL &= ~MC__UP;  // Stop counting mode
            TB0CTL |= TBCLR;    // Clear timer and dividers
            TB0CTL &= ~CM;
            __delay_cycles(2);
            TB0CCR0 = base_period;
            TB0CCR0 = TB0CCR0;
            TB0CTL |= CM;
            TB0CTL |= MC__UP;   // Start timer
            TB0CCTL0 |= CCIE;  // Enable flag 
            up_down = 0;
        }
        else if(up_down == 1)
        {
            base_period = base_period + 8192;
            TB0CCTL0 &= ~CCIE;  // Disable flag
            TB0CTL &= ~MC__UP;  // Stop counting mode
            TB0CTL |= TBCLR;    // Clear timer and dividers
            TB0CTL &= ~CM;
            __delay_cycles(2);
            TB0CCR0 = base_period;
            TB0CCR0 = TB0CCR0;
            TB0CTL |= CM;
            TB0CTL |= MC__UP;   // Start timer
            TB0CCTL0 |= CCIE;  // Enable flag 
            up_down = 0;
        }
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
        if(UCB0RXBUF == RXData)
        {
            repeat = 1;
        }
        RXData = UCB0RXBUF;                              // Get RX data
        P1OUT ^= BIT4;
        __bic_SR_register_on_exit(LPM0_bits);                       // Vector 24: RXIFG0 break;
    

}

#pragma vector = TIMER0_B0_VECTOR
__interrupt void ISR_TB0_CCR0(void) {
    switch (RXData){
            case 0x16:  clear();
                        RXData = 0;
                        break;
            case 0x14:  clear();
                        pattern0();
                        pattern = 0x14;
                        break;
            case 0x1:   clear();
                        if(repeat == 1)
                        {
                            start1 = 0;
                            repeat = 0;
                        }
                        start1 = pattern1(start1);
                        pattern = 1;
                        break;
            case 0x2:   clear();
                        if(repeat == 1)
                        {
                            start2 = 0;
                            repeat = 0;
                        }
                        start2 = pattern2(start2);
                        pattern = 2;
                        break;
            case 0x3:   clear();
                        if(repeat == 1)
                        {
                            start3 = 0;
                            repeat = 0;
                        }
                        start3 = pattern3(start3);
                        pattern = 3;
                        break;
            case 0x4:   up_down = 2;
                        RXData = pattern;
                        break;
            case 0x8:   up_down = 1;
                        RXData = pattern;
                        break;
            case 0x11:  RXData = pattern;
                        break;
            case 0x12:  RXData = pattern;
                        break;
            default:    break;
        }
    TB0CCTL1 &= ~CCIFG;
}