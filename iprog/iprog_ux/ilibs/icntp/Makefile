# icntp Makefile
#
# The CNTP Library!

INCLUDE=../iobjs

SHLIB_LD		= gcc -pipe -shared
SHLIB_CFLAGS		= -fPIC

### usual defs
#CC_DEBUG_DEF=-DDEBUG
#CC_DEBUG_DEF=-DDEBUG_MIN
CC_DEBUG_DEF=
CC_DEFS=$(CC_DEF) $(CC_BASIC)
CC_DEBUG=-g $(CC_DEFS) $(CC_DEBUG_DEF)
CCINCLUDE=-I$(INCLUDE)
#CFLAGS=-O2 -Wall $(SHLIB_CFLAGS)
CFLAGS=-O2 -Wall -Werror -Wdeprecated -Wwrite-strings $(SHLIB_CFLAGS)
CC=g++
CC_OPT=$(CC_DEBUG) $(CCINCLUDE)
LIB_PATH=$(INCLUDE)

OBJ_PATH=.

LIB_ME=icntp
LIB_OWN=lib$(LIB_ME).so

LINK_LIB=-l$(LIB_ME) -L. -liobjs -L$(LIB_PATH)
LINK_CC=-pipe -rdynamic
LINK_OPT=$(LINK_CC) -Wl,-rpath,.:$(INCLUDE) $(LINK_LIB)

OBJS_ALL=\
	iCIp.o \
	iCntpRawDate.o \
	iCntpRawResp.o \
	iCntproto.o \
	iNetPair.o


.PHONY: goal target clean clearall


goal: $(LIB_OWN)
	cd $(LIB_PATH) && $(MAKE)

$(LIB_OWN): $(OBJS_ALL)
	$(SHLIB_LD) $(SHLIB_CFLAGS) -o $(LIB_OWN) $(OBJS_ALL)

debug: $(LIB_OWN) debug.o
	$(CC) $(CC_OPT) $(CFLAGS) $(LINK_OPT) $(OBJS_ALL) debug.o -o debug

# Below, the implicit rule for all C++ files!
.cpp.o:
	@echo Compiling $< : obj is $@
	$(CC) $(CC_OPT) $(CFLAGS) -o $(OBJ_PATH)/$@ -c $<

target: goal
	cd $(INCLUDE) && $(MAKE) $@
	strip $(LIB_OWN)

clean:
	rm -f core *.o *~ *%

clearall: clean
	rm -f *.a *.so

