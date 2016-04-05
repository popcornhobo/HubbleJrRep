#!/bin/bash

echo "Copying old image list"
cp list.txt list_old.txt
echo "Connecting to Pi"

ssh pi@192.168.1.10 ls /home/pi/images > list.txt

#echo "Listing images"
#cat list.txt

echo "Checking for new images"
NewImage=`diff list_old.txt list.txt | grep -o "[a-z,0-9]*\.jpg$"`

if [ "$NewImage" = "" ]; then
	echo "No new images"
else
	echo "$NewImage" 
	echo "Copying new image"
	`scp pi@192.168.1.10:/home/pi/images/$NewImage ./`
fi

echo "Script Finished"
