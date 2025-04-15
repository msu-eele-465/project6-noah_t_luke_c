#include "intrinsics.h"
#include <msp430.h>
#include "keypad.h"
#include <math.h>

#define unlock_code "1738"

unsigned char data = 0x00;
int lock_status = 1;
int window = 0;
char key_pressed;

float ADC_Result;
float temp;
float calc;
int real_temp;

int letters_set_pattern[] = {0b01010011, 0b01100101, 0b01110100, 0b01000000, 0b01010000, 0b01100001, 0b01110100, 0b01110100, 0b01100101, 0b01110000, 0b01101110};
int letters_set_window[] = {0b01010011, 0b01100101, 0b01110100, 0b01000000, 0b01010111, 0b01101001, 0b01101110, 0b01100100, 0b01101111, 0b01110111, 0b01000000, 0b01010011, 0b01101001, 0b01111010, 0b01100101};
int letters_pattern_static[] = {0b01010011, 0b01110100, 0b01100001, 0b01110100, 0b01101001, 0b01100011};
int letters_pattern_toggle[] = {0b01010100, 0b01111010, 0b01100111, 0b01100111, 0b01101100, 0b01100101};
int letters_pattern_up_counter[] = {0b01010101, 0b01110000, 0b01000000, 0b01100011, 0b01110101, 0b01101110, 0b01110100, 0b01100101, 0b01110000};
int letters_pattern_in_and_out[] = {0b01001001, 0b01101110, 0b01000000, 0b01100001, 0b01101110, 0b01100110, 0b01000000, 0b01100011, 0b01110100};


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



void write_set_pattern(){
    UCB0I2CSA = 0x0B;                      // LCD slave address
    data = 0x00;                           // Send 0 so LCD writes to top line
    UCB0CTLW0 |= UCTXSTT;                  // Transmit
    __delay_cycles(200);
    int i = 0;
    for (i = 0; i < sizeof(letters_set_pattern); i++) {
        data = letters_set_pattern[i];                 // Set data to be transmited to next letter code
        UCB0CTLW0 |= UCTXSTT;              // Transmit
        __delay_cycles(200);
    }
    __delay_cycles(200);
    UCB0I2CSA = 0x00;                      // Reset slave address
}

void write_set_window(){
    UCB0I2CSA = 0x0B;                      // LCD slave address
    data = 0x00;                           // Send 0 so LCD writes to top line
    UCB0CTLW0 |= UCTXSTT;                  // Transmit
    int i = 0;
    for (i = 0; i < sizeof(letters_set_window); i++) {
        data = letters_set_window[i];                 // Set data to be transmited to next letter code
        UCB0CTLW0 |= UCTXSTT;              // Transmit
    }
    UCB0I2CSA = 0x00;                      // Reset slave address
}

void write_pattern_static(){
    UCB0I2CSA = 0x0B;                      // LCD slave address
    data = 0x00;                           // Send 0 so LCD writes to top line
    UCB0CTLW0 |= UCTXSTT;                  // Transmit
    int i = 0;
    for (i = 0; i < sizeof(letters_pattern_static); i++) {
        data = letters_pattern_static[i];  // Set data to be transmited to next letter code
        UCB0CTLW0 |= UCTXSTT;              // Transmit
    }
    UCB0I2CSA = 0x00;                      // Reset slave address
}

void write_pattern_toggle(){
    UCB0I2CSA = 0x0B;                      // LCD slave address
    data = 0x00;                           // Send 0 so LCD writes to top line
    UCB0CTLW0 |= UCTXSTT;                  // Transmit
    int i = 0;
    for (i = 0; i < sizeof(letters_pattern_toggle); i++) {
        data = letters_pattern_toggle[i];  // Set data to be transmited to next letter code
        while (UCB0CTL1 & UCTXSTP);
        UCB0CTLW0 |= UCTXSTT;              // Transmit
    }
    UCB0I2CSA = 0x00;                      // Reset slave address
}

void write_pattern_up_counter(){
    UCB0I2CSA = 0x0B;                      // LCD slave address
    data = 0x00;                           // Send 0 so LCD writes to top line
    UCB0CTLW0 |= UCTXSTT;                  // Transmit
    int i = 0;
    for (i = 0; i < sizeof(letters_pattern_up_counter); i++) {
        data = letters_pattern_up_counter[i];  // Set data to be transmited to next letter code
        UCB0CTLW0 |= UCTXSTT;                  // Transmit
    }
    UCB0I2CSA = 0x00;                          // Reset slave address
}

