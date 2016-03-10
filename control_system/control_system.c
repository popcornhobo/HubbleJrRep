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
double P[3] = {0,0,0};
double I[3]  = {0,0,0};
double D[3] = {0,0,0};

quaternion reference;
static int isInitialized = False;

/* pitch servo */
void* pitch_servo_set_rate;
void* pitch_servo_set_pos;
void* pitch_servo_set_ccw;
void* pitch_servo_set_cw;

void* pitch_servo_get_error;
void* pitch_servo_get_status;

void* pitch_servo_reset;


/* roll servo */
void* roll_servo_set_rate;
void* roll_servo_set_pos;
void* roll_servo_set_ccw;
void* roll_servo_set_cw;

void* roll_servo_get_error;
void* roll_servo_get_status;

void* roll_servo_reset;


/* yaw servo */
void* yaw_servo_set_rate;
void* yaw_servo_set_pos;
void* yaw_servo_set_ccw;
void* yaw_servo_set_cw;

void* yaw_servo_get_error;
void* yaw_servo_get_status;

void* yaw_servo_reset;


/* Function Definitions */
int control_system_update(float *xerrOut, float *yerrOut, float *zerrOut)
{
	/* update time_step */
	static unsigned long int prev_time = 0;
	static unsigned long int curr_time = 0;
	float time_step = 0;
	struct timeval current_time;
	
	gettimeofday(&current_time, NULL); 
	curr_time = current_time.tv_sec * 1000000 + current_time.tv_usec;
	
	if(prev_time == 0){
		time_step = 0;
	} else{
		time_step = curr_time - prev_time;
	}
	
	prev_time = curr_time;
	float time_step_secs = (float) time_step / (float) 1000000;
	
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

		// Set the error values to be sent via UDP to a laptop
		*xerrOut = xerr;
		*yerrOut = yerr;
		*zerrOut = zerr;

		//printf("Error: Xerr:%lf   Yerr:%lf   Zerr:%lf\n", xerr, yerr, zerr);

		
		/* Update Servos
		 * X is yaw
		 * Y is roll
		 * Z is pitch */
		 
		double error_vect[3];
		
		error_vect[0] = zerr;
		error_vect[1] = xerr;
		error_vect[2] = yerr;
		 
		pid_loop(error_vect, time_step_secs);
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
	
	/* Pitch Servo  */
	pitch_servo_set_rate     	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_SET_RATE_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	pitch_servo_set_pos 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_SET_POS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	pitch_servo_set_ccw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_SET_CCW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	pitch_servo_set_cw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_SET_CW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	pitch_servo_get_error 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_GET_ERROR_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	pitch_servo_get_status 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_GET_STATUS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	pitch_servo_reset 			= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_RESET_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	
	/* Roll Servo  */
	roll_servo_set_rate     	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_SET_RATE_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	roll_servo_set_pos 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_SET_POS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	roll_servo_set_ccw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_SET_CCW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	roll_servo_set_cw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_SET_CW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	roll_servo_get_error 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_GET_ERROR_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	roll_servo_get_status 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_GET_STATUS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	roll_servo_reset 			= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_RESET_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	
	/* Yaw Servo  */
	yaw_servo_set_rate     	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_SET_RATE_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	yaw_servo_set_pos 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_SET_POS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	yaw_servo_set_ccw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_SET_CCW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	yaw_servo_set_cw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_SET_CW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	yaw_servo_get_error 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_GET_ERROR_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	yaw_servo_get_status 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_GET_STATUS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	yaw_servo_reset 			= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_RESET_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	
	
	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}
	//----------------------------------------------------------------------//
	
	/* Reset Servos */
	
	//printf("Servo Status Register Yaw: %u\n", (*(uint32_t *) Servo_Status_Yaw));
	//Servo_Reset_Yaw = 0xAA;
	//printf( "Reset : %u\n", Servo_Reset_Yaw);
	//printf("Servo Status Register Yaw: %u\n", (*(uint32_t *) Servo_Status_Yaw));
	
	//printf("THIS IS NEW CODE NUMBER 1\n");
	
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
	VN100_SPI_Packet * ReturnPacket = VN100_SPI_GetQuatRates(sensorID, q, rates);
	//printf("Return:%u \n", *(uint8_t*)ReturnPacket);
	reference.q0 = q[0];
	reference.q1 = q[1];
	reference.q2 = q[2];
	reference.q3 = q[3];
	
	//printf("Position from VN100 q0:%f   q1:%f   q2:%f   q3:%f\n", q[0], q[1], q[2], q[3]);
	
}

void rotate_current_position(float pitch, float yaw, float roll)
{

}

void update_gains(float new_P[], float new_I[], float new_D[])
{
	int i;
	for(i = 0; i < 3; i++){
		P[i] = (double) new_P[i];
		I[i] = (double) new_I[i];
		D[i] = (double) new_D[i];
	}
}

void pid_loop(double error[], float time_step)
{
	static float integral_error[3] = {0, 0, 0};
	static float derivative_error[3] = {0, 0, 0};

	float servo_output[3];
	double last_error[3] = {0,0,0};
	
	int axis;
	for(axis= 0; axis< 3; axis++){
		integral_error[axis] += error[axis] * time_step;
		derivative_error[axis] =  (error[axis] - last_error[axis])/time_step;
	
		servo_output[axis] = P[axis] * error[axis] + I[axis] * integral_error[axis] + D[axis] * derivative_error[axis];
		
		last_error[axis] = error[axis];
	}
	
	update_servos(servo_output[0], servo_output[1], servo_output[2]);
}



/* Local Functions */

/* update_servos
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

	//uint32_t pre_status = (*(uint32_t *) Servo_Status_Yaw);
	
	/* Send the servo commands */	
	*(uint32_t *) pitch_servo_set_rate	= (int)floor(Pitch);
	
	*(uint32_t *) yaw_servo_set_rate 	= (int)floor(Yaw);
	
	*(uint32_t *) roll_servo_set_rate		= (int)floor(Roll);
	
//	printf("Servo Rate Value: %u\n", floor(Yaw));
	if(counter >= 5){
		//printf("Servo Rate Register Pitch: %u\n", (*(uint32_t *) Servo_Set_Pitch));
		//printf("Servo Status Register Yaw: %u\n", pre_status);
		//printf("Servo Rate Register Yaw: %u\n", (*(uint32_t *) Servo_Set_Yaw));
		//printf("Servo Status Register Yaw: %u\n", (*(uint32_t *) Servo_Status_Yaw));
		//printf("Servo Rate Register Yaw: %u\n", (*(uint32_t *) Servo_Set_Roll));
		counter = 0;
	} else{
		counter++;
	}
    
}
