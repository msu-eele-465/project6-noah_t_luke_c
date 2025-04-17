#include "intrinsics.h"
#include <msp430.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

unsigned char RXData = 0;
unsigned char final = 0;

char temp_set = 0;
char plant_set = 0;

#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

const char temperature_set[] = {0b01010100, 0b01100101, 0b01101101, 0b01110000, 0b01100101, 0b01110010, 0b01110100, 0b01110100, 0b01100101, 0b01110000, 0b01101110};
const char letters_set_window[] = {0b01010011, 0b01100101, 0b01110100, 0b01000000, 0b01010111, 0b01101001, 0b01101110, 0b01100100, 0b01101111, 0b01110111, 0b01000000, 0b01010011, 0b01101001, 0b01111010, 0b01100101};
const char letters_pattern_static[] = {0b01010011, 0b01110100, 0b01100001, 0b01110100, 0b01101001, 0b01100011};
const char letters_pattern_toggle[] = {0b01010100, 0b01111010, 0b01100111, 0b01100111, 0b01101100, 0b01100101};
const char letters_pattern_up_counter[] = {0b01010101, 0b01110000, 0b00100000, 0b01100011, 0b01010100, 0b01110101, 0b01101110, 0b01110100, 0b01100101, 0b01110010};
const char letters_pattern_in_and_out[] = {0b01001001, 0b01101110, 0b01000000, 0b01100001, 0b01101110, 0b01100110, 0b01000000, 0b01100011, 0b01110100};

void lcd_init()
{
    P1DIR |= BIT1;  
    P1OUT &= ~BIT1;  
    P1DIR |= BIT0;  
    P1OUT &= ~BIT0;  
    P1DIR |= BIT4;  
    P1OUT &= ~BIT4;  
    P1DIR |= BIT5;  
    P1OUT &= ~BIT5;
    P1DIR |= BIT6;
    P1OUT &= ~BIT6;  
    P1DIR |= BIT7;  
    P1OUT &= ~BIT7;  
    P2DIR |= BIT0;  
    P2OUT &= ~BIT0;
}

// Brings cursor to last position on LCD screen
void position(int steps)
{
    return_home();
    int i = 0;
    for(i = 0; i<steps; i++)
    {
        cursor_right();
    } 
}

void lcd_setup()
{
    lcd_write(0b00100010);
    __delay_cycles(500);
    DB5(0);
    DB7(1);
    DB6(1);
    DB4(0);
    P1OUT |= BIT6;
    __delay_cycles(1000);
    P1OUT &= ~BIT6;
    __delay_cycles(500);
    lcd_write(0b00001111);
    lcd_write(0b00000001);
    __delay_cycles(10000);
    lcd_write(0b00000110);
}

void DB7(int status)
{
    
    if(status == 1)
    {
        P1OUT |= BIT1; 
    }
    else 
    {
        P1OUT &= ~BIT1;
    }
}

/*
Function to change output of DB6 depending on int input.
*/
void DB6(int status)
{
    if(status == 1)
    {
        P1OUT |= BIT0; 
    }
    else 
    {
        P1OUT &= ~BIT0;
    }
}

/*
Function to change output of DB5 depending on int input.
*/
void DB5(int status)
{
    if(status == 1)
    {
        P1OUT |= BIT4; 
    }
    else 
    {
        P1OUT &= ~BIT4;
    }
}

/*
Function to change output of DB4 depending on int input.
*/
void DB4(int status)
{
    if(status == 1)
    {
        P1OUT |= BIT5; 
    }
    else 
    {
        P1OUT &= ~BIT5;
    }
}

void cursor_right()
{
    P2OUT &= ~BIT0;
    lcd_write(0b00010100);
    __delay_cycles(500);
    P2OUT |= BIT0;
}

void clear_cgram()
{
    P2OUT &= ~BIT0;
    lcd_write(0b00000001);
    P2OUT |= BIT0;
}

void return_home()
{
    P2OUT &= ~BIT0;
    lcd_write(0b00000010);
    P2OUT |= BIT0;
}


