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
 * @file	   asi.h
 * @brief	   AudioStreaming interface definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __AUDIOSTREAMING_INTERFACE_H__
#define __AUDIOSTREAMING_INTERFACE_H__

#include "interface.h"
#include "alt.h"

#include "asdatep.h"
#include "asfbep.h"

/*****************************************************************************
 * Classes
 */
class CUsbConfiguration;
class CAudioControlInterface;

/*****************************************************************************
 *//*! @class CAudioStreamingInterface
 *****************************************************************************
 */
class CAudioStreamingInterface
:	public CUsbAlternateSetting
{
private:
	CUsbConfiguration *								m_UsbConfiguration;
	CAudioControlInterface *						m_AudioControlInterface;
	PUSB_AUDIO_10_CS_AS_INTERFACE_DESCRIPTOR		m_CsAsInterfaceDescriptor;
	PUSB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR		m_CsAsFormatTypeDescriptor;
	PUSB_AUDIO_10_COMMON_FORMAT_SPECIFIC_DESCRIPTOR	m_CsAsFormatSpecificDescriptor;

	VOID _ConfigureClocks
	(	void
	);

	BOOL _IsSamplingFrequencySupported
	(
		IN		ULONG	SamplingFrequency
	);

	NTSTATUS _EnumerateEndpoints
	(
		IN		PUSB_INTERFACE_DESCRIPTOR	InterfaceDescriptor
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CAudioStreamingInterface() : CUsbAlternateSetting() {}
    /*! @brief Destructor. */
    ~CAudioStreamingInterface();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CAudioControlInterface public methods
     *
     * These are public member functions.  See CONFIG.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbConfiguration *			UsbConfiguration,
		IN		CUsbDevice *				UsbDevice,
		IN		PUSB_INTERFACE_DESCRIPTOR	InterfaceDescriptor
	);

	BOOL FindEntity
	(
		IN		UCHAR		EntityID,
		OUT		PENTITY *	OutEntity
	);

	VOID GetAudioFormatInformation
	(
		OUT		USHORT *										OutFormatTag,
		OUT		PUSB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR *	OutFormatTypeDescriptor,
		OUT		ULONG *											OutSamplingFrequency
	);

	BOOL IsAudioFormatSupported
	(
		IN		USHORT										FormatTag,										
		IN		PUSB_AUDIO_10_COMMON_FORMAT_TYPE_DESCRIPTOR	FormatTypeDescriptor,
		IN		ULONG										SamplingFrequency
	);

    /*************************************************************************
	 * The other USB-Audio specification descriptions.
     */
	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
	friend class CList<CAudioStreamingInterface>;
};

typedef CAudioStreamingInterface * PAUDIOSTREAMING_INTERFACE;

#endif // __AUDIOSTREAMING_INTERFACE_H__
