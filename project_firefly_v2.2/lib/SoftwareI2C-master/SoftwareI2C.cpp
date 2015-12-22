

#include "SoftwareI2C.h"
#include "Energia.h"

uint8_t _pinSDA;
uint8_t _pinSCL;


// MSP EXP430-RF2500 pins
    //  8 - P4.3 SDA
    //  9 - P4.4 SCL
	
#define SDAHigh pinMode(_pinSDA, INPUT)		// write 1, external pull-ups will tie SDA to VCC  
#define SCLHigh pinMode(_pinSCL, INPUT)		// write 1, external pull ups will tie SCL to VCC  
#define SDALow  pinMode(_pinSDA, OUTPUT)	// write 0, port will actively drive 0, has to be setup in init function  
#define SCLLow  pinMode(_pinSCL, OUTPUT)	// write 0, port will actively drive 0, has to be setup in init function  

static const uint8_t P_SDA = 15;
static const uint8_t P_SCL = 14;

// empty constructor
SWI2C::SWI2C() {
	SWI2C(P_SDA, P_SCL);
}

// constructor with variables
SWI2C::SWI2C(uint8_t pinSDA,uint8_t pinSCL){
	_pinSDA = pinSDA;
	_pinSCL = pinSCL;
}

void SWI2C::setI2Cpins(uint8_t pinSDA,uint8_t pinSCL)
{
	_pinSDA = pinSDA;
	_pinSCL = pinSCL;
}

// setup routine
void SWI2C::begin() {
	i2cInit();
}

//------------------------
// To initialize the bus 
//------------------------
void SWI2C::i2cInit()
{
	digitalWrite(_pinSDA, LOW);	// initialize value 0, port will switch between Input=1, Output=0
	digitalWrite(_pinSCL, LOW);	// initialize value 0, port will switch between Input=1, Output=0
	SCLHigh;               		// set SCL to High, external pullup
	SDAHigh;			// set SDA to High, external pullup
}

//------------------------
// Start Condition
// Hi/Lo edge at SDA during SCL high
//------------------------
void SWI2C::i2cStart()
{
	SCLHigh;
	SDAHigh;
	SDALow;
	delayMicroseconds(10);
	SCLLow;
	delayMicroseconds(10);
}

//------------------------
// Stop Condition
// Lo/Hi edge at SDA during SCL high
//------------------------
void SWI2C::i2cStop()
{
	SCLLow;
	SDALow;
	SCLHigh;
	delayMicroseconds(10);
	SDAHigh;
	delayMicroseconds(10);    
}

//------------------------
// Write  Transfer
// parameter: uint8_t txData - data to be transmitted to slave
// return: uint8_t  ack - returns the acknowledge signal
//------------------------
/*uint8_t SWI2C::i2cWrite(uint8_t txData)
{
  uint8_t i;
  uint8_t ack;

  for (i=0; i<8; i++)
  {
     SCLLow;
     if (txData & 0x80) SDAHigh;
     else SDALow;
     SCLHigh;
     txData <<= 1;
  }
  SCLLow;

  // ack Read 
  SDAHigh;
  SCLHigh;

  if (digitalRead(_pinSDA)) ack = NO_I2C_ACK;      
  else ack = OK_I2C_ACK;
  
  SCLLow;
  return(ack);
}*/
uint8_t SWI2C::i2cWrite(uint8_t txData)
{
	uint8_t mask,error=0,ack;
	for (mask=0x80; mask>0; mask>>=1){ 		//shift bit for masking (8 times)
		if ((mask & txData) == 0) SDALow;	//masking txByte, write bit to SDA-Line
		else SDAHigh;
		delayMicroseconds(1); 		//data set-up time (t_SU;DAT)
		SCLHigh; 			//generate clock pulse on SCL
		delayMicroseconds(5); 		//SCL high time (t_HIGH)
		SCLLow;
		delayMicroseconds(1); 		//data hold time(t_HD;DAT)
	}
	
	SDAHigh; //release SDA-line
	SCLHigh; //clk #9 for ack
	delayMicroseconds(1); //data set-up time (t_SU;DAT)
	if (digitalRead(_pinSDA)) ack = NO_I2C_ACK; //check ack from i2c slave
	else ack = OK_I2C_ACK;
	SCLLow;
	delayMicroseconds(20); //wait time to see byte package on scope
	return ack;
}

uint8_t SWI2C::i2cWrite2(uint8_t txData)
{
	uint8_t mask,error=0,ack;
	for (mask=0x80; mask>0; mask>>=1){ 		//shift bit for masking (8 times)
		if ((mask & txData) == 0) SDALow;	//masking txByte, write bit to SDA-Line
		else SDAHigh;
		delayMicroseconds(1); 		//data set-up time (t_SU;DAT)
		SCLHigh; 			//generate clock pulse on SCL
		delayMicroseconds(5); 		//SCL high time (t_HIGH)
		SCLLow;
		delayMicroseconds(1); 		//data hold time(t_HD;DAT)
	}
	
	SDAHigh; //release SDA-line
	SCLHigh; //clk #9 for ack
	delayMicroseconds(1); //data set-up time (t_SU;DAT)
	if (digitalRead(_pinSDA)) ack = NO_I2C_ACK; //check ack from i2c slave
	else ack = OK_I2C_ACK;
	SCLLow;
	delayMicroseconds(20); //wait time to see byte package on scope
	SCLHigh;
	return ack;
}

//------------------------
// Read  Transfer
// parameter: uint8_t ack - shall ack be evaluated?
// return: uint8_t  rxData - received byte
//------------------------
/*uint8_t  SWI2C::i2cRead(uint8_t ack)
{
  uint8_t i;
  uint8_t rxData = 0x00;

  SCLLow;
  SDAHigh;

  for (i=0; i<8; i++)
  {
     rxData = rxData << 1;
     SCLHigh;
     if (digitalRead(_pinSDA)) rxData = rxData  + 1;
     SCLLow;
  }

  if (ack) SDALow;
  
  SCLHigh;
  SCLLow;

  return (rxData);
}*/

//==============================================================================
uint8_t  SWI2C::i2cRead(uint8_t ack)
//==============================================================================
{
	uint8_t mask,rxByte=0;
	SDAHigh; //release SDA-line
	for (mask=0x80; mask>0; mask>>=1){ //shift bit for masking (8 times)
		SCLHigh; //start clock on SCL-line
		delayMicroseconds(1); //data set-up time (t_SU;DAT)
		delayMicroseconds(3); //SCL high time (t_HIGH)
		if (digitalRead(_pinSDA)) rxByte=(rxByte | mask); //read bit
		SCLLow;
		delayMicroseconds(1); //data hold time(t_HD;DAT)
	}
	if (ack) SDALow;
	delayMicroseconds(1); //data set-up time (t_SU;DAT)
	SCLHigh; //clk #9 for ack
	delayMicroseconds(5); //SCL high time (t_HIGH)
	SCLLow;
	SDAHigh; //release SDA-line
	delayMicroseconds(20); //wait time to see byte package on scope
	return rxByte; //return error code
}