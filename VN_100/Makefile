#
CROSS_COMPILE = arm-linux-gnueabihf-
CFLAGS = -static -g -Wall  -I${SOCEDS_DEST_ROOT}/ip/altera/hps/altera_hps/hwlib/include
LDFLAGS =  -g -Wall -lm 
CC = $(CROSS_COMPILE)gcc
ARCH= arm

all: VN_lib.so VN_user.so VN_math.so soft_spi.so VN100.so 

VN_lib.so: VN_lib.o
	$(CC) $(LDFLAGS) $^ -shared -o $@  
VN_user.so: VN_user.o
	$(CC) $(LDFLAGS) $^ -shared -o $@  
VN_math.so: VN_math.o
	$(CC) $(LDFLAGS) $^ -shared -o $@  
VN100.so: VN100.o
	$(CC) $(LDFLAGS) $^ -shared -o $@  
soft_spi.so: soft_spi.o
	$(CC) $(LDFLAGS) -fPIC $^ -shared -o $@  
	
%.o : %.c
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

clean:
	rm -f $(TARGET) *.a *.o *.so *~ 