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
 * @file       asiodll.h
 * @brief      ASIO DLL entry points private definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _ASIO_DLL_H_
#define _ASIO_DLL_H_

#include <windows.h>
#include <mmreg.h>
#include <mmsystem.h>

#include "stdunk.h"
#include "dbg.h"
#include "asio.h"
#include "iasiodrv.h"
#include "ksaudio.h"

#ifndef SIZEOF_ARRAY
#define SIZEOF_ARRAY(ar)    (sizeof(ar)/sizeof(ar[0]))
#endif // SIZEOF_ARRAY

#endif // _ASIO_DLL_H_
