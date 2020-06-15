/* errdiag.h */

    /* prototype for definition of ERRCHK macro */

void errDiag(int errNumber, char *file, int line);

    /* USE THIS MACRO in your code to call errDiag. Just pass it the
    error that your API call returns */

#define ERRCHK(err) {if (err) {errDiag(err,__FILE__,__LINE__); exit(0);}}

