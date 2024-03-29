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

#include <canna/mfdef.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>

/*********************************************************************
 *                      wchar_t replace begin                        *
 *********************************************************************/
#ifdef wchar_t
#error "wchar_t is already defined"
#endif
#define wchar_t cannawc

#define DEFAULT_COLUMN_WIDTH 70

extern char* CANNA_initfilename;
extern char saveapname[];

static int
insertEmptySlots(uiContext d);
static int
callCallback(uiContext d, int res);
static void
freeKeysup(void);
static void
freeBuffer(void);
static void
freeExtra(void);
extern int ckverbose;

static int
doInitializeFunctions(uiContext d)
{
  BYTE* p;
  int res = 0;
  wcKanjiStatus ks, *pks;
  extern BYTE* initfunc;
  wchar_t xxxx[10];

  d->ch = 0;
  d->buffer_return = xxxx;
  d->n_buffer = sizeof(xxxx) / sizeof(wchar_t);
  d->nbytes = 0;

  ks.length = -1;
  ks.info = 0;

  if (initfunc) {
    pks = d->kanji_status_return;
    d->kanji_status_return = &ks;
    for (p = initfunc; *p; p++) {
      res = _doFunc(d, *p);
    }
    res = _afterDoFunc(d, res);
    d->kanji_status_return = pks;
  }
  return res;
}

/* uiContext の初期化 */

int
initRomeStruct(uiContext d, int flg)
{
  yomiContext yc;

  bzero(d, sizeof(uiContextRec));

  if (insertEmptySlots(d) < 0) {
    return -1;
  }

  /* コンテクストを最初にデュプリケートしていた過去があったなあ */
  d->contextCache = -1;

  /* 初期モード(アルファベットモード)を入れる */
  d->majorMode = d->minorMode = CANNA_MODE_AlphaMode;
  yc = (yomiContext)d->modec;
  if (flg) {
    yc->minorMode = CANNA_MODE_ChikujiYomiMode;
    yc->generalFlags |= CANNA_YOMI_CHIKUJI_MODE;
  }
  alphaMode(d);

  /* 初期 func を実行する */

  doInitializeFunctions(d);
  return 0;
}

static void
freeModec(mode_context modec)
{
  coreContext cc;
  union
  {
    coreContext c;
    yomiContext y;
    ichiranContext i;
    forichiranContext f;
    mountContext m;
    tourokuContext t;
  } gc;

  cc = (coreContext)modec;
  while (cc) {
    switch (cc->id) {
      case CORE_CONTEXT:
        gc.c = cc;
        cc = (coreContext)gc.c->next;
        freeCoreContext(gc.c);
        break;
      case YOMI_CONTEXT:
        gc.y = (yomiContext)cc;
        cc = (coreContext)gc.y->next;
        /* yc->context の close はいらないのかなあ。1996.10.30 今 */
        freeYomiContext(gc.y);
        break;
      case ICHIRAN_CONTEXT:
        gc.i = (ichiranContext)cc;
        cc = (coreContext)gc.i->next;
        freeIchiranContext(gc.i);
        break;
      case FORICHIRAN_CONTEXT:
        gc.f = (forichiranContext)cc;
        cc = (coreContext)gc.f->next;
        freeForIchiranContext(gc.f);
        break;
      case MOUNT_CONTEXT:
        gc.m = (mountContext)cc;
        cc = (coreContext)gc.m->next;
        freeIchiranContext(gc.i);
        break;
      case TOUROKU_CONTEXT:
        gc.t = (tourokuContext)cc;
        cc = (coreContext)gc.t->next;
        free(gc.t);
        break;
      default:
        break;
    }
  }
}

static void
freeCallbacks(struct callback* cb)
{
  struct callback* nextcb;

  for (; cb; cb = nextcb) {
    nextcb = cb->next;
    free(cb);
  }
}

void
freeRomeStruct(uiContext d)
{
  freeModec(d->modec);
  if (d->cb) {
    freeCallbacks(d->cb);
  }
  if (d->contextCache >= 0) {
    if (RkwCloseContext(d->contextCache) < 0) {
      if (errno == EPIPE) {
        jrKanjiPipeError();
      }
    }
  }
#ifndef NO_EXTEND_MENU
  freeAllMenuInfo(d->minfo);
#endif
  if (d->selinfo) {
    selectinfo *p, *q;

    for (p = d->selinfo; p; p = q) {
      q = p->next;
      free(p);
    }
  }
  if (d->attr) {
    free(d->attr->u.attr);
    free(d->attr);
  }
  free(d);
}

static int
insertEmptySlots(uiContext d)
{
  extern KanjiModeRec empty_mode;
  yomiContext yc;

  if (pushCallback(d,
                   (mode_context)NULL,
                   NO_CALLBACK,
                   NO_CALLBACK,
                   NO_CALLBACK,
                   NO_CALLBACK) == NULL)
    return NoMoreMemory();

  yc = newYomiContext(NULL,
                      0, /* 結果は格納しない */
                      CANNA_NOTHING_RESTRICTED,
                      !CANNA_YOMI_CHGMODE_INHIBITTED,
                      !CANNA_YOMI_END_IF_KAKUTEI,
                      CANNA_YOMI_INHIBIT_NONE);
  if (yc == (yomiContext)0) {
    popCallback(d);
    return NoMoreMemory();
  }
  yc->majorMode = yc->minorMode = CANNA_MODE_HenkanMode;
  d->majorMode = d->minorMode = CANNA_MODE_HenkanMode;
  d->modec = (mode_context)yc;

  d->current_mode = yc->curMode = yc->myEmptyMode = &empty_mode;
  yc->romdic = romajidic;
  d->ncolumns = DEFAULT_COLUMN_WIDTH;
  d->minfo = NULL;
  d->selinfo = NULL;
  d->prevMenu = NULL;
  return 0;
}

/*

  display と window の組や コンテクストID を実際のコンテクストに対応付
  けるためのハッシュテーブル

  display と window から作られるキーのところを引くと、そこにコンテクスト
  が入っている確率が高い。もし入っていなくとも、ポインタチェーンをたどって
  行くといつかはコンテクストが得られるに違いない。

 */

#define HASHTABLESIZE 96

static struct bukRec
{
  unsigned int data1, data2;
  uiContext context;
  struct bukRec* next;
}* conHash[HASHTABLESIZE];

/* ハッシュテーブルを調べてコンテクストがあるかどうか調べる関数 */

static int
countContext()
{
  struct bukRec* hash;

  int i, c;
  for (i = 0, c = 0; i < HASHTABLESIZE; i++) {
    for (hash = conHash[i]; hash && hash->context; hash = hash->next) {
      c++;
    }
  }
#if defined(DEBUG)
  fprintf(stderr, "合計=%d\n", c);
#endif
  if (c) {
    return 0;
  } else {
    return 1;
  }
}

/* ハッシュキーを作る関数(いい加減) */

static unsigned int
makeKey(unsigned int data1, unsigned int data2)
{
  unsigned int key;

  key = data1 % HASHTABLESIZE;
  key += data2 % HASHTABLESIZE;
  key %= HASHTABLESIZE;
  return key;
}

/*

  keyToContext -- Display と Window の組などからコンテクストを割り出す処理

  display と window の組がコンテクストを持っていればそのコンテクスト
  を返す。

  持っていないのであれば、NULL を返す。

  */

uiContext
keyToContext(unsigned int data1, unsigned int data2)
{
  unsigned int key;
  struct bukRec* p;

  key = makeKey(data1, data2);
  for (p = conHash[key]; p; p = p->next) {
    if (p->data1 == data1 && p->data2 == data2) {
      /* こりゃあコンテクストが見つかりましたな */
      return p->context;
    }
  }
  return (uiContext)0; /* 見つかりませんでした。 */
}

