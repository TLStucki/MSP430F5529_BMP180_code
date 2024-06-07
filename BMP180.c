/*
 * BMP180.c
 *
 *  Created on: 12.01.2024
 *      Author: Tobias Lars Stucki
 *      published under MIT license
 *      V1.0
 */

#include <msp430.h>
#include <stdio.h>
#include <math.h>

#include "BMP180.h"

//BMP180 7bit slave Address
#define SLAVE_ADDR 0x77

//define Register for EEPROM data
#define EEPROM_DAT_REG 0xAA

// define calibration parameter names
#define AC1 0
#define AC2 1
#define AC3 2
#define AC4 3
#define AC5 4
#define AC6 5
#define B1  6
#define B2  7
#define MB  8
#define MC  9
#define MD  10

#define BMP180_ERROR 255

volatile int GviCalib_Param [11] = {};

//*****************************************
//     function to initiate i2c module
//*****************************************

void init_i2c_(void)
{
    UCB0CTL1 |= UCSWRST;                        // Enable SW reset
    UCB0CTL0 = UCMST | UCMODE_3 | UCSYNC;       // I2C Master, synchronous mode
    UCB0CTL1 |= UCSSEL_3;                       // Use SMCLK, keep SW reset
    UCB0BR0 = 12;                               // fSCL = 100kHz
    UCB0BR1 = 0;
    UCB0I2CSA = SLAVE_ADDR;                     // set slave address
    UCB0CTL1 &= ~UCSWRST;                       // Clear SW reset, resume operation
    UCB0IE |= UCTXIE;                           // Enable TX interrupt
    UCB0IE |= UCRXIE;                           // Enable RX interrupt
}

//*****************************************
// function to read calibration parameters
//*****************************************

void BMP180_Get_Calib_Param(void)
{
    char cByte_Count = 22;                      //set read byte count
    char cCalib_Param_Count = 0;

    UCB0CTL1 |= UCTR | UCTXSTT;                 //send start condition

    while (!(UCB0IFG & UCTXIFG));               //wait until buffer is clear
    UCB0TXBUF = EEPROM_DAT_REG;                 //send read register

    while (!(UCB0IFG & UCTXIFG));               //wait until buffer is clear
    UCB0CTL1 &= ~UCTR;                          //restart and read
    UCB0CTL1 |= UCTXSTT;

    while (cByte_Count != 0)                    //loop till all bytes are received
    {
        if (cByte_Count == 1)                   //when all but one byte got received send stop condition
        {
            UCB0CTL1 |= UCTXSTP;
        }

        if ((cByte_Count % 2) == 0)             //read only every other byte
        {
            while (!(UCB0IFG & UCRXIFG));               //wait until buffer is clear
            GviCalib_Param [cCalib_Param_Count] = UCB0RXBUF << 8;
        }
        else
        {
            while (!(UCB0IFG & UCRXIFG));               //wait until buffer is clear
            GviCalib_Param [cCalib_Param_Count] += UCB0RXBUF;
        }

        cByte_Count--;                          //decrement byte counter

        if ((cByte_Count % 2) == 0)             //if byte counter has been decremented by 2 increment array position
        {

            cCalib_Param_Count++;
        }
    }
}

//*****************************************
// function to read and calculate pressure
//*****************************************

long long int BMP180_get_press(int oss) //oss is resolution and can be configured from 0-3 with RES1 - RES4
{
    char i = 0;

    int UT = 0;

    long long int RAM = 0;
    long long int UP = 0;

    long long int p = 0;
    long long int pressure = 0;

    int iRegister_Select = 0x34 + (oss << 6);

    int iCalib_Param[11] = {};

    long long int X1 = 0;
    long long int X2 = 0;
    long long int X3 = 0;
    long long int B3 = 0;
    long long int B4 = 0;
    long long int B5 = 0;
    long long int B6 = 0;
    long long int B7 = 0;

    //get calibration parameters from global variable and make them local

    for (i = 11; i > 0; i--)
    {
        iCalib_Param[i-1] = GviCalib_Param[i-1];
    }

    UCB0CTL1 |= UCTR | UCTXSTT;             //start condition

    while (!(UCB0IFG & UCTXIFG));           //wait until buffer is clear
    UCB0TXBUF = 0xF4;                       //set write register

    while (!(UCB0IFG & UCTXIFG));           //wait until buffer is clear
    UCB0TXBUF = iRegister_Select;           //write read register

    while (!(UCB0IFG & UCTXIFG));           //wait until buffer is clear
    UCB0CTL1 |= UCTXSTP;                    //send stop condition

    //wait for specific resolution time
    if      (oss == 0)
    {
        __delay_cycles(4500);
    }
    else if (oss == 1)
    {
        __delay_cycles(7500);
    }
    else if (oss == 2)
    {
        __delay_cycles(13500);
    }
    else if (oss == 3)
    {
        __delay_cycles(25500);
    }


    UCB0CTL1 |= UCTR | UCTXSTT;             //send start condition and write bit

    while (!(UCB0IFG & UCTXIFG));           //wait until buffer is clear
    UCB0TXBUF = 0xF6;                       //send register that has to be read from

    while (!(UCB0IFG & UCTXIFG));           //wait until buffer is clear
    UCB0CTL1 &= ~UCTR;                      //set to read
    UCB0CTL1 |= UCTXSTT;                    //send restart condition

    while (!(UCB0IFG & UCRXIFG));           //wait until buffer is clear
    RAM = UCB0RXBUF;                        //receive MSB
    UP = RAM << 16;

    while (!(UCB0IFG & UCRXIFG));           //wait until buffer is clear
    UP += UCB0RXBUF << 8;                   //receive middle byte

    UCB0CTL1 |= UCTXSTP;

    while (!(UCB0IFG & UCRXIFG));           //wait until buffer is clear
    UP += UCB0RXBUF;                        //receive LSB

    UP = UP >> (8-oss);                     //delete unneeded bits

    UT = get_temp_();                       //get temperature and write to UT

    //      calculate true pressure
    //-----------------------------------
    X1 = (UT - ((unsigned int)iCalib_Param[AC6])) * ((unsigned int)iCalib_Param[AC5]) / 32768;
    X2 = (iCalib_Param[MC] * 2048) / (X1 + iCalib_Param[MD]);
    B5 = X1 + X2;
    B6 = B5 - 4000;
    X1 = (iCalib_Param[B2] * ((B6*B6) / 4096)) / 2048;
    X2 = iCalib_Param[AC2] * B6 / 2048;
    X3 = X1 + X2;
    B3 = ( ( (iCalib_Param[AC1]*4 + X3) << oss) + 2 ) / 4;
    X1 = iCalib_Param[AC3] * B6 / 8192;
    X2 = (iCalib_Param[B1] * ((B6*B6) / 4096)) / 65536;
    X3 = ((X1 + X2) + 2) / 4;
    B4 = ((unsigned int)iCalib_Param[AC4]) * (unsigned long long)(X3 + 32768) / 32768;
    B7 = ((unsigned long long)UP - B3) * (50000 >> oss);

    if(B7 < 0x80000000)
    {
        p = (B7 * 2) / B4;
    }
    else
    {
        p = (B7 / B4) * 2;
    }

    X1 = (p / 256) * (p / 256);
    X1 = (X1 * 3038) / 65536;
    X2 = (-7537 * p) / 65536;

    p = p + (X1 + X2 + 3791) / 16;

    pressure = p;                       //write true pressure to pressure variable

    return pressure;                    //return pressure
}

