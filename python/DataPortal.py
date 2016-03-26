import socket
import threading
import sys

"""
	This code section defines the recvThread class and overides the default methods "__init__()"
	and "run()". It also implements safe thread termination through the stop() function. 
	It is used to read from the UDP port of interest.

	The expected packet format is:
		[0xFF,0xFF,packetID,dataLength,dataBytes,checksum]
"""
class recvThread (threading.Thread):
    def __init__(self,):
        threading.Thread.__init__(self)
        self._stop = threading.Event()
        self.id = None
        self.status = False;
        self.dataBytes = []

    def run(self,):
    	global comSocket
        while not self._stop.isSet():
			try:
				tempDataBuffer, addr = comSocket.recvfrom(1024)		# buffer size is 1024
				with idlock:
					self.id = ord(tempDataBuffer[2])				# Get the packet ID
				checksum = 0
				inLength = ord(tempDataBuffer[3])					# Get the data packet Length
				self.dataBytes = []
				for i in range(0,len(tempDataBuffer)-1):
					try:
						intVal = ord(tempDataBuffer[i])
						inChecksum += intVal			# Convert incoming character to int and add to checksum
						if i >= 4:
							self.dataBytes.append(intVal)					# Grab the data bytes now incase they are valid
					except IndexError:
						break

				inChecksum = (inChecksum%255)

				if(inChecksum == ord(tempDataBuffer[len(tempDataBuffer)-1])):		# Ensure the checksum is correct
					self.status = True
				else:
					self.status = False
					self.id = -2
					dataBytes = []
			except socket.timeout:
				dataBytes = []
				self.status = False

    def stop(self):
        self._stop.set()

"""----------------------------------------------------------------------------------"""
"""
	Bellow are the getter and setter methods for interaction with the receiver thread's internal flags.
	This allows access to the threads parameters without having to access the thread object directly from 
	the main controller. 
"""
def portalStatus():
	return receiver.status

def dataStatus():					# Returns -1 if the receiver has yet to receive a packet since the last time otherwise returns the packet id received
	global receiver
	intID = receiver.id
	if intID != -1:
		with idlock:
			receiver.id = -1
	return intID

def shutDownPortal():
	global receiver
	#comSocket.close()
	receiver.stop()

def portalIsAlive():
	global receiver
	return receiver.isAlive()

def getDataBuffer():
	return receiver.dataBytes
"""----------------------------------------------------------------------------------"""
"""
	This function will send the passed arguements to the host PC via the UDP socket with
	the proper packet formating. The input should be a list with format [id,dataLength,dataBytes]
"""
def sendData(dataPacket):
	totalBuffer = chr(0xFF) + chr(0xFF)
	checksum = 0xFF + 0xFF
	for byte in dataPacket:
		totalBuffer += chr(byte)
		checksum += byte
	checksum = checksum%255
	totalBuffer += chr(checksum)
	comSocket.sendto(totalBuffer, (UDP_IP,UDP_OUTPUT_PORT))

"""----------------------------------------------------------------------------------"""
"""
	Define the global variables
"""
comSocket = None
receiver = None
UDP_IP = ""
UDP_OUTPUT_PORT = ""
idlock = threading.Lock()

"""----------------------------------------------------------------------------------"""
"""
	Initialize the UDP socket for data communication using the given IPs and ports. If the socket intializes start a
	receiver thread to begin listening for commands. 
"""
def portalInit(IP,hostIP,outPort,inPort):
	global UDP_IP, UDP_OUTPUT_PORT, comSocket, receiver
	UDP_IP = IP					# Ip of the receiver
	UDP_HOSTIP = hostIP 		# Ip of the sender
	UDP_OUTPUT_PORT = outPort   # Output port is input port of DE0
	UDP_INPUT_PORT = inPort		# Input port is reply port of DE0

	try:
		comSocket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)	#intialise socket for UDP
	except socket.error, msg:
		print 'Socket Error:' + str(msg[0]) + " Message:" + str(msg[1])
		sys.exit()

	comSocket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)	# Allow a bidirectional socket
	comSocket.setblocking(1)											# Block on socket commands
	comSocket.settimeout(None)										# Exert socket.timeout never

	try:
		comSocket.bind((UDP_HOSTIP,UDP_INPUT_PORT))		# Bind socket to the port and IP
		receiver = recvThread()
		receiver.start()									# Start the receiver thread if no exceptions thrown
	except socket.error, msg:
		print 'Bind Error:' + str(msg[0]) + " Message:" + str(msg[1])
		sys.exit()
"""----------------------------------------------------------------------------------"""
"""
	Send the error data via UDP from the IMU to the data requestor for graphing.
"""
def sendErrorData(xerr,yerr,zerr):
	packetBuffer = [xerr,yerr,zerr]
	packId = 0x01
	packetLength = 3
	checkSum = 0xFF + 0xFF + packetId + packetLength
	completeBuffer = chr(0xFF) + chr(0xFF) + chr(packetId) + chr(packetLength)

	for value in packetBuffer:
		checkSum += value
		completeBuffer += chr(value)

	check_sum = (check_sum%255)
	completeBuffer += chr(check_sum)
	comSocket.sendto(complete_buffer, (UDP_IP,UDP_OUTPUT_PORT))
"""----------------------------------------------------------------------------------"""
