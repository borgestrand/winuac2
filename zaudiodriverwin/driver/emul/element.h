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
 * @file	   element.h
 * @brief	   MIDI element definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "entity.h"

/*****************************************************************************
 * Defines
 */
#define dB			65536
#define INFINITY	(-32768)	

/*****************************************************************************
 * Classes
 */
class CUsbDevice;

/*****************************************************************************
 *//*! @class CMidiElement
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * MIDI element object.
 */
class CMidiElement
:	public CEntity
{
private:

protected:
	PUSB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR	m_MidiElementDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CMidiElement() {}
    /*! @brief Destructor. */
	~CMidiElement();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CMidiElement public methods
     *
     * These are public member functions.  See ELEMENT.CPP for specific
	 * descriptions.
     */
	UCHAR ElementID
	(	void
	);

	UCHAR iElement
	(	void
	);

	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_MIDI_ELEMENT_DESCRIPTOR	MidiElementDescriptor
	);

	BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID,
		OUT		UCHAR * OutSourcePin
	);

	BOOL ParseCapabilities
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutCapability
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
	friend class CList<CMidiElement>;
};

typedef CMidiElement * PMIDI_ELEMENT;

#endif // __ELEMENT_H__ 
