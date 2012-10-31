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
 * @file       ksaudio.h
 * @brief      KS audio definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _KS_AUDIO_H_
#define _KS_AUDIO_H_

#include <windows.h>
#include <mmsystem.h>
#include <winioctl.h>

#include "stdunk.h"
#include "dbg.h"

#include <ks.h>
#include <ksmedia.h>
#include <tchar.h>
#include <assert.h>
#include <stdio.h>

// filter types
typedef enum 
{
	KS_TECHNOLOGY_TYPE_UNKNOWN = 0,
	KS_TECHNOLOGY_TYPE_AUDIO_RENDER,
	KS_TECHNOLOGY_TYPE_AUDIO_CAPTURE
} KS_TECHNOLOGY_TYPE;

#define KS_FORMAT_MATCHING_CRITERIA_FORMAT_TAG	0x00000001
#define KS_FORMAT_MATCHING_CRITERIA_CHANNEL		0x00000002
#define KS_FORMAT_MATCHING_CRITERIA_BIT_DEPTH	0x00000004
#define KS_FORMAT_MATCHING_CRITERIA_FREQUENCY	0x00000008
#define KS_FORMAT_MATCHING_CRITERIA_DEFAULT		0x0000000F

/*typedef struct
{
    KSP_PIN			Ksp;
    KSMULTIPLE_ITEM KsMultipleItem;
} INTERSECTION;/**/

#include "tlist.h"
#include "irptgt.h"
#include "filter.h"
#include "pin.h"
#include "node.h"
#include "audpin.h"
#include "audfilter.h"
#include "enum.h"

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(ar)    (sizeof(ar)/sizeof(ar[0]))
#endif // SIZEOF_ARRAY

#endif // _KS_AUDIO_H_
