import threading
import DataPortal as GimbalCom
import socket
import time

def graphControlSystemError(errorData,time):
	print "Time: ", time
	print "Buffer: ", errorData

gimbalIP = "192.168.1.200"
yourIP = "192.168.1.100"
inputPort = 18002
outputPort = 18001

graphRate = 100

GimbalCom.portalInit(gimbalIP,yourIP,outputPort,inputPort)

startTime = time.time()
prevTime = 0
while (1):
	if time.time() - prevTime >= graphRate:
		prevTime = time.time()
		GimbalCom.sendData([0x01,0x01,0x01])
		while GimbalCom.dataStatus() == -1:
			time.sleep(0.001)
		status = GimbalCom.dataStatus()
		if status == 0x01:				# Id is 1 so gimbal returned control system error values
			buf = GimbalCom.getBuffer()
			graphControlSystemError(buf,startTime-prevTime)
		elif status == 0x00:			# If Id is 0 then the gimbal got a checksum error so reset timer and send again
			prevTime = 0
		elif status == -2:
			prevTime = 0 				# If -2 is returned there was a checksum error so reset timer and send again