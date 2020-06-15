PROJ = ASCII

CC  = cl
CFLAGS  = /Ox /Lp /Fb /F 2000 /AL /G2 /BATCH

all: $(PROJ).exe

.SUFFIXES:
.SUFFIXES: .exe .c

.c.exe :
        $(CC) $(CFLAGS) $<