/* internContext -- ハッシュテーブルに登録する

  このとき、既に、display と window の組が存在するのであれば、
  その先につながっているコンテクストをフリーするので注意！！

*/

struct bukRec*
internContext(unsigned int data1, unsigned int data2, uiContext context)
{
  unsigned int key;
  struct bukRec *p, **pp;

  key = makeKey(data1, data2);
  for (pp = &conHash[key]; (p = *pp) != NULL; pp = &(p->next)) {
    if (p->data1 == data1 && p->data2 == data2) {
      freeRomeStruct(p->context);
      p->context = context;
      return p;
    }
  }
  p = *pp = (struct bukRec*)malloc(sizeof(struct bukRec));
  if (p) {
    p->data1 = data1;
    p->data2 = data2;
    p->context = context;
    p->next = NULL;
  }
  return p;
}

/* rmContext -- ハッシュテーブルから削除する

*/

void
rmContext(unsigned int data1, unsigned int data2)
{
  unsigned int key;
  struct bukRec *p, *q, **pp;

  key = makeKey(data1, data2);
  pp = &conHash[key];
  for (p = *pp; p; p = q) {
    q = p->next;
    if (p->data1 == data1 && p->data2 == data2) {
      *pp = q;
      free(p);
    } else {
      pp = &(p->next);
    }
  }
}

/* cfuncdef

  freeBukRecs() -- ポイントされている先のバケットのフリー

  バケットによってポイントされているデータをすべてフリーする。
  フリーの対象には uiContext も含まれる。

*/

static void
freeBukRecs(struct bukRec* p)
{
  struct bukRec* nextp;

  if (p) { /* reconfirm that p points some structure */
    freeRomeStruct(p->context);
    nextp = p->next;
    if (nextp) {
      freeBukRecs(nextp);
    }
    free(p);
  }
}

/* cfuncdef

  clearHashTable() -- ハッシュテーブルの内容をすべてフリーする。

*/

static void
clearHashTable()
{
  int i;
  struct bukRec* p;

  for (i = 0; i < HASHTABLESIZE; i++) {
    p = conHash[i];
    conHash[i] = 0;
    if (p) {
      freeBukRecs(p);
    }
  }
}

#define NWARNINGMESG 64
static char* WarningMesg[NWARNINGMESG + 1]; /* +1 は最後の NULL ポインタの分 */
static int nWarningMesg = 0;

static void
initWarningMesg()
{
  int i;

  for (i = 0; i < nWarningMesg; i++) {
    free(WarningMesg[i]);
    WarningMesg[i] = NULL;
  }
  nWarningMesg = 0;
}

void
addWarningMesg(char* s)
{
  int n;
  char* work;

  if (nWarningMesg < NWARNINGMESG) {
    n = strlen(s);
    work = (char*)malloc(n + 1);
    if (work) {
      strcpy(work, s);
      WarningMesg[nWarningMesg++] = work;
    }
  }
}

static int
KC_keyconvCallback(uiContext d, char* arg)
/* ARGSUSED */
{
  extern void (*keyconvCallback)();

  if (arg) {
    keyconvCallback = (void (*)())arg;
  } else {
    keyconvCallback = NULL;
  }
  return 0;
}

static int
KC_initialize(uiContext d, char* arg)
/* ARGSUSED */
{
  extern int FirstTime;

  if (FirstTime) {
    InitCannaConfig(&cannaconf);

    debug_message("KC_INITIALIZE "
                  "\244\362\313\334\305\366\244\313\244\271\244\353\244\276\n",
                  0,
                  0,
                  0);
    /* を本当にするぞ */

#ifndef NO_EXTEND_MENU
    if (initExtMenu() < 0) {
      jrKanjiError = "Insufficient memory.";
      if (arg)
        *(char***)arg = NULL;
      return -1;
    }
#endif

    /* ワイドキャラクタ用メモリ管理の初期化 */

    WStringOpen();
#ifndef NO_EXTEND_MENU
    if (initBushuTable() != NG) {
      if (initGyouTable() != NG) {
        if (initHinshiTable() != NG) {
          if (initUlKigoTable() != NG) {
            if (initUlKeisenTable() != NG) {
              if (initOnoffTable() != NG) {
                initKigoTable(); /* 何もしていない */
                if (initHinshiMessage() != NG) {
#endif

                  /* ウォーニングメッセージの初期化 */
                  initWarningMesg();

                  /* モード名の初期化 */
                  initModeNames();

                  /* キーテーブルの初期化 */
                  if (initKeyTables() != NG) {

                    /* 初期化ファイルの読み込み */
#ifdef BINARY_CUSTOM
                    binparse();
#else
      parse();
#endif

                    /* keyconvCallback は parse 後は不要なのでクリアする */
                    KC_keyconvCallback(d, NULL);

                    /* 一覧関係文字列の初期化 */
                    if (initIchiran() != NG) {

                      /* ローマ字かな変換の初期化 */
                      RomkanaInit();

                      /* カナ漢字変換の初期化 */
                      if (ckverbose || !cannaconf.DelayConnect)
                        KanjiInit();
                      /* ここでもエラーは無視します。
                         漢字にならなくてもいいし。 */

                      if (arg) {
                        *(char***)arg = nWarningMesg ? WarningMesg : NULL;
                      }
                      FirstTime = 0;
                      return 0;
                    }
                    /* uiContext の検索のためのハッシュテーブルをクリア
                       uiContext も一緒にフリーする */
                    clearHashTable();

                    /* 記号定義を消す */
                    freeKeysup();

                    /* 色々とカスタマイズされている所をもとに戻す。 */
                    restoreBindings();

                    /* 拡張機能のinitfile表示用のバッファを解放する */
                    freeBuffer();

#ifndef NO_EXTEND_MENU
                    /* メニュー関連のメモリの開放 */
                    finExtMenu();
#endif

                    /* デフォルト以外のモード用メモリの開放 */
                    freeExtra();

                    /* キーマップテーブルのクリア */
                    restoreDefaultKeymaps();
                  }
                  /* モード文字列のフリー */
                  resetModeNames();
#ifndef NO_EXTEND_MENU
                }
              }
            }
          }
        }
      }
    }
#endif
    /* ワイドキャラクタ用自動メモリ管理の終了処理 */
    WStringClose();

    /* サーバ名保持用メモリの開放 */
    /* RkSetServerName((char *)0); してはいけないのでは？ */

    return -1;
  } else {
    /* 前にInitializeをしている場合にはもうメッセージをださないことにする */
    if (arg) {
      *(char***)arg = NULL;
    }
    return -1;
  }
}

static void
freeKeysup()
{
  int i;
  extern keySupplement keysup[];
  extern int nkeysup;

  for (i = 0; i < nkeysup; i++) {
    if (keysup[i].cand) {
      free(keysup[i].cand);
      keysup[i].cand = NULL;
    }
    if (keysup[i].fullword) {
      free(keysup[i].fullword);
      keysup[i].fullword = NULL;
    }
  }
  nkeysup = 0;
}

extern int nothermodes;

static void
freeBuffer()
{
  if (CANNA_initfilename) {
    free(CANNA_initfilename);
  }
  CANNA_initfilename = NULL;
}

