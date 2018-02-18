#!/bin/bash

#Checks wether a specified file is a valid P5 pmg image
fname=$1

type=$(head -c2 $fname)
if [ $type = "P5" ]
then
    exit 0
else
    exit 1	 
fi
	

