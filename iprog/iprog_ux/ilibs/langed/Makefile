# langed Makefile

INCLUDE=../iobjs
INCLUDE_U=../imedia

SHLIB_LD		= gcc -pipe -shared
SHLIB_CFLAGS		= -fPIC

### usual defs
#CC_DEBUG_DEF=-DDEBUG
#CC_DEBUG_DEF=-DDEBUG_MIN
CC_DEBUG_DEF=
CC_DEFS=$(CC_DEF) $(CC_BASIC)
CC_DEBUG=-g $(CC_DEFS) $(CC_DEBUG_DEF)
CCINCLUDE=-I$(INCLUDE) \
		-I$(INCLUDE_U)
CFLAGS=-O2 -Wall $(SHLIB_CFLAGS)
CC=g++
CC_OPT=$(CC_DEBUG) $(CCINCLUDE)
LIB_PATH=$(INCLUDE)

LIB_ME=langed
LIB_OWN=lib$(LIB_ME).so

LIB_U=imedia
LIB_U_PATH=$(LIB_PATH)/../$(LIB_U)
LIB_U_WHERE=$(LIB_U_PATH)/lib$(LIB_U).so

LIBS_OTHER=-l$(LIB_U)

LINK_LIB=-l$(LIB_ME) $(LIBS_OTHER) \
		-L. -L$(LIB_PATH) \
		-L$(INCLUDE_U) \
		-liobjs \
		-l$(LIB_U)
LINK_CC=-pipe -rdynamic
LINK_OPT=$(LINK_CC) -Wl,-rpath,.:$(INCLUDE) $(LINK_LIB)


OBJ_PATH=.

OBJS_ALL=\
	langed.o \
	wordunicode.o


.PHONY: goal target clean clearall


goal: $(LIB_OWN) $(LIB_U_WHERE)
	cd $(LIB_U_PATH) && $(MAKE)

$(LIB_OWN): $(OBJS_ALL)
	cd $(LIB_PATH) && $(MAKE)
	$(SHLIB_LD) $(SHLIB_CFLAGS) -o $(LIB_OWN) $(OBJS_ALL)

$(LIB_U_WHERE):
	cd $(LIB_U_PATH) && $(MAKE)

debug: $(LIB_OWN) $(LIB_U_WHERE) $(OBJS_ALL) debug.o
	echo ::: $(LIB_U_WHERE)
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