static void
freeExtra()
{
  extern extraFunc* extrafuncp;
  extraFunc *p, *q;

  for (p = extrafuncp; p; p = q) {
    q = p->next;
    switch (p->keyword) {
      case EXTRA_FUNC_DEFMODE:
        if (p->u.modeptr->romdic_owner && p->u.modeptr->romdic != NULL) {
          RkwCloseRoma(p->u.modeptr->romdic);
        }
        free(p->u.modeptr->emode);
        if (p->u.modeptr->romaji_table) {
          free(p->u.modeptr->romaji_table);
        }
        free(p->u.modeptr);
        break;
      case EXTRA_FUNC_DEFSELECTION:
        free(p->u.kigoptr->kigo_str);
        free(p->u.kigoptr->kigo_data);
        free(p->u.kigoptr);
        break;
#ifndef NO_EXTEND_MENU
      case EXTRA_FUNC_DEFMENU:
        freeMenu(p->u.menuptr);
        break;
#endif
    }
    free(p);
  }
  extrafuncp = (extraFunc*)0;
}

static int
KC_finalize(uiContext d, char* arg)
/* ARGSUSED */
{
  extern int FirstTime;
  int res;

  /* ウォーニングメッセージの初期化 */
  initWarningMesg();
  if (arg) {
    *(char***)arg = 0;
  }

  if (FirstTime) {
    jrKanjiError =
      "\275\351\264\374\262\275\244\342\244\265\244\354\244\306"
      "\244\244\244\312\244\244\244\316\244\313\241\330\275\252\244\357"
      "\244\354\241\331\244\310\270\300\244\357\244\354\244\336\244\267"
      "\244\277";
    /* 初期化もされていないのに『終われ』と言われました */
    return -1;
  } else {
    FirstTime = 1;

    /* カナ漢字変換の終了 */
    res = KanjiFin();

    /* ローマ字かな変換の終了 */
    RomkanaFin();

    /* free all uiContexts and hash tables here */

    /* キーマップテーブルのクリア */
    restoreDefaultKeymaps();

    /* モード文字列のフリー */
    resetModeNames();

    /* uiContext の検索のためのハッシュテーブルをクリア
       uiContext も一緒にフリーする */
    clearHashTable();

    /* 記号定義を消す */
    freeKeysup();

    /* 色々とカスタマイズされている所をもとに戻す。 */
    restoreBindings();

    /* 拡張機能のinitfile表示用のバッファを解放する */
    freeBuffer();

    /* ワイドキャラクタ用自動メモリ管理の終了処理 */
    WStringClose();

    /* サーバ名保持用メモリの開放 */
    /* RkSetServerName((char *)0); してはいけないのでは？ */

#ifndef NO_EXTEND_MENU
    /* メニュー関連のメモリの開放 */
    finExtMenu();
#endif

    /* デフォルト以外のモード用メモリの開放 */
    freeExtra();

    if (arg) {
      *(char***)arg = nWarningMesg ? WarningMesg : NULL;
    }
    return res;
  }
}

static int
KC_setWidth(uiContext d, caddr_t arg)
{
  d->ncolumns = (int)(POINTERINT)arg;
  return 0;
}

static int
KC_setBunsetsuKugiri(uiContext d, caddr_t arg)
/* ARGSUSED */
{
  cannaconf.BunsetsuKugiri = (int)(POINTERINT)arg;
  return 0;
}

static long gflags[] = {
  0L,
  CANNA_YOMI_BASE_HANKAKU,
  CANNA_YOMI_KATAKANA,
  CANNA_YOMI_KATAKANA | CANNA_YOMI_HANKAKU | CANNA_YOMI_BASE_HANKAKU,
  CANNA_YOMI_ROMAJI | CANNA_YOMI_ZENKAKU,
  CANNA_YOMI_ROMAJI | CANNA_YOMI_BASE_HANKAKU,
  CANNA_YOMI_KAKUTEI,
  CANNA_YOMI_BASE_HANKAKU | CANNA_YOMI_KAKUTEI,
  CANNA_YOMI_KATAKANA | CANNA_YOMI_KAKUTEI,
  CANNA_YOMI_KATAKANA | CANNA_YOMI_HANKAKU | CANNA_YOMI_BASE_HANKAKU |
    CANNA_YOMI_KAKUTEI,
  CANNA_YOMI_ROMAJI | CANNA_YOMI_ZENKAKU | CANNA_YOMI_KAKUTEI,
  CANNA_YOMI_ROMAJI | CANNA_YOMI_BASE_HANKAKU | CANNA_YOMI_KAKUTEI,
};

static int
KC_changeMode(uiContext d, wcKanjiStatusWithValue* arg)
{
  coreContext cc;
  yomiContext yc;

  d->buffer_return = arg->buffer;
  d->n_buffer = arg->n_buffer;
  d->kanji_status_return = arg->ks;

  bzero(d->kanji_status_return, sizeof(wcKanjiStatus));

  d->nbytes = escapeToBasicStat(d, CANNA_FN_Quit);
  cc = (coreContext)d->modec;
  d->kanji_status_return->info &= ~(KanjiThroughInfo | KanjiEmptyInfo);

  if (cc->majorMode == CANNA_MODE_AlphaMode) {
    /* αモードだったら抜ける。
       ベーシックモードはαモードか変換モードぐらいしかないと思う。 */
    if (arg->val == CANNA_MODE_AlphaMode) {
      return 0;
    } else {
      cc = (coreContext)cc->next; /* 次のコンテクスト */
      yc = (yomiContext)cc;

      if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
        CannaBeep();
        arg->val = 0;
        return 0;
      }

      doFunc(d, CANNA_FN_JapaneseMode);
    }
  } else {
    yc = (yomiContext)cc;

    if (yc->generalFlags & CANNA_YOMI_CHGMODE_INHIBITTED) {
      CannaBeep();
      arg->val = 0;
      return 0;
    }
  }

  switch (arg->val) {
    case CANNA_MODE_AlphaMode:
      arg->val = doFunc(d, CANNA_FN_AlphaMode);
      return 0;

    case CANNA_MODE_HenkanMode:
      arg->val = doFunc(d, CANNA_FN_HenkanNyuryokuMode);
      return 0;

#ifndef NO_EXTEND_MENU
    case CANNA_MODE_HexMode:
      arg->val = doFunc(d, CANNA_FN_HexMode);
      return 0;
#endif /* not NO_EXTEND_MENU */

    case CANNA_MODE_BushuMode:
      arg->val = doFunc(d, CANNA_FN_BushuMode);
      return 0;

    case CANNA_MODE_KigoMode:
      arg->val = doFunc(d, CANNA_FN_KigouMode);
      return 0;

    case CANNA_MODE_TourokuMode:
      arg->val = doFunc(d, CANNA_FN_ExtendMode);
      return 0;

    case CANNA_MODE_HanKataHenkanMode:
    case CANNA_MODE_HanKataKakuteiMode:
      if (cannaconf.InhibitHankakuKana) {
        CannaBeep();
        arg->val = 0;
        return 0;
      }
    case CANNA_MODE_ZenHiraHenkanMode:
    case CANNA_MODE_HanHiraHenkanMode:
    case CANNA_MODE_ZenKataHenkanMode:
    case CANNA_MODE_ZenAlphaHenkanMode:
    case CANNA_MODE_HanAlphaHenkanMode:
    case CANNA_MODE_ZenHiraKakuteiMode:
    case CANNA_MODE_HanHiraKakuteiMode:
    case CANNA_MODE_ZenKataKakuteiMode:
    case CANNA_MODE_ZenAlphaKakuteiMode:
    case CANNA_MODE_HanAlphaKakuteiMode:
      yc->generalFlags &= ~(CANNA_YOMI_ATTRFUNCS | CANNA_YOMI_BASE_HANKAKU);
      yc->generalFlags |= gflags[arg->val - CANNA_MODE_ZenHiraHenkanMode];
      EmptyBaseModeInfo(d, yc);
      arg->val = 0;
      return 0;
    default:
      return (-1);
  }
  /* NOTREACHED */
}

