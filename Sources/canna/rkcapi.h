/* Copyright (c) 2003 Canna Project. All rights reserved.
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for any purpose is hereby granted without
 * fee, provided that the above copyright notice appear in all copies
 * and that both that copyright notice and this permission notice
 * appear in supporting documentation, and that the name of the
 * author and contributors not be used in advertising or publicity
 * pertaining to distribution of the software without specific, written
 * prior permission.  The author and contributors no representations
 * about the suitability of this software for any purpose.  It is
 * provided "as is" without express or implied warranty.
 *
 * THE AUTHOR AND CONTRIBUTORS DISCLAIMS ALL WARRANTIES WITH REGARD TO
 * THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS, IN NO EVENT SHALL THE AUTHOR AND CONTRIBUTORS BE LIABLE FOR
 * ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTUOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
 */

/* $Id: rkcapi.h,v 1.2 2003/09/23 07:11:30 aida_s Exp $ */

#ifndef RKCAPI_H
#define RKCAPI_H

typedef void (*RkcConfigErrorProc)(const char *const *errors);
int RkcGetServerFD(void);
int RkcConnectIrohaServer(char *servername);
void RkcListenConfigErrors(RkcConfigErrorProc handler);
int RkcRecvEReply(unsigned char *buf, int bufsize,
      int requiredsize, int *len_return);
int RkcSendERequest(const unsigned char *Buffer, int size);
int RkcRecvWReply(unsigned char *buf, int bufsize,
      int *len_return, unsigned char **allocptr);
int RkcSendWRequest(const unsigned char *Buffer, int size);
int RkwGetProtocolVersion(int *majorp, int *minorp);

#endif /* RKCAPI_H */
/* vim: set sw=2: */
