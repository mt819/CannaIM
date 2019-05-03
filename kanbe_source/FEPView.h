/********************************************************
 FEPView.h	by T.Murai 1997
 Inter Application Messageing by Kodama Takeshi 1997
********************************************************/
#pragma once
#include <View.h>

#define K_CONVBUFSIZE 50

class FEPView : public BView {

public:
					FEPView(BRect);
					~FEPView();

		void		SetTarget(BMessenger target);

private:
		void		MouseDown(BPoint where);
		void		KeyDown(const char *bytes, int32 numBytes);

		void		KDrawString(BView *drawView, const uchar *euc, int32 len, BPoint loc);
		float		KStringWidth(const uchar *euc, int32 len);

		void		AttachedToWindow(void);

		void		Draw(BRect);

		void		checkIfFunctionalChar(int *buffer_return);

		void		CopyToClipBoard(const char *euc, int32 len);

		BBitmap		*mOffscreen;
		BView		*mDrawView;
		BMessenger	mTarget;
};

