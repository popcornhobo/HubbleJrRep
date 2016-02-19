/* quaternion_math.c
 * 
 * Source file for c functions that compute useful quaternion operations.
 *
 * Authors:	Seth Kreitinger
 * 
 *   
 * Revions:
 * 2016-02-12	v0.1	Initial Revison
 * 2016-02-12	v0.9    Untested Code Complete
 *
 * */
 
#include "quaternion_math.h"

quaternion quatMult(p,q)
{
	double crossProd_q1 = p.q2*q.q3 - p.q3*q.q2;
	double crossProd_q2 = p.q3*q.q1 - p.q1*q.q3;
	double crossProd_q3 = p.q1*q.q2 - p.q2*q.q1;

	double p0_q1 = p.q0 * q.q1;
	double p0_q2 = p.q0 * q.q2;
	double p0_q3 = p.q0 * q.q3;

	double q0_p1 = q.q0 * p.q1;
	double q0_p2 = q.q0 * p.q2;
	double q0_p3 = q.q0 * p.q3;

	double dotProd = p.q0*q.q0 + p.q1*q.q1 + p.q2*q.q2 + p.q3*q.q3;

	quaternion result;
	result.q0 = p.q0*q.q0 - dotProd;
	result.q1 = p0_q1 + q0_p1 + crossProd_q1;
	result.q2 = p0_q2 + q0_p2 + crossProd_q2;
	result.q3 = p0_q3 + q0_p3 + crossProd_q3;

	return result;
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
