/* control_system.c
 * 
 * Source file for c functions relating to the control system
 *
 * Authors:	Thomas Rader, Seth Kreitinger
 * 
 *
 * Revions:  
 * 2016-02-11	v0.1	Initial Revison 
 * 2016-02-12	v0.2    Added Quaternion Error Calculation
 * */

#include "control_system.h"
#include "quaternion_math.h"
#include "..\\VN_100\\VN_lib.h"
#include "..\\VN_100\\soft_spi.h"


// User defines
#define True 1
#define False 0

#define SET_RATE_ADDR_PITCH		0x00000100
#define SERVO_ERROR_ADDR_PITCH 	0x0000010C

#define SET_RATE_ADDR_ROLL		0x00000180
#define SERVO_ERROR_ADDR_ROLL 	0x0000018C

#define SET_RATE_ADDR_YAW		0x00000200
#define SERVO_ERROR_ADDR_YAW 	0x0000020C

// User Global Variables
unsigned char sensorID = 0;
static int isInitialized = False;
void* Servo_Set_Pitch;
void* Servo_Error_Pitch;
void* Servo_Set_Roll;
void* Servo_Error_Roll;
void* Servo_Set_Yaw;
void* Servo_Error_Yaw;

/* Function Definitions */
double control_system_update(double q0, double q1, double q2, double q3)
{
	/*
	* Get current position from VN-100
	* Calculate Error with Seth's mathsauce (the math sauce king)
	* Multiply gain by error
	* Command Servos
	*/
	
	double quaternionSum = 0;
	
	if(isInitialized)
	{
		double xerr,yerr,zerr;
		float q[4];
		float rates[3];
		quaternion qerr;

		VN100_SPI_GetQuatRates(sensorID, q, rates);
		quaternion qpos = {.q0 = q[0], .q1 = q[1], .q2 = q[2], .q3 = q[3]};
		quaternion qref = {.q0 = q0, .q1 = q1, .q2 = q2, .q3 = q3};

		/*
		*	Normalize position and reference Quaternion before calculating the Quaternion
		*	error.
		*/
		// Normalize the quaternions
		qerr = quatNorm(qpos);
		qref = quatNorm(qref);
		// Calculate the quaternion error
		qerr = quatMult(qpos, quatConj(qref));
		// The quaternion error needs to be adjusted to represent the shortest path
		if(qerr.q0 < 0)
		{
			qerr = quatConj(qerr);
		}
		// The three imaginary components now represent the per-axis errors of the system
		xerr = qerr.q1;
		yerr = qerr.q2;
		zerr = qerr.q3;
		/*------------------------------------------------------------------------------*/
 
		quaternionSum = q0+q1+q2+q3;	//Ensure data was received properly	
	}
	else
	{
		if(control_system_init())
		{
			return -1;
		}
	}
	return quaternionSum;
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

void update_servos(int Pitch, int Yaw, int Roll)
{
	
}
