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
 * @file       PrvProp.h
 * @brief      Private property definitions.
 * @details
 *				This header file defines the private property (the GUID) and 
 *				the "messages" therein. It also defines the structure passed 
 *				down.
 * @copyright  E-MU Systems, 2004.
 * @author     <put-your-name-here>.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _PRIVATE_PROPERTY_H_
#define _PRIVATE_PROPERTY_H_

// {4B8F9AFB-EE9F-4890-94EC-636414BD3768}
#define STATIC_KSCATEGORY_AUDIOCONTROL\
	0x4b8f9afb, 0xee9f, 0x4890, 0x94, 0xec, 0x63, 0x64, 0x14, 0xbd, 0x37, 0x68
DEFINE_GUID(KSCATEGORY_AUDIOCONTROL, 
0x4b8f9afb, 0xee9f, 0x4890, 0x94, 0xec, 0x63, 0x64, 0x14, 0xbd, 0x37, 0x68);

// {83BDBE60-0C31-4634-9C95-F4DF21DEA769}
#define STATIC_KSCATEGORY_MIDICONTROL\
	0x83bdbe60, 0xc31, 0x4634, 0x9c, 0x95, 0xf4, 0xdf, 0x21, 0xde, 0xa7, 0x69
DEFINE_GUID(KSCATEGORY_MIDICONTROL, 
0x83bdbe60, 0xc31, 0x4634, 0x9c, 0x95, 0xf4, 0xdf, 0x21, 0xde, 0xa7, 0x69);

// {50894FA4-9206-4a91-8728-CF6F268F271D}
#define STATIC_KSCATEGORY_DEVICECONTROL\
	0x50894fa4, 0x9206, 0x4a91, 0x87, 0x28, 0xcf, 0x6f, 0x26, 0x8f, 0x27, 0x1d
DEFINE_GUID(KSCATEGORY_DEVICECONTROL, 
0x50894fa4, 0x9206, 0x4a91, 0x87, 0x28, 0xcf, 0x6f, 0x26, 0x8f, 0x27, 0x1d);

/*****************************************************************************
 * Private property set {F6D7C04D-C5C3-4181-9ED7-504ABE0CDDFF}
 */
/*! @brief KSPROPSETID_DeviceControl GUID. */
DEFINE_GUID(KSPROPSETID_DeviceControl, 0xf6d7c04d, 0xc5c3, 0x4181, 0x9e, 0xd7, 0x50, 0x4a, 0xbe, 0xc, 0xdd, 0xff);

/*!
 * @brief
 * Device control property set.
 */
typedef enum
{
	// USB device properties...
    KSPROPERTY_DEVICECONTROL_DEVICE_DESCRIPTOR = 0,			// GET only
    KSPROPERTY_DEVICECONTROL_CONFIGURATION_DESCRIPTOR,		// GET only
    KSPROPERTY_DEVICECONTROL_INTERFACE_DESCRIPTOR,			// GET only
    KSPROPERTY_DEVICECONTROL_ENDPOINT_DESCRIPTOR,			// GET only
    KSPROPERTY_DEVICECONTROL_STRING_DESCRIPTOR,				// GET only
    KSPROPERTY_DEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR,	// GET only	
    KSPROPERTY_DEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR,		// GET only
    KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND,				// GET & SET
	// Firmware upgrade support mechanism.
	KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_LOCK = 0x20,	// SET only
	KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_UNLOCK,		// SET only
	// Pin properties...
	KSPROPERTY_DEVICECONTROL_PIN_OUTPUT_CFIFO_BUFFERS = 0x10000,	// SET only
	KSPROPERTY_DEVICECONTROL_PIN_INPUT_CFIFO_BUFFERS,				// SET only
	KSPROPERTY_DEVICECONTROL_PIN_SYNCHRONIZE_START_FRAME			// SET only
} KSPROPERTY_DEVICECONTROL;

// Defines the structures used in the properties above.
typedef struct
{
	KSPROPERTY	Property;
} DEVICECONTROL_DEVICE_DESCRIPTOR, *PDEVICECONTROL_DEVICE_DESCRIPTOR;

typedef struct
{
	KSPROPERTY	Property;
} DEVICECONTROL_CONFIGURATION_DESCRIPTOR, *PDEVICECONTROL_CONFIGURATION_DESCRIPTOR;

typedef struct
{
	LONG	InterfaceNumber;
	LONG	AlternateSetting;
	LONG	InterfaceClass;
	LONG	InterfaceSubClass;
	LONG	InterfaceProtocol;
} INTERFACE_PARAMETERS, *PINTERFACE_PARAMETERS;

typedef struct
{
	KSPROPERTY				Property;
	INTERFACE_PARAMETERS	Parameters;
} DEVICECONTROL_INTERFACE_DESCRIPTOR, *PDEVICECONTROL_INTERFACE_DESCRIPTOR;

typedef struct
{
	UCHAR	InterfaceNumber;
	UCHAR	AlternateSetting;
	UCHAR	EndpointIndex;
} ENDPOINT_PARAMETERS, *PENDPOINT_PARAMETERS;

typedef struct
{
	KSPROPERTY			Property;
	ENDPOINT_PARAMETERS	Parameters;
} DEVICECONTROL_ENDPOINT_DESCRIPTOR, *PDEVICECONTROL_ENDPOINT_DESCRIPTOR;

typedef struct
{
    UCHAR	Index;
    USHORT	LanguageId;
} STRING_PARAMETERS, *PSTRING_PARAMETERS;

typedef struct
{
	KSPROPERTY			Property;
	STRING_PARAMETERS	Parameters;
} DEVICECONTROL_STRING_DESCRIPTOR, *PDEVICECONTROL_STRING_DESCRIPTOR;

typedef struct
{
	UCHAR	InterfaceNumber;
	UCHAR	AlternateSetting;
	UCHAR	ClassSpecificDescriptorType;
} CLASS_INTERFACE_PARAMETERS, *PCLASS_INTERFACE_PARAMETERS;

typedef struct
{
	KSPROPERTY					Property;
	CLASS_INTERFACE_PARAMETERS	Parameters;
} DEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR, *PDEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR;

typedef struct
{
	UCHAR	InterfaceNumber;
	UCHAR	AlternateSetting;
	UCHAR	EndpointAddress;
	UCHAR	ClassSpecificDescriptorType;
} CLASS_ENDPOINT_PARAMETERS, *PCLASS_ENDPOINT_PARAMETERS;

typedef struct
{
	KSPROPERTY					Property;
	CLASS_ENDPOINT_PARAMETERS	Parameters;
} DEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR, *PDEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR;

typedef struct
{
    UCHAR	RequestType;
    UCHAR	Request;
    USHORT	Value;
    USHORT	Index;
	USHORT	Alignment;
	ULONG	Reserved;
    ULONG	BufferLength;
    UCHAR	Buffer[1];
} CUSTOM_COMMAND_PARAMETERS, *PCUSTOM_COMMAND_PARAMETERS;

typedef struct
{
	KSPROPERTY					Property;
	CUSTOM_COMMAND_PARAMETERS	Parameters;
} DEVICECONTROL_CUSTOM_COMMAND, *PDEVICECONTROL_CUSTOM_COMMAND;

#endif // _PRIVATE_PROPERTY_H_

