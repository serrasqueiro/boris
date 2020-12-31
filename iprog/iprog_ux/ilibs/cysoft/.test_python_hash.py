#!/usr/bin/python

from time         import time, ctime, localtime, strftime
from socket       import socket, AF_UNIX, SOCK_STREAM, error as SocketError, gethostname
from os           import remove, rename, chmod, chown, getuid, getpid, isatty
from pwd          import getpwnam
from grp          import getgrnam
from os.path      import join, exists
from sys          import version as PyVersion, stdout, stderr, exit, exc_info
from signal       import signal, SIGTERM, SIGHUP, SIGUSR1, SIG_IGN, SIG_DFL
from syslog       import openlog, syslog, LOG_NOTICE, LOG_WARNING, LOG_ERR
from select       import select
from inspect      import getargspec


### Ensure that we can run this program
if PyVersion < "2.3":
    stderr.write("This program requires Python 2.3 or newer\n")
    exit(1)


def saveToFile (datafile, dictionary, perm=0600):
    try:
        fp = file(datafile, 'w')

        chmod(datafile, perm)
        
        for (section, subdict) in dictionary.items():
            fp.write("[%s]\n"%section)

            for (key, value) in subdict.items():
                if type(value) in (list, tuple):
                    value = " ".join(map(str, value))
                fp.write("%s = %s\n"%(key, value))

            fp.write("\n")

        fp.close()

    except IOError, e:
        raise RunException, "Cannot write to %s: %s"%(datafile, e[1])

    except OSError, e:
        raise RunException, \
              "Cannot set mode 0%o on %s: %s"%(perm, datafile, e[1])



def do_list (options, socket):
    if not config[DATA][TRIPLETFILE] or not config[DATA][SAVETRIPLETS]:
        raise CommandError, "Original triplet data is not retained."

    do_save()

    try:
        infile = file(config[DATA][TRIPLETFILE], "r")
        error = listTriplets(infile, socket, options)
        infile.close()

    except IOError, e:
        raise CommandError, \
              "Cannot read from '%s': %s\n"%(config[DATA][TRIPLETFILE], e[1])
    

def do_clear (options):
    for listkey in (options or datatypes):
        if listkey in data:
            data[listkey].clear()
            data[STATS][listkey] = 0
        else:
            raise CommandError, "Invalid list: '%s'"%listkey

    if not options:
        data[STATS][START] = int(time())

    return "data and statistics cleared"


def do_reload ():
    do_save()
    loadConfigAndData()
    return "configuration and data reloaded"


def do_save ():
    now = int(time())
    expireKeys(now)

    ### Save data hashes and timestamps
    saveData(config[DATA][STATEFILE])

    ### Save unhashed triplets
    if newTriplets:
        syncTriplets(config[DATA][TRIPLETFILE])

    data[STATS][LASTSAVE] = now

    return "greylistd data has been saved"



def nodata ():
    raise CommandError, "No data received"



stderr.write("Hi!\n");
out=stdout;

num=123;

s="a";
h=hash( s );
out.write( "%s\t" %s );
out.write( "%d\n" %h );

s="";
h=hash( s );
out.write( "1d\t" );
out.write( "%d\n" %h );

s="";
h=hash( s );
out.write( "2d\t" );
out.write( "%d\n" %h );

s="";
h=hash( s );
out.write( "3d\t" );
print( "%d 0x %x\n" %(h,h) );