static int
baseModeP(uiContext d)
{
  extern KanjiModeRec alpha_mode, empty_mode;

  return (d->current_mode == &alpha_mode) ||
         (d->current_mode == &empty_mode &&
          ((yomiContext)(d->modec))->next == (mode_context)0);
}

/*

  基本的な状態にもどる。すなわち、読みが入っていたり変換中の状態から抜
  ける。いかに抜けるかは第２引数で指定する。抜け方としては

  ・QUIT (C-g) で抜ける
  ・確定 (Return) で抜ける

  がある。

*/

int
escapeToBasicStat(uiContext d, int how)
{
  int len = 0, totallen = 0;
  wchar_t* p = d->buffer_return;
  int totalinfo = 0;
  int maxcount = 32;

  do {
    if (d->kanji_status_return) {
      d->kanji_status_return->length = 0;
      totalinfo |= (d->kanji_status_return->info & KanjiModeInfo);
    } else {
      return -1;
    }
    d->kanji_status_return->info = 0;
    d->nbytes = 0; /* この値を入力文字長として使う場合があるのでクリアする */
    len = doFunc(d, how);
    d->buffer_return += len;
    d->n_buffer -= len;
    totallen += len;
    maxcount--;
  } while (maxcount > 0 && !baseModeP(d));
  d->kanji_status_return->info |= KanjiGLineInfo | totalinfo;
  d->kanji_status_return->gline.length = 0;
  d->kanji_status_return->gline.revPos = 0;
  d->kanji_status_return->gline.revLen = 0;
  d->buffer_return = p;
  return totallen;
}

static int
KC_setUFunc(uiContext d, caddr_t arg)
/* ARGSUSED */
{
  extern int howToBehaveInCaseOfUndefKey;

  howToBehaveInCaseOfUndefKey = (int)(POINTERINT)arg;
  return 0;
}

static int
KC_setModeInfoStyle(uiContext d, caddr_t arg)
/* ARGSUSED */
{
  int tmpval;
  extern int howToReturnModeInfo;

  if ((tmpval = (int)(POINTERINT)arg) < 0 || tmpval > MaxModeInfoStyle)
    return (-1);
  howToReturnModeInfo = (int)(POINTERINT)arg;
  return 0;
}

static int
KC_setHexInputStyle(uiContext d, caddr_t arg)
/* ARGSUSED */
{
  cannaconf.hexCharacterDefiningStyle = (int)(POINTERINT)arg;
  return 0;
}

static int
KC_inhibitHankakuKana(uiContext d, caddr_t arg)
/* ARGSUSED */
{
  cannaconf.InhibitHankakuKana = (int)(POINTERINT)arg;
  return 0;
}

#ifndef NO_EXTEND_MENU
extern void popTourokuMode(uiContext);

static int
popTourokuWithGLineClear(uiContext d, int retval, mode_context env)
/* ARGSUSED */
{
  tourokuContext tc;

  popCallback(d); /* 読みを pop */

  tc = (tourokuContext)d->modec;
  if (tc->udic) {
    free(tc->udic);
  }
  popTourokuMode(d);
  popCallback(d);
  GlineClear(d);
  currentModeInfo(d);
  return 0;
}
#endif

static int
KC_defineKanji(uiContext d, wcKanjiStatusWithValue* arg)
{
#ifdef NO_EXTEND_MENU
  return 0;
#else
  d->buffer_return = arg->buffer;
  d->n_buffer = arg->n_buffer;
  d->kanji_status_return = arg->ks;

  if (arg->ks->length > 0 && arg->ks->echoStr && arg->ks->echoStr[0]) {
    wchar_t xxxx[ROMEBUFSIZE]; /* BIGARRAY */

    WStrncpy(xxxx, arg->ks->echoStr, arg->ks->length);
    xxxx[arg->ks->length] = (wchar_t)0;

    bzero(d->kanji_status_return, sizeof(wcKanjiStatus));

    d->nbytes = escapeToBasicStat(d, CANNA_FN_Quit);
    d->kanji_status_return->info &= ~(KanjiThroughInfo | KanjiEmptyInfo);
    dicTourokuControl(d, xxxx, popTourokuWithGLineClear);
    arg->val = d->nbytes;
  } else {
    d->nbytes = escapeToBasicStat(d, CANNA_FN_Quit);
    d->kanji_status_return->info &= ~(KanjiThroughInfo | KanjiEmptyInfo);
    arg->val = dicTourokuControl(d, 0, popTourokuWithGLineClear);
  }
  arg->val = callCallback(d, arg->val);

  return 0;
#endif /* NO_EXTEND_MENU */
}

/* cfuncdef

  RK コンテクストを無効にする。
  flag が０以外なら RkwClose() も行う。

 */

static void
closeRK(int* cxp, int flag)
{
  if (flag && *cxp >= 0) {
    RkwCloseContext(*cxp);
  }
  *cxp = -1;
}

/* cfuncdef

   closeRKContextInUIContext -- uiContext 中の RK コンテクストを close する。

 */

static void
closeRKContextInUIContext(uiContext /*d*/, int /*flag*/);

static void
closeRKContextInUIContext(uiContext d,
                          int flag) /* ０以外ならクローズもする。 */
{
  coreContext cc;

  closeRK(&(d->contextCache), flag);
  for (cc = (coreContext)d->modec; cc; cc = (coreContext)cc->next) {
    if (cc->id == YOMI_CONTEXT) {
      closeRK(&(((yomiContext)cc)->context), flag);
    }
  }
}

/* cfuncdef

  closeRKContextInMemory() -- すべての RK コンテクストのクローズ

  バケットによってポイントされているデータ内の全ての RK コンテクストを
  クローズする。

*/

static void
closeRKContextInMemory(struct bukRec* p, int flag)
{

  while (p) { /* reconfirm that p points some structure */
    closeRKContextInUIContext(p->context, flag);
    p = p->next;
  }
}

/* cfuncdef

  makeContextToBeClosed() -- ハッシュテーブル内のコンテクストを無効にする

*/

void
makeAllContextToBeClosed(int flag)
{
  int i;
  struct bukRec* p;

  for (i = 0; i < HASHTABLESIZE; i++) {
    p = conHash[i];
    if (p) {
      closeRKContextInMemory(p, flag);
    }
  }
}

static int
KC_kakutei(uiContext d, wcKanjiStatusWithValue* arg)
{
  d->buffer_return = arg->buffer;
  d->n_buffer = arg->n_buffer;
  d->kanji_status_return = arg->ks;

  bzero(d->kanji_status_return, sizeof(wcKanjiStatus));

  d->nbytes = escapeToBasicStat(d, CANNA_FN_Kakutei);
  if (!baseModeP(d)) {
    d->nbytes = escapeToBasicStat(d, CANNA_FN_Quit);
  }
  d->kanji_status_return->info &= ~KanjiThroughInfo;
  arg->val = d->nbytes;
  return d->nbytes;
}

static int
KC_kill(uiContext d, wcKanjiStatusWithValue* arg)
{
  d->buffer_return = arg->buffer;
  d->n_buffer = arg->n_buffer;
  d->kanji_status_return = arg->ks;

  bzero(d->kanji_status_return, sizeof(wcKanjiStatus));

  d->nbytes = escapeToBasicStat(d, CANNA_FN_Quit);
  d->kanji_status_return->info &= ~KanjiThroughInfo;
  arg->val = d->nbytes;
  return d->nbytes;
}

