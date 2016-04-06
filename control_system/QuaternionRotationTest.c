#include "quaternion_math.h"
#include <stdio.h>
#define PI 3.14159

void rotate_current_position(float pitch, float yaw, float roll);


quaternion reference = {.q0 = 0, .q1 = 0, .q2 = 0.7071, .q3 = 0.7071};
int main()
{
	int x = 90; //yaw
	int y = 0; //pitch
	int z = 0; //roll
	rotate_current_position(y,x,z);
	printf("---------------------------------------------------\n");
	x = 270; //yaw
	y = 0; //pitch
	z = 0; //roll
	rotate_current_position(y,x,z);
	return 1;
}

void rotate_current_position(float pitch, float yaw, float roll)
{
	float pitch_rad = (pitch * PI)/180.00;	// Convert incoming angles to radians
	float roll_rad = (roll * PI)/180.00;
	float yaw_rad = (yaw * PI)/180.00;
	
	printf("Rotating with inputs: pitch:%f yaw:%f roll:%f\n", pitch, yaw, roll);
	printf("Original Quat: %f %f %f %f\n", reference.q0, reference.q1, reference.q2, reference.q3);
	
	quaternion qrot_roll  	= {.q0 = cos(pitch_rad/2.0), .q1 = 0, .q2 = 0, .q3 = sin(roll_rad/2.0)};
	
	quaternion qrot_yaw 	= {.q0 = cos(yaw_rad/2.0), .q1 = sin(yaw_rad/2.0), .q2 = 0, .q3 = 0};
	
	quaternion qrot_pitch 	= {.q0 = cos(roll_rad/2.0), .q1 = 0, .q2 = sin(pitch_rad/2.0), .q3 = 0};
	
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
	
	printf("New Quat w/ conj: %f %f %f %f\n", reference.q0, reference.q1, reference.q2, reference.q3);
	printf("New Quat w/o conj: %f %f %f %f\n", firstMult.q0, firstMult.q1, firstMult.q2, firstMult.q3);
}
