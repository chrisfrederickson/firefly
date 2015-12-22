#include <SoftwareI2C.h>

#include <pfatfs.h>
#include <pffconf.h>

//#include<Wire.h>

//#include <I2cMaster.h>

//#include <I2Cdev.h>
//#include <Wire.h>
//#include <helper_3dmath.h>
//#include <MPU6050.h>

#define SCL_PIN 14
#define SDA_PIN 15

bool LED_active = false;
SWI2C myI2C;    // instanciate softwareI2C class

// variables 
uint8_t Ack=1;

//SoftI2cMaster rtc(SDA_PIN, SCL_PIN);

const int MPU_addr=0xD2;  // I2C address of the MPU-6050 69
//const int MPU_addr=0xD0;  // I2C address of the MPU-6050 68
uint16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

//MPU6050 accelgyro;

//int16_t ax, ay, az;
//int16_t gx, gy, gz;

/*----------------------------------------------------------------------*/
/* Petit FatFs sample project for generic uC  (C)ChaN, 2010             */
/*----------------------------------------------------------------------*/
/*-----------------------------------------------------------------------
   Ported to Energia by calinp
   Copy the file LOG.txt from this file's location to the root of the `[
   SD-Card.

   12/21/2013
   Modified by bluehash(43oh) to log temperature data from the MSP430G2553
   internal temperature sensor and log it to a file on the SD-Card
   References:
   http://forum.43oh.com/topic/3209-energia-library-petit-fatfs-sd-card-library/
 
   This project was later modified by Nick Felker, Elijah Neville, and Mike McCaffrey
   The system now uses a variety of sensors and writes them to a predefined CSV file
 */
#include "SPI.h" 
#include "pfatfs.h"

#define cs_pin      5             // chip select pin P1.5
#define read_buffer 128             // size (in bytes) of read buffer
#define LED 11
//#define LED 14

//GO HERE TO SET CONFIGURATION VARIABLES
int WRITE_SPEED = 20; //How many milliseconds should be used between writes? (1000Hz is the max thereotical speed, but it's more like 25Hz with write delays)
String filename = "LOG.CSV"; //Make sure the file is preallocated with enough data to store all the logs you want. (See http://43oh.com/2013/12/interfacing-the-launchpad-to-an-sd-card-a-walkthrough/ )
//In Windows' CMD, you can execute the command `fsutil file createnew <filename> 1048576` to create a new file with a specific number of bytes
boolean stopExecutionIfSDError = false; //If your wiring is incorrect,
//or you remove the SD card during progam execution, the system will
//start to fail and will print errors. If you set this to true,
//the program will completely stop working instead of continuing to fail
boolean continueIncrementingIfSDError = true; //If there is a problem, do you want the system to pause incrementing time points, or continue and leave those data as undefined? 
//It depends on whether you want blank data points with the correct time scale (true)
//Or if you want each data point to have its own weight (false)

//Here are some system variables
unsigned short int bw, br;
char buffer[read_buffer];
int rc; //Return code for File I/O. 0 means okay, else means a specific error

char buf[128];
uint32_t counter = 0;
uint32_t AccStringLength = 0;
uint32_t lastWrite = 0;  
boolean SDError = false; //Used for asynchronous operations, can inform if there's a problem with the SD card

void setup() {
  delay(1000);
  myI2C.i2cStart();
  delayMicroseconds(10);
  myI2C.i2cWrite(MPU_addr | 0);
  delayMicroseconds(10);
  myI2C.i2cWrite(0x6B); //0x3B
  delayMicroseconds(10);
  myI2C.i2cWrite(0);   // 0b10000000  sensor addres + read attempt
  delayMicroseconds(10);
  myI2C.i2cStop();        // end I2C transmission
  delay(1000);
  
  //myI2C.begin();
  //myI2C.i2cWrite(MPU_addr | 0);
  //myI2C.i2cWrite(0x6B);
  //myI2C.i2cWrite(0);
  //myI2C.i2cStop();        // end I2C transmission
  
  
  //Wire.begin();
  //Wire.beginTransmission(MPU_addr);
  //Wire.write(0x6B);  // PWR_MGMT_1 register
  //Wire.write(0);     // set to zero (wakes up the MPU-6050)
  //Wire.endTransmission(true);
  
  /*
  if (rtc.start(MPU_addr | I2C_WRITE)) {
      rtc.write(0x6B); // PWR_MGMT_1 register
      rtc.write(0); // set to zero (wakes up the MPU-6050)
    }
    rtc.stop();
  */
    //Serial.begin(4800);                // initialize the //Serial terminal
    //Serial.println("Loading...");
    lastWrite = millis(); //We keep storing the lastWrite time for asynchronous SD writing
    
    //Wire.begin();
    //accelgyro.initialize();
    
    analogReference(INTERNAL1V5);
    analogRead(TEMPSENSOR);           // first reading usually wrong
    FatFs.begin(cs_pin, 3);              // initialize FatFS library calls
    // initialize the digital pin as an output.
    pinMode(LED, OUTPUT);     
    //Serial.println("**********\r\n MSP430 Temperature Logger \n\r**********\n\r\n\r");
}
void loop() {
  if(millis() - lastWrite > WRITE_SPEED) { //Write only so often, but check in a non-blocking way
    lastWrite = millis();
    write();
    digitalWrite(LED,HIGH);
  }
}
         
