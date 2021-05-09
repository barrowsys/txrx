#!/bin/bash
###############################################################
# THIS FILE IS LICENSED UNDER MIT                             #
# THE FOLLOWING MESSAGE IS NOT A LICENSE                      #
#                                                             #
# <barrow@tilde.team> wrote this file.                        #
# by reading this message, you are reading "TRANS RIGHTS".    #
# this file and the content within it is the queer agenda.    #
# if we meet some day, and you think this stuff is worth it,  #
# you can buy me a beer, tea, or something stronger.          #
# -Ezra Barrow                                                #
#                                                             #
# This script deletes all the NanoNet.h files in subdirs      #
# and re-hardlinks them from the main file.                   #
###############################################################

DIRECTORIES=`find . -mindepth 2 -maxdepth 2 -name NanoNet.h -exec dirname {} \; -delete` #-exec rm {} \;
for DIR in $DIRECTORIES; do
	ln NanoNet.h $DIR
done

