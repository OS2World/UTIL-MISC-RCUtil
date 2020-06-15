
/********************************************************/
/*                                                      */
/*  Function: new                                       */
/*                                                      */
/*   Purpose: Prints files created in the last nn days. */
/*              nn defaults to 1 day.                   */
/*                                                      */
/*   To build: CL /F 2000 new.c                         */
/*                                                      */
/*   To use:  new /nn template                          */
/*                                                      */
/*      File: new.c                                     */
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
#include <time.h>
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
  char sp1;                             // space for formatting
  char fname[8];                        // 8 char filename
  char decpt;                           // A '.' if there is an extention
  char ext[3];                          // 3 char extension
  char size[8];                         // 8 digit # bytes in file
  char sp2;                             // space for formatting
  char date[6];                         // 6 char YYMMDD date
  char sp3;                             // space for formatting
  char time[4];                         // 4 char military time
  char dummy[3];                        // 3 available spaces if desired
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

MYDIRSTRCT build_array, *dest_ptr;      // Structs for building/moving dir info
FILEFINDBUF *src_ptr;                   // Ptr to record being interpreted
time_t time_abs;
struct tm time_local;
union FTIME_CMP {                       // Time we are looking since (6AM)
  FTIME ft;                             // To access in bit mode
  USHORT us;                            // To access as an unsigned short
} target_time;
union FDATE_CMP {                       // Date we are looking since (current)
  FDATE fd;                             // To access in bit mode
  USHORT us;                            // To access as an unsigned short
} target_date;
long daysback = 0;                      // # days back to look
int numkept = 0;                        // # entries not discarded

main(argc, argv)
int argc;
char *argv[];
{
  if (argc > 3) errex("\n\nProper usage: 'new [/nn] [template]'\n\n");
  if (argc > 1)
  {
    if ( (*argv[1] == '-') || (*argv[1] == '/') )   // If there is a switch
    {
      daysback = atol( argv[1]+1 ) - 1; // Interpret as # days to look back
      argv++;                           // Remove the 1st argument
      argc--;
    }
  }
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

  api_err = time(&time_abs);
  time_abs -= 60L*60L*24L*daysback;     // Count backwards n days
  time_local = *localtime(&time_abs);
  if (time_local.tm_hour < 6) {         // If now is between midnight and 6AM
    time_abs -= 60L*60L*24L;            // Read from 6AM yesterday
    time_local = *localtime(&time_abs);
  }
  target_date.fd.year  = time_local.tm_year; // Build the 1st date & time we should
  target_date.fd.month = time_local.tm_mon+1;// keep file info for
  target_date.fd.day   = time_local.tm_mday;
  target_date.fd.year -= 80;            // Get to same base as DosFindFirst
  target_date.fd.year %= 100;           // Take mod 100 of year

  target_time.ft.hours   = 6;           // Always read from 6 AM
  target_time.ft.minutes =
  target_time.ft.twosecs = 0;

                                        // Get disk usage of current disk
  api_err=DosQFSInfo(0, FSIL_ALLOC, (PBYTE)&fsallocate, sizeof(fsallocate));
  ERRCHK(api_err);                      // Check for errors & print

  printf("Directory of files written since %d/%d/%d at 6 AM.\n",
            target_date.fd.month, target_date.fd.day, target_date.fd.year+1980);
  ii = sizeof(filefindbuf) - sizeof(build_array);
  dest_ptr = (MYDIRSTRCT *)             // Init the end pointer
     ( (char *)filefindbuf + ii );
  src_ptr = (FILEFINDBUF *)filefindbuf; // Init the start pointer

  build_array.term = '\0';              // Only needs done once (NULL term struct)
  build_array.dummy[0] =
  build_array.dummy[1] =
  build_array.dummy[2] = ' ';           // Blank out the dummies
  for (ii=0; ii<usSearchCount; ii++,    // Point to next file in source buffer
              src_ptr = (FILEFINDBUF *)&src_ptr->achName[src_ptr->cchName+1])
  {
    char *fnameptr, *extptr;
    union FDATE_CMP *dispdate;
    union FTIME_CMP *disptime;

    fnameptr = strtok(src_ptr->achName, "."); // NULL term fname, remove '.'
    extptr = strtok(NULL, ".");         // Get extension string
    fnameptr = src_ptr->achName;        // Guarantee filename for '.' & '..'
    if (extptr == NULL)
      extptr=nulldummy;                 // Guarantee NULL ext available

    dispdate = (union FDATE_CMP *)&src_ptr->fdateLastWrite;
    disptime = (union FTIME_CMP *)&src_ptr->ftimeLastWrite;

    if (dispdate->us < target_date.us)
      continue;                         // File is too old to be of interest

    if ( (dispdate->us == target_date.us) &&
          (disptime->us < target_time.us) )
      continue;                         // File is too old to be of interest

    numkept++;                          // It's a keeper

    sprintf( (char *)&build_array, "%c%c%c%c %-8.8s %-3.3s%8lu ",
              ((src_ptr->attrFile & FILE_SYSTEM)    ? 'S' : '_'),
              ((src_ptr->attrFile & FILE_HIDDEN)    ? 'H' : '_'),
              ((src_ptr->attrFile & FILE_READONLY)  ? 'R' : '_'),
              ((src_ptr->attrFile & FILE_DIRECTORY) ? '/' : '_'),
              fnameptr, extptr, src_ptr->cbFile);

    sprintf( (char *)build_array.date, "%02d%02d%02d %02d%02d",
              (dispdate->fd.year+80)%100, dispdate->fd.month, dispdate->fd.day,
              disptime->ft.hours, disptime->ft.minutes);

    build_array.dummy[0] = ' ';         // Get rid of NULL from sprintf above

    total_bytes += src_ptr->cbFile;     // Add # bytes in this file to total

    if (extptr == nulldummy)            // Did we need to dummy out extension?
      build_array.decpt = ' ';          // No extension, no dot
    else build_array.decpt = '.';       // Put a dot before extension

    *dest_ptr-- = build_array;          // Copy structure to dest & next dest
  }
  dest_ptr++;                           // Point to last item converted

  if (numkept==0)                       // No recent files
  {
    printf("No files found. %lu bytes free (on current disk)\n",
            fsallocate.cSectorUnit*fsallocate.cUnitAvail*fsallocate.cbSector);
    exit(0);
  }
                                        // Sort based on flags and fname/ext
  qsort((void *)dest_ptr, numkept, sizeof(MYDIRSTRCT), fnamecmp );

  for (ii=0; ii<numkept; ii++)
  {
    fputs((char *)dest_ptr++, stdout);
//    if (ii&1) fputc('\n', stdout);
  }
  if (ii&1) fputc('\n', stdout);

  printf("Total of %d files using %lu bytes. %lu bytes free (on current disk)\n",
    numkept, total_bytes,
            fsallocate.cSectorUnit*fsallocate.cUnitAvail*fsallocate.cbSector);
}

int fnamecmp(MYDIRSTRCT *buf1, MYDIRSTRCT *buf2)
{
  long datecmp;

  datecmp = strncmp((void *)buf2->date, (void *)buf1->date, 11);
  if (datecmp != 0) return(datecmp);
  return( strncmp((void *)buf1, (void *)buf2, 15) );
}

