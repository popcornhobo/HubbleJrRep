import os

try:
	cmdfifo = open("Command_fifo.fifo", "w")
	cmdfifo.write('y')
	print "Fifo Written"
except:
	print "Fifo Failed"


