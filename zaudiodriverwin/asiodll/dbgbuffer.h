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


#include <stdio.h>
#include <stdarg.h>

#if 1
#define DBG_NUM_LINES 16384
static CHAR dbgBuffer[DBG_NUM_LINES][256] = {0};
static ULONG currline = 0;

void dbgprintf(const char *str, ...)
{
	va_list arglist;
	va_start(arglist, str);

	if(currline == DBG_NUM_LINES)
		currline = 0;

	LONG len = vsprintf(dbgBuffer[currline], str, arglist);

	va_end(arglist);

	dbgBuffer[currline][len] = '\0';

	currline++;
}

void resetdbgbuffer()
{
	currline = 0;
}

void spewdbgbuffer()
{
	for (ULONG i=0; i<currline; i++)
	{
		DbgPrint("%s", dbgBuffer[i]);
	}
}
#else
void dbgprintf(const char *str, ...)
{
}
#define resetdbgbuffer()
#define spewdbgbuffer()
#endif
