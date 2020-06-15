
/********************************************************/
/*                                                      */
/*  Function: ddir                                      */
/*                                                      */
/*   Purpose: Prints current directory contents, sorted */
/*              in double width.                        */
/*                                                      */
/*   To build: CL /F 2000 ddir.c                        */
/*                                                      */
/*   To use:  ddir template                             */
/*                                                      */
/*      File: ddir.c                                    */
/*                                                      */
/********************************************************/

        /* Include system calls/library routines/macros */
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSDATETIME
#include <os2.h>
#include <process.h>
#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "errdiag.h"

        /* Define constants */
#define true -1
#define false 0
#define ALLFILETYPES  (FILE_READONLY|FILE_HIDDEN|FILE_SYSTEM|FILE_DIRECTORY)
#define errex(sp) {printf(sp); exit(0);}
#define MAXFILES 200

typedef struct _MYDIRSTRCT {            // Setup for directory output
  char sys_desig;                       // 'S' if a system file, else '_'
  char hid_desig;                       // 'H' if hidden, else '_'
  char rdo_desig;                       // 'R' if read-only, else '_'
  char dir_desig;                       // '/' if a directory, else '_'
  char fname[8];                        // 8 char filename
  char decpt;                           // A '.' if there is an extention
  char ext[3];                          // 3 char extension
  char size[8];                         // 8 digit # bytes in file
  char sp1;                             // space for formatting
  char date[6];                         // 6 char YYMMDD date
  char sp2;                             // space for formatting
  char time[4];                         // 4 char military time
  char written_later;                   // 'L' if file written to more than one
                                        //      day after it's creation date
  char recent_write;                    // 'R' if file written to in last week
  char write;                           // 'W' if either of above true
  char recent_access;                   // 'A' if accessed in last week
  char term;                            // NULL terminator
} MYDIRSTRCT;

int fnamecmp(MYDIRSTRCT *buf1, MYDIRSTRCT *buf2);

        /* Define general variables */
ULONG ii;
ULONG total_bytes=0;                    // Total bytes in all files
USHORT api_err;                         // Generic error returns
PSZ FileSpec;                           // Pointer to a file spec
HDIR hdir=HDIR_CREATE;                  // Create a new search handle
USHORT usSearchCount = 25*MAXFILES;     // Setup to have error if too many files
FILEFINDBUF filefindbuf[MAXFILES];      // Receives file info
FSALLOCATE fsallocate;                  // Receives disk allocation info
char *allfiles="*.*";                   // Default search spec (all files)
char *nulldummy="";                     // Dummy NULL string
char BuildTemp[60];                     // An array to build a template into

MYDIRSTRCT build_array, *dest_ptr;
FILEFINDBUF *src_ptr;
DATETIME datetime;

        /* Define file access structures */

