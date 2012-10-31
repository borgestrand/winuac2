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
#ifndef _DBG_H_
#define _DBG_H_

#ifdef DBG
#define DEBUG
#endif

#define FILE_LOGGING

enum
{
    DEBUGLVL_ERROR = 0,
    DEBUGLVL_TERSE,
    DEBUGLVL_VERBOSE,
    DEBUGLVL_BLAB
};

#define DBG_VARIABLE DEBUGLVL_BLAB

extern void DbgPrint(IN LPSTR,...);

#ifdef DEBUG
#ifndef FILE_LOGGING
#define _DbgPrintF(lvl, strings) \
     {\
        if ((lvl) <= DBG_VARIABLE)\
        {\
            OutputDebugString(STR_MODULENAME);\
            DbgPrint##strings;\
            OutputDebugString("\n");\
        }\
     }
#else
#define _DbgPrintF(lvl, strings) \
     {\
        if ((lvl) <= DBG_VARIABLE)\
        {\
            DbgPrint(STR_MODULENAME);\
            DbgPrint##strings;\
            DbgPrint("\n");\
        }\
     }
#endif
#define ASSERT(exp) \
     if (!(exp)) DebugBreak()
#else
#define _DbgPrintF(lvl, strings)
#define ASSERT(exp)
#endif


#endif // _DBG_H_
