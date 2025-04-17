#include "intrinsics.h"
#include <msp430.h>
#include <stdbool.h>

void LEDbarInit(void){
    // Setting GPIO pins as outputs to control the LED bar
    // LED1
    P1DIR |= BIT1;
    P1OUT &= ~ BIT1;
    // LED2
    P1DIR |= BIT0;
    P1OUT &= ~ BIT0;
    // LED3
    P2DIR |= BIT7;
    P2OUT &= ~ BIT7;
    // LED4
    P2DIR |= BIT6;
    P2OUT &= ~BIT6;
    // LED5
    P1DIR |= BIT5;
    P1OUT &= ~ BIT5;
    // LED6
    P1DIR |= BIT6;
    P1OUT &= ~ BIT6;
    // LED7
    P1DIR |= BIT7;
    P1OUT &= ~ BIT7;
    // LED8
    P2DIR |= BIT0;
    P2OUT &= ~ BIT0;
}

void ON(int led){
    // Turns LED on the LED bar ON depending on the given int
    switch(led){
        case 1:     P1OUT |= BIT1;
                    break;
        case 2:     P1OUT |= BIT0;
                    break;
        case 3:     P2OUT |= BIT7;
                    break;
        case 4:     P2OUT |= BIT6;
                    break;
        case 5:     P1OUT |= BIT5;
                    break;
        case 6:     P1OUT |= BIT6;
                    break;
        case 7:     P1OUT |= BIT7;
                    break;
        case 8:     P2OUT |= BIT0;
                    break;
        default:    break;                                                                                                                              
    }
}

void OFF(int led){
    switch(led){
        case 1:     P1OUT &= ~BIT1;
                    break;
        case 2:     P1OUT &= ~BIT0;
                    break;
        case 3:     P2OUT &= ~BIT7;
                    break;
        case 4:     P2OUT &= ~BIT6;
                    break;
        case 5:     P1OUT &= ~BIT5;
                    break;
        case 6:     P1OUT &= ~BIT6;
                    break;
        case 7:     P1OUT &= ~BIT7;
                    break;
        case 8:     P2OUT &= ~BIT0;
                    break;
        default:    break;                                                                                                                               
    }
}

void allOn(void){
    // Sets LED bar to a 0101010101 static pattern
    ON(1);
    ON(2);
    ON(3);
    ON(4);
    ON(5);
    ON(6);
    ON(7);
    ON(8);
}

void clear(void){
    // Turns every LED off
    OFF(1);
    OFF(2);
    OFF(3);
    OFF(4);
    OFF(5);
    OFF(6);
    OFF(7);
    OFF(8);
}

int fillRight(int current) {
    switch (current) {
        case 1: ON(1);
                break;
        case 2: ON(1);
                ON(2); 
                break;
        case 3: ON(1);
                ON(2);
                ON(3); 
                break;
        case 4: ON(1);
                ON(2);
                ON(3);
                ON(4); 
                break;
        case 5: ON(1);
                ON(2);
                ON(3);
                ON(4);
                ON(5); 
                break;
        case 6: ON(1);
                ON(2);
                ON(3);
                ON(4);
                ON(5);
                ON(6); 
                break;
        case 7: ON(1);
                ON(2);
                ON(3);
                ON(4);
                ON(5);
                ON(6);
                ON(7); 
                break;
        case 8: ON(1);
                ON(2);
                ON(3);
                ON(4);
                ON(5);
                ON(6);
                ON(7);
                ON(8); 
                break;
        default: break;
    }

    
    if (current > 9) {
        current = 1;
        clear(); // Reset LED bar after filling
    }
    current++;

    return current;
}

int fillLeft(int current) {
    
switch (current) {
        case 1: ON(8);
                break;
        case 2: ON(8);
                ON(7); 
                break;
        case 3: ON(8);
                ON(7);
                ON(6); 
                break;
        case 4: ON(8);
                ON(7);
                ON(6);
                ON(5); 
                break;
        case 5: ON(8);
                ON(7);
                ON(6);
                ON(5);
                ON(4); 
                break;
        case 6: ON(8);
                ON(7);
                ON(6);
                ON(5);
                ON(4);
                ON(3); 
                break;
        case 7: ON(8);
                ON(7);
                ON(6);
                ON(5);
                ON(4);
                ON(3);
                ON(2); 
                break;
        case 8: ON(8);
                ON(7);
                ON(6);
                ON(5);
                ON(4);
                ON(3);
                ON(2);
                ON(1); 
                break;
        default: break;
    }
    
    if (current > 9) {
        current = 1;
        clear(); // Reset LED bar after filling
    }
    current++;

    return current;
}
