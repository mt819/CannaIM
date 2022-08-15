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

#include	"canna.h"
#include <errno.h>

/*********************************************************************
 *                      wchar_t replace begin                        *
 *********************************************************************/
#ifdef wchar_t
# error "wchar_t is already defined"
#endif
#define wchar_t cannawc

extern int getForIchiranContext(uiContext);

#ifndef NO_EXTEND_MENU

static
char *srussia_data[] =
{
  /*"А", "Б", "В", "Г", "Д", "Е", "Ё", "Ж",*/
  "\247\241", "\247\242", "\247\243", "\247\244", "\247\245", "\247\246", "\247\247", "\247\250",

  /*"З", "И", "Й", "К", "Л", "М", "Н", "О", */
  "\247\251", "\247\252", "\247\253", "\247\254", "\247\255", "\247\256", "\247\257", "\247\260",

  /*"П", "Р", "С", "Т", "У", "Ф", "Х", "Ц",*/
  "\247\261", "\247\262", "\247\263", "\247\264", "\247\265", "\247\266", "\247\267", "\247\270",

  /*"Ч", "Ш", "Щ", "Ъ", "Ы", "Ь", "Э", "Ю", */
  "\247\271", "\247\272", "\247\273", "\247\274", "\247\275", "\247\276", "\247\277", "\247\300",

  /*"Я", "а", "б", "в", "г", "д", "е", "ё",*/
  "\247\301", "\247\321", "\247\322", "\247\323", "\247\324", "\247\325", "\247\326", "\247\327",

  /*"ж", "з", "и", "й", "к", "л", "м", "н",*/
  "\247\330", "\247\331", "\247\332", "\247\333", "\247\334", "\247\335", "\247\336", "\247\337",

  /*"о", "п", "р", "с", "т", "у", "ф", "х",*/
  "\247\340", "\247\341", "\247\342", "\247\343", "\247\344", "\247\345", "\247\346", "\247\347",

  /*"ц", "ч", "ш", "щ", "ъ", "ы", "ь", "э",*/
  "\247\350", "\247\351", "\247\352", "\247\353", "\247\354", "\247\355", "\247\356", "\247\357",

  /*"ю", "я", */
  "\247\360", "\247\361",
};

#define	UURD_SZ	(sizeof(srussia_data) / sizeof(char *))

static
char *sgreek_data[] =
{
  /* "Α", "Β", "Γ", "Δ", "Ε", "Ζ", "Η", "Θ", */
  "\246\241", "\246\242", "\246\243", "\246\244", "\246\245", "\246\246", "\246\247", "\246\250",

  /* "Ι", "Κ", "Λ", "Μ", "Ν", "Ξ", "Ο", "Π", */
  "\246\251", "\246\252", "\246\253", "\246\254", "\246\255", "\246\256", "\246\257", "\246\260",

  /* "Ρ", "Σ", "Τ", "Υ", "Φ", "Χ", "Ψ", "Ω", */
  "\246\261", "\246\262", "\246\263", "\246\264", "\246\265", "\246\266", "\246\267", "\246\270",

  /* "α", "β", "γ", "δ", "ε", "ζ", "η", "θ", */
  "\246\301", "\246\302", "\246\303", "\246\304", "\246\305", "\246\306", "\246\307", "\246\310",

  /* "ι", "κ", "λ", "μ", "ν", "ξ", "ο", "π", */
  "\246\311", "\246\312", "\246\313", "\246\314", "\246\315", "\246\316", "\246\317", "\246\320",

  /* "ρ", "σ", "τ", "υ", "φ", "χ", "ψ", "ω", */
  "\246\321", "\246\322", "\246\323", "\246\324", "\246\325", "\246\326", "\246\327", "\246\330",
};

#define	UUGD_SZ	(sizeof(sgreek_data) / sizeof(char *))

static wchar_t *russia_data[UURD_SZ];
static wchar_t *greek_data[UUGD_SZ];

int
initUlKigoTable()
{
  int retval;

  retval = setWStrings(russia_data, srussia_data, UURD_SZ);
  if (retval != NG) {
    retval = setWStrings(greek_data, sgreek_data, UUGD_SZ);
  }
  return retval;
}

#ifdef pcux_r32
static
char *skeisen_data[] =
{
  /* "", "", "", "", "", "", "", "", */
  "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244",

  /* "", "", "", "", "", "", "", "", */
  "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244",

  /* "", "", "", "", "", "", "", "", */
  "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244",

  /* "", "", "", "", "", "", "", "", */
  "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244",

  /* "", "", "", "", "", "", "", "", */
  "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244",

  /* "", "", "", "", "", "", "", "", */
  "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244",

  /* "", "", "", "", "", "", "", "", */
  "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244",

  /* "", "", "", "", "", "", "", "", */
  "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244",

  /* "", "", "", "", "", "", "", "", */
  "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244", "\254\244",

  /*  "", "", "", "", */
  "\254\244", "\254\244", "\254\244", "\254\244",
};
#else /* EWS-UX/V */
static
char *skeisen_data[] =
{
  /* "─", "│", "┌", "┐", "┘", "└", "├", "┬", */
  "\250\241", "\250\242", "\250\243", "\250\244", "\250\245", "\250\246", "\250\247", "\250\250",

  /* "┤", "┴", "┼", "━", "┃", "┏", "┓", "┛", */
  "\250\251", "\250\252", "\250\253", "\250\254", "\250\255", "\250\256", "\250\257", "\250\260",

  /* "┗", "┣", "┳", "┫", "┻", "╋", "┠", "┯", */
  "\250\261", "\250\262", "\250\263", "\250\264", "\250\265", "\250\266", "\250\267", "\250\270",

  /* "┨", "┷", "┿", "┝", "┰", "┥", "┸", "╂", */
  "\250\271", "\250\272", "\250\273", "\250\274", "\250\275", "\250\276", "\250\277", "\250\300",
};
#endif


