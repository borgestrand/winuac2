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
#include <windows.h>

#include "dbg.h"
#include <stdio.h>

#ifdef DEBUG
#ifndef FILE_LOGGING
void DbgPrint(IN LPSTR pszFormat, ...)
{
    char debugstr[256];
    va_list vaList;

    va_start(vaList, pszFormat);
    wvsprintf(debugstr,pszFormat,vaList);
    va_end(vaList);

    OutputDebugString(debugstr);
}
#else
void DbgPrint(IN LPSTR pszFormat, ...)
{
    char debugstr[256];
    va_list vaList;

    va_start(vaList, pszFormat);
    wvsprintf(debugstr,pszFormat,vaList);
    va_end(vaList);

    FILE * f = fopen(".\\hulasvc.log", "a+");
    if (f)
    {
        fprintf(f, "%s", debugstr);
        fclose(f);
    }
}
#endif
#else
#ifdef FILE_LOGGING
void DbgPrint(IN LPSTR pszFormat, ...)
{
    char debugstr[256];
    va_list vaList;

    va_start(vaList, pszFormat);
    wvsprintf(debugstr,pszFormat,vaList);
    va_end(vaList);

    FILE * f = fopen(".\\asiodll.log", "a+");
    if (f)
    {
        fprintf(f, "%s", debugstr);
        fclose(f);
    }
}
#endif
#endif // DEBUG

