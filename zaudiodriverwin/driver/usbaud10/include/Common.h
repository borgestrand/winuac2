/*
   This file is part of the EMU CA0189 USB Audio Driver.

   Copyright (C) 2008 EMU Systems/Creative Technology Ltd. 

   This driver is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This driver is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library.   If not, a copy of the GNU Lesser General Public 
   License can be found at <http://www.gnu.org/licenses/>.
*/
/*
 *****************************************************************************
 *//*!
 * @file       Common.h
 * @brief      Common header files & definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _COMMON_H_
#define _COMMON_H_

#include "KsAudio.h"
#include "CList.h"

#ifndef DEBUG_LEVEL
/*! @brief Debug level ERROR. */
#define DEBUG_LEVEL 4 //DEBUGLVL_ERROR /* How noisy you want it to be. */
#endif

#include <ksdebug.h>

#define GTI_SECONDS(t)      (ULONGLONG(t)*10000000)
#define GTI_MILLISECONDS(t) (ULONGLONG(t)*10000)
#define GTI_MICROSECONDS(t) (ULONGLONG(t)*10)

// USB-Audio 1.0 Component ID
#define STATIC_KSCOMPONENTID_USBAUDIO_10 \
	0x2eab1ca, 0xde02, 0x4775, 0xb2, 0x87, 0x15, 0xce, 0x7f, 0xbb, 0x99, 0x35
DEFINE_GUIDSTRUCT("02EAB1CA-DE02-4775-B287-15CE7FBB9935", KSCOMPONENTID_USBAUDIO_10);
#define KSCOMPONENTID_USBAUDIO_10 DEFINE_GUIDNAMED(KSCOMPONENTID_USBAUDIO_10)

// USB-MIDI 1.0 Component ID
#define STATIC_KSCOMPONENTID_USBMIDI_10 \
	0xbde0cd2, 0xa17, 0x4148, 0xac, 0xd7, 0x8c, 0xce, 0xe1, 0x9d, 0x3, 0x49
DEFINE_GUIDSTRUCT("0BDE0CD2-0A17-4148-ACD7-8CCEE19D0349", KSCOMPONENTID_USBMIDI_10);
#define KSCOMPONENTID_USBMIDI_10 DEFINE_GUIDNAMED(KSCOMPONENTID_USBMIDI_10)

#endif // _COMMON_H_