void lcd_write(int in)
{
        __delay_cycles(500);
        if(CHECK_BIT(in,7) != 0){
            DB7(1);
        } else{
            DB7(0);
        }
        if(CHECK_BIT(in,6) != 0){
            DB6(1);
        } else{
            DB6(0);
        }
        if(CHECK_BIT(in,5) != 0){
            DB5(1);
        } else{
            DB5(0);
        }
        if(CHECK_BIT(in,4) != 0){
            DB4(1);
        } else{
            DB4(0);
        }
        __delay_cycles(500);
        P1OUT |= BIT6;
        __delay_cycles(1000);
        P1OUT &= ~BIT6;
        __delay_cycles(500);
        if(CHECK_BIT(in,3) != 0){
            DB7(1);
        } else{
            DB7(0);
        }
        if(CHECK_BIT(in,2) != 0){
            DB6(1);
        } else{
            DB6(0);
        }
        if(CHECK_BIT(in,1) != 0){
            DB5(1);
        } else{
            DB5(0);
        }
        if(CHECK_BIT(in,0) != 0){
            DB4(1);
        } else{
            DB4(0);
        }   
        __delay_cycles(500);
        P1OUT |= BIT6;
        __delay_cycles(1000);
        P1OUT &= ~BIT6;
        __delay_cycles(500);
}

void lcd_print(const char input[], int length)
{
    int i = 0;
    for(i = 0; i < length; i++){
        lcd_write(input[i]);
    }
}

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
    UCB0I2COA0 = 0x0B | UCOAEN;                           //SLAVE0 own address is 0x0A| enable
    UCB0CTLW0 &=~UCSWRST;                                 //clear reset register


    //__bis_SR_register(LPM0_bits | GIE);                   // Enter LPM0 w/ interrupts
    __no_operation();

    PM5CTL0 &= ~LOCKLPM5;  // Enable GPIO


    lcd_init();
    lcd_setup();
    lcd_setup();
    __delay_cycles(500);
    clear_cgram();
    __delay_cycles(1000);
    lcd_print("Off",3);
    __delay_cycles(1000);
    position(8);
    __delay_cycles(1000);
    lcd_print("A:",2);
    __delay_cycles(1000);
    position(40);
    __delay_cycles(1000);
    lcd_write('4');
    __delay_cycles(1000);
    position(48);
    __delay_cycles(1000);
    lcd_print("P:",2);
    __delay_cycles(1000);
    return_home();

    __enable_interrupt();  // Enable global interrupts
    UCB0IE |=  UCRXIE0 | UCRXIE1| UCRXIE2 | UCRXIE3;      //receive interrupt enable

    while(1)
    {
        final = RXData;
        // Switch statement prints the pattern description depending on recieved data
        switch(RXData)
        {
            case 0:     break;
            case 0x1:   return_home();
                        lcd_print("Heat ",5);
                        RXData = 0;
                        break;
            case 0x2:   return_home();
                        lcd_print("Cool ",5);
                        RXData = 0;
                        break;
            case 0x3:   return_home();
                        lcd_print("Match",5);
                        return_home();
                        RXData = 0;
                        break;
            case 0x04:  return_home();
                        lcd_print("Off  ",5);
                        return_home();
                        RXData = 0;
                        break;
            

            default:    break;
        }

        if(RXData == 0x88 | 0x89){
            return_home();
            lcd_print("Match", 5);
        }
        
        // This switch statement prints the last pressed key to the final position of the LCD

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
        if(RXData == 0xAD)
        {
            position(14);
            lcd_write(0b11011111);
            lcd_write(0b01000011);
            position(10);
            temp_set = 3;
        }
        if(temp_set != 0 && RXData != 0xAD && RXData != 0xAC)
        {
            if(temp_set == 2)
            {
                lcd_write(RXData);
                lcd_write('.');
            }
            else{
                lcd_write(RXData);
            }
            temp_set--;
        }
        if(RXData == 0xAC)
        {
            position(54);
            lcd_write(0b11011111);
            lcd_write(0b01000011);
            position(50);
            plant_set = 3;
        }
        if(plant_set != 0 && RXData != 0xAC && RXData != 0xAD)
        {
            if(plant_set == 2)
            {
                lcd_write(RXData);
                lcd_write('.');
            }
            else{
                lcd_write(RXData);
            }
            plant_set--;
        }
        P2OUT ^= BIT6;
        
        __bic_SR_register_on_exit(LPM0_bits);                       // Vector 24: RXIFG0 break;
    

}