//********************************************
// function to read and calculate temperature
//********************************************

float BMP180_get_temp(void)
{
    int i = 0;

    int iTemp = 0;
    int iTrue_temp = 0;
    float fTrue_temp = 0;

    float fCalib_Param[11] = {};

    float X1 = 0;
    float X2 = 0;
    float B5 = 0;

    //   get calibration parameters from global variable and make them local
    for (i = 11; i > 0; i--)
    {
        fCalib_Param[i-1] = (float)GviCalib_Param[i-1];
    }

    UCB0CTL1 |= UCTR | UCTXSTT;             //start condition

    while (!(UCB0IFG & UCTXIFG));           //wait till buffer is empty
        UCB0TXBUF = 0xF4;                   //send address that has to be written to

    while (!(UCB0IFG & UCTXIFG));           //wait till buffer is empty
        UCB0TXBUF = 0x2E;                   //send address for temperature measurement

    while (!(UCB0IFG & UCTXIFG));           //wait till buffer is empty
    UCB0CTL1 |= UCTXSTP;                    //send stop condition

    __delay_cycles(4500);                   //wait for A/D conversion

    UCB0CTL1 |= UCTR | UCTXSTT;             //send start condition

    while (!(UCB0IFG & UCTXIFG));           //wait till buffer is empty
    UCB0TXBUF = 0xF6;                       //send address that is read from

    while (!(UCB0IFG & UCTXIFG));           //wait till buffer is empty
    UCB0CTL1 &= ~UCTR;                      //set to read
    UCB0CTL1 |= UCTXSTT;                    //send restart condition

    while (!(UCB0IFG & UCRXIFG));           //wait till buffer is empty
    iTemp = UCB0RXBUF << 8;                 //receive LSB

    UCB0CTL1 |= UCTXSTP;                    //send stop condition

    while (!(UCB0IFG & UCRXIFG));           //wait till buffer is empty
    iTemp += UCB0RXBUF;                     //receive MSB



    X1 = (iTemp - fCalib_Param[AC6]) * fCalib_Param[AC5] / 32768.0;
    X2 = fCalib_Param[MC] * 2048.0 / (X1 + fCalib_Param[MD]);
    B5 = X1 + X2;
    iTrue_temp = ((B5 + 8.0) / 16.0);
    fTrue_temp = (float)iTrue_temp / 10.0;

    return fTrue_temp;
}

//**************************************************
//                  NOT USER RELEVANT
//**************************************************

float get_temp_(void)
{
    float iTemp = 0;

    UCB0CTL1 |= UCTR | UCTXSTT;             //start condition

    while (!(UCB0IFG & UCTXIFG));
        UCB0TXBUF = 0xF4;

    while (!(UCB0IFG & UCTXIFG));
        UCB0TXBUF = 0x2E;

    while (!(UCB0IFG & UCTXIFG));
    UCB0CTL1 |= UCTXSTP;

    __delay_cycles(4500);

    UCB0CTL1 |= UCTR | UCTXSTT;

    while (!(UCB0IFG & UCTXIFG));
    UCB0TXBUF = 0xF6;

    while (!(UCB0IFG & UCTXIFG));
    UCB0CTL1 &= ~UCTR;
    UCB0CTL1 |= UCTXSTT;

    while (!(UCB0IFG & UCRXIFG));
    iTemp = UCB0RXBUF << 8;

    UCB0CTL1 |= UCTXSTP;

    while (!(UCB0IFG & UCRXIFG));
    iTemp += UCB0RXBUF;

    return iTemp;
}
