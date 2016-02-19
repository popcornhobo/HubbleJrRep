/* soft_spi.h
 * 
 * Source file for c functions relating to the control system
 *
 * Authors:	Zach Frazee
 * 
 *
 * Revions:
 * 2016-02-11	v0.1	Initial Revison
 *
 * */
 
#ifndef SOFT_SPI_H
#define SOFT_SPI_H

//----------------------------------------------------------------------//
//	SYSTEM DEFINES	//

#ifndef soc_cv_av
	#define soc_cv_av
#endif

#define HW_REGS_BASE ( ALT_STM_OFST )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

//----------------------------------------------------------------------//
//	SYSTEM INCLUDES	//

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "soc_cv_av/socal/socal.h"
#include "soc_cv_av/socal/hps.h"
#include "soc_cv_av/socal/alt_gpio.h"
#include "hps_0.h"

//---------------------------------------------------------------------//
//	FPGA SPI PIO ADDRESSES	//
#define SPI_ADDR_PIO_DATA			0x000000A0
#define SPI_ADDR_PIO_DATADIR		0x000000A4


//---------------------------------------------------------------------//
// FUNCTION PROTOTYPE DECLARATIONS	//

//Initializes SPI
void spi_init();

//performs an SPI read write
int SPI_Read_Write(uint8_t data_out);

//controls SPI clock line
void SPI_Clock(char state);

//controls MOSI
void SPI_MOSI(char state);

//controls SS
void SPI_SS(char state);

//reads MISO
int SPI_MISO();

#endif