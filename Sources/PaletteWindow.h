/*
 * Copyright 2011 Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Copyright 1999 M.Kawamura
 */


#ifndef PALETTEWINDOW_H
#define PALETTEWINDOW_H

#include <Window.h>

extern Preferences gSettings;
class BackBox;
class BLooper;
class BPictureButton;

class PaletteWindow : public BWindow
{
private:
	BLooper*			cannaLooper;
	BPictureButton*		HiraButton;
	BPictureButton*		KataButton;
	BPictureButton*		ZenAlphaButton;
	BPictureButton*		HanAlphaButton;
	BPictureButton*		ExtendButton;
	BPictureButton*		KigoButton;
	BPictureButton*		HexButton;
	BPictureButton*		BushuButton;
	BPictureButton*		TorokuButton;
	BackBox*			fBack;
	void				AllButtonOff();
public:
	PaletteWindow( BRect rect, BLooper* looper );
	virtual void	MessageReceived( BMessage *msg );
	virtual void	FrameMoved( BPoint screenPoint );
};

#endif

