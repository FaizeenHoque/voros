#!/bin/bash

make clean
make 
./create_iso.sh
./create_disk_image.sh