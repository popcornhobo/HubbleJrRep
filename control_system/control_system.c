/* control_system.c
 * 
 * Source file for c functions relating to the control system
 *
 * Authors:	Thomas Rader, Seth Kreitinger, Zach Frazee
 * 
 *
 * Revions:  
 * 2016-02-11	v0.1	Initial Revison 
 * 2016-02-12	v0.2    Added Quaternion Error Calculation
 * 2016-02-19	v0.3	Moved includes to .h
 * 			Moved defines to .c
 * 			Added set position commands
 * 2016-03-07	v0.4	Added function descriptions	
 * */

#include "control_system.h"

/* Local Function Prototypes */
void update_servos(double Pitch, double Yaw, double Roll);

/* Global User Variables */
unsigned char sensorID = 0;
double P, I, D =0.0;
quaternion reference;
static int isInitialized = False;
void* Servo_Set_Pitch;
void* Servo_Error_Pitch;
void* Servo_Set_Roll;
void* Servo_Error_Roll;
void* Servo_Set_Yaw;
void* Servo_Error_Yaw;
void* Servo_Status_Yaw;
void* Servo_Reset_Yaw;

/* Function Definitions */

/* control_system_update
 * Description: This function is called by the python wrapper to initate an update
 * 	Input Parameters:
 * 		none
 *
 * 	Returns:
 * 		int - status of whether the update completed successfully
 */
int control_system_update()
{
	/*
	* Get current position from VN-100
	* Calculate Error with Seth's mathsauce (the math sauce king)
	* Multiply gain by error
	* Command Servos
	*/
	
	if(isInitialized)
	{
		double xerr,yerr,zerr;
		float q[4];
		float rates[3];
		quaternion qerr;

		VN100_SPI_GetQuatRates(sensorID, q, rates);
		quaternion qpos = {.q0 = q[0], .q1 = q[1], .q2 = q[2], .q3 = q[3]};
		quaternion qref = {.q0 = reference.q0, .q1 = reference.q1, .q2 = reference.q2, .q3 = reference.q3};

		/*
		*	Normalize position and reference Quaternion before calculating the Quaternion
		*	error.
		*/
		// Normalize the quaternions
		qpos = quatNorm(qpos);
		qref = quatNorm(qref);
		// Calculate the quaternion error
		qerr = quatMult(qpos, quatConj(qref));
		// The quaternion error needs to be adjusted to represent the shortest path
		
		//printf("Quaternion Error: Q0:%lf   Q1:%lf   Q2:%lf   Q3:%lf\n", qerr.q0, qerr.q1, qerr.q2, qerr.q3);
		
		// The three imaginary components now represent the per-axis errors of the system
		xerr = qerr.q1*qerr.q0;
		yerr = qerr.q2*qerr.q0;
		zerr = qerr.q3*qerr.q0;
		
		//printf("Error: Xerr:%lf   Yerr:%lf   Zerr:%lf\n", xerr, yerr, zerr);

		/* Apply the gains */
		xerr *= P;
		yerr *= 10*P;
		zerr *= 10*P;
		/* Update Servos
		 * X is yaw
		 * Y is roll
		 * Z is pitch */
		update_servos(zerr, xerr, yerr);
		/*------------------------------------------------------------------------------*/
	}
	else
	{
		if(control_system_init())
		{
			return -1;
		}
	}
	return 0;
}
/*------------------------------ END control_system_update ------------------------------*/
/* control_system_init
 * Description: Initializes the memerymaping of registers
 * 	Input Parameters:
 * 		none
 *
 * 	Returns:
 * 		int - status of successful initializaion
 *
*/
int control_system_init()
{
	/* INITIALIZE PORT REGISTERS */	
	
	void *virtual_base;
	int fd;	
	
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );
	
	Servo_Set_Pitch	    	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + SET_RATE_ADDR_PITCH ) & ( unsigned long)( HW_REGS_MASK ));
	Servo_Error_Pitch	    = virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + SERVO_ERROR_ADDR_PITCH ) & ( unsigned long)( HW_REGS_MASK ));
	Servo_Set_Roll	    	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + SET_RATE_ADDR_ROLL ) & ( unsigned long)( HW_REGS_MASK ));
	Servo_Error_Roll	    = virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + SERVO_ERROR_ADDR_ROLL) & ( unsigned long)( HW_REGS_MASK ));
	
	Servo_Set_Yaw	    	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + SET_RATE_ADDR_YAW ) & ( unsigned long)( HW_REGS_MASK ));
	Servo_Error_Yaw	   	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + SERVO_ERROR_ADDR_YAW) & ( unsigned long)( HW_REGS_MASK ));
	Servo_Reset_Yaw		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + RESET_ADDR_YAW) & ( unsigned long)( HW_REGS_MASK ));
	Servo_Status_Yaw     = virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + STATUS_ADDR_YAW) & ( unsigned long)( HW_REGS_MASK ));
	
	
	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}
	
	/* Reset Servos */
	
	printf("Servo Status Register Yaw: %u\n", (*(uint32_t *) Servo_Status_Yaw));
	Servo_Reset_Yaw = 0xAA;
	printf( "Reset : %u\n", Servo_Reset_Yaw);
	printf("Servo Status Register Yaw: %u\n", (*(uint32_t *) Servo_Status_Yaw));
	
	printf("THIS IS NEW CODE NUMBER 1\n");
	
	/* ADDITIONAL MEMORY MAPPING */	
	spi_init(virtual_base);
	isInitialized = True;
	return 0;
}
/*------------------------------ END control_system_init------------------------------*/ 