/* Print dying message. This may stop program execution. */    
void die(int pff_err) {
  //Serial.print("Failed with rc=");
  //Serial.print(pff_err,DEC);
    /*digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(250);               // wait for a second
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
    delay(250);               // wait for a second
    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(250);               // wait for a second
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);               // wait for a second*/
  SDError = true;
  if(stopExecutionIfSDError) {
    for (;;) ; //Endless loop that prevents the system from returning to normal operation
  } else {    
    //Serial.println(" but we keep going"); 
    delay(50);
    //Try to reset the FatFs object
    FatFs.begin(cs_pin, 3); 
    delay(50);
  }
}

/*-----------------------------------------------------------------------*/
/* Program Main                                                          */
/*-----------------------------------------------------------------------*/
void write() {
  //Grab our sensors data
  myI2C.i2cStart();
  delayMicroseconds(10);
  myI2C.i2cWrite(MPU_addr | 0);
  delayMicroseconds(10);
  myI2C.i2cWrite(0x42); //0x3B
  delayMicroseconds(10);
  myI2C.i2cStart();
  delayMicroseconds(10);
  myI2C.i2cWrite(MPU_addr | 1);   // 0b10000000  sensor addres + read attempt
  delayMicroseconds(10);
  AcX = myI2C.i2cRead(1)<<8;  // get meas value MSByte
  delayMicroseconds(10);
  AcX = AcX | myI2C.i2cRead(1);
  delayMicroseconds(10);
  AcY = myI2C.i2cRead(1)<<8;  // get meas value MSByte
  delayMicroseconds(10);
  AcY= AcY | myI2C.i2cRead(1);
  delayMicroseconds(10);
  AcZ = myI2C.i2cRead(1)<<8;  // get meas value MSByte
  delayMicroseconds(10);
  AcZ = AcZ | myI2C.i2cRead(1);
  delayMicroseconds(10);
  Tmp = myI2C.i2cRead(1)<<8;  // get meas value MSByte
  delayMicroseconds(10);
  Tmp = Tmp | myI2C.i2cRead(1);
  delayMicroseconds(10);
  GyX = myI2C.i2cRead(1)<<8;  // get meas value MSByte
  delayMicroseconds(10);
  GyX = GyX | myI2C.i2cRead(1);
  delayMicroseconds(10);
  GyY = myI2C.i2cRead(1)<<8;  // get meas value MSByte
  delayMicroseconds(10);
  GyY = GyY | myI2C.i2cRead(1);
  delayMicroseconds(10);
  GyZ = myI2C.i2cRead(1)<<8;  // get meas value MSByte
  delayMicroseconds(10);
  GyZ = GyZ | myI2C.i2cRead(0);
  delayMicroseconds(10);
  myI2C.i2cStop();        // end I2C transmission
  //accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  /*
    if (rtc.start(MPU_addr | I2C_WRITE)) {
      rtc.write(0x3C); // PWR_MGMT_1 register
    }
    if (rtc.start(MPU_addr | I2C_READ)) {
      AcX = rtc.read(true); // PWR_MGMT_1 register
    }
    rtc.stop();
    */
  /*
  Wire.beginTransmission(MPU_addr);
  Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers
  AcX=myI2C.i2cRead(1)<<8|  ();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
  AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
  AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
  Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
  GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
  GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
  */
  
  //GO HERE TO DECLARE YOUR SENSORS
   uint32_t temp = ((uint32_t)analogRead(TEMPSENSOR)*27069 - 18169625) *10 >> 16; //Formula to temp sensor voltage to Celcius
   uint32_t barometer = ((uint32_t) 3); //We can read from all kinds of sensors here
   //Serial.println();
   //Serial.println("Opening log file to write data.");
   delay(20);
   
   SDError = false; //We'll be optimisitc and then change if there's still a problem
   rc = FatFs.open(filename.c_str()); //Open a file at the given filename
   if (rc){
     die(rc); //We may get an SD card error here. Let's not try to write if we cannot.
     
    /*digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);               // wait for a second
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
    delay(100);               // wait for a second
    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);               // wait for a second
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
    delay(100);               // wait for a second
    digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100);               // wait for a second
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);               // wait for a second*/
    
   }

   if(!SDError || continueIncrementingIfSDError) {
     delay(20); //Delays are placed so the MSP430 has enough time to properly open the file
     bw=0;
     //GO HERE IF YOU WANT TO CHANGE THE FILE OUTPUT
     //To insert a long, use  %lu (lowercase L), %s for a string of characters
     
     
     //sprintf(buf, "%lu,%lu.%lu,%lu\r\n", counter, temp/10, temp%10, barometer); //Write a CSV row to a buffer
     sprintf(buf, "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\r\n", counter, AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ); //Write a CSV row to a buffer
     
     
     
     counter++; //Increment the index of the CSV file
     uint8_t StringLength =  strlen(buf); //Grab the buffer
     //Serial.println(buf);        
  
    /*digitalWrite(LED, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(1000);               // wait for a second
    digitalWrite(LED, LOW);    // turn the LED off by making the voltage LOW
    delay(1000);               // wait for a second*/
    
     rc = FatFs.lseek(AccStringLength); //Append to your file instead of overwriting
     if (rc) die(rc);
     AccStringLength =  AccStringLength + 512;
     rc = FatFs.write(buf, StringLength,&bw); //Start writing the buffer
     if (rc) die(rc);
     rc = FatFs.write(0, 0, &bw);  //Finalize write
     if (rc) die(rc);
     rc = FatFs.close();  //Close file
          if (rc) die(rc);
   }
}


