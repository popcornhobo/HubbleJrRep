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

/* Saturation Flags */
char saturated[3];

/* Function Definitions */
int control_system_update()
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
		qpos = quatNorm(qpos);
		qref = quatNorm(qref);
		qerr = quatMult(qpos, quatConj(qref));	// Calculate the quaternion error
		
		
		if (qerr.q0 <0)
		{
			qerr = quatConj(qerr);	// The quaternion error needs to be adjusted to represent the shortest path
		}
		//printf("Quaternion Error: Q0:%lf   Q1:%lf   Q2:%lf   Q3:%lf\n", qerr.q0, qerr.q1, qerr.q2, qerr.q3);
		
		/*
		* The three imaginary components i,j,k now represent the per-axis errors of the system
		*/ 
		xerr = qerr.q1; //!!!experimentally determine this order!!!
		yerr = qerr.q2;
		zerr = qerr.q3;
		
		// Set the error values to be sent via UDP to a laptop
		//*xerrOut = xerr;
		//*yerrOut = yerr;
		//*zerrOut = zerr;

		//printf("Error: Xerr:%lf   Yerr:%lf   Zerr:%lf\n\n", xerr, yerr, zerr);

		
		/* Update Servos
		 * X is yaw
		 * Y is pitch
		 * Z is roll
		 */
		 
		double error_vect[3];
		double rate_vect[3];
		
		error_vect[0] = yerr; //pitch
		error_vect[1] = -xerr;	//yaw !!!Make sure this is correct!!!
		error_vect[2] = zerr; //roll 
		
		rate_vect[0] = rates[3]; 
		rate_vect[1] = rates[1]; 
		rate_vect[2] = rates[2]; 
		 
		pid_loop(error_vect, rate_vect, time_step_secs);
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
	
	printf("Hey im initialized\n");
	
	void *virtual_base;
	int fd;	
	
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}

	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );
	
	/* Pitch Servo  */
	pitch_servo_set_rate    = virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_SET_RATE_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	pitch_servo_set_pos 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_SET_POS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	pitch_servo_set_ccw 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_SET_CCW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	pitch_servo_set_cw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_SET_CW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	pitch_servo_get_error 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_GET_ERROR_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	pitch_servo_get_status 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_GET_STATUS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	pitch_servo_reset 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + PITCH_SERVO_RESET_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	
	/* Roll Servo  */
	roll_servo_set_rate     = virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_SET_RATE_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	roll_servo_set_pos 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_SET_POS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	roll_servo_set_ccw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_SET_CCW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	roll_servo_set_cw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_SET_CW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	roll_servo_get_error 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_GET_ERROR_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	roll_servo_get_status 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_GET_STATUS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	roll_servo_reset 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + ROLL_SERVO_RESET_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	
	/* Yaw Servo  */
	yaw_servo_set_rate     	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_SET_RATE_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	yaw_servo_set_pos 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_SET_POS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	yaw_servo_set_ccw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_SET_CCW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	yaw_servo_set_cw 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_SET_CW_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	yaw_servo_get_error 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_GET_ERROR_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	yaw_servo_get_status 	= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_GET_STATUS_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	yaw_servo_reset 		= virtual_base + ((unsigned long )( ALT_LWFPGASLVS_OFST + YAW_SERVO_RESET_ADDR ) & ( unsigned long)( HW_REGS_MASK ));
	
	
	
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
	
	/* INITIALIZE SPI */
	SPI_Init(virtual_base, 0, 5);
	
	/* SETUP SERVOS */
	
	#ifdef USE_PITCH_JOINT_MODE
		usleep(2000);
		*(uint32_t *)pitch_servo_set_ccw = PITCH_CCW_LIMIT;
		usleep(2000);
		printf("Pitch CCW Error: %d\n", *(uint32_t *)pitch_servo_get_error);
		
		
		*(uint32_t *)pitch_servo_set_cw = PITCH_CW_LIMIT;
		usleep(2000);
		printf("Pitch CW Error: %d\n", *(uint32_t *)pitch_servo_get_error);
		
		//*(uint32_t *)pitch_servo_set_rate =  RESET_SPEED;
		//*(uint32_t *)pitch_servo_set_pos = 512;
	#else
		usleep(2000);
		*(uint32_t *)pitch_servo_set_ccw = 0;
		usleep(2000);
		*(uint32_t *)pitch_servo_set_cw = 0;
		usleep(2000);
	#endif
	
	
	#ifdef USE_ROLL_JOINT_MODE
		usleep(2000);
		*(uint32_t *)roll_servo_set_ccw = ROLL_CCW_LIMIT;
		usleep(2000);
		*(uint32_t *)roll_servo_set_cw = ROLL_CW_LIMIT;
		usleep(2000);
		//*(uint32_t *)roll_servo_set_rate =  RESET_SPEED;
		//*(uint32_t *)roll_servo_set_pos = 512;
	#else
		usleep(2000);
		*(uint32_t *)roll_servo_set_ccw = 0;
		usleep(2000);
		*(uint32_t *)roll_servo_set_cw = 0;
		usleep(2000);
	#endif
	
	isInitialized = True;
	return 0;
}
 

