# ilambda Makefile
#
# The ilambda Library!

INCLUDE=../iobjs
INCLUDE_L=../ilog	# due to lib_ilog.h include

SHLIB_LD		= gcc -pipe -shared
SHLIB_CFLAGS		= -fPIC

### usual defs
#CC_DEBUG_DEF=-DDEBUG
#CC_DEBUG_DEF=-DDEBUG_MIN
CC_DEBUG_DEF=
CC_DEFS=$(CC_DEF) $(CC_BASIC)
CC_DEBUG=-g $(CC_DEFS) $(CC_DEBUG_DEF)
CCINCLUDE=-I$(INCLUDE) -I$(INCLUDE_L)
#CFLAGS=-O2 -Wall $(SHLIB_CFLAGS)
CFLAGS=-O2 -Wall -Werror -Wdeprecated -Wwrite-strings $(SHLIB_CFLAGS)
CC=g++
CC_OPT=$(CC_DEBUG) $(CCINCLUDE)
LIB_PATH=$(INCLUDE)

OBJ_PATH=.

LIB_ME=ilambda
LIB_OWN=lib$(LIB_ME).so

LINK_LIB=-l$(LIB_ME) -L. -liobjs -L$(LIB_PATH) -L$(INCLUDE_L)
LINK_CC=-pipe -rdynamic
LINK_OPT=$(LINK_CC) -Wl,-rpath,.:$(INCLUDE):$(INCLUDE_L) $(LINK_LIB)

OBJS_ALL=\
	dba.o \
	fcrc32.o \
	icrc32.o \
	suma.o \
	iJson.o \
	iID3vx.o \
	iLambdaNetPair.o


.PHONY: goal target clean clearall


goal: $(LIB_OWN) $(LIB_PATH)/libiobjs.so
	cd $(LIB_PATH) && $(MAKE)

$(LIB_OWN): $(OBJS_ALL)
	$(SHLIB_LD) $(SHLIB_CFLAGS) -o $(LIB_OWN) $(OBJS_ALL)

$(LIB_PATH)/libiobjs.so:
	cd $(LIB_PATH) && $(MAKE)


debug: $(LIB_OWN) ../ilog/libilog.so debug.o iJson.o
	$(CC) $(CC_OPT) -lilog $(CFLAGS) $(LINK_OPT) $(OBJS_ALL) debug.o -o debug

../ilog/libilog.so:
	cd ../ilog && $(MAKE)

# Below, the implicit rule for all C++ files!
.cpp.o:
	@echo Compiling $< : obj is $@
	$(CC) $(CC_OPT) $(CFLAGS) -o $(OBJ_PATH)/$@ -c $<

target: goal
	cd $(INCLUDE) && $(MAKE) $@
	strip $(LIB_OWN)

clean:
	rm -f core *.o *~ *% libilambda.so debug

clearall: clean
	rm -f *.a *.so