static int
KC_modekeys(uiContext d, unsigned char* arg)
{
  int n = 0;
  int i;
  extern KanjiModeRec alpha_mode;
  int func;

  for (i = 0; i < 256; i++) {
    func = alpha_mode.keytbl[i];
    if (func != CANNA_FN_SelfInsert && func != CANNA_FN_FunctionalInsert &&
        func != CANNA_FN_Undefined && func != CANNA_FN_FuncSequence &&
        func != CANNA_FN_UseOtherKeymap &&
        alpha_mode.func(d, &alpha_mode, KEY_CHECK, 0 /*dummy*/, func)) {
      arg[n++] = i;
    }
  }
  return n;
}

static int
KC_queryMode(uiContext d, wchar_t* arg)
{
  return queryMode(d, arg);
}

static int
KC_queryConnection(uiContext d, unsigned char* arg)
/* ARGSUSED */
{
  extern int defaultContext;

  if (defaultContext != -1) {
    return 1;
  } else {
    return 0;
  }
}

static int
KC_parse(uiContext d, char** arg)
/* ARGSUSED */
{
  initWarningMesg();

  parse_string(*arg);

  *(char***)arg = nWarningMesg ? WarningMesg : NULL;

  return nWarningMesg;
}

int yomiInfoLevel = 0;

static int
KC_yomiInfo(uiContext d, int arg)
/* ARGSUSED */
{
  yomiInfoLevel = arg;
  return 0;
}

static int
KC_storeYomi(uiContext d, wcKanjiStatusWithValue* arg)
{
  extern KanjiModeRec yomi_mode, cy_mode;
  coreContext cc;
  wchar_t *p, *q;
  int len = 0;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  wchar_t buf[2048];
#else
  wchar_t* buf = (wchar_t*)malloc(sizeof(wchar_t) * 2048);
  if (!buf) {
    /* This should the 'no more memory' message on the guide line... */
    arg->val = 0;
    arg->ks->length = 0;
    arg->ks->info = 0;
    return len;
  }
#endif

  p = arg->ks->echoStr;
  q = arg->ks->mode;
  if (p) {
    WStrcpy(buf, p);
    p = buf;
    len = WStrlen(buf);
  }
  if (q) {
    WStrcpy(buf + len + 1, q);
    q = buf + len + 1;
  }
  KC_kill(d, arg);
  cc = (coreContext)d->modec;
  if (cc->majorMode == CANNA_MODE_AlphaMode) {
    doFunc(d, CANNA_FN_JapaneseMode);
  }
  d->kanji_status_return = arg->ks;
  d->kanji_status_return->info &= ~(KanjiThroughInfo | KanjiEmptyInfo);
  RomajiStoreYomi(d, p, q);
  if (p && *p) {
    d->current_mode =
      (((yomiContext)d->modec)->generalFlags & CANNA_YOMI_CHIKUJI_MODE)
        ? &cy_mode
        : &yomi_mode;
  }
  makeYomiReturnStruct(d);
  arg->val = 0;
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  free(buf);
#endif
  return 0;
}

char* initFileSpecified = NULL;

static int
KC_setInitFileName(uiContext d, char* arg)
/* ARGSUSED */
{
  int len;

  if (initFileSpecified) { /* 以前のものをフリーする */
    free(initFileSpecified);
  }

  if (arg && *arg) {
    len = strlen(arg);
    initFileSpecified = malloc(len + 1);
    if (initFileSpecified) {
      strcpy(initFileSpecified, arg);
    } else {
      return -1;
    }
  } else {
    initFileSpecified = NULL;
  }
  return 0;
}

static int
KC_do(uiContext d, wcKanjiStatusWithValue* arg)
{
  d->buffer_return = arg->buffer;
  d->n_buffer = arg->n_buffer;
  d->kanji_status_return = arg->ks;
  d->ch = (unsigned)*(d->buffer_return);
  d->nbytes = 1;

  bzero(d->kanji_status_return, sizeof(wcKanjiStatus));

  arg->val = doFunc(d, arg->val);
  return arg->val;
}

#ifndef NO_EXTEND_MENU
/*

  トップレベルにはないモードに対して何らかの作業を
  させたいときに呼び出す関数。fnum == 0 で d->ch を見る。

  '91.12.28 現在で uldefine.c からしか呼ばれておらず modec の値は
  yomi_mode しか入っていない。

  この関数はその先で呼ぶ関数によってモードがプッシュされた時など復帰さ
  せることが出来なくなってしまうのでモード変更を伴う機能の呼出しを一時
  的に禁止することとする。

 */

int
_do_func_slightly(uiContext d, int fnum, mode_context mode_c, KanjiMode c_mode)
{
  wcKanjiStatus ks;
  long gfback = 0;
  BYTE inhback = 0;
  int retval = 0;
  yomiContext yc = (yomiContext)0;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  uiContextRec f, *e = &f;
#else
  uiContext e = (uiContext)malloc(sizeof(uiContextRec));

  if (e) {
#endif

  bzero(e, sizeof(uiContextRec));
  e->buffer_return = e->genbuf;
  e->n_buffer = ROMEBUFSIZE;
  e->kanji_status_return = &ks;

  e->nbytes = d->nbytes;
  e->ch = d->ch;

  e->status = 0; /* モードについて"処理中"のステータスを既定値とする */
  e->more.todo = 0;
  e->modec = mode_c;
  e->current_mode = c_mode;
  e->cb = NULL;

  if (((coreContext)mode_c)->id == YOMI_CONTEXT) {
    yc = (yomiContext)mode_c;
    gfback = yc->generalFlags;
    inhback = yc->henkanInhibition;
    yc->generalFlags |= CANNA_YOMI_CHGMODE_INHIBITTED;
    yc->henkanInhibition |= CANNA_YOMI_INHIBIT_ALL;
  }

  retval = (*c_mode->func)(e, c_mode, KEY_CALL, e->ch, fnum);

  if (yc) {
    yc->generalFlags = gfback;
    yc->henkanInhibition = inhback;
  }
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  free(e);
}
#endif
return retval;
}

#endif

/* コールバックを呼ぶ */
static int
callCallback(uiContext d, int res)
{
  struct callback* cbp;

  for (cbp = d->cb; cbp;) {
    int index;
    int (*callbackfunc)(uiContext, int, mode_context);

    index = d->status;
    d->status = 0; /* Callback がなくても EXIT、QUIT、AUX はクリアする */
    callbackfunc = cbp->func[index];
    if (callbackfunc) {
      d->kanji_status_return->info &= ~KanjiEmptyInfo;
      if (index) { /* everytime 以外 */
        res = (*callbackfunc)(d, res, cbp->env);
        cbp = d->cb; /* コールバック関数によりコールバックが
                        呼び出されるのを支援するため入れ直す */
        /* ここでコールバック関数をポップアップしようかどうか考えどころ */
        continue;
      } else {
        res = (*callbackfunc)(d, res, cbp->env);
      }
    }
    cbp = cbp->next;
  }
  return res;
}