/* set_as_current_position
 * Description: sets the reference position to be the position currently reported by the VN-100
 * 	Input Parameters:
 * 		none
 *
 * 	Returns:
 * 		none
*/
void set_as_current_position()
{
	// reference = Get_Position_From_VN-100
	float q[4];
	float rates[3];
	VN100_SPI_Packet * ReturnPacket = VN100_SPI_GetQuatRates(sensorID, q, rates);
	//printf("Return:%u \n", *(uint8_t*)ReturnPacket);
	reference.q0 = q[0];
	reference.q1 = q[1];
	reference.q2 = q[2];
	reference.q3 = q[3];
	
	//printf("Position from VN100 q0:%f   q1:%f   q2:%f   q3:%f\n", q[0], q[1], q[2], q[3]);
	
}
/*------------------------------ END set_as_current_position ------------------------------*/

/* rotate_current_position
 * Description: rotates the reference position by the given values
 * 	Input Parameters
 * 		float pitch - desired pitch change
 * 		float yaw - desired yaw change
 * 		float roll - desired roll change
 *
 * 	Returns:
 * 		none
 *
*/
void rotate_current_position(float pitch, float yaw, float roll)
{

}
/*------------------------------ END rotate_current_position ------------------------------*/

/* update_gains
 * Description: Updates the gains for the PID controller with the given parameters
 * 	Input Parameters:
 * 		float new_P - the new proportional gain
 * 		float new_I - the new integral gain
 *		flaot new_D - the new derivative gain
 *
 *	Returns:
 *		none
*/
void update_gains( float new_P, float new_I, float new_D)
{
	P = new_P;
	I = new_I;
	D = new_D;
	
	//printf("New Gains P:%f   I:%f   D:%f\n", P, I, D);
}
/*------------------------------ END update_gains ------------------------------*/


/* Local Functions */

/* update_servos
 * Description: Update the servos to a given rate
 * 	Input Parameters: 
 * 		double Pitch - rate for the Pitch Servo
 * 		double Yaw - rate for the Yaw servo
 * 		double Roll - rate for the Roll servo
 *
 * 	Returns:
 * 		none
*/
void update_servos(double Pitch, double Yaw, double Roll)
{
	/* Calculate */	
	/* First determine if value is positive or negatve
	 * then load the value into the servo
	*/

	/* Scale Error to match motor */
	static int counter = 0;
	
	Pitch *= 1023;
	Yaw   *= 1023;
	Roll  *= 1023;
	/* Update Pitch */
	if (Pitch < 0) 
	{
		Pitch = abs(Pitch) + 1024; // 1024 is where negative values start for the motor

		if(Pitch > 2047)
		{
			Pitch = 2047;
		}
	}
	else
	{
		if(Pitch > 1023)
		{
			Pitch = 1023;
		}
	}

	/* Update Yaw */
	if (Yaw < 0)
	{
		Yaw = abs(Yaw) + 1024; // 1024 is where negative values start for the servo
		if(Yaw > 2047)
		{
			Yaw = 2047;
		}
	}
	else
	{
		if(Yaw > 1023)
		{
			Yaw = 1023;
		}
	}
	
	/* Update Roll */
	if (Roll < 0)
	{
		Roll = abs(Roll) + 1024; // 1024 is where negative values start for the servo
		if(Roll> 2047)
		{
			Roll = 2047;
		}
	}
	else
	{
		if(Roll > 1023)
		{
			Roll = 1023;
		}
	}

	uint32_t pre_status = (*(uint32_t *) Servo_Status_Yaw);
	
	/* Send the servo commands */	
	*(uint32_t *) Servo_Set_Pitch= (int)floor(Pitch);
	
	*(uint32_t *) Servo_Set_Yaw = (int)floor(Yaw);
	
	*(uint32_t *) Servo_Set_Roll = (int)floor(Roll);
	
//	printf("Servo Rate Value: %u\n", floor(Yaw));
	if(counter >= 5){
		//printf("Servo Rate Register Pitch: %u\n", (*(uint32_t *) Servo_Set_Pitch));
		printf("Servo Status Register Yaw: %u\n", pre_status);
		printf("Servo Rate Register Yaw: %u\n", (*(uint32_t *) Servo_Set_Yaw));
		printf("Servo Status Register Yaw: %u\n", (*(uint32_t *) Servo_Status_Yaw));
		//printf("Servo Rate Register Yaw: %u\n", (*(uint32_t *) Servo_Set_Roll));
		counter = 0;
	} else{
		counter++;
	}
    
}
/*------------------------------ END update_servos ------------------------------*/