void set_as_current_position()
{
	// reference = Get_Position_From_VN-100
	float q[4];
	float rates[3];
	VN100_SPI_Packet * ReturnPacket = VN100_SPI_GetQuatRates(sensorID, q, rates);	// Retrieve the current VN-100 orientation
	//printf("Return:%u \n", *(uint8_t*)ReturnPacket);
	reference.q0 = q[0];	// Updated the reference quaternion to reflect the current VN-100 position
	reference.q1 = q[1];
	reference.q2 = q[2];
	reference.q3 = q[3];
	
	reference = quatNorm(reference);
	
	
	//printf("Position from VN100 q0:%f   q1:%f   q2:%f   q3:%f\n", q[0], q[1], q[2], q[3]);
	
}

void rotate_current_position(float pitch, float yaw, float roll)
{
	float pitch_rad = (pitch * PI)/180.00;	// Convert incoming angles to radians
	float roll_rad = (roll * PI)/180.00;
	float yaw_rad = (yaw * PI)/180.00;
	
	printf("Rotating with inputs: pitch:%f yaw:%f roll:%f\n", pitch, yaw, roll);
	printf("Original Quat: %f %f %f %f\n", reference.q0, reference.q1, reference.q2, reference.q3);
	
	quaternion qrot_roll  	= {.q0 = cos(pitch_rad/2.0), .q1 = 0, .q2 = 0, .q3 = sin(pitch_rad/2.0)};
	
	quaternion qrot_yaw 	= {.q0 = cos(yaw_rad/2.0), .q1 = sin(yaw_rad/2.0), .q2 = 0, .q3 = 0};
	
	quaternion qrot_pitch 	= {.q0 = cos(roll_rad/2.0), .q1 = 0, .q2 = sin(roll_rad/2.0), .q3 = 0};
	
	printf("Quat Pitch: %f %f %f %f\n", qrot_pitch.q0, qrot_pitch.q1, qrot_pitch.q2, qrot_pitch.q3);
	printf("Quat Yaw: %f %f %f %f\n", qrot_yaw.q0, qrot_yaw.q1, qrot_yaw.q2, qrot_yaw.q3);
	printf("Quat Roll: %f %f %f %f\n", qrot_roll .q0, qrot_roll .q1, qrot_roll .q2, qrot_roll .q3);

	/*
	*	Calculate the per axis scalar componenets that will make up the rotation quaternion
	*//* 
	float c1 = cos(yaw_rad/2.0);
	float c2 = cos(pitch_rad/2.0);
	float c3 = cos(roll_rad/2.0);
	
	float s1 = sin(yaw_rad/2.0);
	float s2 = sin(pitch_rad/2.0);
	float s3 = sin(roll_rad/2.0); */

	/*
	* Calculate the quaternion values that correspond to the given relative euler angle rotations
	*/
	/* float res_q0 = sqrt(1.0 + c1 *c2 + c1 * c3 - s1 * s2 *s3 + c2 * c3)/2.0;
	float res_q1 = (c2 * s3 + c1 * s3 + s1 * s2 * c3)/(4.0 * res_q0);
	float res_q2 = (s1 * c2 + s1 * c3 + c1 * s2 * s3)/(4.0 * res_q0);
	float res_q3 = (-s1 * s3 + c1 * s2 * c3 + s2)/(4.0* res_q0); */

	quaternion qrot_pitch_conj = quatConj(qrot_pitch);
	quaternion qrot_roll_conj = quatConj(qrot_roll);
	quaternion qrot_yaw_conj = quatConj(qrot_yaw);
	
	quaternion qrot_res = quatMult(qrot_pitch, qrot_yaw);
	qrot_res = quatMult(qrot_roll,qrot_res);// Create a quaternion object from the calculated pieces
	
	quaternion qrot_res_conj = quatMult(qrot_pitch_conj, qrot_yaw_conj);
	qrot_res_conj = quatMult(qrot_roll_conj, qrot_res_conj);// Create a quaternion object from the calculated pieces
	printf("Quat Rot_Res_conj1: %f %f %f %f\n", qrot_res_conj.q0, qrot_res_conj.q1, qrot_res_conj.q2, qrot_res_conj.q3);
	printf("Quat Rot_Res: %f %f %f %f\n", qrot_res.q0, qrot_res.q1, qrot_res.q2, qrot_res.q3);
	
	qrot_res_conj = quatConj(qrot_res);	// Create a quaternion object that is the conjugate of the rotation quat
	printf("Quat Rot_Res_conj: %f %f %f %f\n", qrot_res_conj.q0, qrot_res_conj.q1, qrot_res_conj.q2, qrot_res_conj.q3);
	
	quaternion firstMult = quatMult(qrot_res,reference);		// Multiply the rotation quaternion by the reference position
	quaternion secondMult = quatMult(firstMult, qrot_res_conj);	// Then multiple the result of first mulitply by the conjugate of the rotation quaternion

	reference = quatNorm(secondMult);	// The new rotated reference is now assigned to the global reference variable
	
	printf("New Quat: %f %f %f %f\n", reference.q0, reference.q1, reference.q2, reference.q3);
}

