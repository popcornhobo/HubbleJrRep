/* soft_spi.h
 * 
 * Source file for c functions relating to the VN-100's SPI
 *
 * Authors:	Zach Frazee
 * 
 *
 * Revions:
 * 2016-02-11	v0.1	Initial Revison
 * 2016-03-12	v0.2	Removed unnessesary delays and modified to conform to new coding standard 
 *
 * */
 
#ifndef SOFT_SPI_H
#define SOFT_SPI_H

/*----------------------------------------------------------------------*/
/*	SYSTEM DEFINES	*/

#ifndef soc_cv_av
	#define soc_cv_av
#endif

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

/*----------------------------------------------------------------------*/
/*	SYSTEM INCLUDES	*/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "soc_cv_av/socal/socal.h"
#include "soc_cv_av/socal/hps.h"
#include "soc_cv_av/socal/alt_gpio.h"
#include "hps_0.h"

/*---------------------------------------------------------------------*/
/*	FPGA SPI ADDRESSES */
#define SPI_ADDR_OUTPUT_MODE	    0x00000280
#define SPI_ADDR_DIRECT_OUTPUT	0x00000284
#define SPI_ADDR_CLOCK_DIVISOR	0x00000288
#define SPI_ADDR_WRITE 				0x0000028C
#define SPI_ADDR_READ					0x00000290
#define SPI_ADDR_STATUS 				0x00000294

/*---------------------------------------------------------------------*/
/* PUBLIC FUNCTION PROTOTYPE DECLARATIONS */


/* SPI_Init:
		Initializes the SPI's memory mapping and data lines
			Params:
				virtual_base - base for memory mapping
				operating_mode - ( 0 ) for software bitbanging, ( 1 ) for hardware spi
				clock_divsisor - spi_clock_speed = 50MHz / (2 * clock_divsisor) 
			Returns:
				Void 
*/
void SPI_Init(void * virtual_base, int operating_mode, int clock_divsisor);


/* SPI_Read_Write:
		Preforms a SPI read write operation
			Params:
				data_out - The byte of data to be written
			Returns:
				uint8_t - The byte of data read */
uint8_t SPI_Read_Write(uint8_t data_out);


/*	SPI_SS
		Changes the SS line
			Params:
				state - 1 for SS high, 0 for SS low
			Returns:
				void 
*/
void SPI_SS(char state);

#endif