int
_doFunc(uiContext d, int fnum)
{
  int res = 0, tmpres, ginfo = 0;
  int reallyThrough = 1;
  wchar_t *prevEcho = NULL, *prevGEcho = NULL;
  int prevEchoLen = -1, prevRevPos = 0, prevRevLen = 0;
  int prevGEchoLen = 0, prevGRevPos = 0, prevGRevLen = 0;

  d->status = 0; /* モードについて"処理中"のステータスを既定値とする */
  d->more.todo = 0;
  tmpres = d->current_mode->func(d, d->current_mode, KEY_CALL, d->ch, fnum);

  if (d->flags & MULTI_SEQUENCE_EXECUTED) {
    d->flags &= ~MULTI_SEQUENCE_EXECUTED;
    return tmpres;
  }

  /* コールバックを実行する */
  res = tmpres = callCallback(d, tmpres);

  if (d->kanji_status_return->length >= 0) {
    prevEcho = d->kanji_status_return->echoStr;
    prevEchoLen = d->kanji_status_return->length;
    prevRevPos = d->kanji_status_return->revPos;
    prevRevLen = d->kanji_status_return->revLen;
  }
  if (d->kanji_status_return->info & KanjiGLineInfo) {
    ginfo = 1;
    prevGEcho = d->kanji_status_return->gline.line;
    prevGEchoLen = d->kanji_status_return->gline.length;
    prevGRevPos = d->kanji_status_return->gline.revPos;
    prevGRevLen = d->kanji_status_return->gline.revLen;
  }

  /* moreToDo も実行しなくては */
  while (d->more.todo) {
    if (!(d->kanji_status_return->info & KanjiThroughInfo)) {
      reallyThrough = 0;
    }
    d->kanji_status_return->info &= ~(KanjiThroughInfo | KanjiEmptyInfo);
    d->more.todo = 0;
    d->ch = d->more.ch; /* moreTodo に more.ch はいらないのでは？ */
    d->nbytes = 1;
    d->buffer_return += tmpres;
    d->n_buffer -= tmpres;

    {
      int check;
      /* ２回目以降に以下のデータが失われている場合があるので入れ直す。 */
      d->buffer_return[0] = key2wchar(d->ch, &check);
      if (!check) {
        d->nbytes = 0;
      }
    }

    tmpres = _doFunc(d, d->more.fnum);

    if (tmpres >= 0) {
      res += tmpres;
      if (d->kanji_status_return->length >= 0) {
        prevEcho = d->kanji_status_return->echoStr;
        prevEchoLen = d->kanji_status_return->length;
        prevRevPos = d->kanji_status_return->revPos;
        prevRevLen = d->kanji_status_return->revLen;
      }
      if (d->kanji_status_return->info & KanjiGLineInfo) {
        ginfo = 1;
        prevGEcho = d->kanji_status_return->gline.line;
        prevGEchoLen = d->kanji_status_return->gline.length;
        prevGRevPos = d->kanji_status_return->gline.revPos;
        prevGRevLen = d->kanji_status_return->gline.revLen;
      }
    }
  }
  if (!reallyThrough) {
    d->kanji_status_return->info &= ~KanjiThroughInfo;
  }

  d->kanji_status_return->length = prevEchoLen;
  if (prevEchoLen >= 0) {
    d->kanji_status_return->echoStr = prevEcho;
    d->kanji_status_return->revPos = prevRevPos;
    d->kanji_status_return->revLen = prevRevLen;
  }
  if (ginfo) {
    d->kanji_status_return->gline.line = prevGEcho;
    d->kanji_status_return->gline.length = prevGEchoLen;
    d->kanji_status_return->gline.revPos = prevGRevPos;
    d->kanji_status_return->gline.revLen = prevGRevLen;
    d->kanji_status_return->info |= KanjiGLineInfo;
  }

  return res;
}

int
_afterDoFunc(uiContext d, int retval)
{
  int res = retval;
  wcKanjiStatus* kanji_status_return = d->kanji_status_return;

  /* GLine を消せと言うのなら消しましょう */
  if (d->flags & PLEASE_CLEAR_GLINE) {
    if (d->flags & PCG_RECOGNIZED) { /* 前の前以前なら */
      if (res >= 0 && kanji_status_return->length >= 0) {
        d->flags &= ~(PLEASE_CLEAR_GLINE | PCG_RECOGNIZED);
        /* これで役目を果たしました */
        if (!(kanji_status_return->info & KanjiGLineInfo)) {
          GlineClear(d);
        }
      }
    } else {
      d->flags |= PCG_RECOGNIZED;
    }
  }
  return res;
}

/* cfuncdef

  doFunc -- _doFunc を読んで、さらに ClearGLine 処理や、コールバックの
            処理をする。

 */

int
doFunc(uiContext d, int fnum)
{
  return _afterDoFunc(d, _doFunc(d, fnum));
}

static int
KC_getContext(uiContext d, int arg)
/* ARGSUSED */
{
  extern int defaultContext, defaultBushuContext;

  switch (arg) {
    case 0:
      return RkwDuplicateContext(defaultContext);
    case 1:
      return RkwDuplicateContext(defaultBushuContext);
    case 2:
      return defaultContext;
    default:
      return (-1);
  }
  /* NOTREACHED */
}

static int
KC_closeUIContext(uiContext d, wcKanjiStatusWithValue* arg)
{
  int ret;

  d->buffer_return = arg->buffer;
  d->n_buffer = arg->n_buffer;
  d->kanji_status_return = arg->ks;

  bzero(d->kanji_status_return, sizeof(wcKanjiStatus));

  if ((d->nbytes = escapeToBasicStat(d, CANNA_FN_Quit)) < 0)
    return -1;
  d->kanji_status_return->info &= ~KanjiThroughInfo;
  arg->val = d->nbytes;
  freeRomeStruct(d);

  ret = countContext();
#if defined(DEBUG)
  fprintf(stderr, "ret=%d\n", ret);
#endif
  return ret;
}

static yomiContext
getYomiContext(uiContext d)
{
  coreContext cc = (coreContext)d->modec;
  yomiContext yc;

  switch (cc->id) {
    case YOMI_CONTEXT:
      yc = (yomiContext)cc;
      break;
    default:
      if (cc->minorMode == CANNA_MODE_AlphaMode) {
        yc = (yomiContext)(cc->next);
      } else {
        yc = (yomiContext)0;
      }
      break;
  }
  return yc;
}

static int
KC_inhibitChangeMode(uiContext d, int arg)
{
  yomiContext yc;

  yc = getYomiContext(d);
  if (yc) {
    if (arg) {
      yc->generalFlags |= CANNA_YOMI_CHGMODE_INHIBITTED;
    } else {
      yc->generalFlags &= ~CANNA_YOMI_CHGMODE_INHIBITTED;
    }
    return 0;
  } else {
    return -1;
  }
}

static int
KC_letterRestriction(uiContext d, int arg)
{
  yomiContext yc;

  yc = getYomiContext(d);
  if (yc) {
    yc->allowedChars = arg;
    return 0;
  } else {
    return -1;
  }
}

static int
countColumns(wchar_t* str)
{
  int len = 0;
  wchar_t* p;

  if (str) {
    for (p = str; *p; p++) {
      switch (WWhatGPlain(*p)) {
        case 0:
        case 2:
          len += 1;
          break;
        case 1:
        case 3:
          len += 2;
          break;
      }
    }
  }
  return len;
}

static int
KC_queryMaxModeStr(uiContext d, int arg)
/* ARGSUSED */
{
  int i, maxcolumns = 0, ncols;
  extern struct ModeNameRecs ModeNames[];
  extern extraFunc* extrafuncp;
  extraFunc* ep;

  for (i = 0; i < CANNA_MODE_MAX_IMAGINARY_MODE; i++) {
    ncols = countColumns(ModeNames[i].name);
    if (ncols > maxcolumns) {
      maxcolumns = ncols;
    }
  }
  for (ep = extrafuncp; ep; ep = ep->next) {
    ncols = countColumns(ep->display_name);
    if (ncols > maxcolumns) {
      maxcolumns = ncols;
    }
  }
  return maxcolumns;
}

