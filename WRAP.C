
/********************************************************/
/*                                                      */
/*  Function: wrap80                                    */
/*                                                      */
/*   Purpose: Wraps lines >80 columns.  Assumes stdin   */
/*            to stdout because it solves a problem     */
/*            relating to 80 column screens.            */
/*                                                      */
/*   To build: CL /F 2000 /Lp /Fb wrap80.c              */
/*                                                      */
/*   To use:  wrap80                                    */
/*                                                      */
/*                                                      */
/*      File: wrap80.c                                  */
/*                                                      */
/********************************************************/

        /* Include system calls/library routines/macros */
#include <stdio.h>

        /* Define general variables */
char read_ln[81];

main(argc, argv)
int argc;
char *argv[];
  {

  if (argc != 1)
    errex("\n\nProper usage: 'wrap80'\n\n");

  for (;;)      /* Until exit */
  {
    fgets(read_ln, sizeof(read_ln), stdin); // Up to 80 chars + \0
    if ( feof(stdin) )  break;              // End of file ==> all done
    fputs(read_ln, stdout);                 // Write it back out

    if ( read_ln[strlen(read_ln)-1] != '\n' ) // Did we get a newline?
    {
      register int ch;

      fputc('\n', stdout);                  // No, output one
      if ( (ch=getc(stdin)) != '\n' )       // If next char is not the newline
        ungetc(ch, stdin);                  // Push it back
    }
    if ( feof(stdin) )  break;              // End of file ==> all done
  }
}     /* of main */

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

printf(mess_ptr);
exit();
}
