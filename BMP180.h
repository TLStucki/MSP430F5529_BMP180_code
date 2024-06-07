/*
 * BMP180.h
 *
 *  Created on: 12.01.2024
 *      Author: Tobias Lars Stucki
 *      published under MIT license
 *      V1.0
 */

#ifndef BMP180_H_
#define BMP180_H_

#define RES1 0      //Pressure resolution parameter lowest:  RES1
#define RES2 1      //Pressure resolution parameter
#define RES3 2      //Pressure resolution parameter
#define RES4 3      //Pressure resolution parameter highest: RES4

void init_i2c_(void);                       //function to initialize I2C module (has to be changed depending on which module/controller is used default is: MSP430F5529)

void BMP180_Get_Calib_Param(void);          //function to read calibration parameters from BMP180 has to be used once at startup

float BMP180_get_temp(void);                //function to get temperature reading with one digit after point e.g. 20.1C

long long int BMP180_get_press(int oss);    //function to get pressure in Pa



//**************************************************
//                  NOT USER RELEVANT
//**************************************************
float get_temp_(void);


#endif /* BMP180_H_ */