static int
KC_setListCallback(uiContext d, jrListCallbackStruct* arg)
{
  if (cannaconf.iListCB) {
    d->client_data = NULL;
    d->list_func = (int (*)(char*, int, wchar_t**, int, int*))0;
    return -1;
  }
  if (arg->callback_func) {
    d->list_func = arg->callback_func;
    if (arg->callback_func == &EUCListCallback
#if SUPPORT_OLD_WCHAR
        || arg->callback_func == &owcListCallback
#endif
    ) {
      /* arg->client_dataはスタック上にあるのでuiContextに引っ越し */
      d->elistcb = *((const jrEUCListCallbackStruct*)arg->client_data);
      d->client_data = (char*)&d->elistcb;
    } else {
      d->client_data = arg->client_data;
    }
  } else {
    d->client_data = NULL;
    d->list_func = (int (*)(char*, int, wchar_t**, int, int*))0;
  }
  return 0;
}

static int
KC_setVerbose(uiContext d, int arg)
/* ARGSUSED */
{
  extern int ckverbose;

  ckverbose = arg;
  return 0;
}

/* kanjiInitialize かな漢字変換の初期化 KC_INITIALIZEと等価である。 */

int
kanjiInitialize(char*** mes)
{
  return KC_initialize((uiContext)0, (char*)mes);
}

/* kanjiFinalize KC_FINALIZEと等価である。 */

int
kanjiFinalize(char*** mes)
{
  return KC_finalize((uiContext)0, (char*)mes);
}

/* createKanjiContext コンテクストを作成するものである。 */

unsigned char context_table[100] = "";

int
createKanjiContext()
{
  int i;

  for (i = 0; i < 100; i++) {
    if (!context_table[i]) {
      context_table[i] = 1;
      return i;
    }
  }
  return -1; /* コンテクストを取得できなかった。 */
}

/* wcCloseKanjiContext コンテクストをクローズするもの。 */

int
wcCloseKanjiContext(const int context, wcKanjiStatusWithValue* ksva)
{
  context_table[context] = 0;
  return XwcKanjiControl2(0, context, KC_CLOSEUICONTEXT, (BYTE*)ksva);
}

/* jrCloseKanjiContext コンテクストをクローズするもの。 */

int
jrCloseKanjiContext(const int context, jrKanjiStatusWithValue* ksva)
{
  context_table[context] = 0;
  return XKanjiControl2(0, context, KC_CLOSEUICONTEXT, (BYTE*)ksva);
}

int
ToggleChikuji(uiContext d, int flg)
{
  yomiContext yc = (yomiContext)d->modec;
  extern KanjiModeRec empty_mode;
  extern struct CannaConfig cannaconf;

  if ((yc->generalFlags & CANNA_YOMI_CHIKUJI_MODE) && yc->context != -1) {
    RkwEndBun(yc->context, 0);
    abandonContext(d, yc);
  }
  escapeToBasicStat(d, CANNA_FN_Kakutei);
  d->kanji_status_return->info &= ~KanjiThroughInfo;

  if (flg) {
    /* 逐次自動に変える */
    yc->generalFlags |= CANNA_YOMI_CHIKUJI_MODE;
    yc->majorMode = CANNA_MODE_HenkanMode;
    cannaconf.chikuji = 1;
  } else {
    /* 連文節に変える */
    yc->generalFlags &= ~CANNA_YOMI_CHIKUJI_MODE;
    yc->majorMode = CANNA_MODE_HenkanMode;
    cannaconf.chikuji = 0;
  }
  yc->minorMode = getBaseMode(yc);
  d->majorMode = d->minorMode = CANNA_MODE_AlphaMode; /* ダミー */
  currentModeInfo(d);
  return 0;
}

static int
KC_lispInteraction(uiContext d, int arg)
/* ARGSUSED */
{
  clisp_main();
  return 0;
}

/*
 * サーバとの接続を切る
 */
static int
KC_disconnectServer(uiContext d, int arg)
/* ARGSUSED */
{

#if defined(DEBUG)
  fprintf(stderr, "サーバとの接続を切る\n");
#endif
  jrKanjiPipeError();
  return 0;
}

static int
KC_setAppName(uiContext d, unsigned char* arg)
/* ARGSUSED */
{
  extern int defaultContext;

  if (strlen((char*)arg) > CANNA_MAXAPPNAME) {
    strncpy(saveapname, (char*)arg, CANNA_MAXAPPNAME);
    saveapname[CANNA_MAXAPPNAME - 1] = '\0';
  } else {
    strcpy(saveapname, (char*)arg);
  }
  RkwSetAppName(defaultContext, saveapname);

  return (0);
}

static int
KC_debugmode(uiContext d, int arg)
/* ARGSUSED */
{
  extern int iroha_debug;

  iroha_debug = arg;
  return 0;
}

static void
debug_yomibuf(yomiContext yc)
/* ARGSUSED */
{
#if defined(DEBUG)
  char kana[1024], roma[1024], ka[1024], ya[1024], *kanap, *romap, *kap, *yap;
  int len, i, j, k, maxcol, columns, tmp;
  wchar_t xxx[1024];

#define MANYSPACES "                                                  "

  kanap = kana;
  romap = roma;
  kap = ka;
  yap = ya;

  for (i = 0, j = 0; i < yc->kEndp || j < yc->rEndp;) {
    maxcol = 0;

    if (i < yc->kEndp) {
      k = i + 1;
      columns = 0;
      tmp = (WIsG0(yc->kana_buffer[i]) || WIsG2(yc->kana_buffer[i])) ? 1 : 2;
      if (i == yc->kRStartp && i != yc->kCurs) {
        *kanap++ = '\'';
        *kap++ = '\'';
        columns++;
      }
      if (yc->kAttr[i] & HENKANSUMI) {
        *kap++ = ' ';
      } else {
        *kap++ = 'x';
      }
      if (tmp > 1) {
        *kap++ = ' ';
      }
      columns += tmp;
      while (!(yc->kAttr[k] & SENTOU)) {
        tmp = (WIsG0(yc->kana_buffer[k]) || WIsG2(yc->kana_buffer[k])) ? 1 : 2;
        columns += tmp;
        if (yc->kAttr[k] & HENKANSUMI) {
          *kap++ = ' ';
        } else {
          *kap++ = 'x';
        }
        if (tmp > 1) {
          *kap++ = ' ';
        }
        k++;
      }
      WStrncpy(xxx, yc->kana_buffer + i, k - i);
      xxx[k - i] = (wchar_t)0;
      sprintf(kanap, "%ws ", xxx);
      *kap++ = ' ';
      len = strlen(kanap);
      if (columns > maxcol) {
        maxcol = columns;
      } else {
        strncpy(kanap + len, MANYSPACES, maxcol - columns);
        strncpy(kap, MANYSPACES, maxcol - columns);
        kap += maxcol - columns;
        len += maxcol - columns;
      }
      kanap += len;
      i = k;
    }

    if (j < yc->rEndp) {
      k = j + 1;
      columns =
        (WIsG0(yc->romaji_buffer[j]) || WIsG2(yc->romaji_buffer[j])) ? 1 : 2;
      if (j == yc->rStartp && j != yc->rCurs) {
        *romap++ = '\'';
        columns++;
      }
      while (!(yc->rAttr[k] & SENTOU)) {
        columns +=
          (WIsG0(yc->romaji_buffer[k]) || WIsG2(yc->romaji_buffer[k])) ? 1 : 2;
        k++;
      }
      WStrncpy(xxx, yc->romaji_buffer + j, k - j);
      xxx[k - j] = (wchar_t)0;
      sprintf(romap, "%ws ", xxx);
      len = strlen(romap);
      if (columns > maxcol) {
        strncpy(kanap, MANYSPACES, columns - maxcol);
        kanap += columns - maxcol;
        strncpy(kap, MANYSPACES, columns - maxcol);
        kap += columns - maxcol;
        maxcol = columns;
      } else {
        strncpy(romap + len, MANYSPACES, maxcol - columns);
        len += maxcol - columns;
      }
      romap += len;
      j = k;
    }
  }
  *kap = *kanap = *romap = '\0';
  printf("%s\n", roma);
  printf("%s\n", kana);
  printf("%s\n", ka);
#endif /* DEBUG */
}

