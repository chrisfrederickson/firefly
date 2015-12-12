#include<msp430g2231.h>
void tempInit(){
    ADC10CTL0=SREF_1 + REFON + ADC10ON + ADC10SHT_3 ; //1.5V ref,Ref on,64 clocks for sample
    ADC10CTL1=INCH_10+ ADC10DIV_3; //temp sensor is at 10 and clock/4
}
void ledInit(){
	P2DIR |= 0x01;
}
int tempOut(){
    int t=0;
    __delay_cycles(1000);              //wait 4 ref to settle
    ADC10CTL0 |= ENC + ADC10SC;      //enable conversion and start conversion
    while(ADC10CTL1 & BUSY);         //wait..i am converting..pum..pum..
    t=ADC10MEM;                       //store val in t
    ADC10CTL0&=~ENC;                     //disable adc conv
    return(int) ((t * 27069L - 18169625L) >> 16); //convert and pass
}
void flashOn(){
	P2OUT = 0x01;
	__delay_cycles(1000000);
}
void flashOff(){
	P2OUT = 0x00;
	__delay_cycles(1000000);
}
void transmitBinary(int temp){
	volatile short short_temp = (short)temp;
	short masked_temp = 0;
	int i;
	for(i=0;i<8;i++){
		masked_temp = short_temp & 0x01;
		if (masked_temp == 0){
			flashOff();
		}else{
			flashOn();
		}
		short_temp = short_temp >> 1;
	}
}
void main(void){
	volatile int temp;    //initialise
    WDTCTL = WDTPW + WDTHOLD; //stop..bow..boww
    temp=0;
    tempInit();//initialise adc
    ledInit();
    while(1)
    {
        __delay_cycles(500); //wait and set break point
        temp=tempOut();           //read temp
        __delay_cycles(500);  //wait and set breakpoint
        transmitBinary(temp);

    }
}
