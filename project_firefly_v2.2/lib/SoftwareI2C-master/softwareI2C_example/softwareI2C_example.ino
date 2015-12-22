#include <SoftwareI2C.h>

SWI2C myI2C;    // instanciate softwareI2C class

// variables
uint8_t Ack=1;
uint8_t data1, data2;

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Software I2C!");
  
  myI2C.begin();
}

void loop()
{
  // ===========================================================
  //              SHT21 TEMPERATURE MEASUREMENT
  // ===========================================================
  myI2C.i2cStart();
  myI2C.i2cWrite(0x80);   // 0b10000000  sensor addres + write attempt
  myI2C.i2cWrite(0xE6);   // write register
  myI2C.i2cWrite(0x82);   // set resolution to 11bit
  myI2C.i2cStart();
  myI2C.i2cWrite(0x80);   // 0b10000000  sensor addres + write attempt
  myI2C.i2cWrite(0xF3);   // trigger temp measure; no hold master
  delay(43);              // wait for measurment end
  myI2C.i2cStart();       
  myI2C.i2cWrite(0x81);   // 0b10000000  sensor addres + read attempt
  data1 = myI2C.i2cRead(1);   // get meas value MSByte
  data2 = myI2C.i2cRead(1);   // get meas value LSByte
  myI2C.i2cRead(0);           // get CRC8 value
  myI2C.i2cStop();        // end I2C transmission
  
  Serial.println(data1, HEX);
  Serial.println(data2, HEX);     
  Serial.println(Ack);
  
  delay(2000);          // wait 2s 
}
