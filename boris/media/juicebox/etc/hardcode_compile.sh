#!/bin/sh

g++ -g   -I../../shlibs/ilibs/iobjs -I../../shlibs/ilibs/ilog -I../../shlibs/ilibs/imaudio -I../../shlibs/ilibs/imedia -pipe -rdynamic -Wl,-rpath,../../shlibs/ilibs/iobjs:../../shlibs/ilibs/ilog:../../shlibs/ilibs/imaudio:../../shlibs/ilibs/imedia crosslink.o matches.o juicebox.o -O2 -Wall -L../../shlibs/ilibs/iobjs -L../../shlibs/ilibs/ilog -L../../shlibs/ilibs/imaudio -L../../shlibs/ilibs/imedia -liobjs ../../shlibs/ilibs/ilog/*.o ../../shlibs/ilibs/imaudio/*.o ../../shlibs/ilibs/imedia/*.o  -o juicebox

