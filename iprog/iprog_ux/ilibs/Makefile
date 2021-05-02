# libraries, Makefile

# VPATH=.


.PHONY: goal target strip clean

OBJS=\
	iobjs/libiobjs.so \
	ilog/libilog.so \
	imedia/libimedia.so

goal: $(OBJS)


target: strip
	cp -a */*.so /usr/local/bin/

strip: goal libiobjall.so
	strip $(OBJS)
	strip libiobjall.so

clean:
	rm -f $(OBJS)

iobjs/libiobjs.so:
	cd iobjs && make

ilog/libilog.so:
	cd ilog && make

imedia/libimedia.so:
	cd imedia && make

libiobjall.so: iobjs/libiobjs.so ilog/libilog.so imedia/libimedia.so
	g++ -pipe -shared -fPIC -o libiobjall.so iobjs/*.o ilog/*.o imaudio/*.o imedia/*.o ilambda/*.o
