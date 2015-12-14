#include <msp430.h> 

#include "MPU6050.h"
#include "LPS25.h"
//#include "MMC.h"
#include "LED.h"
#include "AUDIO.h"

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // Set clock speed (default = 1 MHz)
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL  = CALDCO_1MHZ;

    // Intialization Routines
    //init_mmc();
    init_led();
    init_audio();
    init_mpu6050();
    init_lps25();

    //Data logging

	return 0;
}
