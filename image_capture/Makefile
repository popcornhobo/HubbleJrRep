#http://stackoverflow.com/questions/14884126/build-so-file-from-c-file-using-gcc-command-line

ver = debug
platform = armv6

CC = g++
#INCLIB = /usr/local/include
#LDLIB = /usr/local/lib
OPENCV = $(shell pkg-config --cflags opencv) $(shell pkg-config --libs opencv)
USB =  -I../libusb/include  -L../libusb/$(platform) -lusb-1.0  



LIBSPATH = -L../lib/$(platform) -I../include



ifeq ($(ver), debug)
DEFS = -D_LIN -D_DEBUG 
CFLAGS = -g  -I $(INCLIB) -L $(LDLIB) $(DEFS) $(COMMON) $(LIBSPATH) -lpthread  $(USB) -DGLIBC_20
else
DEFS = -D_LIN 
CFLAGS =  -O3 -I $(INCLIB) -L $(LDLIB) $(DEFS) $(COMMON) $(LIBSPATH)  -lpthread  $(USB) -DGLIBC_20
endif

ifeq ($(platform), mac32)
CC = g++
CFLAGS += -D_MAC -m32
OPENCV = -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_ts -lopencv_video -lopencv_videostab -I/usr/local/include/opencv
endif

ifeq ($(platform), mac64)
CC = g++
CFLAGS += -D_MAC -m64
OPENCV = -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_ts -lopencv_video -lopencv_videostab -I/usr/local/include/opencv
endif

ifeq ($(platform), mac)
CC = g++
CFLAGS += -D_MAC -arch i386 -arch x86_64
OPENCV = -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_ts -lopencv_video -lopencv_videostab -I/usr/local/include/opencv
endif

ifeq ($(platform), x86)
CFLAGS += -m32
CFLAGS += -lrt
endif


ifeq ($(platform), x64)
CFLAGS += -m64
CFLAGS += -lrt
endif

ifeq ($(platform), armv5)
CC = arm-none-linux-gnueabi-g++
AR= arm-nonelinux-gnueabi-ar
CFLAGS += -march=armv5
CFLAGS += -lrt
endif


ifeq ($(platform), armv6)
CC = arm-linux-gnueabi-g++
AR= arm-linux-gnueabi-ar
CFLAGS += -march=armv6
CFLAGS += -lrt

OPENCV = -lopencv_calib3d -lopencv_contrib -lopencv_core -lopencv_features2d -lopencv_flann -lopencv_highgui -lopencv_imgproc -lopencv_legacy -lopencv_ml -lopencv_objdetect -lopencv_photo -lopencv_stitching -lopencv_video -lopencv_videostab -I/usr/local/include/opencv

endif

ifeq ($(platform), armv7)
CC = arm-linux-gnueabihf-g++
AR= arm-linux-gnueabihf-ar
CFLAGS += -march=armv7 -mcpu=cortex-m3 -mthumb
endif
#ifeq ($(platform), armhf)
#CC = arm-linux-gnueabihf-g++
#AR= arm-linux-gnueabihf-ar
#CFLAGS += -march=armv5
#LDLIB += -lrt
#endif


all:test_console test_gui1_video test_gui1_snap test_gui2_snap
test_console: main_console.cpp
	$(CC) main_console.cpp -o test_console $(CFLAGS) $(OPENCV) -lASICamera
	cp test_console bin/$(platform)/
	
libZWO_camera_interface.so: ZWO_camera_interface.o
	$(CC) -shared -o $@ $^ $(CFLAGS) $(OPENCV) -lASICamera

ZWO_camera_interface.o: ZWO_camera_interface.c
	$(CC) $(CFLAGS) -c -fPIC -Wall $< -o $@ $(OPENCV) -lASICamera
	
camera: camera.c
	$(CC) camera.c -o camera $(CFLAGS) $(OPENCV) -lASICamera
	cp test_console bin/$(platform)/

camera_capture: ZWO_camera_interface.c
	$(CC) ZWO_camera_interface.c -o camera_capture $(CFLAGS) $(OPENCV) -lASICamera
	
test_gui1_video:main_SDK1_video.cpp
	$(CC)  main_SDK1_video.cpp -o test_gui1_video $(CFLAGS) $(OPENCV) -lASICamera
	cp test_gui1_video bin/$(platform)/
test_gui1_snap:main_SDK1_snap.cpp
	$(CC)  main_SDK1_snap.cpp -o test_gui1_snap $(CFLAGS) $(OPENCV) -lASICamera
	cp test_gui1_snap bin/$(platform)/
test_gui2_snap:main_SDK2_snap.cpp
	$(CC)  main_SDK2_snap.cpp -o test_gui2_snap $(CFLAGS) $(OPENCV) -lASICamera2
	cp test_gui2_snap bin/$(platform)/

#%.o : %.c
#	$(CC) $(CFLAGS) -fPIC -c $< -o $(CFLAGS) $(OPENCV) -lASICamera$@

clean:
	rm -f test_gui test_console allsky
#pkg-config libusb-1.0 --cflags --libs
#pkg-config opencv --cflags --libs

