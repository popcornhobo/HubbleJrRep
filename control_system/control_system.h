/* control_system.h
 *
 * Header file for control_system.c
 *
 * Authors:	Thomas Rader
 *
 *
 * Revisions:
 * 2016-02-11	v0.1	File Creation
 * 2016-02-19	v0.2	Added Set functions
 * 			Moved includes to .h
 * */ 

#ifndef __CONTROL_SYSTEM_H__
#define __CONTROL_SYSTEM_H__

#include "quaternion_math.h"
#include "../VN_100/VN_lib.h"
#include "../VN_100/soft_spi.h"
#include <stdio.h>
#include <math.h>

/* User Defines */
#define True 1
#define False 0
#define SET_RATE_ADDR_PITCH             0x00000100
#define SERVO_ERROR_ADDR_PITCH  	0x0000010C
#define SET_RATE_ADDR_ROLL              0x00000180
#define SERVO_ERROR_ADDR_ROLL  	 	0x0000018C
#define SET_RATE_ADDR_YAW               0x00000200
#define SERVO_ERROR_ADDR_YAW    	0x0000020C


/* Function Prototypes */
int control_system_update();

int control_system_init();

void set_as_current_position();

void rotate_current_positon(float pitch, float yaw, float roll);

void update_gains(float new_P, float new_I, float new_D);

#endif // __CONTROL_SYSTEM_H__
