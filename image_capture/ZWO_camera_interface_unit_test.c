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

int main(){
	int setup = ZWO_Setup();

	if(setup == 0){
		printf("Setup Succesful\n");	
	} else {
		printf("Setup Failed %d\n", setup);	
	}

	ZWO_Start_Exposure(100);
	printf("Exposure Started...\n");

	usleep(100*1000);
	printf("Done Sleeping\n");	
	
	int expose = 0;	
	do{
	 expose = ZWO_Check_Exposure_Status();
	}while(expose == 2);

	if(expose == 0){
		printf("exposure success");	
	}else{
		printf("exposure failed");
	}

	ZWO_End_Exposure(0);
	printf("Image Captured");

	ZWO_Stop();
	printf("stoped");	

	return 0;
}
