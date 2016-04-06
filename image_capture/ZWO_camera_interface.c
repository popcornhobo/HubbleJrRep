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

//Global Error
char error = 0;

int main(int argc, char *argv[]){
	if(argc != 3){
		printf("Wrong number of arguments\n");
		return -1;
	}
	
	int exposure = atoi(argv[1]);
	int imageNumber = atoi(argv[2]);;

	int setup = ZWO_Setup();
	
	ZWO_Start_Exposure(exposure);

	usleep(exposure*1000);
	
	int expose = 0;	
	do{
		expose = ZWO_Check_Exposure_Status();
		usleep(100);
	}while(expose == 2);

	ZWO_End_Exposure(imageNumber);

	ZWO_Stop();
	
	//Write to FIFO
	if(access("Image_Capture.fifo", F_OK ) != -1){
		FILE * fifo = fopen("Image_Capture.fifo", "w");
		fprintf(fifo, "%c", error);
	} else {
		printf("No FIFO found...\n");
	}

	return 0;
}

//ZWO_Setup
//Sets up the camera's configurating and connection, returns an error packet
int ZWO_Setup(){
	if(getNumberOfConnectedCameras() < 1){
		printf("No cameras detected\n");
		error = 1;
		return 1;
	}
	
	if(!openCamera(CAMERA_NUMBER)){
		printf("Failed to open camera\n");
		error = 2;
		return 2;
	}

	if(!initCamera()){
		printf("Failed to initialize camera\n");		
		error = 3;
		return 3;
	}	
	
	if(!setImageFormat(RESOLUTION_W, RESOLUTION_H, 1, IMG_RGB24)){
		printf("Failed to set image format\n");		
		error = 5;
		return 5;
	}
	
	pRgb=cvCreateImage(cvSize(RESOLUTION_W, RESOLUTION_H), IPL_DEPTH_8U, 3);
	
	if(pRgb == NULL){
		printf("Failed to create image\n");		
		error = 4;
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
		setValue(CONTROL_EXPOSURE, desired_exp_time*1000, false); //ms//auto
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
			printf("Camera is idle\n");
			error = 6;
			return 1; 
			break;
		
		case EXP_WORKING:
			return 2;
			break;
		
		case EXP_SUCCESS:
			return 0;
			break;
		
		case EXP_FAILED:
			printf("Exposure Failed\n");
			error = 7;
			return 3;
			break;
			
		default:
			error = 8;
			return 4;
	}
}

//ZWO_End_Exposure
//Stops the current exposure and saves the resulting image to a file named image_(date)_(image_number).jpg
int ZWO_End_Exposure(int image_number){
 
	if(!getImageAfterExp((unsigned char*)pRgb->imageData, pRgb->imageSize)){
		error = 9;
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
	
	name[8] = (image_number +  48);
	
	name[9] = '.';
	name[10] = 'j';
	name[11] = 'p';
	name[12] = 'g';
	name[13] = '\0';
	
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
