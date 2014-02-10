tuscfmt
=======

tusc is a HP-UX command which used to track HP-UX system calls.

This utility (tuscfmt) could format tusc log to convert UNIX timestamp to human readable timestamp

eg:

When we run tusc command with "-T" argument, but forget to specifiy time format, we will get below output

--sniped from a tusc logfile

( Attached to process 2766 ("ora_lgwr_R11203") [64-bit] )

1388730764.600953 [2766] pw_wait(0x9fffffffffffb290) ..... [sleeping]

1388730765.780235 [2766] getrusage(RUSAGE_SELF, 0x9fffffffffffbc40) = 0

1388730765.780378 [2766] getrusage(RUSAGE_SELF, 0x9fffffffffffbc10) = 0

--end 

1388730764.600953 is a UNIX timestamp and hard to interpret.

This tool is used to convert it to readable time format: 2014-01-03 14:32:44.600953

How to compile it:
=======

--64bit

$ gcc -m64 -o ./tuscfmt ./tuscfmt.c

--32bit

$ gcc -m32 -o ./tuscfmt ./tuscfmt.c


How to use it:
=======

eg
<<<<<<< HEAD
$ ./tuscfmt /root/C/tusc/tusc.log ./tusc.fmt.log


Substitute AWR command
=======

In fact, we could use below AWR command to do the same thing as this utility

awk -F. --re-interval '$1~/^[0-9]{10}$/ && $2~/^[0-9]{6}\s+*/ {$1=strftime("%F %T",$1)}1' OFS=. tusc.log

