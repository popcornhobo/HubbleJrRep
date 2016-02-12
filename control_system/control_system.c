/* control_system.c
 * 
 * Source file for c functions relating to the control system
 *
 * Authors:	Thomas Rader
 * 
 *
 * Revions:
 * 2016-02-11	v0.1	Initial Revison
 *
 * */

#include"control_system.h"


/* Function Definitions */
double control_system_update(double q0, double q1, double q2, double q3)
{

	// Get current position from VN-100
	// Calculate Error with Seth's mathsauce (the math sauce king)
	// Multiply gain by error
	// Command Servos
	double quaternionSum = q0+q1+q2+q3;
	return quaternionSum;
}
