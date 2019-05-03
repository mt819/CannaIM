/**************************************************************
 FEPView.cpp	by T.Murai 1997
 kanBe and Application Interface by Kodama Takeshi 1997
***************************************************************/

// potions of this software is from Canna32pl2 (sample/sample.c)
// copyrighted 1992 NEC Corporation, Tokyo, Japan.

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

#include <Window.h>
#include <UTF8.h>	// for converting
#include <string.h>
#include <AppKit.h>
#include <Alert.h>
#include <Bitmap.h>
#include <InterfaceDefs.h>

#include "jrkanji.h"
#include "FEPView.h"
#include "kanbeDefs.h"


#define InnerBorder   2


int32				max_mode_strlen;
int32				asc, des, cw;
int32				CursorWidth;
jrKanjiStatus      kanji_status;
uchar				saveMode[K_CONVBUFSIZE];
uchar				saveStr[K_CONVBUFSIZE * 2];
char				saveStrlen, saveLen, savePos;
app_info			a_info;
const int32		    WinWidth = K_CONVBUFSIZE;

/////////////////////////////////////////////////////////////////
// constructer
/////////////////////////////////////////////////////////////////
FEPView::FEPView(BRect frame)
	   	   		:BView(	frame,"FEPView", B_FOLLOW_ALL, B_WILL_DRAW)
{
	char			**warn;
	font_height		h_info;
	uint32			max_width;

	jrKanjiControl(0, KC_INITIALIZE, (char *)&warn);
    if (warn) {
		(new BAlert("Error", "Error has happened in initalizeing Canna.",
		"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go();
		be_app->PostMessage(B_QUIT_REQUESTED);
    }

 	max_mode_strlen = jrKanjiControl(0, KC_QUERYMAXMODESTR, 0) + 2;

	max_width = WinWidth - 2 - max_mode_strlen;
	jrKanjiControl(0, KC_SETWIDTH, (char *)(int32) max_width);

	jrKanjiControl(0, KC_SETHEXINPUTSTYLE, (char *)(int32) 1);


	GetFontHeight(&h_info);
	asc = h_info.ascent;
	des = h_info.descent;

	cw = StringWidth(B_UTF8_HIROSHI); // kanji width
	CursorWidth = cw / 2;

}

/////////////////////////////////////////////////////////////////
// destructer
/////////////////////////////////////////////////////////////////
FEPView::~FEPView()
{
	char              **warn;

	jrKanjiControl(0, KC_FINALIZE, (char *)&warn);

    if (warn) {
		(new BAlert("Error", "Error has happened in finalizeing Canna.",
		"OK", NULL, NULL, B_WIDTH_AS_USUAL, B_WARNING_ALERT))->Go();
    }

	delete mOffscreen;
}

/////////////////////////////////////////////////////////////////
// AttachedToWindow
/////////////////////////////////////////////////////////////////
void FEPView::AttachedToWindow()
{
	jrKanjiStatusWithValue	ks;
	#define BSIZE 256
	uchar buf[K_CONVBUFSIZE];

	// set initial mode japanese input mode.
	ks.val = CANNA_MODE_HenkanMode;
	ks.buffer = buf;
	ks.bytes_buffer = K_CONVBUFSIZE;
	ks.ks = &kanji_status;
	jrKanjiControl(0, KC_CHANGEMODE, (char *)&ks);
	jrKanjiControl(0, KC_QUERYMODE, (char *)saveMode);

	saveStr[0] = 0;
	saveStrlen = 0;
	savePos = 0;
	saveLen = 0;

	mOffscreen = new BBitmap(Bounds(), B_COLOR_8_BIT, true);
	mDrawView = new BView(Bounds(), B_EMPTY_STRING, B_FOLLOW_NONE, 0);
	BFont aFont;
	GetFont(&aFont);
	mDrawView->SetFont(&aFont);
	mOffscreen->AddChild(mDrawView);

	MakeFocus();
}

/////////////////////////////////////////////////////////////////
// Function key check.
/////////////////////////////////////////////////////////////////
void
FEPView::checkIfFunctionalChar(int *buffer_return)
{
	uint32	mod = modifiers();
	int	keysym = *buffer_return;

	switch (*buffer_return) {
	case B_FUNCTION_KEY:
	int32	key;
	BMessage *m;
	m = Window()->CurrentMessage();
	m->FindInt32("key", &key);
		switch(key){
			case B_F1_KEY:
			case B_F2_KEY:
			case B_F3_KEY:
			case B_F4_KEY:
			case B_F5_KEY:
			case B_F6_KEY:
			case B_F7_KEY:
			case B_F8_KEY:
			case B_F9_KEY:
			case B_F10_KEY:
			case B_F11_KEY:
			case B_F12_KEY:
				keysym = (int)CANNA_KEY_F1 + (int)key - (int)B_F1_KEY;
				break;
		}
		break;
	case B_INSERT:
		keysym = CANNA_KEY_Insert;
		break;
	case B_PAGE_UP:
		keysym = CANNA_KEY_Rollup;
		break;
	case B_PAGE_DOWN:
		keysym = CANNA_KEY_Rolldown;
		break;
	case B_UP_ARROW:
		if (mod & (B_SHIFT_KEY | B_CONTROL_KEY) /* control-shifted */ )
			keysym = CANNA_KEY_Cntrl_Up;
		else if (mod & B_SHIFT_KEY /* shifted */ )
			keysym = CANNA_KEY_Shift_Up;
		else
			keysym = CANNA_KEY_Up;
		break;
	case B_DOWN_ARROW:
		if (mod & (B_SHIFT_KEY | B_CONTROL_KEY) /* control-shifted */ )
			keysym = CANNA_KEY_Cntrl_Down;
		else if (mod & B_SHIFT_KEY /* shifted */ )
			keysym = CANNA_KEY_Shift_Down;
		else
			keysym = CANNA_KEY_Down;
		break;
	case B_RIGHT_ARROW:
		if (mod & (B_SHIFT_KEY | B_CONTROL_KEY) /* control-shifted */ )
			keysym = CANNA_KEY_Cntrl_Right;
		else if (mod & B_SHIFT_KEY /* shifted */ )
			keysym = CANNA_KEY_Shift_Right;
		else
			keysym = CANNA_KEY_Right;
		break;
	case B_LEFT_ARROW:
		if (mod & (B_SHIFT_KEY | B_CONTROL_KEY) /* control-shifted */ )
			keysym = CANNA_KEY_Cntrl_Left;
		else if (mod & B_SHIFT_KEY /* shifted */ )
			keysym = CANNA_KEY_Shift_Left;
		else
			keysym = CANNA_KEY_Left;
		break;
	case B_END:
		keysym = CANNA_KEY_Help;
		break;
	case B_HOME:
		keysym = CANNA_KEY_Home;
		break;
	default:
		break;
	}
	*buffer_return = keysym;
}


/////////////////////////////////////////////////////////////////
// Handle Key down.
// Modified by Kodama Takeshi 1997
/////////////////////////////////////////////////////////////////
void
FEPView::KeyDown(const char *bytes, int32 numBytes)
{
int32	n;
char	kakuteiStr[K_CONVBUFSIZE];

	key_info	key;
	get_key_info(&key);
	if ((key.modifiers & B_SHIFT_KEY) != 0 &&
		numBytes == 1 && *bytes == B_SPACE && mTarget.IsValid()) {
		BMessage	msg(KANBE_MESSAGE);
		msg.AddInt32("command", KANBE_COMMAND_ACTIVATE);
		mTarget.SendMessage(&msg);
		return;
	}

	// hide the cursor and caret
	be_app->ObscureCursor();

	int inKey = bytes[0];
	checkIfFunctionalChar(&inKey);

	n = jrKanjiString(0, inKey, kakuteiStr, K_CONVBUFSIZE, &kanji_status);

	if (n > 0 && mTarget.IsValid()) {
		if (n == 1) {
			if (*(uchar*) kakuteiStr == CANNA_KEY_Up)
				*kakuteiStr = B_UP_ARROW;
			else if (*(uchar*) kakuteiStr == CANNA_KEY_Left)
				*kakuteiStr = B_LEFT_ARROW;
			else if (*(uchar*) kakuteiStr == CANNA_KEY_Right)
				*kakuteiStr = B_RIGHT_ARROW;
			else if (*(uchar*) kakuteiStr == CANNA_KEY_Down)
				*kakuteiStr = B_DOWN_ARROW;
		}
		BMessage	msg(KANBE_MESSAGE);
		msg.AddInt32("command", KANBE_COMMAND_INSERT_TEXT);
		char *utf = new char[n * 4];
		int32	len = n * 4;
		int32 state;
		convert_to_utf8(B_EUC_CONVERSION, (const char *)kakuteiStr, &n, utf, &len, &state);
		msg.AddData("text/plain", B_MIME_TYPE, utf, len);
		delete utf;
		mTarget.SendMessage(&msg);
	}

	if (n > 0 ) {
		CopyToClipBoard(kakuteiStr, n);
	}

	Draw(Bounds());
}

/////////////////////////////////////////////////////////////////
// Mouse Down -> Show About
/////////////////////////////////////////////////////////////////
void
FEPView::MouseDown(BPoint /*where*/)
{
	int32 num;

	Window()->CurrentMessage()->FindInt32("clicks", &num);
	if ( num == 2 ) {
		be_app->PostMessage(B_ABOUT_REQUESTED);
	}
}

/////////////////////////////////////////////////////////////////
//Draw
/////////////////////////////////////////////////////////////////
void
FEPView::Draw(BRect)
{
float	ulstart, ulwidth, posy;
	mOffscreen->Lock();
	mDrawView->FillRect(Bounds(), B_SOLID_LOW);

	if(kanji_status.length != -1){
		KDrawString(mDrawView, kanji_status.echoStr, kanji_status.length, BPoint(InnerBorder, InnerBorder + asc));
		strncpy((char *)saveStr, (char *)kanji_status.echoStr, kanji_status.length);
		saveStrlen = kanji_status.length;
		savePos = kanji_status.revPos;
		saveLen = kanji_status.revLen;
	}else{
		KDrawString(mDrawView, saveStr, saveStrlen, BPoint(InnerBorder, InnerBorder + asc));
	}

	if(kanji_status.gline.length > 0){
		KDrawString(mDrawView, kanji_status.gline.line,
					kanji_status.gline.length,
					BPoint(InnerBorder + max_mode_strlen * CursorWidth * 2,
							 2 * InnerBorder + 2 * asc + des + 2));
	}

	if(kanji_status.info & KanjiModeInfo){
		strcpy((char *)saveMode, (char *)kanji_status.mode);
	}

	KDrawString(mDrawView, saveMode, strlen((char *)saveMode),
				BPoint(InnerBorder, InnerBorder*2 +  asc*2 + des + 2));

	// Underline Current Bunsetu.
	if(saveLen > 0 ){
		ulstart = KStringWidth(saveStr, savePos) + InnerBorder;
		ulwidth = KStringWidth(saveStr + savePos, saveLen);
		posy = InnerBorder + asc + des + 1;

		mDrawView->SetHighColor(0, 0, 255);
		mDrawView->StrokeLine(BPoint(ulstart, posy), BPoint(ulstart + ulwidth , posy));
		mDrawView->SetHighColor(0, 0, 0);
	}
	// Underline Current Kouho
	if(kanji_status.gline.revLen > 0){
		ulstart = KStringWidth(kanji_status.gline.line, kanji_status.gline.revPos) + max_mode_strlen * CursorWidth * 2;
		ulwidth = KStringWidth(kanji_status.gline.line + kanji_status.gline.revPos, kanji_status.gline.revLen);
		posy = InnerBorder*2 + asc*2 + des*2 + 2;

		mDrawView->SetHighColor(0, 0, 255);
		mDrawView->StrokeLine(BPoint(InnerBorder + ulstart , posy), BPoint(InnerBorder + ulstart + ulwidth, posy));
		mDrawView->SetHighColor(0, 0, 0);
	}

	mDrawView->Sync();
	DrawBitmap(mOffscreen);
	mOffscreen->Unlock();
}


/////////////////////////////////////////////////////////////////
//Draw EUC wchar string (length = len) to view
/////////////////////////////////////////////////////////////////
void
FEPView::KDrawString(BView *mDrawView, const uchar *euc, int32 len, BPoint loc)
{
	char	*utf = new char[len * 4];
	int32	dstLen = len * 4;
	int32 state;
	convert_to_utf8(B_EUC_CONVERSION, (const char *)euc, &len, utf, &dstLen, &state);
	mDrawView->DrawString(utf, dstLen, loc);

	delete[] utf;
}
/////////////////////////////////////////////////////////////////
// Get Width of EUC string
/////////////////////////////////////////////////////////////////
float
FEPView::KStringWidth(const uchar *euc, int32 len)
{
char	*utf = new char[len * 4];
float	w;
int32	dstLen = len * 4;
int32	state;
	convert_to_utf8(B_EUC_CONVERSION, (const char *)euc, &len, utf, &dstLen, &state);
	w = StringWidth(utf, dstLen);

	delete[] utf;

	return w;
}

/////////////////////////////////////////////////////////////////
// Copy EUC wchar string (length = len) to system clipboard
/////////////////////////////////////////////////////////////////
void
FEPView::CopyToClipBoard(const char *euc, int32 len)
{
char	*utf = new char[len * 4];
int32	dstLen = len * 4;
int32	state;
	convert_to_utf8(B_EUC_CONVERSION, (const char *)euc, &len, utf, &dstLen, &state);

	be_clipboard->Lock();
	be_clipboard->Clear();
	BMessage *clipper = be_clipboard->Data();
	clipper->AddData("text/plain", B_MIME_TYPE, utf, dstLen);
	be_clipboard->Commit();
	be_clipboard->Unlock();

	delete utf;
}

/////////////////////////////////////////////////////////////////
// By Kodama Takeshi 1997.
/////////////////////////////////////////////////////////////////
void FEPView::SetTarget(BMessenger target)
{
	mTarget = target;
}

