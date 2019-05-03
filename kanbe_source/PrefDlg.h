#pragma once

#include <Window.h>

class BMenuField;
class BTextControl;
class PrefDlg : public BWindow
{
public:
					PrefDlg();
private:
	void			MessageReceived(BMessage *msg);
	BMenuField		*makeFontMenu(BRect r, char *defaultfamily);

	BTextControl	*mSizebox;	// font size
	BTextControl	*mPathbox;	// dic base path
	BMenuField		*mFontMenuField;

	float			mFontSize;
	char			mBasePath[B_PATH_NAME_LENGTH];
	char 			*mFontFamily, *mFontStyle;

};

