/********************************************************/
/*                                                      */
/*  Function: ascii                                     */
/*                                                      */
/*   Purpose: Search for text in any file.              */
/*                                                      */
/*   To build: CL /Ox /Lp /Fb /F 2000 ascii.c           */
/*                                                      */
/*   To use:  ascii [/nn] filename                      */
/*  Defaults:         4    NONE                         */
/*                                                      */
/*      File: ascii.c                                   */
/*                                                      */
/********************************************************/

        /* Include system calls/library routines/macros */
#include <stdio.h>
#include <ctype.h>

        /* Define constants */
#define true -1
#define false 0
#define check if (ret!=SS$_NORMAL) exit(ret)
#define SAVEARSIZ 65
#define MAXCOL 78

        /* Define general variables */
char inp_buf[16384];
int i, k, numasc, column, retval;
long offset;
int minstr=4;           /* Default min size string to print */
char savar[SAVEARSIZ];          /* Array that holds ascii strings before print */
int ch;
char *usage_string = "\n\
Correct usage is ascii [-nn] input-file(s)\n\
             Defaults:    4   NONE\n\
  -nn switch determines # of ascii characters required before printing\n";

        /* Define file access structures */
FILE *inp_file;

main(argc, argv)
int argc;
char *argv[];
{
  if (argc < 2)
    errex(usage_string);

  for (i=1; i<argc; i++)
  {
    if ( *argv[i] == '/' || *argv[i] == '-' ) {
      minstr = atoi(++argv[i]);
      if (minstr > SAVEARSIZ) {
        printf("\n%d characters is more than I can handle!\n", minstr);
        exit(2);
      }
      if (minstr < 1) {
        errex(usage_string);
      }
    }
    else {
      inp_file = fopen(argv[i], "rb");
      if (inp_file == NULL) {
        printf("Error opening %s\n", argv[i]);
        errex("\n\nGiving up the ghost");
      }
      retval=setvbuf(inp_file, inp_buf, _IOFBF, sizeof(inp_buf) );
      if (retval!=0) errex("Unable to set input buffer\n");
      offset = numasc = 0;
      for (; !feof(inp_file); offset++ ) {
        ch = fgetc(inp_file);
        if (numasc >= minstr) {         /* If we are printing now */
          if ( isprint(ch) ) {          /* And this is printable */
            if (column++ > MAXCOL) {    /* If we've run out of column */
              printf("\n          ");   /* Start a new line */
              column = 11;
            }
            putchar(ch);                /* Output the character */
          }
          else {
            numasc = 0;                 /* Clear the output flag/counter */
            putchar('\n');              /* and terminate the line */
          }
        }
        else {                          /* Not yet outputting */
          if ( !isprint(ch) )
            numasc = 0;
          else {                        /* Log the char & check for output now */
            savar[numasc++] = ch;
            if (numasc >= minstr) {
              printf("%08lX: ",offset-minstr+1); /* Start a new line */
              for (k=0; k<numasc; k++) {
                putchar(savar[k]);      /* Output the character */
              }
              column = minstr+10;
            }   /* Start printing now */
          }     /* Saving characters */
        }       /* Examining characters */
      }         /* Reading a file */
      putchar('\n');    /* Mark end of file */
      fclose(inp_file);
    }   /* Not a switch */
  }     /* Reading parameters */
}       /* of main */

/********************************************************/
/*                                                      */
/*  Function: errex                                     */
/*                                                      */
/*   Purpose: Prints an error message to the screen and */
/*              exits.                                  */
/*                                                      */
/********************************************************/

errex(mess_ptr)
char *mess_ptr;
{

perror(mess_ptr);
exit();
}
