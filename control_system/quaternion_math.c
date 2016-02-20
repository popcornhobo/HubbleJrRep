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

quaternion quatMult(quaternion p, quaternion q)
{
	double crossProd_q1 = (double)p.q2*q.q3 -  (double)p.q3*q.q2;
	double crossProd_q2 = (double)p.q3*q.q1 -  (double)p.q1*q.q3;
	double crossProd_q3 = (double)p.q1*q.q2 -  (double)p.q2*q.q1;

	double p0_q1 =  (double)p.q0 * q.q1;
	double p0_q2 =  (double)p.q0 * q.q2;
	double p0_q3 =  (double)p.q0 * q.q3;

	double q0_p1 =  (double)q.q0 * p.q1;
	double q0_p2 =  (double)q.q0 * p.q2;
	double q0_p3 =  (double)q.q0 * p.q3;

	double dotProd =  (double)p.q0*q.q0 +  (double)p.q1*q.q1 +  (double)p.q2*q.q2 +  (double)p.q3*q.q3;

	quaternion result;
	result.q0 = (double)p.q0*q.q0 - dotProd;
	result.q1 = p0_q1 + q0_p1 + crossProd_q1;
	result.q2 = p0_q2 + q0_p2 + crossProd_q2;
	result.q3 = p0_q3 + q0_p3 + crossProd_q3;

	return result;
}

quaternion quatConj(quaternion quat)
{
	quat.q1 *= -1;
	quat.q2 *= -1;
	quat.q3 *= -1;

	return quat;
}

quaternion quatNorm(quaternion quat)
{
	double mag = sqrt((double)quat.q0*quat.q0 + (double)quat.q1*quat.q1 + (double)quat.q2*quat.q2 + (double)quat.q3*quat.q3);
	quat.q0 /= mag;
	quat.q1 /= mag;
	quat.q2 /= mag;
	quat.q3 /= mag;
	return quat;
}
