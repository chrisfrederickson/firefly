#include <SoftwareI2C.h>

#include <pfatfs.h>
#include <pffconf.h>

#define SCL_PIN 14
#define SDA_PIN 15

bool LED_active = false;
SWI2C myI2C;    // instanciate softwareI2C class

const int MPU_addr=0xD2;  // I2C address of the MPU-6050 69
uint16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ;

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
   
   This code was futher modified by Christopher Frederickson, Rich Sbresney, and Andrew Getler.
 */
#include "SPI.h" 
#include "pfatfs.h"

#define cs_pin      5             // chip select pin P1.5
#define read_buffer 128             // size (in bytes) of read buffer
#define LED 11

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
  
  lastWrite = millis(); //We keep storing the lastWrite time for asynchronous SD writing
    pinMode(LED, OUTPUT); //Initilize the LED as an ouput
    digitalWrite(LED,HIGH);
  
  //MPU6050 Initilization Routine
  delay(1000);
  myI2C.i2cStart();
  delayMicroseconds(10);
  myI2C.i2cWrite(MPU_addr | 0);
  delayMicroseconds(10);
  myI2C.i2cWrite(0x6B);
  delayMicroseconds(10);
  myI2C.i2cWrite(0); 
  delayMicroseconds(10);
  myI2C.i2cStop();
  delay(1000);

  FatFs.begin(cs_pin, 3); // initialize FatFS library calls

}
void loop() {
  if(millis() - lastWrite > WRITE_SPEED) { //Write only so often, but check in a non-blocking way
    lastWrite = millis();
    write();
  }
}
         
/* Print dying message. This may stop program execution. */    
void die(int pff_err) {
  SDError = true;
  if(stopExecutionIfSDError) {
    for (;;) ; //Endless loop that prevents the system from returning to normal operation
  } else {    
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
  //Grab our sensors data via burst transaction
  //All data is 2 bytes long and sent in 1 byte increments with the MSByte first
  //Short delays are perhaps unnecessary, but helped with the I2C debugger
  myI2C.i2cStart();
  delayMicroseconds(10);
  myI2C.i2cWrite(MPU_addr | 0);
  delayMicroseconds(10);
  myI2C.i2cWrite(0x42);
  delayMicroseconds(10);
  myI2C.i2cStart();
  delayMicroseconds(10);
  myI2C.i2cWrite(MPU_addr | 1);
  delayMicroseconds(10);
  AcX = myI2C.i2cRead(1)<<8;
  delayMicroseconds(10);
  AcX = AcX | myI2C.i2cRead(1);
  delayMicroseconds(10);
  AcY = myI2C.i2cRead(1)<<8;
  delayMicroseconds(10);
  AcY= AcY | myI2C.i2cRead(1);
  delayMicroseconds(10);
  AcZ = myI2C.i2cRead(1)<<8;
  delayMicroseconds(10);
  AcZ = AcZ | myI2C.i2cRead(1);
  delayMicroseconds(10);
  Tmp = myI2C.i2cRead(1)<<8;
  delayMicroseconds(10);
  Tmp = Tmp | myI2C.i2cRead(1);
  delayMicroseconds(10);
  GyX = myI2C.i2cRead(1)<<8;
  delayMicroseconds(10);
  GyX = GyX | myI2C.i2cRead(1);
  delayMicroseconds(10);
  GyY = myI2C.i2cRead(1)<<8;
  delayMicroseconds(10);
  GyY = GyY | myI2C.i2cRead(1);
  delayMicroseconds(10);
  GyZ = myI2C.i2cRead(1)<<8;
  delayMicroseconds(10);
  GyZ = GyZ | myI2C.i2cRead(0);
  delayMicroseconds(10);
  myI2C.i2cStop();
  
   delay(20);
   
   SDError = false; //We'll be optimisitc and then change if there's still a problem
   rc = FatFs.open(filename.c_str()); //Open a file at the given filename
   if (rc){
     die(rc); //We may get an SD card error here. Let's not try to write if we cannot.
   }

   if(!SDError || continueIncrementingIfSDError) {
     delay(20); //Delays are placed so the MSP430 has enough time to properly open the file
     bw=0;
     //GO HERE IF YOU WANT TO CHANGE THE FILE OUTPUT
     //To insert a long, use  %lu (lowercase L), %s for a string of characters

     sprintf(buf, "%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu\r\n", counter, AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ); //Write a CSV row to a buffer
     
     counter++; //Increment the index of the CSV file
     uint8_t StringLength =  strlen(buf); //Grab the buffer

     rc = FatFs.lseek(AccStringLength); //Append to your file instead of overwriting
     if (rc) die(rc);
     AccStringLength =  AccStringLength + 512; //TODO 512 is perhaps unneccessary due to beffer being only 128 bytes
     rc = FatFs.write(buf, StringLength,&bw); //Start writing the buffer
     if (rc) die(rc);
     rc = FatFs.write(0, 0, &bw);  //Finalize write
     if (rc) die(rc);
     rc = FatFs.close();  //Close file
          if (rc) die(rc);
   }
}


