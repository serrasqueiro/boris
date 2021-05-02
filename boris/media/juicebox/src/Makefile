# juicebox - Makefile

# shlibs/ilibs is, or should be, a soft-link to ilibs
SH_PATH=../../shlibs
#
INCLUDE=$(SH_PATH)/ilibs
INCLUDE1=$(INCLUDE)/iobjs
INCLUDE2=$(INCLUDE)/ilog
# imaudio and imedia -libs
INCLUDE3=$(INCLUDE)/imaudio
INCLUDE4=$(INCLUDE)/imedia
# ilambda add-ons
INCLUDE5=$(INCLUDE)/ilambda

#CC_DEFS=-DDEBUG
#CC_DEFS=-DDEBUG -DDEBUG_MIN
#CC_DEFS=-DDEBUG_OBJ
CC_DEBUG=-g $(CC_DEFS) $(CC_BASIC)

CCINCLUDE1=-I$(INCLUDE1)
CCINCLUDE2=-I$(INCLUDE2)
CCINCLUDE3=-I$(INCLUDE3)
CCINCLUDE4=-I$(INCLUDE4)
CCINCLUDE5=-I$(INCLUDE5)

CFLAGS=-O2 -Wall -Werror
LINK_OPT=-Wl,-rpath,$(INCLUDE1):$(INCLUDE2):$(INCLUDE3):$(INCLUDE4)
CC=g++
CC_OPT=$(CC_DEBUG) \
		$(CCINCLUDE1) \
		$(CCINCLUDE2) \
		$(CCINCLUDE3) \
		$(CCINCLUDE4) \
		$(CCINCLUDE5) \
		-pipe -rdynamic $(LINK_OPT)
LIB=iobjs
LIB_PATH=$(INCLUDE1)
LIB_OBJ=$(LIB_PATH)/lib$(LIB).so

LIB2=ilog
LIB_PATH2=$(INCLUDE2)
LIB_OBJ2=$(LIB_PATH2)/lib$(LIB2).so

LIB3=imaudio
LIB_PATH3=$(INCLUDE3)
LIB_OBJ3=$(LIB_PATH3)/lib$(LIB3).so

LIB4=imedia
LIB_PATH4=$(INCLUDE4)
LIB_OBJ4=$(LIB_PATH4)/lib$(LIB4).so

LIB5=ilambda
LIB_PATH5=$(INCLUDE5)
LIB_OBJ5=$(LIB_PATH5)/lib$(LIB5).so

SH_LIBS=-l$(LIB) \
		-l$(LIB2) \
		-l$(LIB3) \
		-l$(LIB4) \
		-l$(LIB5)

LIBS=-L$(LIB_PATH) \
		-L$(LIB_PATH2) \
		-L$(LIB_PATH3) \
		-L$(LIB_PATH4) \
		-L$(LIB_PATH5) \
		$(SH_LIBS)

OBJ_PATH=.
OBJ_OWN=juicebox.o

OBJS_ALL=\
	auxsfv.o \
	auxvpl.o \
	crosslink.o \
	matches.o \
	smart.o \
	$(OBJ_OWN)

STRIP_OPT=-s


EXECNAME=juicebox

.PHONY: goal target clean clearall \
	half_static iobjall


goal: $(EXECNAME) $(LIB_OBJ) $(LIB_OBJ2) $(LIB_OBJ3) $(LIB_OBJ4) $(LIB_OBJ5)

$(EXECNAME): $(OBJS_ALL) $(LIB_OBJ2) $(LIB_OBJ3) $(LIB_OBJ4) $(LIB_OBJ5)
	$(CC) $(CC_OPT) $(OBJS_ALL) $(CFLAGS) $(LIBS) -o $(EXECNAME)

$(LIB_OBJ2):
	@echo "Need to rebuild lib$(LIB2)"
	cd $(LIB_PATH2) && $(MAKE)

$(LIB_OBJ3):
	@echo "Need to rebuild lib$(LIB3)"
	cd $(LIB_PATH3) && $(MAKE)

$(LIB_OBJ4):
	@echo "Need to rebuild lib$(LIB4)"
	cd $(LIB_PATH4) && $(MAKE)

$(LIB_OBJ5):
	@echo "Need to rebuild lib$(LIB5)"
	cd $(LIB_PATH5) && $(MAKE)

target: goal
	cd $(LIB_PATH5) && $(MAKE) $@
	strip $(EXECNAME)

half_static: target
	rm -f $(INCLUDE)/*/debug
	$(CC) $(STRIP_OPT) *.o \
		$(INCLUDE2)/*.o \
		$(INCLUDE3)/*.o \
		$(INCLUDE4)/*.o \
		$(INCLUDE5)/*.o \
		-L$(LIB_PATH) -l$(LIB) \
		-o $(EXECNAME)_static

iobjall: juicebox
	g++ *.o -L../../shlibs/ilibs -liobjall -s -o juicebox

# Below, the implicit rule for all C++ files!
.cpp.o:
	@echo "Compiling $< : obj is $@"
	$(CC) $(CC_OPT) $(CFLAGS) -o $(OBJ_PATH)/$@ -c $<

clean:
	rm -f core $(OBJS_ALL) *~ *%

clearall: clean
	cd $(LIB_PATH) && $(MAKE) clean
	rm -f $(EXECNAME)

