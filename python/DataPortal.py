import socket
import threading

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
        self.graphRequest = False
        self.status = False;

    def run(self,):
    	global comSocket
        while not self._stop.isSet():
			try:
				tempDataBuffer, addr = comSocket.recvfrom(1024)	# buffer size is 1024
				inId = ord(tempDataBuffer[2])						# Get the packet ID
				inChecksum  = inId
				inLength = ord(tempDataBuffer[3])					# Get the data packet Length
				dataBytes = ''
				for i in range(0,inLength-1):
					try:
						inChecksum += ord(tempDataBuffer[i])			# Convert incoming character to int and add to checksum
						if i >= 4:
							dataBytes += tempDataBuffer[i]					# Grab the data bytes now incase they are valid
					except IndexError:
						break

				inChecksum = (inChecksum%255)

				if(inChecksum == ord(tempDataBuffer[len(tempDataBuffer)-1])):		# Ensure the checksum is correct
					self.status = True
					if(inId == 0x01):												# Check if it is a graph request packet
						self.graphRequest = True
					else:
						self.graphRequest = False
				else:
					self.status = False
					data_buffer = ''
			except socket.timeout:
				data_buffer = ''
				self.status = False

    def stop(self):
        self._stop.set()

    def getStatus(self):			# If the status of the receiver is consitently False then the receiver is timing out or getting checksum failures
    	return self.status

"""----------------------------------------------------------------------------------"""
"""
	Bellow are the getter and setter methods for interaction with the receiver thread's internal flags.
	This allows access to the threads parameters without having to access the thread object directly from 
	the main controller. 
"""
def portalStatus():
	return receiver.getStatus

def graphDataRequested():
	return receiver.graphRequest

def shutDownPortal():
	com_socket.close()
	receiver.stop()
"""----------------------------------------------------------------------------------"""
"""
	This function will send the passed arguements to the host PC via the UDP socket with
	the proper packet formating. The input should be a list with format [id,dataLength,dataBytes]
"""
def sendDataToHost(dataPacket):
	totalBuffer = chr(0xFF) + chr(0xFF)
	checksum = 0xFF + 0xFF
	for byte in dataPacket:
		totalBuffer += chr(byte)
		checksum += byte
	checksum = checksum%255
	totalBuffer += chr(checksum)
	com_socket.sendto(totalBuffer, (UDP_IP,UDP_OUTPUT_PORT))


"""----------------------------------------------------------------------------------"""
"""
	Define the global variables
"""
com_socket = None
receiver = None
UDP_IP
UDP_OUTPUT_PORT

"""----------------------------------------------------------------------------------"""

"""
	Initialize the UDP socket for data communication using the given IPs and ports. If the socket intializes start a
	receiver thread to begin listening for commands. 
"""

def portalInit(IP,hostIP,outPort,inPort):
	global UDP_IP, UDP_OUTPUT_PORT, com_socket, receiver
	UDP_IP = IP					# Ip of the DE0
	UDP_HOSTIP = hostIP 		# Ip of controling computer
	UDP_OUTPUT_PORT = outPort   # Output port is input port of DE0
	UDP_INPUT_PORT = inPort		# Input port is reply port of DE0

	try:
		com_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)	#intialise socket for UDP
	except socket.error, msg:
		print 'Socket Error:' + str(msg[0]) + " Message:" + str(msg[1])

	com_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)	# Allow a bidirectional socket
	com_socket.setblocking(1)											# Block on socket commands
	com_socket.settimeout(1)											# Exert socket.timeout after one second

	try:
		com_socket.bind((UDP_HOSTIP,UDP_INPUT_PORT))		# Bind socket to the port and IP
		receiver = recvThread()
		receiver.start()									# Start the receiver thread if no exceptions thrown
	except socket.error, msg:
		print 'Bind Error:' + str(msg[0]) + " Message:" + str(msg[1])

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
	com_socket.sendto(complete_buffer, (UDP_IP,UDP_OUTPUT_PORT))