void update_gains(float P_pitch, float P_yaw, float P_roll, float I_pitch, float I_yaw, float I_roll,  float D_pitch, float D_yaw, float D_roll)
{
		P[0] = P_pitch;
		P[1] = P_yaw;
		P[2] = P_roll;
		
		I[0] = I_pitch;
		I[1] = I_yaw;
		I[2] = I_roll;
		
		D[0] = D_pitch;
		D[1] = D_yaw;
		D[2] = D_roll;
}

void pid_loop(double error[], double rates[], float time_step)
{
	static float integral_error[3] = {0, 0, 0};
	static float derivative_error[3] = {0, 0, 0};

	float servo_output[3];
	double last_error[3] = {0,0,0};
	
	int axis;
	for(axis= 0; axis< 3; axis++){
		if(I[axis] == 0.0){
			integral_error[axis] = 0;
		}else if(saturated[axis]){
			//Do nothing 
		}else{
			integral_error[axis] += error[axis] * time_step;
		}
		derivative_error[axis] =  (error[axis] - last_error[axis])/time_step;
	
		servo_output[axis] = P[axis] * error[axis] + I[axis] * integral_error[axis] - D[axis] * derivative_error[axis];
		
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
	
	#ifdef USE_PITCH_JOINT_MODE
		if(P[0] != 0){ //Only update for a non zero gain
			/* Update Pitch */
			if (Pitch < 0)
			{
				*(uint32_t *) pitch_servo_set_pos = PITCH_CW_LIMIT;
			}
			else
			{
				*(uint32_t *) pitch_servo_set_pos = PITCH_CCW_LIMIT;
			}
			
			/* Abs Roll */
			Pitch = abs(Pitch);
			
			/* Check saturation limits*/
			if(Pitch > PITCH_SATURATION){
				Pitch = PITCH_SATURATION;
			}
			
			*(uint32_t *) pitch_servo_set_rate = (int)floor(Roll);
		} else {
			*(uint32_t *)pitch_servo_set_rate = RESET_SPEED;
			*(uint32_t *)pitch_servo_set_pos = 512;
		}
	#else
		/* Update Pitch */
		saturated[0] = 0;
		
		if (Pitch < 0) 
		{
			Pitch = abs(Pitch) + 1024; // 1024 is where negative values start for the motor

			if(Pitch > (PITCH_SATURATION + 1024))
			{
				Pitch = PITCH_SATURATION + 1024;
				saturated[0] = 1;
			}
		}
		else
		{
			if(Pitch > PITCH_SATURATION)
			{
				Pitch = PITCH_SATURATION;
				saturated[0] = 1;
			}
		}
		
		*(uint32_t *) pitch_servo_set_rate	= (int)floor(Pitch);
	#endif
	
	
	/* Update Yaw */
	saturated[1] = 0;
	
	if (Yaw < 0)
	{
		Yaw = abs(Yaw) + 1024; // 1024 is where negative values start for the servo
		if(Yaw > (YAW_SATURATION + 1024))
		{
			Yaw = YAW_SATURATION + 1024;
			saturated[1] = 1;
		}
	}
	else
	{
		if(Yaw > YAW_SATURATION)
		{
			Yaw = YAW_SATURATION;
			saturated[1] = 1;
		}
	}
	
	*(uint32_t *) yaw_servo_set_rate = (int)floor(Yaw);
	
	
	#ifdef USE_ROLL_JOINT_MODE
		if(P[2] != 0){ //Only update for a non zero gain
			/* Update Roll */
			if (Roll < 0)
			{
				*(uint32_t *) roll_servo_set_pos = ROLL_CW_LIMIT;
			}
			else
			{
				*(uint32_t *) roll_servo_set_pos = ROLL_CCW_LIMIT;
			}
			
			/* Abs Roll */
			Roll = abs(Roll);
			
			/* Check saturation limits*/
			if(Roll > ROLL_SATURATION){
				Roll = ROLL_SATURATION;
			}
			
			*(uint32_t *) roll_servo_set_rate = (int)floor(Roll);
		} else { //Reset for a zero gain
			*(uint32_t *)roll_servo_set_rate = RESET_SPEED;
			*(uint32_t *)roll_servo_set_pos = 512;
		}
	#else 
		/* Update Roll */
		saturated[2] = 0;
		
		if (Roll < 0)
		{
			Roll = abs(Roll) + 1024; // 1024 is where negative values start for the servo
			if(Roll> (ROLL_SATURATION + 1024))
			{
				Roll = ROLL_SATURATION + 1024;
				saturated[2] = 1;
			}
		}
		else
		{
			if(Roll > ROLL_SATURATION)
			{
				Roll = ROLL_SATURATION;
				saturated[2] = 1;
			}
		}
		
		*(uint32_t *) roll_servo_set_rate = (int)floor(Roll);
	#endif
	
	//uint32_t pre_status = (*(uint32_t *) Servo_Status_Yaw);
	
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
	
	//printf("Hey im alive");
    
}
