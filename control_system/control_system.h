/* control_system.h
 *
 * Header file for control_system.c
 *
 * Authors:	Thomas Rader
 *
 *
 * Revisions:
 * 2016-02-11	v0.1	File Creation
 *
 * */ 

#ifndef __CONTROL_SYSTEM_H__
#define __CONTROL_SYSTEM_H__


/* Function Prototypes */
double control_system_update(double q0, double q1, double q2, double q3);

int control_system_init();

void update_servos(int Pitch, int Yaw, int Roll);

#endif // __CONTROL_SYSTEM_H__
