""" ImageCaptureWrapper.py
* 
* Python Wrapper that allows the Image Capture Control fucntions in C to be accessed via Python
*
* Authors:	Seth Kreitinger
* 
*
* Revions:
* 2016-02-11	v0.1	Initial Revison
*
"""
import ctypes

print "in the wrapper"
_cameraInterface = ctypes.CDLL('ZWO_camera_interface.so')

print "inerface setup"
_cameraInterface.ZWO_Setup.restype = ctypes.c_int

_cameraInterface.ZWO_Start_Exposure.argtypes = (ctypes.c_int,)
_cameraInterface.ZWO_Start_Exposure.restype = ctypes.c_int

_cameraInterface.ZWO_Check_Exposure_Status.restype = ctypes.c_int

_cameraInterface.ZWO_End_Exposure.argtypes = (ctypes.c_int,)
_cameraInterface.ZWO_End_Exposure.restype = ctypes.c_int

_cameraInterface.ZWO_Stop.restype = ctypes.c_int


def ZWO_Setup():
	global _cameraInterface
	errorCode = _cameraInterface.ZWO_Setup()
	return errorCode

def ZWO_Start_Exposure(exposureTime):
	global _cameraInterface
	errorCode = _cameraInterface.ZWO_Start_Exposure(ctypes.c_int(exposureTime))
	return errorCode

def ZWO_Check_Exposure_Status():
	global _cameraInterface
	errorCode = _cameraInterface.ZWO_Check_Exposure_Status()
	return errorCode

def ZWO_End_Exposure(imageNumber):
	global _cameraInterface
	errorCode = _cameraInterface.ZWO_End_Exposure(ctypes.c_int(imageNumber))
	return errorCode

def ZWO_Stop():
	global _cameraInterface
	errorCode = _cameraInterface.ZWO_Stop()
	return errorCode

