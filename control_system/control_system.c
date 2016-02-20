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

/* Function Definitions */
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
		
		// The three imaginary components now represent the per-axis errors of the system
		xerr = qerr.q1*qerr.q0;
		yerr = qerr.q2*qerr.q0;
		zerr = qerr.q3*qerr.q0;

		/* Apply the gains */
		xerr *= P;
		yerr *= P;
		zerr *= P;
		/* Update Servos
		 * X is pitch
		 * Y is roll
		 * Z is yaw */
		update_servos(xerr, zerr, yerr);
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

int control_system_init()
{
	//----------------------------------------------------------------------//
	//	INITIALIZE PORT REGISTERS	//
	
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
	Servo_Error_Yaw	    	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + SERVO_ERROR_ADDR_YAW) & ( unsigned long)( HW_REGS_MASK ));
	
	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}
	//----------------------------------------------------------------------//
	
	//	ADDITIONAL MEMORY MAPPING	//
	spi_init(virtual_base);
	isInitialized = True;
	return 0;
}


void set_as_current_position()
{
	// reference = Get_Position_From_VN-100
	float q[4];
	float rates[3];
	VN100_SPI_GetQuatRates(sensorID, q, rates);
	reference.q0 = q[0];
	reference.q1 = q[1];
	reference.q2 = q[2];
	reference.q3 = q[3];
	
}

void rotate_current_position(float pitch, float yaw, float roll)
{

}

void update_gains( float new_P, float new_I, float new_D)
{
	P = new_P;
	I = new_I;
	D = new_D;
}



/* Local Functions */

/* update_servos
 * X is pitch
 * Y is Roll
 * Z is Yaw
*/
void update_servos(double Pitch, double Yaw, double Roll)
{
	/* Calculate */	
	/* First determine if value is positive or negatve
	 * then load the value into the servo
	*/
	/* Scale Error to match motor */
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
		if(Pitch > 1023)
		{
			Pitch = 1023;
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
		if(Pitch > 1023)
		{
			Pitch = 1023;
		}
	}

	/* Send the servo commands */	
	*(uint32_t *) Servo_Set_Pitch= floor(Pitch);
	*(uint32_t *) Servo_Set_Yaw= floor(Yaw);
	*(uint32_t *) Servo_Set_Roll= floor(Roll);
}
