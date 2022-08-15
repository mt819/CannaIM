/* Copyright 1992 NEC Corporation, Tokyo, Japan.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of NEC
 * Corporation not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  NEC Corporation makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * NEC CORPORATION DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN
 * NO EVENT SHALL NEC CORPORATION BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
 * USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTUOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "canna.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

/*********************************************************************
 *                      wchar_t replace begin                        *
 *********************************************************************/
#ifdef wchar_t
# error "wchar_t is already defined"
#endif
#define wchar_t cannawc

extern char *CANNA_initfilename;

#define BUF_LEN 1024

static char CANNA_rcfilename[BUF_LEN] = "";

static void DISPLAY_to_hostname(char *name, char *buf, int bufsize);

/* cfuncdef

   YYparse -- カスタマイズファイルを読む。

   ファイルディスクリプタで指定されたファイルを読み込む。

*/

extern int ckverbose;


/* cfuncdef

  parse -- .canna ファイルを探してきて読み込む。

  parse はカスタマイズファイルを探し、そのファイルをオープンしパースす
  る。

  パース中のファイルの名前を CANNA_rcfilename に入れておく。

  */

#define NAMEBUFSIZE 1024
#define RCFILENAME  ".canna"
#define FILEENVNAME "CANNAFILE"



static int
make_initfilename()
{
  if(!CANNA_initfilename) {
    CANNA_initfilename = malloc(1024);
    if (!CANNA_initfilename) {
      return -1;
    }
    strcpy(CANNA_initfilename, CANNA_rcfilename);
  }
  else {
    strcat(CANNA_initfilename, ",");
    strcat(CANNA_initfilename, CANNA_rcfilename);
  }
  return 0;
}

static void
fit_initfilename()
{
  char *tmpstr;

  if (CANNA_initfilename) {
    tmpstr = malloc(strlen(CANNA_initfilename) + 1);
    if (!tmpstr) return;
    strcpy(tmpstr, CANNA_initfilename);
    free(CANNA_initfilename);
    CANNA_initfilename = tmpstr;
  }
}

