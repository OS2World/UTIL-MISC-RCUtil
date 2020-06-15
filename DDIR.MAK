PROJ = DDIR

CC  = cl
CFLAGS_G  = /AL /W2 /G2 /Zp /BATCH
CFLAGS_R  = /Otileg /Gs
MAPFILE_R  = NUL
LFLAGS_G  =  /NOI /NOE  /BATCH
LFLAGS_R  = /EXE /FAR /PACKC /PACKD /PMTYPE:VIO
LINKER  = link
LRF  = echo > NUL
LLIBS_R  =  /NOD:LLIBCE LLIBCEP

OBJS  = ddir.obj errdiag.obj

all: $(PROJ).exe

.SUFFIXES:
.SUFFIXES: .obj .c

ddir.obj : ddir.c


$(PROJ).exe : $(OBJS)
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_R)
$(LLIBS_G: = +^
) +
$(LLIBS_R: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_R);
<<
        $(LINKER) @$(PROJ).lrf

.c.obj :
        $(CC) /c $(CFLAGS_G) $(CFLAGS_R) /Fo$@ $<
