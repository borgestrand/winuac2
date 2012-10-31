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
 * @file       Profile.cpp
 * @brief      Private profile routines definition.
 * @copyright  E-MU Systems, 2004.
 * @author     <put-your-name-here>.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _PROFILE_H_
#define _PROFILE_H_

#include "Common.h"

// In hexadecimal...
ULONG 
DrvProfileGetUlong
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		ULONG	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
);
USHORT 
DrvProfileGetUshort
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		USHORT	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
);
UCHAR 
DrvProfileGetUchar
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		UCHAR	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
);
// In decimal...
LONG 
DrvProfileGetLong
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		LONG	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
);
SHORT 
DrvProfileGetShort
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		SHORT	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
);
CHAR 
DrvProfileGetChar
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		CHAR	Default,		// return value if key name is not found
    IN		LPWSTR	FilePathName	// address of initialization filename
);
// In ASCII string...
ULONG 
DrvProfileGetString
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		LPSTR	Default,		// return value if key name is not found
    IN		LPSTR	ReturnedString,	// points to destination buffer 
    IN		DWORD	Size,			// size of destination buffer 
    IN		LPWSTR	FilePathName	// address of initialization filename
);
// In hex-byte...
ULONG 
DrvProfileGetUnicodeString
(
    IN		LPSTR	SectionName,	// address of section name
    IN		LPSTR	KeyName,		// address of key name
    IN		PWCHAR	Default,		// return value if key name is not found
    IN		PWCHAR	ReturnedString,	// points to destination buffer 
    IN		DWORD	Size,			// size of destination buffer in characters
    IN		LPWSTR	FilePathName	// address of initialization filename
);

#endif // _PROFILE_H_
