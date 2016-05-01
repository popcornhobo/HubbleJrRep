import ControlSystemWrapper
import DataPortal as DataCom
import threading 
import time
import re
import os

"""
    This thread class handles all user inputs and alters a global status variable to alter program flow, adjust 
    runtime parameters and ensure all threads are properly terminated.
"""
class userInputThread(threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)
        self._stop = threading.Event()

    def run(self,):
		global runStatus, runStatusLock
		global p, i, d, controlSystemLock
		global maxImageCount, exposureTime, captureRate

		# pre-compile all the necessary command-parse regular expressions
		regex_input = re.compile(r'\s*(\w+)\s*\:\s*.*')
		regex_cmd = re.compile(r'\s*(\w+)\s*\:')
		regex_val_int = re.compile(r'(\d+)')
		regex_val_float = re.compile(r'(\d+\.*\d*)')
		regex_three_floats = re.compile(r'(\-?\d+\.*\d*),(\-?\d+\.*\d*),(\-?\d+\.*\d*)')

		# notify the user of available commands
		print "Valid Entries Are \t'Pyaw:#.##'\n\t\t\t'Iyaw:#.##'\n\t\t\t'Dyaw:#.##'"
		print "\t\t\t'Ppitch:#.##'\n\t\t\t'Ipitch:#.##'\n\t\t\t'Dpitch:#.##'"
		print "\t\t\t'Proll:#.##'\n\t\t\t'Iroll:#.##'\n\t\t\t'Droll:#.##'"
		print "\t\t\t'Rotate: yaw,pitch,roll' in Degrees #.##"
		print "\t\t\t'HoldPos:'\n\t\t\t'Stop:'\n\t\t\t'Start:'"
		print "\t\t\t'Help:'\n\t\t\t'Quit:'"

		while (not self._stop.isSet()):
			input = raw_input("Enter Command: ")
			match = regex_input.match(input)
			if match:                                   # Use regular Expressions to parse command line inputs
				cmd = regex_cmd.match(input)
				if(cmd.group() == "Stop:"):
					print "Stopping..."					# Stop correction
					with runStatusLock:
						runStatus = "Stop"

				elif(cmd.group() == "Start:"):
					print "Starting..."					# Allow correction
					with runStatusLock:
						runStatus = "Start"
					with controlSystemLock:
						ControlSystemWrapper.update_gains(p,i,d)

				elif(cmd.group() == "Quit:"):
					print "Quitting..."					# Exit the progrma
					self._stop.set()
					with runStatusLock:
						runStatus = "Quit"

				elif(cmd.group() == "Help:"):
					print "Valid Entries Are \t'Pyaw:#.##'\n\t\t\t'Iyaw:#.##'\n\t\t\t'Dyaw:#.##'"		# Print the command list on help call
					print "\t\t\t'Ppitch:#.##'\n\t\t\t'Ipitch:#.##'\n\t\t\t'Dpitch:#.##'"
					print "\t\t\t'Proll:#.##'\n\t\t\t'Iroll:#.##'\n\t\t\t'Droll:#.##'"
					print "\t\t\t'Rotate: yaw,pitch,roll' in Degrees #.##"
					print "\t\t\t'Capture: ExposureTime,CaptureRate,MaxPics' in Seconds,Seconds,Pics"
					print "\t\t\t'HoldPos:'\n\t\t\t'Stop:'\n\t\t\t'Start:'"
					print "\t\t\t'Help:'\n\t\t\t'Quit:'"

				elif(cmd.group() == "Capture:"):
					match = regex_three_floats.search(input)
					if match:
						(str1,str2,str3) = match.groups()
						imCap.imageNumber = 0
						(exposureTime,captureRate,maxImageCount) = (float(str1),float(str2),float(str3))
						print "Stared Capture"

				elif(cmd.group() == "Pyaw:"):
					match = regex_val_float.search(input)
					if match:
						val = float(match.group())	# Converts the regex parsed input value to a float
						print "Pyaw: ", val
						with controlSystemLock:
							p[1] = val
							ControlSystemWrapper.update_gains(p,i,d)	# Update control system Gains after aquiring lock
					else:
						print "Invalid Value for 'Pyaw:'"

				elif(cmd.group() == "Iyaw:"):
					match = regex_val_float.search(input)
					if match:
						val = float(match.group())	# Converts the regex parsed input value to a float
						print "Iyaw: ", val
						with controlSystemLock:
							i[1] = val
							ControlSystemWrapper.update_gains(p,i,d)	# Update control system Gains after aquiring lock
					else:
						print "Invalid Value for 'Iyaw:'"

				elif(cmd.group() == "Dyaw:"):
					match = regex_val_float.search(input)
					if match:
						val = float(match.group())	# Converts the regex parsed input value to a float
						print "Dyaw: ", val
						with controlSystemLock:
							d[1] = val 
							ControlSystemWrapper.update_gains(p,i,d)	# Update control system Gains after aquiring lock
					else:
						print "Invalid Value for 'Dyaw:'"

				elif(cmd.group() == "Proll:"):
					match = regex_val_float.search(input)
					if match:
						val = float(match.group())	# Converts the regex parsed input value to a float
						print "Proll: ", val
						with controlSystemLock:
							p[2] = val
							ControlSystemWrapper.update_gains(p,i,d)	# Update control system Gains after aquiring lock
					else:
						print "Invalid Value for 'Proll:'"

				elif(cmd.group() == "Iroll:"):
					match = regex_val_float.search(input)
					if match:
						val = float(match.group())	# Converts the regex parsed input value to a float
						print "Iroll: ", val
						with controlSystemLock:
							i[2] = val
							ControlSystemWrapper.update_gains(p,i,d)	# Update control system Gains after aquiring lock
					else:
						print "Invalid Value for 'Iroll:'"

				elif(cmd.group() == "Droll:"):
					match = regex_val_float.search(input)
					if match:
						val = float(match.group())	# Converts the regex parsed input value to a float
						print "Droll: ", val
						with controlSystemLock:
							d[2] = val 
							ControlSystemWrapper.update_gains(p,i,d)	# Update control system Gains after aquiring lock
					else:
						print "Invalid Value for 'Droll:'"

				elif(cmd.group() == "Ppitch:"):
					match = regex_val_float.search(input)
					if match:
						val = float(match.group())	# Converts the regex parsed input value to a float
						print "Ppitch: ", val
						with controlSystemLock:
							p[0] = val
							ControlSystemWrapper.update_gains(p,i,d)	# Update control system Gains after aquiring lock
					else:
						print "Invalid Value for 'Ppitch:'"

				elif(cmd.group() == "Ipitch:"):
					match = regex_val_float.search(input)
					if match:
						val = float(match.group())	# Converts the regex parsed input value to a float
						print "Ipitch: ", val
						with controlSystemLock:
							i[0] = val
							ControlSystemWrapper.update_gains(p,i,d)	# Update control system Gains after aquiring lock
					else:
						print "Invalid Value for 'Ipitch:'"

				elif(cmd.group() == "Dpitch:"):
					match = regex_val_float.search(input)
					if match:
						val = float(match.group())	# Converts the regex parsed input value to a float
						print "Dpitch: ", val
						with controlSystemLock:
							d[0] = val 
							ControlSystemWrapper.update_gains(p,i,d)	# Update control system Gains after aquiring lock
					else:
						print "Invalid Value for 'Dpitch:'"

				elif(cmd.group() == "HoldPos:"):
					print "Re-Zeroing Position..."
					with controlSystemLock:
						ControlSystemWrapper.set_as_current_position()	# Tell the control system to hold its current pos after aquiring lock

				elif(cmd.group() == "Rotate:"):
					match = regex_three_floats.search(input)
					if match:
						(str1,str2,str3) = match.groups()							# Get all the matched substrings 
						(pitch,yaw,roll) = (float(str1),float(str2),float(str3))	# Convert to floats
						print "Will rotate by... pitch: ",pitch," ,yaw: ",yaw," ,roll: ",roll
						confirmation = raw_input("Proceed? 'y'/'n'")
						if confirmation == "y":										# Get confirmation of rotation to avoid disaster
							print "Confirmed"
							with controlSystemLock:
								ControlSystemWrapper.rotate_current_position(pitch,yaw,roll)	# Adjust the goal position after aquiring lock
						else:
							print "Canceled"
					else:
						print "Invalid Value for 'Rotate:'"

				else:
					print"Invalid Command... try 'Help:'"
			else:
				pass

    def stop(self):             # An internal setter function for the Stop flag
        self._stop.set()