main(argc, argv)
int argc;
char *argv[];
  {
  if (argc > 2) errex("\n\nProper usage: 'ddir [template]'\n\n");
  if (argc > 1)
  {
    char lastchar;

    FileSpec=argv[1];
    if (*FileSpec == '.')               // Starts with '.'
    {
      if ( (*(FileSpec+1) != '.') && (*(FileSpec+1) != '\0') )
      {                                 // Is not one of the 'psuedo dirs'
        strcpy(&BuildTemp[1], argv[1]); // Prepend with '*'
        BuildTemp[0] = '*';
      }
      else strcpy(&BuildTemp[0], argv[1]);
    }
    else strcpy(&BuildTemp[0], argv[1]);// One way or another, string in BuildTemp
    FileSpec = BuildTemp;
    if ( (lastchar=BuildTemp[strlen(BuildTemp)-1]) == '.' )
      strcat(BuildTemp, "\\*.*");
    else if ( (lastchar == '\\') || (lastchar == ':') )
      strcat(BuildTemp, "*.*");
  }
  else FileSpec=allfiles;

  api_err = DosFindFirst(FileSpec, &hdir, ALLFILETYPES, filefindbuf,
                                    sizeof(filefindbuf), &usSearchCount, 0L);
  if ( (api_err == ERROR_FILE_NOT_FOUND) || (api_err == ERROR_NO_MORE_FILES) )
  {
    printf("Aint got no %s\n", FileSpec);
    exit(0);
  }
  ERRCHK(api_err);                      // Check for errors & print

  api_err=DosGetDateTime(&datetime);
  ERRCHK(api_err);                      // Check for errors & print
  datetime.year -= 80;                  // Get to same base as DosFindFirst
  datetime.year %= 100;                 // Take mod 100 of year
                                        // Get disk usage of current disk
  api_err=DosQFSInfo(0, FSIL_ALLOC, (PBYTE)&fsallocate, sizeof(fsallocate));
  ERRCHK(api_err);                      // Check for errors & print

  ii = sizeof(filefindbuf) - sizeof(build_array);
  dest_ptr = (MYDIRSTRCT *)             // Init the end pointer
     ( (char *)filefindbuf + ii );
  src_ptr = (FILEFINDBUF *)filefindbuf; // Init the start pointer

  build_array.term = '\0';              // Only needs done once
  for (ii=0; ii<usSearchCount; ii++)
  {
    char *fnameptr, *extptr;
    FDATE *dispdate;
    FTIME *disptime;
    long recent_write, later_write, recent_access;

    fnameptr = strtok(src_ptr->achName, "."); // NULL term fname, remove '.'
    extptr = strtok(NULL, ".");         // Get extension string
    fnameptr = src_ptr->achName;        // Guarantee filename for '.' & '..'
    if (extptr == NULL)
      extptr=nulldummy;                 // Guarantee NULL ext available

    if (src_ptr->fdateCreation.year == 0)
    {
      dispdate = &src_ptr->fdateLastWrite;  // No create date, use last write
      disptime = &src_ptr->ftimeLastWrite;
    }
    else
    {
      dispdate = &src_ptr->fdateCreation;   // Create date exists, use it
      disptime = &src_ptr->ftimeCreation;
    }

    sprintf( (char *)&build_array, "%c%c%c%c%-8.8s %-3.3s%8lu ",
              ((src_ptr->attrFile & FILE_SYSTEM)    ? 'S' : '_'),
              ((src_ptr->attrFile & FILE_HIDDEN)    ? 'H' : '_'),
              ((src_ptr->attrFile & FILE_READONLY)  ? 'R' : '_'),
              ((src_ptr->attrFile & FILE_DIRECTORY) ? '/' : '_'),
              fnameptr, extptr, src_ptr->cbFile);

    sprintf( (char *)build_array.date, "%02d%02d%02d %02d%02d",
              (dispdate->year+80)%100, dispdate->month, dispdate->day,
              disptime->hours, disptime->minutes);

    total_bytes += src_ptr->cbFile;     // Add # bytes in this file to total

    if (extptr == nulldummy)            // Did we need to dummy out extension?
      build_array.decpt = ' ';          // No extension, no dot
    else build_array.decpt = '.';       // Put a dot before extension
    build_array.write = ' ';            // See if file written in last week
    recent_write = 365*(datetime.year - src_ptr->fdateLastWrite.year) +
                    30*(datetime.month - src_ptr->fdateLastWrite.month) +
                       (datetime.day - src_ptr->fdateLastWrite.day);
    if (recent_write <= 7)
    {
      build_array.write='W';
      build_array.recent_write='R';
    }
    else build_array.recent_write=' ';

    if (src_ptr->fdateCreation.year != 0)
    {                                   // See if file written after creation
      later_write = 365*(src_ptr->fdateLastWrite.year - src_ptr->fdateCreation.year) +
                     30*(src_ptr->fdateLastWrite.month - src_ptr->fdateCreation.month) +
                        (src_ptr->fdateLastWrite.day - src_ptr->fdateCreation.day);
    }
    if ( (src_ptr->fdateCreation.year != 0) && (later_write > 1) )
    {
      build_array.write='W';
      build_array.written_later='L';
    }
    else build_array.written_later=' ';

    if (src_ptr->fdateLastAccess.year != 0)
    {                                   // See if file accessed in last week
      recent_access = 365*(datetime.year - src_ptr->fdateLastAccess.year) +
                       30*(datetime.month - src_ptr->fdateLastAccess.month) +
                          (datetime.day - src_ptr->fdateLastAccess.day);
    }
    if ( (src_ptr->fdateLastAccess.year != 0) && (recent_access <= 7) )
          build_array.recent_access='A';
    else  build_array.recent_access=' ';

    *dest_ptr-- = build_array;

        // Point to next file in buffer
    src_ptr = (FILEFINDBUF *)&src_ptr->achName[src_ptr->cchName+1];
  }
  dest_ptr++;                           // Point to last item converted
                                        // Sort based on flags and fname/ext
  qsort((void *)dest_ptr, usSearchCount, sizeof(MYDIRSTRCT), fnamecmp );

  for (ii=0; ii<usSearchCount; ii++)
  {
    fputs((char *)dest_ptr++, stdout);
//    if (ii&1) fputc('\n', stdout);
  }
  if (ii&1) fputc('\n', stdout);

  printf("Total of %d files using %lu bytes. %lu bytes free (on current disk)\n",
    usSearchCount, total_bytes,
            fsallocate.cSectorUnit*fsallocate.cUnitAvail*fsallocate.cbSector);
}

int fnamecmp(MYDIRSTRCT *buf1, MYDIRSTRCT *buf2)
{
  return( strncmp((void *)buf1, (void *)buf2, 15) );
}

