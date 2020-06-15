
     Enclosed are a few utilities which I like to use along with build
procedures to generate them.  Build is setup for NMAKE: for instance
NMAKE -F NEW.MAK<cr> will build new.exe.

    ascii [/nn] filename    - Dumps ascii strings found in specified file.
    ddir [template]         - Prints current directory contents, sorted
                              in double width.
    new [/n] [template]     - Lists (in ddir format) all files written
                              in the last 'n' days.  Days are calculated
                              from 6 A.M.

     Note that the 'template' in 'ddir' and 'new' are very picky when
used in the 'DosFindFirst' call.  I have attempted to fake the input
argument to more follow the Dos-OS/2 'dir' command syntax.  This attempt
on my part is by no means bullet-proof, but problems only show up when
you try to abbreviate pathnames (i.e. using 'new \os2' to mean 'new
\os2\*.*').  The error diagnostics should give you sufficient
information if you run afoul of the limitations.

     Also, the output of NEW and DDIR has no 'newlines': in the interest
of squeezing as much information as is possible on each line, 80
characters are output and the default terminal driver's automatic wrap
is relied on to get to the next line.  The only problem is if the output
is redirected to a file, there will be NO CR/LF anywhere in the file.
This can be corrected by the 'wrap' routine included.  There is no make
file for 'wrap', but the instructions for build are included in the
header of the source file.  Wrap is meant to be part of a pipe: (e.g.
ddir | wrap >outfile.lst.)

     ASCII is generated as a 'bound' utility and will run in either
Dos or OS/2 mode.  DDIR and NEW are not because of the way in which
DosFindFirst is used.


     Included in this source code are MSC 6.0 samples of:

DosFindFirst
DosGetDateTime
time
localtime
DosQFSInfo
strtok
qsort
setvbuf
perror

