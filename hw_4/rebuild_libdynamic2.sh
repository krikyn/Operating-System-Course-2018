#!/bin/bash
echo "Start rebuild libdynamic2.so"
gcc -c -fPIC libdynamic2.c
echo "..."
gcc -shared -o libdynamic2.so libdynamic2.o
echo "Success"
	