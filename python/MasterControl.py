import ControlSystemWrapper
#import ImageCaptureWrapper
import threading 
import time
import re

"""
    This thread class handles all user inputs and alters a global status variable to alter program flow, ensure all threads
    are properly terminated, and program termination is done safely.
"""

print "Hey im alive!!!"

class userInputThread(threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)
        self._stop = threading.Event()

    def run(self,):
		global run_status
		global p, i, d
		global run_status_lock
		regex_input = re.compile(r'(.+)\:+\s*(\d*\.*\d*)')
		regex_cmd = re.compile(r'(.+)\:+')
		regex_val_int = re.compile(r'(\d+)')
		regex_val_float = re.compile(r'(\d+\.*\d*)')

		while (not self._stop.isSet()):
			input = raw_input("Enter Command: ")
			match = regex_input.match(input)
			if match:                                   #Use regular Expressions to parse command line inputs
				cmd = regex_cmd.match(input)
				if(cmd.group() == "Stop:"):
					print "Stopping..."
					with run_status_lock:
						run_status = "Stop"
				elif(cmd.group() == "Start:"):
					print "Starting..."
					with run_status_lock:
						run_status = "Start"
				elif(cmd.group() == "Quit:"):
					print "Quitting..."
					self._stop.set()
					with run_status_lock:
						run_status = "Quit"
				elif(cmd.group() == "P:"):
					val = regex_val_int.search(input)
					if val:
						print "P: ", int(val.group(),10)
						with control_system_lock:
							p = int(val.group())        #Converts the regex parsed input value to an int and store as tilt center
							ControlSystemWrapper.update_gains(p,i,d)
					else:
						print "Invalid Value:"
				elif(cmd.group() == "I:"):
					val = regex_val_int.search(input)
					if val:
						print "I: ", int(val.group())
						with control_system_lock:
							i = int(val.group())       #Converts the regex parsed input value to an int and store as pan center
							ControlSystemWrapper.update_gains(p,i,d)
					else:
						print "Invalid Value"
				elif(cmd.group() == "D:"):
					val = regex_val_int.search(input)
					if val:
						print "D: ", int(val.group())
						with control_system_lock:
							d = int(val.group())       #Converts the regex parsed input value to an int and store as pan center
							ControlSystemWrapper.update_gains(p,i,d)
					else:
						print "Invalid Value"
				else:
					print "Invalid Entry: \n"
					print "Valid Entries Are \t'P:#'\n\t\t\t'I:#'"
					print "\t\t\t'D:#'\n\t\t\t'Stop:'"
					print "\t\t\t'Start:'\n\t\t\t'Quit:'"
			else:
				print "Invalid Entry: \n"
				print "Valid Entries Are \t'Rate:#.#(deg/s)'\n\t\t\t'ScanBoundTop:#(deg)'"
				print "\t\t\t'ScanCenterPan:#(deg)'\n\t\t\t'ScanCenterTilt:#(deg)'"
				print "\t\t\t'ScanRangePan:'#(deg)\n\t\t\t'TiltInc:#(deg)'\n\t\t\t'Stop:'"
				print "\t\t\t'Start:'\n\t\t\t'Quit:'"

    def stop(self):             # A setter function for the Stop flag
        self._stop.set()

"""----------------------------------------------------------------------------------"""

class updateControlSystemThread(threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)
        self._stop = threading.Event()
    
    def run(self):
        global run_status
        print "Starting Cntrl\n"
        startTime = 0
        status = 0
        while not (run_status == "Quit" and self._stop.isSet()):
            with control_system_lock:
                ControlSystemWrapper.update_gains(p,i,d)
            while (run_status == "Start") and (time.time() - startTime > 1/refreshRate) and not(self._stop.isSet()):
                startTime = time.time()
                with control_system_lock:
                    status = ControlSystemWrapper.control_system_update()
                    if status == -1:
                        print "Control System Init Error\n"
                        with run_status_lock:
                            run_status = "Quit"
            with control_system_lock:
                ControlSystemWrapper.update_gains(0,0,0)
        print "Exiting Cntrl\n"

    def stop(self):
        self._stop.set()

class captureImage(threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)

    def run(self):
        global captureStart
        print "Starting StrExp\n"
        #insert call to python wrapper for camera start exposure
        print "Exiting StrExp\n"

ui = userInputThread()
ui.start()

print "Thread Started"

refreshRate = 100
exposureTime = 20

p = 0
i = 0
d = 0

run_status = "Stop"
run_status_lock = threading.Lock()
control_system_lock = threading.Lock()

threadPool = 3
curThreadCount = 1
prevCntrlTime = time.time()
captureStart = False

test = ControlSystemWrapper.control_system_update()

if test == -1:
	print "Failed to initialize!!!"
	
	
ControlSystemWrapper.set_as_current_position()

control_system = updateControlSystemThread()
control_system.start()

while not(run_status == "Quit"):
    pass

if ui.isAlive():
    ui.stop()
    ui.join()
if control_system.isAlive():
    control_system.stop()
    control_system.join()