"""----------------------------------------------------------------------------------"""
"""
	This thread class handles updating the control system via the C control system library at the set, global refreshRate
"""
class updateControlSystemThread(threading.Thread):
	def __init__(self,):
		threading.Thread.__init__(self)
		self._stop = threading.Event()
    
	def run(self):
		global runStatus, errx,erry,errz
		global runStatusLock, controlSystemLock
		print "Starting Cntrl\n"
		startTime = 0
		status = 0
		while not (runStatus == "Quit" and self._stop.isSet()):
			with controlSystemLock:								
				ControlSystemWrapper.update_gains(p,i,d)				# Ensure the gains are intialized before starting

			while (runStatus == "Start") and (time.time() - startTime > 1/refreshRate) and not(self._stop.isSet()):
				startTime = time.time()
				with controlSystemLock:
					status = ControlSystemWrapper.control_system_update()		# Trigger a control system update and get the current error values
					if status == -1:
						print "Control System Init Error\n"				# The control system was not intialized properly for an unknown reason						
						with runStatusLock:
							runStatus = "Quit"
			if runStatus != "Start":
				with controlSystemLock:
					ControlSystemWrapper.update_gains([0,0,0], [0,0,0], [0,0,0])				# On quit set the gains to zero to stop all movement
					ControlSystemWrapper.control_system_update()
		print "Exiting Cntrl\n"

	def stop(self):
		self._stop.set()


