/* quaternion_math.c
 * 
 * Source file for c functions that compute useful quaternion operations.
 *
 * Authors:	Seth Kreitinger
 * 
 *
 * Revions:
 * 2016-02-12	v0.1	Initial Revison
 *
 * */

#include "quaternion_math.h"

quaternion quatMult(quaternion)
{

}

quaternion quatConj(quaternion)
{
	quaternion.q1 *= -1;
	quaternion.q2 *= -1;
	quaternion.q3 *= -1;

	return quaternion;
}

quaternion quatNorm(quaternion)
{
	double mag = sqrt(quaternion.q0*quaternion.q0 + quaternion.q1*quaternion.q1 + quaternion.q2*quaternion.q2 + quaternion.q3*quaternion.q3);
	quaternion.q0 /= mag;
	quaternion.q1 /= mag;
	quaternion.q2 /= mag;
	quaternion.q3 /= mag;
	return quaternion;
}