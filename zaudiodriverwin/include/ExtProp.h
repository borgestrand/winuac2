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
 * @file       ExtProp.h
 * @brief      Device specific extension property definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     <put-your-name-here>.
 * @changelog  03-10-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _EXTENSION_PROPERTY_H_
#define _EXTENSION_PROPERTY_H_

/*****************************************************************************
 * Defines
 */
// {26BB<ext>-<pid>-<vid>-896A-CAAAA12300B8}
#define INIT_XU_PROPSETID(guid, vid, pid, ext)\
{\
    (guid)->Data1 = 0x26bb0000 + (USHORT)(ext);\
    (guid)->Data2 = pid;\
    (guid)->Data3 = vid;\
    (guid)->Data4[0] = 0x89;\
    (guid)->Data4[1] = 0x6a;\
    (guid)->Data4[2] = 0xca;\
    (guid)->Data4[3] = 0xaa;\
    (guid)->Data4[4] = 0xa1;\
    (guid)->Data4[5] = 0x23;\
    (guid)->Data4[6] = 0x00;\
    (guid)->Data4[7] = 0xb8;\
}

#define DEFINE_XU_PROPSETID_GUID(vid, pid, ext)\
    0x26bb0000+(USHORT)(ext), pid, vid, 0x89, 0x6a, 0xca, 0xaa, 0xa1, 0x23, 0x00, 0xb8

#define IS_COMPATIBLE_XU_PROPSETID(guid)\
    (((guid)->Data1 >= 0x26bb0000) &&\
    ((guid)->Data1 < 0x26bb0000 + 0xffff) &&\
    ((guid)->Data4[0] == 0x89) &&\
    ((guid)->Data4[1] == 0x6a) &&\
    ((guid)->Data4[2] == 0xca) &&\
    ((guid)->Data4[3] == 0xaa) &&\
    ((guid)->Data4[4] == 0xa1) &&\
    ((guid)->Data4[5] == 0x23) &&\
    ((guid)->Data4[6] == 0x00) &&\
    ((guid)->Data4[7] == 0xb8))

#define EXTRACT_XU_PID(guid)\
    (USHORT)((guid)->Data2)

#define EXTRACT_XU_MID(guid)\
    (USHORT)((guid)->Data3)

#define EXTRACT_XU_CODE(guid)\
    (USHORT)((guid)->Data1 - 0x26bb0000)

// Extension unit code.
#define XU_CODE_CLOCK_RATE				0xE301
#define XU_CODE_CLOCK_SOURCE			0xE302
#define XU_CODE_DIGITAL_IO_STATUS		0xE303
#define XU_CODE_DEVICE_OPTIONS			0xE304
#define XU_CODE_DIRECT_MONITOR			0xE305
#define XU_CODE_DRIVER_RESYNC			0xE3FE

// Properties.
enum
{
	KSPROPERTY_XU_CLOCK_RATE_UNDEFINED = 0,
	KSPROPERTY_XU_CLOCK_RATE_SUPPORT,
	KSPROPERTY_XU_CLOCK_RATE_SELECTOR,
};

enum
{
	KSPROPERTY_XU_CLOCK_SOURCE_UNDEFINED = 0,
	KSPROPERTY_XU_CLOCK_SOURCE_SELECTOR
};

enum
{
	KSPROPERTY_XU_DEVICE_OPTIONS_UNDEFINED = 0,
	KSPROPERTY_XU_DEVICE_OPTIONS_ANALOG_PAD,
	KSPROPERTY_XU_DEVICE_OPTIONS_SOFT_LIMIT
};

enum
{
	KSPROPERTY_XU_DIGITAL_IO_STATUS_UNDEFINED = 0,
	KSPROPERTY_XU_DIGITAL_IO_STATUS_SAMPLE_RATE,
	KSPROPERTY_XU_DIGITAL_IO_STATUS_SYNC_SOURCE_LOCK_STATE,
	KSPROPERTY_XU_DIGITAL_IO_STATUS_ASYNC_SRC_STATE,
	KSPROPERTY_XU_DIGITAL_IO_STATUS_SPDIF_FORMAT
};

enum
{
	KSPROPERTY_XU_DIRECT_MONITOR_UNDEFINED = 0,
	KSPROPERTY_XU_DIRECT_MONITOR_MONO_STEREO_SWITCH,
	KSPROPERTY_XU_DIRECT_MONITOR_ROUTING
};

// Defines the structures used in the properties above.
#define XU_CLOCK_RATE_SR_44kHz			0x0
#define XU_CLOCK_RATE_SR_48kHz			0x1
#define XU_CLOCK_RATE_SR_88kHz			0x2
#define XU_CLOCK_RATE_SR_96kHz			0x3
#define XU_CLOCK_RATE_SR_176kHz			0x4
#define XU_CLOCK_RATE_SR_192kHz			0x5
#define XU_CLOCK_RATE_SR_UNSPECIFIED	0xFF

#define XU_CLOCK_SOURCE_INTERNAL		0x0
#define XU_CLOCK_SOURCE_SPDIF_EXTERNAL	0x1

#define XU_DIGITAL_IO_STATUS_SPDIF_FORMAT_NONE						0x0
#define XU_DIGITAL_IO_STATUS_SPDIF_FORMAT_CONSUMER_COPYRIGHT_OFF	0x1
#define XU_DIGITAL_IO_STATUS_SPDIF_FORMAT_CONSUMER_COPYRIGHT_ON		0x2
#define XU_DIGITAL_IO_STATUS_SPDIF_FORMAT_PRO						0x3

#define XU_DEVICE_OPTIONS_ANALOG_PAD_PLUS_FOUR	0x1
#define XU_DEVICE_OPTIONS_ANALOG_PAD_MINUS_TEN	0x2

#include <pshpack1.h>

typedef struct
{
	UCHAR	Rates;
} XU_CLOCK_RATE_SUPPORT;

typedef struct
{
	UCHAR	Rate;
} XU_CLOCK_RATE_SELECTOR;

typedef struct
{
	UCHAR	Source;
} XU_CLOCK_SOURCE_SELECTOR;

typedef struct
{
	union
	{
		UCHAR	Index;
		UCHAR	NumberOfOutputs;
	} u;
	UCHAR	Options[1];
} XU_DEVICE_OPTIONS_ANALOG_PAD;

typedef struct
{
	UCHAR	OnOff;
} XU_DEVICE_OPTIONS_SOFT_LIMIT;

typedef struct
{
	ULONG	SampleRate;
} XU_DIGITAL_IO_STATUS_SAMPLERATE;

typedef struct
{
	UCHAR	Lock;
} XU_DIGITAL_IO_STATUS_SYNC_SOURCE_LOCK_STATE;

typedef struct
{
	UCHAR	Active;
} XU_DIGITAL_IO_STATUS_ASYNC_SRC_STATE;

typedef struct
{
	UCHAR	Format;
} XU_DIGITAL_IO_STATUS_SPDIF_FORMAT;

typedef struct
{
	UCHAR	Mono;
} XU_DIRECT_MONITOR_MONO_STEREO_SWITCH;

typedef struct
{
	LONG	Input;
	LONG	Output;
	LONG	Gain;
	BOOL	State;
	LONG	Pan;
} XU_DIRECT_MONITOR_ROUTING;

#include <poppack.h>

#endif // _EXTENSION_PROPERTY_H_
