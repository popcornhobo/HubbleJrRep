/* ZWO_camera_interface.c
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

#include "ZWO_camera_interface.h"

//ZWO_Setup
//Sets up the camera's configurating and connection, returns an error packet
int ZWO_Setup(){
	if(getNumberOfConnectedCameras() < 1){
		return 1;
	}
	
	if(!openCamera(CAMERA_NUMBER)){
		return 2;
	}

	if(!initCamera()){
		return 3;
	}	
	
	if(!setImageFormat(RESOLUTION_W, RESOLUTION_H, 1, IMG_RAW8)){
		return 5;
	}
	
	pRgb=cvCreateImage(cvSize(RESOLUTION_W, RESOLUTION_H), IPL_DEPTH_8U, 1);
	
	if(pRgb == NULL){
		return 4;
	}

	setValue(CONTROL_GAIN,getMin(CONTROL_GAIN), false); 
	setValue(CONTROL_BANDWIDTHOVERLOAD, getMin(CONTROL_BANDWIDTHOVERLOAD), false); //low transfer speed
	setValue(CONTROL_WB_B, 90, false);
 	setValue(CONTROL_WB_R, 48, false);
  	setAutoPara(getMax(CONTROL_GAIN)/2,MAX_EXPOSURE, DEST_BRIGHTNESS); //max auto gain and exposure and target brightness
	
	return 0;
}

//ZWO_Start_Exposure
//Starts a camera Exposure and returns an error packet
int ZWO_Start_Exposure(int desired_exp_time){
		setValue(CONTROL_EXPOSURE, desired_exp_time, false); //ms//auto
		startExposure();
		return 0;
}

//ZWO_Check_Exposure_Status
//Checks if the camara is ready to have an image pulled from it.
int ZWO_Check_Exposure_Status(){
	EXPOSURE_STATUS status = getExpStatus();
	
	switch(status)
	{
		case EXP_IDLE:
			return 1; 
			break;
		
		case EXP_WORKING:
			return 2;
			break;
		
		case EXP_SUCCESS:
			return 0;
			break;
		
		case EXP_FAILED:
			return 3;
			break;
			
		default:
			return 4;
	}
}

//ZWO_End_Exposure
//Stops the current exposure and saves the resulting image to a file named image_(date)_(image_number).jpg
int ZWO_End_Exposure(int image_number){
 
	if(!getImageAfterExp((unsigned char*)pRgb->imageData, pRgb->imageSize)){
		return 1;
	}
	
	cv::Mat image = cv::cvarrToMat(pRgb);
	
	//if(image == NULL){
	//	return 2;
	//}
	
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	
	char name[50];
	
	name[0] = 'C';
	name[1] = 'a';
	name[2] = 'p';
	name[3] = 't';
	name[4] = 'u';
	name[5] = 'r';
	name[6] = 'e';
	name[7] = '_';
	
	int i = 8;
	if((tm.tm_mon + 1) > 9){
		name[i++] = (tm.tm_mon + 1)/10;
		name[i++] = (tm.tm_mon  - 9);
	}else{
		name[i++] = (tm.tm_mon + 1);
	}
	
	name[i++] = '-';
	
	if(tm.tm_mday > 9){
		name[i++] = (tm.tm_mday /10);
		name[i++] = (tm.tm_mday  - 10*(tm.tm_mday /10));
	} else {
		name[i++] = tm.tm_mday;
	}
	
	name[i++] = '_';
	
	name[i++] = (image_number +  48);
	
	name[i++] = '.';
	name[i++] = 'j';
	name[i++] = 'p';
	name[i++] = 'g';
	name[i++] = '\0';
	
	cv::imwrite(name, image);
	
	return 0;
}


//ZWO_Stop
//Stops all camera capture and closes the usb interface
int ZWO_Stop(){
	stopCapture();
	
	closeCamera();
	
	cvReleaseImage(&pRgb);
	
	return 0;
}
