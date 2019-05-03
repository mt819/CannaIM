#include <Application.h>
#include <Button.h>
#include <StringView.h>
#include <Screen.h>
#include <MenuField.h>
#include <MessageFilter.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <PopUpMenu.h>
#include <TextControl.h>
#include <MenuItem.h>
#include <CheckBox.h>

#include "PrefDlg.h"
#include "kanbeDefs.h"

const rgb_color dlg_back_color = {241,241,254,0};


//////////////////////////////////////////////////////////////////////////////
// Place window to center of Screen.
//////////////////////////////////////////////////////////////////////////////
static BRect
CenterRectOverScreenRect (BRect a)
{
	BRect b = BScreen().Frame();

	b.InsetBy(10,10);

	float width = a.right - a.left;
	float height = a.bottom - a.top;
	float x = b.left + ((b.right - b.left) - width) / 2;
	float y = b.top + ((b.bottom - b.top) - height) / 3;

	a.OffsetTo(x,y);

	return a;
}
//////////////////////////////////////////////////////////////////////////////
// Read setting file
//////////////////////////////////////////////////////////////////////////////
static void
ReadSettings(font_family &f_family, font_style &f_style, float &size, char *basePath)
{
FILE		*fp;
char		buf[B_FILE_NAME_LENGTH+1];

	// Set defaults
	f_family[0] = 0;
	f_style[0] = 0;
	size = 14;
	strcpy(basePath, "/boot/home/config/settings/Canna/");

	if(access(KANBE_SETTING_FILE, R_OK)) return;

	// reading pref file.
	fp = fopen(KANBE_SETTING_FILE, "r");
	while(fgets(buf, sizeof(buf), fp)){
		if(*buf == 'p')	sscanf(buf, "%*s%s" , basePath);
		if(*buf == 'f' )	sscanf(buf, "%*s%*c%[^\n]", f_family);
		if(*buf == 's' && *(buf + 1) == 't')	sscanf(buf, "%*s%*c%[^\n]", f_style);
		if(*buf == 's' && *(buf + 1) == 'i')	sscanf(buf, "%*s%g", &size);
	}
	fclose(fp);
}
//////////////////////////////////////////////////////////////////////////////
// writting setting file.
//////////////////////////////////////////////////////////////////////////////
static void
WriteSettings(font_family *f_family, font_style *f_style, float size, const char *basePath)
{
FILE		*fp;

	// overwrite current settings
	fp = fopen(KANBE_SETTING_FILE, "w");
	fprintf(fp, "path\t%s\n", basePath);
	fprintf(fp, "font\t%s\n", (char *)f_family);
	fprintf(fp, "style\t%s\n", (char*)f_style);
	fprintf(fp, "size\t%g\n", size);
	fclose(fp);
}
//////////////////////////////////////////////////////////////////////////////
// Make Font Menu
//////////////////////////////////////////////////////////////////////////////
BMenuField *
PrefDlg::makeFontMenu(BRect r , char *defaultfamily)
{
	int32	numFonts = count_font_families();
	BPopUpMenu	*pop = new BPopUpMenu("");
	int32	selectItem = 0;

	for(int32 i = 0; i < numFonts; i++){
		font_family family;
		uint32 flags;
		font_style style;
		if ( get_font_family(i, &family, &flags) == B_OK
          && get_font_style(family, 0, &style, &flags) == B_OK ) {
			pop->AddItem(new BMenuItem((char *)family, NULL));
			if(!strcmp((char *)family, defaultfamily))
				selectItem = i;
		}
	}

	(pop->ItemAt(selectItem))->SetMarked(true);
	return 	new BMenuField(r, "", "Default Font :", pop);
}
//////////////////////////////////////////////////////////////////////////////
// PrefDlg
//////////////////////////////////////////////////////////////////////////////
PrefDlg::PrefDlg()
	: BWindow(CenterRectOverScreenRect(BRect(0, 0, 270, 150)),
	"Preference",
	 B_MODAL_WINDOW, B_NOT_RESIZABLE)
{
	Lock();

	// Center dialog over parent window.

	BRect a = Frame();
	CenterRectOverScreenRect(a);
	MoveTo(a.left, a.top);


	// Make the background light gray.

	BView *top = new BView(Bounds(), "", B_FOLLOW_NONE, B_WILL_DRAW);
	AddChild(top);
	top->SetViewColor(dlg_back_color);

	// Check Default Font
	font_family		ffamily;
	font_style		fstyle;
	BFont			Font;

	ReadSettings(ffamily, fstyle, mFontSize, mBasePath);

	const char sizeLabel[] = "Default Font Size :";
	// Font Menu.
	mFontMenuField = makeFontMenu(BRect(6, 12, 250, 33), ffamily);

	top->AddChild(mFontMenuField);
	mFontMenuField->SetDivider(top->StringWidth(sizeLabel));

	char sizebuf[10];
	sprintf(sizebuf, "%g", mFontSize);

	// Font Size.
	mSizebox = new BTextControl(BRect(6,38,150,59),
									"size",
									sizeLabel,
									sizebuf,
									new BMessage('ssiz'));
	top->AddChild(mSizebox);
	mSizebox->SetDivider(top->StringWidth(sizeLabel));

	// Dic Base Path (for /KanBe).
	const char pathLabel[] = "Dic Base Path :";
	mPathbox = new BTextControl(BRect(6,64,265,85),
									"dicpath",
									pathLabel,
									mBasePath,
									NULL);
	top->AddChild(mPathbox);
	mPathbox->SetDivider(top->StringWidth(sizeLabel));

	// OK button.
	BButton *okButton = new BButton(BRect(183, 105, 255, 132),
						"ok", "OK", new BMessage('okok'));
	top->AddChild(okButton);
	SetDefaultButton(okButton);

	// Cancel button.
	BButton *cancelButton = new BButton(BRect(104, 105, 170, 132),
	 					"cancel", "Cancel", new BMessage('cncl'));
	top->AddChild(cancelButton);

	Unlock();
}
//////////////////////////////////////////////////////////////////////////////
// MessageReceived
//////////////////////////////////////////////////////////////////////////////
void PrefDlg::MessageReceived(BMessage *msg)
{
BFont	Font;
	switch (msg->what) {
		case 'okok':
		{
			font_family		ffamily;
			font_style		fstyle;
			uint32			flags;
			BMenu			*m = mFontMenuField->Menu();
			BMenuItem		*item = m->FindMarked();
			int32			index = m->IndexOf(item);

			get_font_family(index, &ffamily, &flags);
			// get first style (Roman or Regular ??)
			get_font_style(ffamily, 0, &fstyle, &flags);

			WriteSettings(&ffamily, &fstyle, mFontSize, mPathbox->Text());
			/* through to case 'cncl' to close window */
		}
		case 'cncl':
			be_app->PostMessage(B_QUIT_REQUESTED);
			Close();
			break;
		case 'ssiz':
			mFontSize = atof(mSizebox->Text());
			if(mFontSize <= 0){
				mFontSize = 14;
				mSizebox->SetText("14");
			}
			break;
		default:
			BWindow::MessageReceived(msg);
			break;
	}
}
