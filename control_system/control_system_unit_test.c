/* ControlSystem_UnitTest.c
 * 
 * Unit test for VN-100 and SPI code
 *
 * Authors:	Zach Frazee
 * 
 *
 * Revions:
 * 2016-3-12	v0.1	Initial Revison
 *
 * */
 
/*----------------------------------------------------------------------*/
/*	SYSTEM DEFINES */

#define soc_cv_av
#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )



/*----------------------------------------------------------------------*/
/*	SYSTEM INCLUDES */

#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "soc_cv_av/socal/socal.h"
#include "soc_cv_av/socal/hps.h"
#include "soc_cv_av/socal/alt_gpio.h"
#include "hps_0.h"



/*-----------------------------------------------------------------------*/
/*	USER INCLUDES */

#include "control_system.h"



/*-----------------------------------------------------------------------*/
/*	MAIN */

int main() {
	
	
	/*INITIALIZE MEMORY MAPPING	*/
	void *virtual_base;
	int fd;	
	
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );
	
	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}
	
	/* SET PID */
	float P_pitch 	= 1.0;
	float I_pitch 	= 1.0;
	float D_pitch	= 1.0;
	
	float P_roll		= 1.0;
	float I_roll		= 1.0;
	float D_roll	= 1.0;
	
	float P_yaw	= 1.0;
	float I_yaw	= 1.0;
	float D_yaw	= 1.0;
	
	update_gains(P_pitch, P_yaw, P_roll, I_pitch, I_yaw, I_roll, D_pitch, D_yaw, D_roll);
	
	/* MAIN LOOP */
	printf("Starting...\n");
	for(;;){
		
		
		/* SETUP TIMER */
		struct  timeval start_time, end_time;
		
		/* Update the control system */
		printf("Getting Quarternion Rates from VN-100\n");
		
		gettimeofday(&start_time, NULL); //Starting time of write
		
		/* Update ControlSystem */
		control_system_update();
		
		unsigned long int time = (end_time.tv_sec * 1000000 + end_time.tv_usec) - (start_time.tv_sec * 1000000 + start_time.tv_usec);
		
		printf("Completed in %uus\n\n\n", time);
		
		usleep(1000);
	}
	
	
	/*----------------------------------------------------------------------*/
	/*	CLEAN UP MEMORY MAPPING	*/
	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

}
