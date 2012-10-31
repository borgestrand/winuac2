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
 * @file	   aci.h
 * @brief	   AudioControl interface definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __AUDIOCONTROL_INTERFACE_H__
#define __AUDIOCONTROL_INTERFACE_H__

#include "interface.h"
#include "alt.h"

#include "terminal.h"
#include "unit.h"
#include "clock.h"

#include "acintep.h"

/*****************************************************************************
 * Classes
 */
class CUsbConfiguration;

/*****************************************************************************
 *//*! @class CAudioControlInterface
 *****************************************************************************
 */
class CAudioControlInterface
:	public CUsbAlternateSetting
{
private:
	CUsbConfiguration *							m_UsbConfiguration;
	PUSB_AUDIO_10_CS_AC_INTERFACE_DESCRIPTOR	m_CsAcInterfaceDescriptor;

	NTSTATUS _ParseCsAcInterfaceDescriptor
	(
		IN		PUSB_INTERFACE_DESCRIPTOR	InterfaceDescriptor
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
	CAudioControlInterface() : CUsbAlternateSetting() {}
    /*! @brief Destructor. */
    ~CAudioControlInterface();
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

	BOOL ParseTerminals
	(
		IN		ULONG		Index,
		OUT		PTERMINAL *	OutTerminal
	);
	BOOL ParseUnits
	(
		IN		ULONG	Index,
		OUT		PUNIT *	OutUnit
	);
	BOOL ParseClocks
	(
		IN		ULONG			Index,
		OUT		PCLOCK_ENTITY *	OutClockEntity
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
	friend class CList<CAudioControlInterface>;
};

typedef CAudioControlInterface * PAUDIOCONTROL_INTERFACE;

#endif // __AUDIOCONTROL_INTERFACE_H__
