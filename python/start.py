import os

try:
	cmdfifo = open("Command_fifo.fifo", "w")
	cmdfifo.write('s')
	print "Fifo Written"
except:
	print "Fifo Failed"


