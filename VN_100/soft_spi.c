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
/* Global Variables */

int spi_operating_mode = 0;
int fd;
void * virtual_base;

/* Initialized Variable */
char is_initialized = 0; 

/*---------------------------------------------------------------------*/
/* Memory Mapped Registers */

void * spi_output_mode;
void * spi_direct_output;
void * spi_clock_divisor;
void * spi_write;
void * spi_read;
void * spi_status;

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

/* SPI_Read_Write_BitBang:
		Preforms a SPI read write operation using bitbanging
			Params:
				data_out - The byte of data to be written
			Returns:
				uint8_t - The byte of data read 
*/
uint8_t SPI_Read_Write_BitBang(uint8_t data_out);

/* SPI_Read_Write_Hardware:
		Preforms a SPI read write operation using hardware
			Params:
				data_out - The byte of data to be written
			Returns:
				uint8_t - The byte of data read 
*/
uint8_t SPI_Read_Write_Hardware(uint8_t data_out);

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
				operating_mode - ( 0 ) for software bitbanging, ( 1 ) for hardware spi
				clock_divsisor - spi_clock_speed = 50MHz / (2 * clock_divsisor) 
			Returns:
				Void 
*/
void SPI_Init(void * virtual_base, int operating_mode, int clock_divsisor){
	spi_output_mode	= virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SPI_ADDR_OUTPUT_MODE ) & ( unsigned long)( HW_REGS_MASK ) );
	spi_direct_output 	= virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SPI_ADDR_DIRECT_OUTPUT ) & ( unsigned long)( HW_REGS_MASK ) );
	spi_clock_divisor	= virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SPI_ADDR_CLOCK_DIVISOR ) & ( unsigned long)( HW_REGS_MASK ) );
	spi_write 				= virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SPI_ADDR_WRITE ) & ( unsigned long)( HW_REGS_MASK ) );
	spi_read				= virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SPI_ADDR_READ ) & ( unsigned long)( HW_REGS_MASK ) );
	spi_status				= virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SPI_ADDR_STATUS ) & ( unsigned long)( HW_REGS_MASK ) );
		
	//Bit order MSB = MOSI - MISO - SS - CLK  
	
	
	/* Initialize outputs */
	*(uint32_t *)spi_output_mode = 0;
	*(uint32_t *)spi_direct_output = 0b1111;
	
	/* Settup Hardware SPI if it is to be used */ 
	if(operating_mode == 1){
		*(uint32_t *)spi_clock_divisor = clock_divsisor;
		
		usleep(50); //Wait
		
		if(*(uint32_t *)spi_status == 1){ //Check Status 
			spi_operating_mode = 1;
			*(uint32_t *)spi_output_mode = 1;
		} else {
			printf("Hardware SPI failed to initialize, reverting to software\n");
			spi_operating_mode = 0;
		}
		
	}
	
	is_initialized = 1;
}

/*---------------------------------------------------------------------------------------------*/


/* SPI_Read_Write:
		Preforms a SPI read write operation using bitbanging
			Params:
				data_out - The byte of data to be written
			Returns:
				uint8_t - The byte of data read 
*/
uint8_t SPI_Read_Write(uint8_t data_out){
	/* receive variable */
	uint8_t data_in = 0;
	
	/* Use appropriate method of data transmission */
	if(spi_operating_mode == 1 && *(uint32_t *)spi_output_mode == 1){
		data_in = SPI_Read_Write_Hardware(data_out);
	} else if (spi_operating_mode == 0 && *(uint32_t *)spi_output_mode == 0){
		data_in = SPI_Read_Write_BitBang(data_out);
	} else {
		printf("Invalid SPI Configuration\n");
	}
	
	return data_in;
}

/*---------------------------------------------------------------------------------------------*/



/* SPI_Read_Write_BitBang:
		Preforms a SPI read write operation using bitbanging
			Params:
				data_out - The byte of data to be written
			Returns:
				uint8_t - The byte of data read 
*/
uint8_t SPI_Read_Write_BitBang(uint8_t data_out){
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



/* SPI_Read_Write_Hardware:
		Preforms a SPI read write operation using hardware
			Params:
				data_out - The byte of data to be written
			Returns:
				uint8_t - The byte of data read 
*/
uint8_t SPI_Read_Write_Hardware(uint8_t data_out){
	/* receive variable */
	uint8_t data_in = 0;
	
	/* write the data over spi */
	*(uint32_t *)spi_write = data_out;
	
	/* wait for a return packet */
	while(*(uint32_t *)spi_status == 0){
		//Do nothing 
	}
	
	/* return the packet received */
	data_in = *(uint32_t *)spi_read;
	
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
		*(uint32_t *)spi_direct_output |= 0x00000001;
	}else{
		*(uint32_t *)spi_direct_output &= 0xFFFFFFFE;
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
		*(uint32_t *)spi_direct_output |= 0x00000008;
	}else{
		*(uint32_t *)spi_direct_output &= 0xFFFFFFF7;
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
		*(uint32_t *) spi_direct_output |= 0x00000002;
	} else{
		/* Ensure that clock line is high */
		SPI_Clock(1);
		
		/* set SS low */
		*(uint32_t *) spi_direct_output &= 0xFFFFFFFD;
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
	return (*(uint32_t *)spi_direct_output >> 2) & 1;
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