void write_pattern_in_and_out(){
    UCB0I2CSA = 0x0B;                      // LCD slave address
    data = 0x00;                           // Send 0 so LCD writes to top line
    UCB0CTLW0 |= UCTXSTT;                  // Transmit
    int i = 0;
    for (i = 0; i < sizeof(letters_pattern_in_and_out); i++) {
        data = letters_pattern_in_and_out[i];  // Set data to be transmited to next letter code
        UCB0CTLW0 |= UCTXSTT;                  // Transmit
    }
    UCB0I2CSA = 0x00;                          // Reset slave address
}

void transmit_pattern_led(){
    UCB0I2CSA = 0x0A;                       // LEDbar slave address
    switch (key_pressed) {
        case '0': 
            data = key_pressed - '0';       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;     
            write_pattern_static();                   
            break;
        case '1': 
            data = key_pressed - '0';       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;  
            write_pattern_toggle();                         
            break;
        case '2': 
            data = key_pressed - '0';       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;
            write_pattern_up_counter();                           
            break;
        case '3': 
            data = key_pressed - '0';       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;
            write_pattern_in_and_out();                           
            break;
    }
        UCB0I2CSA = 0x00;                   // Reset slave address
}

void lcd_window_size_transmit(){
    UCB0I2CSA = 0x0B;                       // LCD slave address
    switch (key_pressed) {
        case '0': 
            data = 0;       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;                        
            break;
        case '1': 
            data = 1;       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;
            while (UCB0CTL1 & UCTXSTP);                           
            break;
        case '2': 
            data = 2;       
            UCB0CTLW0 |= UCTXSTT;                           
            break;
        case '3': 
            data = 3;       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;                           
            break;
        case '4': 
            data = 4;       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;                        
            break;
        case '5': 
            data = 5;       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;                           
            break;
        case '6': 
            data = 6;       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;                           
            break;
        case '7': 
            data = 7;       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;                           
            break;
        case '8': 
            data = 8;       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;                           
            break;
        case '9': 
            data = 9;       // Convert char to integer
            UCB0CTLW0 |= UCTXSTT;                           
            break;
    }
    UCB0I2CSA = 0x00;                       // Reset slave address
}

int main(void)
{
    // Stop WDT
    WDTCTL = WDTPW | WDTHOLD;

    TB0CCTL0 |= CCIE;                                             // TBCCR0 interrupt enabled
    TB0CCR0 = 32;
    TB0CTL = TBSSEL__ACLK | MC__UP;                               // ACLK, UP mode

    // Configure ADC A1 pin
    P1SEL0 |= BIT1;
    P1SEL1 |= BIT1;

    // Configure ADC12
    ADCCTL0 |= ADCSHT_2 | ADCON;                             // ADCON, S&H=16 ADC clks
    ADCCTL1 |= ADCSHP;                                       // ADCCLK = MODOSC; sampling timer
    ADCCTL2 &= ~ADCRES;                                      // clear ADCRES in ADCCTL
    ADCCTL2 |= ADCRES_2;                                     // 12-bit conversion results
    ADCMCTL0 |= ADCINCH_1;                                   // A1 ADC input select; Vref=AVCC
    ADCIE |= ADCIE0;                                         // Enable ADC conv complete interrupt

    // Set up I2C
    i2c_config();

    // Set up I2C
    keypad_init();

    // Disable low power mode
    PM5CTL0 &= ~LOCKLPM5;
    UCB0CTLW0 &= ~UCSWRST;

    // I2C interrupt
    UCB0IE |= UCTXIE0;  

    // Enable interrupts
    __enable_interrupt();
    
    
    while (1)
    {   
        if(lock_status == 1)
        {
            lock_keypad(unlock_code);
            lock_status = 0;
        }   
        UCB0I2CSA = 0x0B;        
        while (UCB0CTL1 & UCTXSTP);
        while(key_pressed != 'A' && key_pressed != 'B') 
        {      
            real_temp = 100*temp;
            int thousands = (real_temp/1000) + 48;
            real_temp %= 1000;
            int hundreds = (real_temp/100) + 48;
            real_temp %= 100;
            int tens = (real_temp/10) + 48;
            int ones = (real_temp%10) + 48;
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
            __delay_cycles(200000);  
 
        }
    }
}


#pragma vector=EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){
    UCB0TXBUF = data;
}

#pragma vector = PORT3_VECTOR
__interrupt void ISR_PORT3_S2(void) {
    key_pressed = scanPad();
    P3IFG &= ~(BIT0 | BIT1 | BIT2 | BIT3);  // Clear the interrupt flag
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
            temp = (calc-1.8663)/(-0.01169);
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
    ADCCTL0 |= ADCENC | ADCSC;                                    // Sampling and conversion start
}