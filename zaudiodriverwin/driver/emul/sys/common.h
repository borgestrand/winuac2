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
#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
// WDM.H does not play well with C++.
extern "C"
{
#include <wdm.h>
}
#else
#include <wdm.h>
#endif

#include <windef.h>
#define NOBITMAP
#include <mmreg.h>
#undef NOBITMAP
#include <stdunk.h>
#include <ks.h>
#include <ksmedia.h>
#include <punknown.h>
#include <drmk.h>

#ifndef DEBUG_LEVEL
/*! @brief Debug level ERROR. */
#define DEBUG_LEVEL 4 //DEBUGLVL_ERROR /* How noisy you want it to be. */
#endif

#include <ksdebug.h>


#include <usb.h>
#include <usb100.h>
#include <usb200.h>

extern "C"
{
#include <usbdrivr.h>
#include <usbdlib.h>
}

#include "usbaud10.h"
#include "usbaud20.h"

#include "clist.h"

#endif // _COMMON_H_