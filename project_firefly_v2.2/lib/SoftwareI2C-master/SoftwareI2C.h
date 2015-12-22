//
//  software I2C for Energia
//  Example for library for Sharp BoosterPack LCD with hardware SPI
//
//
//  Author :  ArcticSaturn
//  Date   :  Nov 16, 2014
//  Version:  1.00
//  File   :  SoftwareI2C.h
//


#ifndef SWI2C_h
#define SWI2C_h
// defaults

#include <Energia.h>

#define NO_I2C_ACK 0
#define OK_I2C_ACK 1

class SWI2C {
public:
	SWI2C();								// empty constructor
	SWI2C(uint8_t pinSDA,uint8_t pinSCL);	// constructor with variables
	void begin();  							// setup I2C
	void i2cStart();						// Start Data Transfer
	void i2cStop();							// Stop Data Transfer
	uint8_t i2cWrite(uint8_t txData);		// write data byte
	uint8_t i2cWrite2(uint8_t txData);		// write data byte
    uint8_t i2cRead(uint8_t ack);			// Read data byte        
	void setI2Cpins(uint8_t pinSDA,uint8_t pinSCL);
	
private:
	void i2cInit();							// Init the ports and the default pin state
};

#endif
