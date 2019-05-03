/********************************************************************
	kanBeApp.cpp 1997 by T.Murai
	kanBe and Application Interface by Kodama Takeshi 1997
********************************************************************/
#include <Application.h>
#include <Roster.h>
#include <support/SupportDefs.h>
#include <interface/Screen.h>
#include <stdio.h>
#include <unistd.h>
#include <Alert.h>
#include <Debug.h>

#include "jrkanji.h"
#include "kanbeDefs.h"
#include "PrefDlg.h"
#include "FEPView.h"
#include "kanBeApp.h"


/////////////////////////////////////////////////////////////////////
// About Box
/////////////////////////////////////////////////////////////////////
void
kanBeApp::AboutRequested()
{
		(new BAlert(B_EMPTY_STRING,
				"kanBe for BeOS R3\n"
				"1998.5 by T.Murai \n"
				"kanBe and Application Interface\n"
				"1997 by Kodama Takeshi \n"
				"\n"
				"Japanese input system CANNA Version 3.5b2\n"
				"Copyright 1994 NEC Corporation, Tokyo, Japan.\n"
				,
				"OK"))->Go();

}

/////////////////////////////////////////////////////////////////////
// kanBeApp constructer
// if setting file is not exist or CTRL key pressed when startup,
// then open preference panel
// else start kanbe.
/////////////////////////////////////////////////////////////////////
kanBeApp::kanBeApp()
		: BApplication(KANBE_SIG)
{
	if(access(KANBE_SETTING_FILE, R_OK) || chkKey()){
		(new PrefDlg)->Show();
	}else{
		makeKanBeWin();
	}
}

/////////////////////////////////////////////////////////////////////
// chkKey
// check key is pressed when app start
// Thanks to Kodama Takeshi.
/////////////////////////////////////////////////////////////////////
bool
kanBeApp::chkKey(void)
{
key_info	info;
bool		ret = false;
status_t	err = get_key_info(&info);

	if (!err && info.modifiers & B_CONTROL_KEY) {
		ret = true;
	}

	return ret;
}

/////////////////////////////////////////////////////////////////////
// makeKanBeWin
/////////////////////////////////////////////////////////////////////
void
kanBeApp::makeKanBeWin(void)
{
BFont			aFont;
font_height		h_info;
float			h, w;
char			basepath[B_FILE_NAME_LENGTH];

	ReadSetting(basepath, &aFont);
	setBasePath(basepath);

	w = aFont.StringWidth(B_UTF8_HIROSHI);	// kanji width
	aFont.GetHeight(&h_info);
	h = h_info.ascent + h_info.descent + h_info.leading;

	mWindow = new BWindow(InitFrame(w, h), "kanBe ver. 0.52", B_TITLED_WINDOW,
						B_NOT_RESIZABLE|B_NOT_CLOSABLE|B_NOT_MINIMIZABLE|B_NOT_ZOOMABLE);
	mWindow->Lock();
	mView = new FEPView(mWindow->Bounds());
	mView->SetFont(&aFont);
	mWindow->AddChild(mView);
	mWindow->Unlock();

	mWindow->Show();
}

/////////////////////////////////////////////////////////////////
//init window frame.
/////////////////////////////////////////////////////////////////
BRect
kanBeApp::InitFrame(float w, float h)
{
BRect			frameRect;

	frameRect.Set(0, 0, K_CONVBUFSIZE * w + 2 * 2, h * 2 + 2 * 3);
	frameRect.OffsetTo(10, BScreen().Frame().bottom - h * 2 - 2 * 3 - 30);

	return frameRect;
}

/////////////////////////////////////////////////////////////////
// Read from settings
/////////////////////////////////////////////////////////////////
void
kanBeApp::ReadSetting(char *path, BFont *aFont)
{
FILE		*fp;
char		buf[B_FILE_NAME_LENGTH+1];
font_family	f_family;
font_style	f_style;
float		size;

	fp = fopen(KANBE_SETTING_FILE, "r");
	if(fp == NULL)	exit(B_ERROR);
	while(fgets(buf, sizeof(buf), fp)){
		if(*buf == 'p')	sscanf(buf, "%*s%s" , path);
		if(*buf == 'f' )	sscanf(buf, "%*s%*c%[^\n]", f_family);
		if(*buf == 's' && *(buf + 1) == 't')	sscanf(buf, "%*s%*c%[^\n]", f_style);
		if(*buf == 's' && *(buf + 1) == 'i')	sscanf(buf, "%*s%g", &size);
	}
	fclose(fp);

	aFont->SetFamilyAndStyle(f_family, f_style);
	aFont->SetSize(size);
}

/////////////////////////////////////////////////////////////////////
// Inter Application massageing
// by Kodama Takeshi 1997
/////////////////////////////////////////////////////////////////////
void
kanBeApp::MessageReceived(BMessage *msg)
{
	if (msg->what == KANBE_MESSAGE) {
		int32		command;
		status_t	err;
		err = msg->FindInt32("command", &command);
		if (!err && command == KANBE_COMMAND_SET_TARGET)
			handle_set_target(msg);
	}

}

void
kanBeApp::handle_set_target(BMessage* msg)
{
	BMessenger	target;
	status_t	err;
	err = msg->FindMessenger("target", &target);
	if (!err) {
		app_info	info;
		GetAppInfo(&info);
		be_roster->ActivateApp(info.team);
		if (mWindow->Lock()) {
			mView->SetTarget(target);
			mWindow->Unlock();
		}
	}
}

