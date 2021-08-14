#!/bin/sh
#
# Makes scntp (single shared-library)

# You can use any args to specify g++ options, e.g. "-DDEBUG"

MAIN_LIB=../../ilibs/iobjs
LINK_LIB="-liobjs -L$MAIN_LIB"

for LIB_PATH in \
	../../ilibs/ilog \
	../../ilibs/icntp \
	../src \
	; do
    for F in ${LIB_PATH}/*.cpp ${LIB_PATH}/*.h ; do
    	[ ! -f "$F" ] && continue
	ln -s $F
    done
done

# Need to make libgobj
P=`pwd`
cd $MAIN_LIB && make
RES=$?
[ "$P" != "" ] && cd $P
if [ $RES != 0 ]; then
	echo "::: This target is half-static, therefore you need to compile libgobj first"
	exit 1
fi


for X in *.cpp ; do
    [ "$X" = "*.cpp" ] && continue
    [ "$X" = "main.cpp" ] && continue
    [ "$X" = "iNtpStats.cpp" ] && continue

    DEFS="$*"
    OBJ=`echo $X | sed 's/\.cpp/.o/'`
    [ "$OBJ" = "" ] && continue
    I="-I$MAIN_LIB"
    LINE="g++ -Wall $DEFS $I -c $X -o $OBJ"
    [ -f $OBJ ] && continue
    echo "Compiling: $LINE"
    $LINE
    if [ $? != 0 ]; then
	echo "Failed"
	exit 1
    fi
done

EXE_NAME=scntp_halfstatic
CMD="g++ *.o $LINK_LIB -o $EXE_NAME"
echo "Linking: $CMD"
$CMD
RES=$?
echo "Done: $EXE_NAME"

strip $EXE_NAME
chmod 6755 $EXE_NAME
chown root.root $EXE_NAME
ln $EXE_NAME scntp


exit $RES

