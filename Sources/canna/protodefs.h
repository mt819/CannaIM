/* Copyright 1993 NEC Corporation, Tokyo, Japan.
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

#define canna_version(majv, minv) ((majv) * 1024 + (minv))

typedef canna_uint16_t Ushort;
typedef unsigned char BYTE;
#ifndef CBUFSIZE
#define CBUFSIZE     512
#define CBIGBUFSIZE 4096
#endif
/* ここまで */

#define OLD_MAX_CX  100

#define FALSE 0
#define TRUE 1

/* #define SERVER_VERSION      "3.5" */
#define E_VERSION           "1.2"
#define W_VERSION           "3.3"

#define IR_SERVICE_NAME	    "canna"
#define IR_DEFAULT_PORT	    5680
#define IR_UNIX_PATH	    IR_UNIX_DIR "/" IR_UNIX_SOCKNAME
#define NAME_LENGTH	    15
#define HOST_NAME	    15
			
#define LOMASK(x)	((x)&255)
#define L4TOL(l4)\
        ((((((((unsigned long) ((unsigned char)(l4)[0])) << 8) | \
                ((unsigned long) ((unsigned char)(l4)[1])))  << 8)  | \
                ((unsigned long) ((unsigned char)(l4)[2])))  << 8)      | \
                ((unsigned long) ((unsigned char)(l4)[3])))

#define	L3TOL(l3)\
	((unsigned long)(((((unsigned char)(l3)[0]<<8)|\
        (unsigned char)(l3)[1])<<8) | (unsigned char)(l3)[2]))
#define	S2TOS(s2) ((unsigned short)(((unsigned char)(s2)[0]<<8)|\
	(unsigned char)(s2)[1]))
#define	LTOL4(l, l4)	{\
	(l4)[0] = LOMASK((l)>>24); (l4)[1] = LOMASK((l)>>16);\
	(l4)[2] = LOMASK((l)>> 8); (l4)[3] = LOMASK((l));\
}
#define	LTOL3(l, l3)	{\
(l3)[0] = LOMASK((l)>>16); (l3)[1] = LOMASK((l)>> 8); (l3)[2] = LOMASK((l));\
}
#define	STOS2(s, s2)	{\
	(s2)[0] = LOMASK((s)>> 8); (s2)[1] = LOMASK((s));\
}

/* IXXtoIXX は必要性を考えてあとで削れるなら削る */
#if 0
#define I16toI32(x) (((x) & 0x8000) ? ((x) | 0xffff8000) : (x))
#endif
#define I16toI32(x) (x)
#define I8toI32(x) (((x) & 0x80) ? ((x) | 0xffffff80) : (x))

#if defined(__bsdi__) || defined(__FreeBSD__)
#include <machine/endian.h>
#endif

#define BUFSIZE 	4096
#define MAXDATA 	1024

/* かんなサーバプロトコル */

#define wInitialize 	   0x01
#define wFinalize	   0x02
#define wCreateContext	   0x03
#define wDuplicateContext  0x04
#define wCloseContext	   0x05
#define wGetDictionaryList 0x06
#define wGetDirectoryList  0x07
#define wMountDictionary   0x08
#define wUnmountDictionary 0x09
#define wRemountDictionary 0x0a
#define wGetMountDictionaryList	   0x0b
#define wQueryDictionary   0x0c
#define wDefineWord	   0x0d
#define wDeleteWord	   0x0e
#define wBeginConvert	   0x0f
#define wEndConvert	   0x10
#define wGetCandidacyList  0x11
#define wGetYomi	   0x12
#define wSubstYomi	   0x13
#define wStoreYomi	   0x14
#define wStoreRange	   0x15
#define wGetLastYomi	   0x16
#define wFlushYomi	   0x17
#define wRemoveYomi	   0x18
#define wGetSimpleKanji	   0x19
#define wResizePause	   0x1a
#define wGetHinshi	   0x1b
#define wGetLex		   0x1c
#define wGetStatus	   0x1d
#define wSetLocale	   0x1e
#define wAutoConvert	   0x1f
#define wQueryExtensions   0x20
#define wSetApplicationName	   0x21
#define wNoticeGroupName   0x22
#define wThrough	   0x23
#define wKillServer	   0x24

#define W_REALREQUEST	   0x24 /* same as the max number */
#define W_MAXREQUESTNO	   (W_REALREQUEST + 1)

#ifdef EXTENSION
#define wGetServerInfo	   0x01
#define wGetAccessControlList	   0x02
#define wCreateDictionary  0x03
#define wDeleteDictionary  0x04
#define wRenameDictionary  0x05
#define wGetWordTextDictionary	   0x06
#define wListDictionary	   0x07
#define wSync		   0x08
#define wChmodDictionary   0x09
#define wCopyDictionary	   0x0a
#endif /* EXTENSION */

#define W_MAXEXTREQUESTNO  0x0b /* the max number plus 1 */

