/* ZWO_camera_interface.h
	Header file for high level camera interface
	HABOP 2016
*/
/* ZWO_camera_interface.h
 * 
 * Source file for c functions relating to the control system
 *
 * Authors:	Zach Frazee
 * 
 *
 * Revions:
 * 2016-02-11	v0.1	Initial Revison
 *
 * */

//DEFINE
#ifndef ZWO_CAM_h
#define ZWO_CAM_h

//INCLUDES
#include "stdio.h"
#include "opencv2/core/core_c.h"
#include "ASICamera.h"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "cv.h"
#include "opencv2/highgui/highgui.hpp"

//DEFINES
#define MAX_EXPOSURE 10
#define DEST_BRIGHTNESS 150
#define CAMERA_NUMBER 1

#define RESOLUTION_W 640
#define RESOLUTION_H 480

//GLOBAL VARIABLES

//image buffer
char buf[128]={0};

//openCV Image
IplImage *pRgb;

//FUNCTION PROTOTYPES

/*ZWO_Setup
Sets up the camera's configurating and connection, returns an error packet
	Return: error Type char
		0 when no error
		1 when no camera detected
		2 when failed to open connection to camera
		3 when failed to initialize camera
		4 when failed to create openCV image
		5 when configuration failed
		6 unknown error
*/
char ZWO_Setup();

/*
ZWO_Start_Exposure
	Starts a camera Exposure and returns an error packet
	Params:
		target_exp_time = target exposure time for an image in ms
	Return: error Type char
		0 when no error
		1 when exception thrown starting exposure
*/
char ZWO_Start_Exposure(int desired_exp_time);


/*ZWO_Check_Exposure_Status
	Checks if the camara is ready to have an image pulled from it. Must be checked before using ZWO_End_Exposure()
	Return:
		0 when camera has had a successful exposure
		1 when camera is idle
		2 when the camera is still working an exposure
		3 when exposure has failed
*/
char ZWO_Check_Exposure_Status();


/*	ZWO_End_Exposure

	Stops the current exposure and saves the resulting image to a file named image_(date)_(image_number).jpg
	Params:
		image_number = a number that will be used in the saved image file name
	Return: error Type char
		0 when no error
		1 when error getting image from camera
		2 when error converting image format
		3 when error saving to file
		4 when unknown error
*/
char ZWO_End_Exposure(int image_number);


/*ZWO_Stop
	Stops all camera capture and closes the usb interface
	Return: error Type char
		0 when success
		1 when exposure still working
		2 when failed to stop capture
		3 when failed to close camera interface
		4 when failed ro release openCV image
		5 when unknown error
*/
char ZWO_Stop();

//END
#endif