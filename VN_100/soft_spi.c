/* soft_spi.c
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
 
#include "soft_spi.h"

//Memory Mapped Registers
void * spi_pio_data;
void * spi_pio_datadir;

int fd;
void * virtual_base;

//Initialized Variable
char is_initialized = 0; 

void spi_init(void * virtual_base ){
	spi_pio_data = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SPI_ADDR_PIO_DATA ) & ( unsigned long)( HW_REGS_MASK ) );
	spi_pio_datadir = virtual_base + ( ( unsigned long  )( ALT_LWFPGASLVS_OFST + SPI_ADDR_PIO_DATADIR ) & ( unsigned long)( HW_REGS_MASK ) );
	
	//Bit order MSB = MOSI - MISO - SS - CLK  
	
	*(uint32_t *)spi_pio_datadir = 0b1011;
	*(uint32_t *)spi_pio_data = 0b1111;
	
	is_initialized = 1;
}

uint8_t SPI_Read_Write(uint8_t data_out){
	//receive variable
	uint8_t data_in = 0;
	
	//Byte loop 
	int i;
	for(i = 0; i < 8; i++){
		//pull clock low
		SPI_Clock(0);
		usleep(5);
		
		//Set MOSI
		SPI_MOSI((data_out >> (7 - i)) & 1);
		usleep(5);
		
		//Clock High
		SPI_Clock(1);
		usleep(5);
		
		//Read MISO
		data_in |= (SPI_MISO() << (7 - i));
		usleep(5);
	}
	
	return data_in;
}

void SPI_Clock(char state){
	if(state == 1){
		*(uint32_t *)spi_pio_data |= 0x00000001;
	}else{
		*(uint32_t *)spi_pio_data &= 0xFFFFFFFE;
	}
}

void SPI_MOSI(char state){
	if(state == 1){
		*(uint32_t *)spi_pio_data |= 0x00000008;
	}else{
		*(uint32_t *)spi_pio_data &= 0xFFFFFFF7;
	}
}

void SPI_SS(char state){
	if(state == 1){
		//Ensure that clock line is high
		SPI_Clock(1);
		
		//set SS high
		*(uint32_t *) spi_pio_data |= 0x00000002;
	} else{
		//Ensure that clock line is high
		SPI_Clock(1);
		
		//set SS low
		*(uint32_t *) spi_pio_data &= 0xFFFFFFFD;
	}
}

uint8_t SPI_MISO(){
	return (*(uint32_t *)spi_pio_data >> 2) & 1;
	
}