#define UUKD_SZ (sizeof(skeisen_data) / sizeof(char *))

static wchar_t *keisen_data[UUKD_SZ];

int
initUlKeisenTable()
{
  return setWStrings(keisen_data, skeisen_data, UUKD_SZ);
}

static int
uuKigoExitDo(d, retval)
uiContext d;
int retval;
{
  popForIchiranMode(d);
  popCallback(d);
  retval = YomiExit(d, retval);
  currentModeInfo(d);

  killmenu(d);

  return(retval);
}

static int
uuKigoRExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  forichiranContext fc;

  popCallback(d); /* 一覧を pop */

  fc = (forichiranContext)d->modec;
  d->currussia = fc->curIkouho;

  return(uuKigoExitDo(d, retval));
}

static int
uuKigoGExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  forichiranContext fc;

  popCallback(d); /* 一覧を pop */

  fc = (forichiranContext)d->modec;
  d->curgreek = fc->curIkouho;

  return(uuKigoExitDo(d, retval));
}

static int
uuKigoKExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  forichiranContext fc;

  popCallback(d); /* 一覧を pop */

  fc = (forichiranContext)d->modec;
  d->curkeisen = fc->curIkouho;

  return(uuKigoExitDo(d, retval));
}

int
uuKigoGeneralExitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  forichiranContext fc;

  popCallback(d); /* 一覧を pop */

  fc = (forichiranContext)d->modec;
  if (fc->prevcurp) {
    *(fc->prevcurp) = fc->curIkouho;
  }

  return(uuKigoExitDo(d, retval));
}

static int
uuKigoQuitCatch(d, retval, env)
uiContext d;
int retval;
mode_context env;
/* ARGSUSED */
{
  popCallback(d); /* 一覧を pop */

  popForIchiranMode(d);
  popCallback(d);
  currentModeInfo(d);

  return prevMenuIfExist(d);
}

int
uuKigoMake(uiContext d, wchar_t **allkouho, int size, char cur, char mode,
	canna_callback_t exitfunc, int *posp)
{
  forichiranContext fc;
  ichiranContext ic;
  unsigned inhibit = 0;
  int retval = 0;

  d->status = 0;

  if(getForIchiranContext(d) == NG) {
    return(GLineNGReturn(d));
  }
  fc = (forichiranContext)d->modec;

  /* selectOne を呼ぶための準備 */
  fc->allkouho = allkouho;
  fc->curIkouho = 0;
  fc->prevcurp = posp;
  inhibit |= (unsigned char)NUMBERING;

  if((retval = selectOne(d, fc->allkouho, &fc->curIkouho, size,
		 KIGOBANGOMAX, inhibit, 0, WITH_LIST_CALLBACK,
		 NO_CALLBACK, exitfunc,
		 uuKigoQuitCatch, uiUtilIchiranTooSmall)) == NG) {
    return(GLineNGReturnFI(d));
  }

  ic = (ichiranContext)d->modec;
  ic->minorMode = mode;
  ic->flags |= cannaconf.quickly_escape ? 0 : ICHIRAN_STAY_LONG;
  currentModeInfo(d);

  *(ic->curIkouho) = (int)cur;

  /* 候補一覧行が狭くて候補一覧が出せない */
  if(ic->tooSmall) {
    d->status = AUX_CALLBACK;
    return(retval);
  }

  if ( !(ic->flags & ICHIRAN_ALLOW_CALLBACK) ) {
    makeGlineStatus(d);
  }

  /* d->status = ICHIRAN_EVERYTIME; */

  return(retval);
}

#if 0
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 記号一覧                                                                  *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

static int
kigoZenpan(d)
uiContext	d;
{
  if(makeKigoIchiran(d, CANNA_MODE_ExtendMode) == NG) /* 0 は拡張の記号一覧 */
    return(GLineNGReturn(d));
  else
    return(0);
}
#endif

#endif /* NO_EXTEND_MENU */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ロシア文字の入力                                                          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
kigoRussia(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }
#ifdef NO_EXTEND_MENU
  d->kanji_status_return->info |= KanjiRussianInfo;
  return 0;
#else
  return(uuKigoMake(d, (wchar_t **)russia_data, UURD_SZ,
           d->currussia, CANNA_MODE_RussianMode, uuKigoRExitCatch, NULL));
#endif
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * ギリシャ文字の入力                                                        *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
kigoGreek(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }
#ifdef NO_EXTEND_MENU
  d->kanji_status_return->info |= KanjiGreekInfo;
  return 0;
#else
  return(uuKigoMake(d, (wchar_t **)greek_data, UUGD_SZ,
           d->curgreek, CANNA_MODE_GreekMode, uuKigoGExitCatch, NULL));
#endif
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * 罫線の入力                                                                *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

int
kigoKeisen(d)
uiContext d;
{
  yomiContext yc = (yomiContext)d->modec;

  if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
    return NothingChangedWithBeep(d);
  }
#ifdef NO_EXTEND_MENU
  d->kanji_status_return->info |= KanjiLineInfo;
  return 0;
#else
  return(uuKigoMake(d, (wchar_t **)keisen_data, UUKD_SZ,
           d->curkeisen, CANNA_MODE_LineMode, uuKigoKExitCatch, NULL));
#endif
}

#ifndef wchar_t
# error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/
