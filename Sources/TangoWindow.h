/*
 * Copyright 2017, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Takashi Murai
 */

#ifndef TANGOWINDOW_H
#define TANGOWINDOW_H


#include <Window.h>

const uint32 MSG_REGISTER				= 'mrew';
const uint32 MSG_TOOLS					= 'mtls';
const uint32 MSG_HIDE_WINDOW			= 'mhdw';

class BString;
class BButton;
class BLooper;
class BMessage;
class BTextControl;


class TangoWindow : public BWindow {
	public:
						TangoWindow(BRect frame, BLooper* looper);

		virtual void	MessageReceived(BMessage* message);
		virtual void	DispatchMessage(BMessage* message, BHandler* handler);
		virtual void	Show();

	private:
		void 			RegistTango();
		status_t		YomiCheck(const char *utf);
		status_t		EucCheck(const char *utf);

		BTextControl	*fTangoString;
		BTextControl	*fYomiString;
		BTextControl	*fHinshiString;
		BButton			*fRegisterButton;
		BButton			*fToolsButton;
		BButton			*fCancelButton;
		BLooper			*fLooper;
};

#endif	// TANGOWINDOW_H

