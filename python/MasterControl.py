import ControlSystemWrapper
import ImageCaptureWrapper
import threading 
import time

"""
    This thread class handles all user inputs and alters a global status variable to alter program flow, ensure all threads
    are properly terminated, and program termination is done safely.
"""

class userInputThread(threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)
        self._stop = threading.Event()

    def run(self,):
        global run_status
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
                        with run_status_lock:
                            run_status = "Stop"
                        p = int(val.group())        #Converts the regex parsed input value to an int and store as tilt center
                        with run_status_lock:
                            run_status = "Start"
                    else:
                        print "Invalid Value:"
                elif(cmd.group() == "I:"):
                    val = regex_val_int.search(input)
                    if val:
                        print "I: ", int(val.group())
                        with run_status_lock:
                            run_status = "Stop"
                        i = int(val.group())       #Converts the regex parsed input value to an int and store as pan center
                        with run_status_lock:
                            run_status = "Start"
                    else:
                        print "Invalid Value"
                elif(cmd.group() == "D:"):
                    val = regex_val_int.search(input)
                    if val:
                        print "D: ", int(val.group())
                        with run_status_lock:
                            run_status = "Stop"
                        d = int(val.group())       #Converts the regex parsed input value to an int and store as pan center
                        with run_status_lock:
                            run_status = "Start"
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

class updateControlSystem(threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)

    def run(self):
        print "Starting Cntrl\n"
        ControlSystemWrapper.control_system_update()
        print "Exiting Cntrl\n"

class startExposure(threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)

    def run(self):
        print "Starting StrExp\n"
        #insert call to python wrapper for camera start exposure
        print "Exiting StrExp\n"

class stopExposure(threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)

    def run(self):
        print "Starting HltExp\n"
        #insert call to python wrapper for camera stop exposure
        print "Exiting HltExp\n"

ui = userInputThread()
ui.start()

refreshRate = 100
exposureTime = 20

run_status = "Stop"
run_status_lock = threading.Lock()

threadPool = 3
curThreadCount = 1
prevCntrlTime = time.time()
captureStart = False

ControlSystemWrapper.set_as_current_position()

while not(run_status == "Quit"):

    while (run_status == "Start"):
    	if(time.time() - prevCntrlTime >= (1/refreshRate)):
    		prevCntrlTime = time.time()
    		cntrlThread = updateControlSystem(p, i, d)
    		while(threading.active_count() > threadPool):
    			pass	#Do Nothing
    		cntrlThread.start()

    	if (not captureStart):
    		imageThread = startExposure()
    		while(threading.active_count() > threadPool):
    			pass	#Do Nothing
    		captureTime = time.time()
    		imageThread.start()
    		captureStart = True
    	else:
    		if(time.time() - captureTime >= exposureTime):
    			imageThread = stopExposure()
    			while(threading.active_count() > threadPool):
    				pass	#Do Nothing
    			imageThread.start()
    			captureStart= False

if ui:
    ui.stop()




