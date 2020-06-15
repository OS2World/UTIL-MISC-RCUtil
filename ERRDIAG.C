/* errdiag.c */
/* Written by Rick Curry */
/* Adapted from errdiag.c code written by Eric Sassaman (ericsa@microsoft) */
/* simple VioPopUp only version */

#define INCL_DOSPROCESS
#define INCL_DOSQUEUES
#define INCL_VIO
#define INCL_DOSMISC
#include <os2.h>
#include <stdio.h>

#define BASE_ERR 1
#define NET_ERR 2

static USHORT fWait = VP_WAIT | VP_OPAQUE;
char errTxt[128]; /* short version of error text (not explanation text) */

static int bseerr(int errNumber, int errType);
void errDiag(int errNumber, char *file, int line);

static int bseerr(int errNumber, int errType)
{
  USHORT err;
  char chBuf[256];
  USHORT cbMsg;
  char *szMsgFileName;

  szMsgFileName = (errType == NET_ERR ? "net.msg" : "oso001.msg");
  err = DosGetMessage(NULL, 0, errTxt, sizeof(chBuf), errNumber,
      szMsgFileName, &cbMsg);
  errTxt[cbMsg] = 0;  /* null terminate the message string */
  if (err)
    return(1);
  puts(errTxt);
  szMsgFileName = (errType == NET_ERR ? "neth.msg" : "oso001h.msg");
  err = DosGetMessage(NULL, 0, chBuf, sizeof(chBuf), errNumber,
      szMsgFileName, &cbMsg);
  chBuf[cbMsg] = 0;  /* null terminate the message string */
  if (err)
    return(1);
  puts(chBuf);
  return(0);
} /* bseerr */


void errDiag(int errNumber, char *file, int line)
{
  char szMsg[128];
  USHORT err;
  int errType; /* either NET_ERR or BASE_ERR */

  if (errNumber >= 2100 && errNumber <=2999)
    errType = NET_ERR;
  else
    errType = BASE_ERR;
  VioPopUp(&fWait, 0);
  sprintf(szMsg, "Error %d has occurred in file %s on line %d.\n", errNumber, file, line);
  puts(szMsg);
  err = bseerr(errNumber, errType);
  if (err)  /* if bseerr() fails, just return */
    return;

  puts("Hit return to continue...");
  getchar();
  VioEndPopUp(0);
  return;
} /* errDiag */
