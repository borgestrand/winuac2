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


#ifndef _DATAPROCESSINGCONSTANTS_H_
#define _DATAPROCESSINGCONSTANTS_H_

#include <math.h>

typedef union _LBYTE
{
    LONG   lValue;
    BYTE   byteValue[4];
}LBYTE, *PLBYTE;

__inline float Int16ToFloat16(SHORT sig)
{
    return ((float)sig)/32768.0f;
}

__inline float UInt8ToFloat16(UCHAR sig)
{
    return (float)(((SHORT)(CHAR)(sig ^ 0x80)) * 256);
}

__inline float Int32ToFloat16(LONG sig)
{
    return ((float)sig)/2147483648.0f;
}

__inline float Int24ToFloat16(LONG sig)
{
    sig <<= 8;
    return Int32ToFloat16(sig);
}

__forceinline SHORT Float16ToInt16(float sig)
{
    LONG lsig = (LONG)(sig * 32768.0f);
    return lsig > 0x7FFF ? (SHORT)0x7FFF :
           lsig < (LONG)0xFFFF8000 ? (SHORT)0x8000 : (SHORT)lsig;
}

__forceinline UCHAR Float16ToUInt8(float sig)
{
    SHORT ssig = Float16ToInt16(sig);
    return ((UCHAR)(ssig >> 8) & 0xff) ^ 0x80;
}

__forceinline LONG Float16ToInt32(float sig)
{
    sig *= 2147483648.0f;
    return sig >= (float)(LONG)0x7FFFFFFF ? 0x7FFFFFFF :
           sig <= (float)(LONG)0x80000000 ? 0x80000000 : (LONG)sig;
}

__forceinline LONG Float16ToInt24(float sig)
{
    LONG lsig = Float16ToInt32(sig);
    lsig = lsig >> 8;

    return lsig > 0x7FFFFF ? 0x7FFFFF :
           lsig < (LONG)0xFF800000 ? (LONG)0xFF800000 : lsig;
}

__forceinline float dB2Amp(LONG dBValue65535)
{
    float dbValF = (float)dBValue65535;
    return pow((float)10.0f, (float)dbValF/(20.0f * 65536.0f));
}

#endif