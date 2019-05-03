/**************************************************************
	kanBeApp.h 1997 by T.Murai
	kanBe and Application Interface by Kodama Takeshi 1997
***************************************************************/

#pragma once
#include <Application.h>

class BFont;
class FEPView;

class kanBeApp : public BApplication {

public:
					kanBeApp();

	virtual	void	MessageReceived(BMessage *msg);

private:
	void			AboutRequested();
	bool			chkKey(void);
	void			makeKanBeWin(void);
	BRect			InitFrame(float w, float h);
	void			ReadSetting(char *path, BFont *aFont);

	void			handle_set_target(BMessage* msg);

private:

	FEPView*		mView;
	BWindow*		mWindow;
};
