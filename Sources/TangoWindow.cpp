/*
 * Copyright 2017, Haiku, Inc. All rights reserved.
 * Distributed under the terms of the MIT License.
 *
 * Authors:
 *		Takashi Murai
 */


#include "TangoWindow.h"

#include <Button.h>
#include <Catalog.h>
#include <GroupLayoutBuilder.h>
#include <GridLayoutBuilder.h>
#include <Looper.h>
#include <Locale.h>
#include <LayoutBuilder.h>
#include <Message.h>
#include <Messenger.h>
#include <String.h>
#include <TextControl.h>


#undef B_TRANSLATION_CONTEXT
#define B_TRANSLATION_CONTEXT "TangoWindow"

TangoWindow::TangoWindow(BRect frame, BLooper* looper)
	: BWindow(frame, B_TRANSLATE("Tango"), B_FLOATING_WINDOW,
		B_NOT_RESIZABLE |  B_NOT_ZOOMABLE | B_ASYNCHRONOUS_CONTROLS
		| B_AUTO_UPDATE_SIZE_LIMITS, B_CURRENT_WORKSPACE)
{
	fTangoString = new BTextControl("", B_TRANSLATE("Tango:"), NULL, NULL);
	fYomiString = new BTextControl("", B_TRANSLATE("Yomi:"), NULL, NULL);
	fHinshiString = new BTextControl("", B_TRANSLATE("Hinshi:"), NULL, NULL);

	fRegisterButton = new BButton("", B_TRANSLATE("Register"),
		new BMessage(MSG_REGISTER));
	fCancelButton = new BButton("", B_TRANSLATE("Cancel"),
		new BMessage(MSG_HIDE_WINDOW));
	fToolsButton = new BButton("", B_TRANSLATE("Tools"),
		new BMessage(MSG_TOOLS));

	SetLayout(new BGroupLayout(B_HORIZONTAL));
	AddChild(BGroupLayoutBuilder(B_VERTICAL, 4)
		.Add(BGridLayoutBuilder(3, 2)
				.Add(fTangoString->CreateLabelLayoutItem(), 0, 0)
				.Add(fTangoString->CreateTextViewLayoutItem(), 1, 0)
				.Add(fYomiString->CreateLabelLayoutItem(), 0, 1)
				.Add(fYomiString->CreateTextViewLayoutItem(), 1, 1)
				.Add(fHinshiString->CreateLabelLayoutItem(), 0, 1)
				.Add(fHinshiString->CreateTextViewLayoutItem(), 1, 1)
				)
		.AddGroup(B_HORIZONTAL, 10)
			.Add(fRegisterButton)
			.Add(fCancelButton)
			.AddGlue()
			.Add(fToolsButton)
		.End()
		.SetInsets(10, 10, 10, 10)
	);

	fToolsButton->MakeDefault(true);

	fLooper = looper;

	fTangoString->MakeFocus(true);

}


void
TangoWindow::MessageReceived(BMessage* msg)
{
	switch (msg->what) {
		case MSG_REGISTER:
			RegistTango();
			break;
		case MSG_TOOLS:
//			_SendMessage(MSG_TOOLS);
			break;

		case MSG_HIDE_WINDOW:
			if (!IsHidden())
				Hide();
			break;
		default:
			BWindow::MessageReceived(msg);
			break;
	}
}


void
TangoWindow::DispatchMessage(BMessage* message, BHandler* handler)
{
	BWindow::DispatchMessage(message, handler);
}


void
TangoWindow::Show()
{
	fTangoString->TextView()->SelectAll();
	BWindow::Show();
}


void
TangoWindow::RegistTango()
{
	if (YomiCheck(fYomiString->Text()) || EucCheck(fTangoString->Text()))
// err dlalog
		return;

	BMessage message(MSG_REGISTER);

	// Add the strings
	message.AddString("Tango", fTangoString->Text());
	message.AddString("Yomi", fYomiString->Text());
	message.AddString("Hinshi", fHinshiString->Text());


	fLooper->PostMessage(&message, fLooper);

	PostMessage(MSG_HIDE_WINDOW);
}

status_t
TangoWindow::YomiCheck(const char *utf)
{
	return B_OK;
}


status_t
TangoWindow::EucCheck(const char *utf)
{
	return B_OK;
}



