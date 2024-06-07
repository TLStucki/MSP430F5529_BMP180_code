#include <msp430.h> 
#include "BMP180.h"

/*
 * main.c
 *
 *  Created on: 12.01.2024
 *      Author: Tobias Lars Stucki
 *      published under MIT license
 *      V0.2
 */


void main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1IN    = 0x00;
    P1OUT   = 0x00;
    P1DIR   = 0x00;
    P1REN   = 0x00;
    P1DS    = 0x00;
    P1SEL   = 0x00;
    P1IES   = 0x00;
    P1IE    = 0x00;
    P1IFG   = 0x00;

    P2IN    = 0x00;
    P2OUT   = 0x00;
    P2DIR   = 0x00;
    P2REN   = 0x00;
    P2DS    = 0x00;
    P2SEL   = 0x00;
    P2IES   = 0x00;
    P2IE    = 0x00;
    P2IFG   = 0x00;

    P3IN    = 0x00;
    P3OUT   = 0x00;
    P3DIR   = 0x00;
    P3REN   = 0x00;
    P3DS    = 0x00;
    P3SEL   = BIT0 | BIT1;

    P4IN    = 0x00;
    P4OUT   = 0x00;
    P4DIR   = 0x00;
    P4REN   = 0x00;
    P4DS    = 0x00;
    P4SEL   = 0x00;

    P5IN    = 0x00;
    P5OUT   = 0x00;
    P5DIR   = 0x00;
    P5REN   = 0x00;
    P5DS    = 0x00;
    P5SEL   = 0x00;

    P6IN    = 0x00;
    P6OUT   = 0x00;
    P6DIR   = 0x00;
    P6REN   = 0x00;
    P6DS    = 0x00;
    P6SEL   = 0x00;

    P7IN    = 0x00;
    P7OUT   = 0x00;
    P7DIR   = 0x00;
    P7REN   = 0x00;
    P7DS    = 0x00;
    P7SEL   = 0x00;

    P8IN    = 0x00;
    P8OUT   = 0x00;
    P8DIR   = 0x00;
    P8REN   = 0x00;
    P8DS    = 0x00;
    P8SEL   = 0x00;

    volatile float temparatur = 0;
    volatile long long int pressure_Pa[4] = {0,0,0,0};

    init_i2c_();
    BMP180_Get_Calib_Param();

    while(1)
    {
        temparatur = BMP180_get_temp();
        pressure_Pa[0] = BMP180_get_press(RES1);
        pressure_Pa[1] = BMP180_get_press(RES2);
        pressure_Pa[2] = BMP180_get_press(RES3);
        pressure_Pa[3] = BMP180_get_press(RES4);
        __no_operation();
    }
}








