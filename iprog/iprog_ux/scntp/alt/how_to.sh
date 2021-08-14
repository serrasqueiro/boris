#!/bin/sh

# run, as root, for openSUSE:
#	zypper install gcc-c++

[ "$DEBUG" ] && g++ -Wall -Werror -g current_timezone.cpp -I/home/henrique/anaceo/reflect/sockets/tcpori/reflect/src -DDEBUG

# '-s' strips debug info:
[ "$DEBUG" = "" ] && g++ -Wall -Werror -s current_timezone.cpp -o current_timezone

for X in Europe America Asia Atlantic ; do
	echo -n $X :
	if [ -d zoneinfo/$X ]; then
		echo "... skipped"
		continue
	fi
	ln -sf /usr/share/zoneinfo/$X zoneinfo/$X
	echo "(created)"
done

