/* VN_UnitTest.c
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

#include "soft_spi.h"
#include "VN_lib.h"



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
	
	
	/* INITIALIZE SPI */
	SPI_Init(virtual_base, 1, 5);
	
	usleep(1000 *1000);
	
	/* MAIN LOOP */
	printf("Starting...\n");
	for(;;){
		
		
		/* SETUP TIMER */
		struct  timeval start_time, end_time;
		
		
		/* Read Data From VN-100 */
		printf("Getting Quarternion Rates from VN-100\n");
		
		unsigned char sensorID = 0;
		float q[4];
		float rates[3];
		
		gettimeofday(&start_time, NULL); //Starting time of write
		
		VN100_SPI_Packet * ReturnPacket = VN100_SPI_GetQuatRates(sensorID, q, rates);
		
		gettimeofday(&end_time, NULL); //Ending time of write
		
		printf("q0: %f\t q1: %f\t q2: %f\t q3: %f\n", q[0], q[1], q[2], q[3]);
		
		printf("rate0: %f\t rate1: %f\t rate2: %f\n", rates[0], rates[1], rates[2]);
		
		printf("Error ID: %u\n", *(uint8_t*)ReturnPacket);
		
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
