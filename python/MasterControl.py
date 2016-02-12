import ControlSystemWrapper
import ImageCaptureWrapper
import threading 
import time

class updateControlSystem(threading.Thread):
    def __init__(self, p, i, d):
        threading.Thread.__init__(self)
        self.p = p
        self.i = i
        self.d = d

    def run(self):
        print "Starting \n"
        #insert call to python wrapper for control system
        print "Exiting \n"

class startExposure(threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)

    def run(self):
        print "Starting \n"
        #insert call to python wrapper for camera start exposure
        print "Exiting \n"

class stopExposure(threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)

    def run(self):
        print "Starting \n"
        #insert call to python wrapper for camera stop exposure
        print "Exiting \n"

"""
Test functions for python wrappers

result = ControlSystemWrapper.control_system_update(0.01,0.001,0.1,0.0001)
print "ControlSystem Should Be 0.1111: ", result

result = ImageCaptureWrapper.ZWO_Setup()
print "ZWO_Setup Should Be 1337: ", result

result = ImageCaptureWrapper.ZWO_Start_Exposure(1337)
print "ZWO_Start_Exposure Should Be 1337: ", result

result = ImageCaptureWrapper.ZWO_Check_Exposure_Status()
print "ZWO_Check_Exposure_Status Should Be 1337: ", result

result = ImageCaptureWrapper.ZWO_End_Exposure(1337)
print "ZWO_End_Exposure Should Be 1337: ", result

result = ImageCaptureWrapper.ZWO_Stop()
print "ZWO_Stop Should Be 1337: ", result

"""

p = raw_input("Enter P:")
i = raw_input("Enter I:")
d = raw_input("Enter D:")

refreshRate = 100
exposureTime = 20

threadPool = 3
curThreadCount = 1
prevCntrlTime = time.time()
captureStart = False
run = True


while(run):
	if(time.time() - prevCntrlTime >= (1/refreshRate)):
		prevCntrlTime = time.time()
		cntrlThread = updateControlSystem(p, i, d)
		while(threading.active_count() > threadPool):
			pass	#Do Nothing
		cntrlThread.start()

	if (!captureStart):
		imageThread = startExposure()
		while(threading.active_count() > threadPool):
			pass	#Do Nothing
		captureTime = time.time()
		imageThread.start()
		captureStart = True
	else
		if(time.time() - captureTime >= exposureTime):
			imageThread = stopExposure()
			while(threading.active_count() > threadPool):
				pass	#Do Nothing
			imageThread.start()
			captureStart= False



