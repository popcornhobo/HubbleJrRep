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

#define soc_cv_av

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "hwlib.h"
#include "soc_cv_av/socal/socal.h"
#include "soc_cv_av/socal/hps.h"
#include "soc_cv_av/socal/alt_gpio.h"
#include "hps_0.h"

#include "quaternion_math.h"
#include "../VN_100/VN_lib.h"
#include "../VN_100/soft_spi.h"

#include <stdlib.h>
#include <math.h>

/* User Defines */
#define True 1
#define False 0

#define PI 	3.14159265354252		//Yummy

/* Pitch Servo 0x180*/ 
#define PITCH_SERVO_SET_RATE_ADDR		0x00000180
#define PITCH_SERVO_SET_POS_ADDR			(0x00000180 + (0x011 * 4))
#define PITCH_SERVO_SET_CCW_ADDR			(0x00000180 + (0x101 * 4))
#define PITCH_SERVO_SET_CW_ADDR			(0x00000180 + (0x110 * 4))

#define PITCH_SERVO_GET_ERROR_ADDR		(0x00000180 + (0x010 * 4))
#define PITCH_SERVO_GET_STATUS_ADDR	(0x00000180 + (0x001 * 4))

#define PITCH_SERVO_RESET_ADDR				(0x00000180 + (0x100 * 4))

/* Yaw Servo 0x200*/ 
#define YAW_SERVO_SET_RATE_ADDR			0x00000200
#define YAW_SERVO_SET_POS_ADDR			(0x00000200 + (0x011 * 4))
#define YAW_SERVO_SET_CCW_ADDR			(0x00000200 + (0x101 * 4))
#define YAW_SERVO_SET_CW_ADDR				(0x00000200 + (0x110 * 4))

#define YAW_SERVO_GET_ERROR_ADDR		(0x00000200 + (0x010 * 4))
#define YAW_SERVO_GET_STATUS_ADDR		(0x00000200 + (0x001 * 4))

#define YAW_SERVO_RESET_ADDR				(0x00000200 + (0x100 * 4))

/* Roll Servo 0x100*/
#define ROLL_SERVO_SET_RATE_ADDR			0x00000100
#define ROLL_SERVO_SET_POS_ADDR			(0x00000100 + (0x011 * 4))
#define ROLL_SERVO_SET_CCW_ADDR			(0x00000100 + (0x101 * 4))  
#define ROLL_SERVO_SET_CW_ADDR				(0x00000100 + (0x110 * 4))

#define ROLL_SERVO_GET_ERROR_ADDR		(0x00000100 + (0x010 * 4))
#define ROLL_SERVO_GET_STATUS_ADDR		(0x00000100 + (0x001 * 4))

#define ROLL_SERVO_RESET_ADDR				(0x00000100 + (0x100 * 4))

/* Joint Mode angle limits */
#define PITCH_CW_LIMIT 1200
#define PITCH_CCW_LIMIT 1200

#define ROLL_CCW_LIMIT 612
#define ROLL_CW_LIMIT 412

/* Axis Saturation Limits */
#define PITCH_SATURATION 400
#define ROLL_SATURATION 200
#define YAW_SATURATION 400

/* Joint mode option */
//#define USE_PITCH_JOINT_MODE
//#define USE_ROLL_JOINT_MODE
  
/* Reset Speed for joint mode */
#define RESET_SPEED 100

/* Function Prototypes */
int control_system_update();

int control_system_init();

void set_as_current_position();

void rotate_current_positon(float pitch, float yaw, float roll);

void update_gains(float P_pitch, float P_yaw, float P_roll, float I_pitch, float I_yaw, float I_roll,  float D_pitch, float D_yaw, float D_roll);

void pid_loop(double error[], double rates[], float time_step);

#endif // __CONTROL_SYSTEM_H__
