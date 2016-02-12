/* quaternion_math.h
 *
 * Header file for quaternion_math.c
 *
 * Authors:	Seth Kreitinger
 *
 *
 * Revisions:
 * 2016-02-12	v0.1	File Creation
 *
 * */

#ifndef _QUATERNION_MATH_H_
#define _QUATERNION_MATH_H_

#include <math.h>

typedef struct quaternion quaternion;
struct quaternion {
	double q0;
	double q1;
	double q2;
	double q3;
};

/* Function Prototypes */

quaternion quatMult(quaternion);

quaternion quatConj(quaternion);

quaternion quatNorm(quaternion);

#endif
