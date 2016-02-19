#
CROSS_COMPILE = arm-linux-gnueabihf-
CFLAGS = -static -g -Wall  -I${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include
LDFLAGS =  -g -Wall -lm 
CC = $(CROSS_COMPILE)gcc
ARCH= arm

all: control_system.so quaternion_math.so

control_system.so: control_system.o
	$(CC) $(LDFLAGS) $^ -shared -o $@  
quaternion_math.so: quaternion_math.o
	$(CC) $(LDFLAGS) $^ -shared -o $@  
	
%.o : %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -f $(TARGET) *.a *.o *.so *~ 