void
parse()
{
  char *p;
  int n;
  int home_canna_exist = 0;
  extern char *initFileSpecified;
#ifdef __HAIKU__
	extern char basepath[];
#endif
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  char buf[256];
#else
  char *buf = malloc(256);
  if (!buf) {
    return;
  }
#endif

  if (clisp_init() == 0) {

    if (ckverbose) {
      printf("カスタマイズファイルは読み込みません。\n");
    }

    addWarningMesg("\245\341\245\342\245\352\244\254\302\255\244\352\244\336"
	"\244\273\244\363\241\243\245\253\245\271\245\277\245\336\245\244"
	"\245\272\245\325\245\241\245\244\245\353\244\362\306\311\244\337"
	"\271\376\244\341\244\336\244\273\244\363\241\243\\n");
          /* メモリが足りません。カスタマイズファイルを読み込めません。 */
    goto quitparse;
  }

  if (initFileSpecified) {
    strcpy(CANNA_rcfilename, initFileSpecified);
    if (YYparse_by_rcfilename(CANNA_rcfilename)) {
      make_initfilename();
      goto quitparse;
    }
    else {
      if (ckverbose) {
	printf("カスタマイズファイルは読み込みません。\n");
      }

      sprintf(buf, "\273\330\304\352\244\265\244\354\244\277\245\253\245\271"
	"\245\277\245\336\245\244\245\272\245\325\245\241\245\244\245\353"
	"\40\45\163\40\244\254\302\270\272\337\244\267\244\336\244\273"
	"\244\363\241\243",
	      CANNA_rcfilename);
              /* 指定されたカスタマイズファイル %s が存在しません。 */
      addWarningMesg(buf);
      goto quitparse;
    }
  }
 #ifdef __HAIKU__
    sprintf(CANNA_rcfilename, "%s%s%s",basepath, "default/", RCFILENAME);
    if (YYparse_by_rcfilename(CANNA_rcfilename)) {
      make_initfilename();
      goto quitparse;
    }
 #else
  p = getenv(FILEENVNAME);
  if (p) {
    strcpy(CANNA_rcfilename, p);
    if (YYparse_by_rcfilename(CANNA_rcfilename)) {
      make_initfilename();
      goto quitparse;
    }
  }
#endif
  p = getenv("HOME");
  if (p) {
    strcpy(CANNA_rcfilename, p);
    strcat(CANNA_rcfilename, "/");
    strcat(CANNA_rcfilename, RCFILENAME);
    n = strlen(CANNA_rcfilename);

    /* $HOME/.canna */

    home_canna_exist = YYparse_by_rcfilename(CANNA_rcfilename);
    if (home_canna_exist) {
      make_initfilename();

      /* $HOME/.canna-DISPLAY */

      p = getenv("DISPLAY");
      if (p) {
	char display[NAMEBUFSIZE];

	DISPLAY_to_hostname(p, display, NAMEBUFSIZE);

	CANNA_rcfilename[n] = '-';
	strcpy(CANNA_rcfilename + n + 1, display);

	if(YYparse_by_rcfilename(CANNA_rcfilename)) {
	  make_initfilename();
	}
      }

      /* $HOME/.canna-TERM */

      p = getenv("TERM");
      if (p) {
	CANNA_rcfilename[n] = '-';
	strcpy(CANNA_rcfilename + n + 1, p);
	if(YYparse_by_rcfilename(CANNA_rcfilename)) {
	  make_initfilename();
	}
      }
    }
  }

  if ( !home_canna_exist ) {
    /* 最後はシステムデフォルトのファイルを読む */
    strcpy(CANNA_rcfilename, CANNALIBDIR);
    n = strlen(CANNA_rcfilename);

    strcpy(CANNA_rcfilename + n, "/default");
    strcat(CANNA_rcfilename + n, RCFILENAME);
    if (YYparse_by_rcfilename(CANNA_rcfilename)) {
      make_initfilename();
      p = getenv("DISPLAY");
      if (p) {
	char display[NAMEBUFSIZE];

	DISPLAY_to_hostname(p, display, NAMEBUFSIZE);

	CANNA_rcfilename[n] = '/';
	strcpy(CANNA_rcfilename + n + 1, display);
	strcat(CANNA_rcfilename, RCFILENAME);
	if(YYparse_by_rcfilename(CANNA_rcfilename)) {
	  make_initfilename();
	}
      }

      p = getenv("TERM");
      if (p) {
	CANNA_rcfilename[n] = '/';
	strcpy(CANNA_rcfilename + n + 1, p);
	strcat(CANNA_rcfilename, RCFILENAME);
	if(YYparse_by_rcfilename(CANNA_rcfilename)) {
	  make_initfilename();
	}
      }
    }
    else {
      if (ckverbose) {
	printf("カスタマイズファイルは読み込みません。\n");
      }
      sprintf(buf,
#ifdef CODED_MESSAGE
      "システムのカスタマイズファイル %s が存在しません。",
#else
      "\245\267\245\271\245\306\245\340\244\316\245\253\245\271"
      "\245\277\245\336\245\244\245\272\245\325\245\241\245\244\245\353"
      "\40\45\163\40\244\254\302\270\272\337\244\267\244\336\244\273"
      "\244\363\241\243",
#endif
	      CANNA_rcfilename);
      /* システムのカスタマイズファイル %s が存在しません。 */
      addWarningMesg(buf);
    }
  }

 quitparse:
  /* CANNA_initfilename をジャストサイズに刈り込む */
  fit_initfilename();
  clisp_fin();

#ifdef USE_MALLOC_FOR_BIG_ARRAY
  free(buf);
#endif
}


static void
DISPLAY_to_hostname(name, buf, bufsize)
char *name, *buf;
int bufsize;
{
  if (name[0] == ':' || !strncmp(name, "unix", 4)) {
    gethostname(buf, bufsize);
  }
  else {
    int i, len = strlen(name);
    for (i = 0 ; i < len && i < bufsize ; i++) {
      if (name[i] == ':') {
	break;
      }
      else {
	buf[i] = name[i];
      }
    }
    if (i < bufsize) {
      buf[i] = '\0';
    }
  }
}

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/
