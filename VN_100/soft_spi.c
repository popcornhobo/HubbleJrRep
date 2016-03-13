/* soft_spi.c
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
 
#include "soft_spi.h"

/*---------------------------------------------------------------------*/
/* MEMORY MAPPED REGISTERS */
void * spi_pio_data;
void * spi_pio_datadir;

int fd;
void * virtual_base;

/* Initialized Variable */
char is_initialized = 0; 



/*---------------------------------------------------------------------*/
/* PRIVATE FUNCTION PROTOTYPE DECLARATIONS */

/*	SPI_Clock
		Clocks the SPI line
			Params:
				state - 1 for clock high, 0 for clock low
			Returns:
				void 
*/
void SPI_Clock(char state);

/*	SPI_MOSI
		Changes the MOSI line
			Params:
				state - 1 for MOSI high, 0 for MOSI low
			Returns:
				void 
*/
void SPI_MOSI(char state);

/* SPI_MISO
		Gets a bit of data from the MISO
			Params:
				None
			Returns:
				uint8_t - Bit from MISO is the LSB
*/
uint8_t SPI_MISO();

/* NOP_Delay
		A precise delay that uses nops
			Params:
				counter - how many times NOP is called
			returns:
				void 
*/
void NOP_Delay(int count); void NOP();

/*---------------------------------------------------------------------*/
/* PRIVATE DEFINES */

//#define USE_CLOCK_DELAY
//#define USE_DATA_DELAY
//#define USE_CLOCK_DELAY_NOP
//#define USE_DATA_DELAY_NOP
//#define USE_SAMPLING

#define CLOCK_DELAY 5
#define DATA_DELAY 5

#define NOP_CLOCK_DELAY 100
#define NOP_DATA_DELAY 100

#define NUMBER_OF_SAMPLES 3
#define SAMPLE_DELAY 1

/*---------------------------------------------------------------------*/
/* FUNCTION IMPLEMENTATIONS*/



/* SPI_Init:
		Initializes the SPI's memory mapping and data lines
			Params:
				virtual_base - base for memory mapping
			Returns:
				Void 
*/
void SPI_Init(void * virtual_base ){
	spi_pio_data = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SPI_ADDR_PIO_DATA ) & ( unsigned long)( HW_REGS_MASK ) );
	spi_pio_datadir = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SPI_ADDR_PIO_DATADIR ) & ( unsigned long)( HW_REGS_MASK ) );
	
	//Bit order MSB = MOSI - MISO - SS - CLK  
	
	*(uint32_t *)spi_pio_datadir = 0b1011;
	*(uint32_t *)spi_pio_data = 0b1111;
	
	is_initialized = 1;
}

/*---------------------------------------------------------------------------------------------*/



/* SPI_Read_Write:
		Preforms a SPI read write operation
			Params:
				data_out - The byte of data to be written
			Returns:
				uint8_t - The byte of data read 
*/
uint8_t SPI_Read_Write(uint8_t data_out){
	/* receive variable */
	uint8_t data_in = 0;
	
	/* Byte loop */
	int i;
	for(i = 0; i < 8; i++){
		/* pull clock low */
		SPI_Clock(0);
		#ifdef USE_CLOCK_DELAY
			usleep(CLOCK_DELAY);
		#endif
		
		#ifdef USE_CLOCK_DELAY_NOP
			NOP_Delay(NOP_CLOCK_DELAY);
		#endif
		
		/* Set MOSI */
		SPI_MOSI((data_out >> (7 - i)) & 1);
		
		/* Wait for VN-100 to change MISO */
		#ifdef USE_DATA_DELAY
			usleep(DATA_DELAY);
		#endif
		
		#ifdef USE_DATA_NOP
			usleep(DATA_DELAY);
		#endif
		
		/* Clock High */
		SPI_Clock(1);
		
		#ifdef USE_CLOCK_DELAY
			usleep(CLOCK_DELAY);
		#endif
		
		#ifdef USE_CLOCK_DELAY_NOP
			NOP_Delay(NOP_CLOCK_DELAY);
		#endif
		
		/* Read MISO */
		#ifdef USE_SAMPLING
			int sum_of_samples = 0;
			int i;
			for(i = 0; i < NUMBER_OF_SAMPLES; i++){
				sum_of_samples += SPI_MISO();
				NOP_Delay(SAMPLE_DELAY);
			}
			
			float voting = (float) sum_of_samples / (float) NUMBER_OF_SAMPLES;
			
			if(voting > 0.5){
				data_in |= (1 << (7 - i));
			}else{
				data_in |= (0 << (7 - i));
			}	
		#else
			data_in |= (SPI_MISO() << (7 - i));
		#endif
		
		#ifdef USE_CLOCK_DELAY
			usleep(CLOCK_DELAY);
		#endif
		
		#ifdef USE_CLOCK_DELAY_NOP
			NOP_Delay(NOP_CLOCK_DELAY);
		#endif
	}
	
	return data_in;
}

/*---------------------------------------------------------------------------------------------*/



/*	SPI_Clock
		Clocks the SPI line
			Params:
				state - 1 for clock high, 0 for clock low
			Returns:
				void 
*/
void SPI_Clock(char state){
	if(state == 1){
		*(uint32_t *)spi_pio_data |= 0x00000001;
	}else{
		*(uint32_t *)spi_pio_data &= 0xFFFFFFFE;
	}
}

/*---------------------------------------------------------------------------------------------*/



/*	SPI_MOSI
		Changes the MOSI line
			Params:
				state - 1 for MOSI high, 0 for MOSI low
			Returns:
				void 
*/
void SPI_MOSI(char state){
	if(state == 1){
		*(uint32_t *)spi_pio_data |= 0x00000008;
	}else{
		*(uint32_t *)spi_pio_data &= 0xFFFFFFF7;
	}
}

/*---------------------------------------------------------------------------------------------*/



/*	SPI_SS
		Changes the SS line
			Params:
				state - 1 for SS high, 0 for SS low
			Returns:
				void 
*/
void SPI_SS(char state){
	if(state == 1){
		/* Ensure that clock line is high */
		SPI_Clock(1);
		
		/* set SS high */
		*(uint32_t *) spi_pio_data |= 0x00000002;
	} else{
		/* Ensure that clock line is high */
		SPI_Clock(1);
		
		/* set SS low */
		*(uint32_t *) spi_pio_data &= 0xFFFFFFFD;
	}
}

/*---------------------------------------------------------------------------------------------*/



/* SPI_MISO
		Gets a bit of data from the MISO
			Params:
				None
			Returns:
				uint8_t - Bit from MISO is the LSB
*/
uint8_t SPI_MISO(){
	return (*(uint32_t *)spi_pio_data >> 2) & 1;
}

/*---------------------------------------------------------------------------------------------*/



/* NOP_Delay
		A precise delay that uses nops
			Params:
				counter - how many times NOP is called
			returns:
				void 
*/
void NOP_Delay(int count){
	for(; count > 0; count--){
		NOP();
	}
}

void NOP(){
	asm("nop;nop;nop;nop;nop;nop;nop;nop;nop;nop");
} 

/*---------------------------------------------------------------------------------------------*/