static int
KC_debugyomi(uiContext d, int arg)
/* ARGSUSED */
{
  if (((coreContext)(d)->modec)->id == YOMI_CONTEXT) {
    debug_yomibuf((yomiContext)d->modec);
  }
  return 0;
}

static int
KC_queryPhono(uiContext d, char* arg)
/* ARGSUSED */
{
  extern struct RkRxDic* romajidic;
  struct RkRxDic** foo = (struct RkRxDic**)arg;

  *foo = romajidic;
  return 0;
}

static int
KC_setUserInfo(uiContext d, jrUserInfoStruct* arg)
/* ARGSUSED */
{
  extern jrUserInfoStruct* uinfo;
  int ret = -1;
  char *uname, *gname, *srvname, *topdir, *cannafile, *romkanatable;
#ifndef USE_MALLOC_FOR_BIG_ARRAY
  char buf[256];
#else
  char* buf = malloc(256);
  if (!buf) {
    return -1;
  }
#endif

  if (arg) {
    uname = arg->uname ? strdup(arg->uname) : NULL;
    if (uname || !arg->uname) {
      gname = arg->gname ? strdup(arg->gname) : NULL;
      if (gname || !arg->gname) {
        srvname = arg->srvname ? strdup(arg->srvname) : NULL;
        if (srvname || !arg->srvname) {
          topdir = arg->topdir ? strdup(arg->topdir) : NULL;
          if (topdir || !arg->topdir) {
            cannafile = arg->cannafile ? strdup(arg->cannafile) : NULL;
            if (cannafile || !arg->cannafile) {
              romkanatable =
                arg->romkanatable ? strdup(arg->romkanatable) : NULL;
              if (romkanatable || !arg->romkanatable) {
                uinfo = (jrUserInfoStruct*)malloc(sizeof(jrUserInfoStruct));
                if (uinfo) {
                  uinfo->uname = uname;
                  uinfo->gname = gname;
                  uinfo->srvname = srvname;
                  uinfo->topdir = topdir;
                  uinfo->cannafile = cannafile;
                  uinfo->romkanatable = romkanatable;

                  if (uinfo->cannafile) {
                    char* p = uinfo->cannafile;

                    if (strlen(p) >= 3 && (p[0] == '\\' || p[0] == '/' ||
                                           (p[1] == ':' && p[2] == '\\') ||
                                           (p[1] == ':' && p[2] == '/')))
                      strcpy(buf, p);
                    else if (uinfo->uname)
                      sprintf(buf,
                              "%s/%s/%s/%s/%s",
                              uinfo->topdir ? uinfo->topdir : "",
                              "dic",
                              "user",
                              uinfo->uname,
                              uinfo->cannafile);
                    else
                      buf[0] = '\0';
                  } else {
                    sprintf(buf,
                            "%s/%s",
                            uinfo->topdir ? uinfo->topdir : "",
                            "default.can");
                  }
                  wcKanjiControl((int)d, KC_SETINITFILENAME, buf);
                  RkwSetUserInfo(uinfo->uname, uinfo->gname, uinfo->topdir);
                  ret = 1;
                  goto return_ret;
                }
                if (romkanatable)
                  free(romkanatable);
              }
              if (cannafile)
                free(cannafile);
            }
            if (topdir)
              free(topdir);
          }
          if (srvname)
            free(srvname);
        }
        if (gname)
          free(gname);
      }
      if (uname)
        free(uname);
    }
#ifdef CODED_MESSAGE
    jrKanjiError = "malloc (SetUserinfo) できませんでした";
#else
    jrKanjiError =
      "malloc (SetUserinfo) "
      "\244\307\244\255\244\336\244\273\244\363\244\307\244\267\244\277";
#endif
  }
  ret = -1;

return_ret:
#ifdef USE_MALLOC_FOR_BIG_ARRAY
  free(buf);
#endif
  return ret;
}

static int
KC_queryCustom(uiContext d, jrCInfoStruct* arg)
/* ARGSUSED */
{
  extern struct CannaConfig cannaconf;
  static char* input_code[CANNA_MAX_CODE] = { "jis", "sjis", "kuten" };

  if (/* 0 <= cannaconf.code_input && // unsigned なので必ず真 */
      cannaconf.code_input <= CANNA_MAX_CODE) {
    strcpy(arg->codeinput, input_code[cannaconf.code_input]);
  }
  arg->quicklyescape = cannaconf.quickly_escape;
  arg->indexhankaku = cannaconf.indexHankaku;
  arg->indexseparator = cannaconf.indexSeparator;
  arg->selectdirect = cannaconf.SelectDirect;
  arg->numericalkeysel = cannaconf.HexkeySelect;
  arg->kouhocount = cannaconf.kCount;

  return 0;
}

static int
KC_closeAllContext(uiContext d, char* arg)
/* ARGSUSED */
{
  makeAllContextToBeClosed(1);
  return 0;
}

static int
KC_attributeInfo(uiContext d, char* arg)
{
  wcKanjiAttributeInternal** p = (wcKanjiAttributeInternal**)arg;

  if (p) {
    if (!d->attr) {
      d->attr =
        (wcKanjiAttributeInternal*)malloc(sizeof(wcKanjiAttributeInternal));
      if (d->attr) {
        d->attr->u.attr = malloc(ROMEBUFSIZE);
        if (d->attr->u.attr) {
          d->attr->len = ROMEBUFSIZE;
          *p = d->attr;
          return 0;
        }
        free(d->attr);
        d->attr = NULL;
      }
    } else { /* called twice */
      *p = d->attr;
      return 0;
    }
  } else if (d->attr) { /* && !p */
    free(d->attr->u.attr);
    free(d->attr);
    d->attr = NULL;
    return 0;
  }
  return -1;
}

/* 学習辞書に書き込む */
static int
KC_syncDictionary(uiContext d, char* arg)
{
  yomiContext yc = (yomiContext)d->modec;
  RkwSync(yc->context, NULL);
  return 0;
}
/* KanjiControlの個々の制御関数へのポインタ */

static int (*kctlfunc[MAX_KC_REQUEST])() = {
  KC_initialize,
  KC_finalize,
  KC_changeMode,
  KC_setWidth,
  KC_setUFunc,
  KC_setBunsetsuKugiri,
  KC_setModeInfoStyle,
  KC_setHexInputStyle,
  KC_inhibitHankakuKana,
  KC_defineKanji,
  KC_kakutei,
  KC_kill,
  KC_modekeys,
  KC_queryMode,
  KC_queryConnection,
  KC_parse,
  KC_yomiInfo,
  KC_storeYomi,
  KC_setInitFileName,
  KC_do,
  KC_getContext,
  KC_closeUIContext,
  KC_inhibitChangeMode,
  KC_letterRestriction,
  KC_queryMaxModeStr,
  KC_setListCallback,
  KC_setVerbose,
  KC_lispInteraction,
  KC_disconnectServer,
  KC_setAppName,
  KC_debugmode,
  KC_debugyomi,
  KC_keyconvCallback,
  KC_queryPhono,
  KC_setUserInfo,
  KC_queryCustom,
  KC_closeAllContext,
  KC_attributeInfo,
  KC_syncDictionary,
};

int
kanjiControl(int request, uiContext d, caddr_t arg)
{
  return kctlfunc[request](d, arg);
}

#ifndef wchar_t
#error "wchar_t is already undefined"
#endif
#undef wchar_t
/*********************************************************************
 *                       wchar_t replace end                         *
 *********************************************************************/
