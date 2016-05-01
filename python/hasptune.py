import os

try:
	cmdfifo = open("Command_fifo.fifo", "w")
	cmdfifo.write('h')
	print "Fifo Written"
except:
	print "Fifo Failed"


