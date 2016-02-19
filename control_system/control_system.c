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

/* Function Definitions */
double control_system_update(double q0, double q1, double q2, double q3)
{
	/*
	* Get current position from VN-100
	* Calculate Error with Seth's mathsauce (the math sauce king)
	* Multiply gain by error
	* Command Servos
	*/
	double xerr,yerr,zerr;
	quaternion qpos;
	quaternion qerr;
	quaternion qref = {.q0 = q0, .q1 = q1, .q2 = q2, .q3 = q3};

	/*
	*	Normalize position and reference Quaternion before calculating the Quaternion
	*	error.
	*/
	qerr = quatNorm(qerr);
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

	double quaternionSum = q0+q1+q2+q3;	//Ensure data was received properly
	return quaternionSum;
}
