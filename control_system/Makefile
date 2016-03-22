#
CROSS_COMPILE = arm-linux-gnueabihf-
CFLAGS = -static -g -Wall  -I${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include
LDFLAGS =  -g -Wall -lm 
CC = $(CROSS_COMPILE)gcc
ARCH= arm

all: quaternion_math.so control_system.so control_system_unit_test

control_system.so: control_system.o quaternion_math.o ../VN_100/soft_spi.o ../VN_100/VN_lib.o ../VN_100/VN_math.o  ../VN_100/VN_user.o  ../VN_100/VN100.o   
	$(CC) $(LDFLAGS) $^ -shared -o $@
	
quaternion_math.so: quaternion_math.o
	$(CC) $(LDFLAGS) $^ -shared -o $@  

control_system_unit_test: control_system_unit_test.o control_system.o quaternion_math.o ../VN_100/soft_spi.o ../VN_100/VN_lib.o ../VN_100/VN_math.o  ../VN_100/VN_user.o  ../VN_100/VN100.o
	$(CC) $(LDFLAGS) $^ -o $@  

%.o : %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -f $(TARGET) *.a *.o *.so *~ control_system_unit_test