"""----------------------------------------------------------------------------------"""
"""
	Image Capture Yet to be Implimented
"""
class captureImage(threading.Thread):
	def __init__(self,):
		threading.Thread.__init__(self)
		self._stop = threading.Event()
		
		global cameraParamsLock
		#Setup FIFO Here
		try:
			os.mkfifo("Image_Capture.fifo")
		except:
			os.system("rm Image_Capture.fifo")
			os.mkfifo("Image_Capture.fifo")
		
		self.fifo = open("Image_Capture.fifo", "r+")
	
	def run(self):
		global exposureTime,captureRate,maxImageCount
		print "Starting StrExp\n"
		startTime = 0
		self.imageNumber = 0
		while True:
			if (not self._stop.isSet()) and (self.imageNumber < maxImageCount):	# Stop captureing once thread is called to exit or the image limit is hit
				if (time.time() - startTime > captureRate):							# Only capture at the desired capture rate
					#Capture an image here
					print "Image Taken"
					os.system("./camera_capture " + str(exposureTime) + " " + str(self.imageNumber))
					
					#Wait for return byte
					while True:
						byte = self.fifo.read(1)
						if not byte:
							time.sleep(0.1)
							continue
						else:
							print "Capture Complete with error code " + byte
							self.imageNumber += 1
							break
				
			else:
				time.sleep(0.1) 		# Try not to bog down the processor waiting until capture time
		
		print "Exiting StrExp\n"
	
	def stop(self):
		self.fifo.close()
		self._stop.set()

"""----------------------------------------------------------------------------------"""
"""
	Intialization Section: Used to declare globals, create secondary threads,
	and start the low level systems
"""
# def simpleImageCaptureTest():
	# print "Starting"
	# os.mkfifo("Image_Capture.fifo")
	# fifo = open("Image_Capture.fifo", "r+")
	
	# #Capture an image here
	# print "Image Taken"
	# os.system("./camera_capture 250 1")
	
	# #Wait for return byte
	# while True:
		# byte = fifo.read(1)
		# if not byte:
			# time.sleep(0.1)
			# continue
		# else:
			# print "Capture Complete with error code " + byte
			# break

#simpleImageCaptureTest()

ui = userInputThread()
ui.start()

print "UI Thread Started"
# Global Refresh Rate in tics/sec
refreshRate = 200

# NOTE! these values should not be written to without the controlSystemLock
controlSystemLock = threading.Lock()
p = [30,20,30] 	 # These lists are in Pitch Yaw Roll order
i = [30,20,30]      # It is good form to update p,i,d immediatley before calling update_gains() to limit lock passes
d = [0,0,0]

errx = 0 
erry = 0 	# These should only be used for storing the control_system_update() returned errors but may be read at any time
errz = 0
# end NOTE!

# NOTE! these values should not be written to without the runStatusLock
runStatusLock = threading.Lock()
runStatus = "Stop"
# end NOTE!

# NOTE! these values should not be written to without the cameraParamsLock
exposureTime = 1000
captureRate = 10
maxImageCount = 0
# end NOTE!

imCap = captureImage()
imCap.start()

#hostIP = "192.168.1.2"
#udpIP = "192.168.1.1"
#inputPort = 18001
#outputPort = 18002
#DataCom.portalInit(udpIP, hostIP, outputPort, inputPort)	# Setup the UDP port using the defined information

test = ControlSystemWrapper.control_system_update()			# Call update for the first time now to intialize the system

if test != -1:												# Check if init failed
	ControlSystemWrapper.set_as_current_position()			# Hold the current IMU position to ensure stable startup
	controlSystem = updateControlSystemThread()
	controlSystem.start()									# Start the control system thread 
else:
	print "Failed to initialize control system data... Exiting"
	with runStatusLock:
		runStatus = "Quit"
"""----------------------------------------------------------------------------------"""
"""
	The main thread 
"""
while not(runStatus == "Quit"):
#	status = DataCom.dataStatus()
#	if status != -1:
#		if status == 0x01:
#			DataCom.sendData([0x01,0x03,xerr,yerr,zerr])		# Order is [packet Id, dataLength, dataBytes]
#		elif status == -2:
#			DataCom.sendData([0x00,0x01,0x00])
#	else:
	time.sleep(0.008)	# Sleep for just under 1/100th of a second to save processor power but not stall DataCom transmissions

#Quit All threads NOW!!!!

#if DataCom:
	#DataCom.shutDownPortal()
if controlSystem.isAlive():
	controlSystem.stop()
	controlSystem.join()
if ui.isAlive():
	ui.stop()
	ui.join()
#if imCap.isAlive():
#	imCap.stop()
#	imCap.join()

#remove the fifo
os.system("rm Image_Capture.fifo")

print controlSystem.isAlive()
print ui.isAlive()
#print DataCom.portalIsAlive()

try:
	os._exit(1)
except:
	print "Exit Failed"
"""----------------------------------------------------------------------------------"""

