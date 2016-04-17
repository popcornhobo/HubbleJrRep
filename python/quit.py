import os

try:
	cmdfifo = open("Command_fifo.fifo", "w")
	cmdfifo.write('q')
	print "Fifo Written"
except:
	print "Fifo Failed"


