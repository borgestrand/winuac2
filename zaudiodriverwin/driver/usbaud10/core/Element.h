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
 * @file	   Element.h
 * @brief	   MIDI element definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __ELEMENT_H__
#define __ELEMENT_H__

#include "Common.h"
#include "UsbDev.h"
#include "usbaudio.h"
#include "Entity.h"


/*****************************************************************************
 * Defines
 */
#define dB			65536
#define INFINITY	(-32768)	

/*****************************************************************************
 * Classes
 */
class CMidiTopology;

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
	CMidiTopology *		m_MidiTopology;

protected:
	PUSB_AUDIO_MIDI_ELEMENT_DESCRIPTOR	m_MidiElementDescriptor;

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
		IN		CMidiTopology *						MidiTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_MIDI_ELEMENT_DESCRIPTOR	MidiElementDescriptor
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

	ULONG NumberOfPins
	(	
		IN		BOOL	Direction
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags = PARAMETER_BLOCK_FLAGS_IO_BOTH
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags = PARAMETER_BLOCK_FLAGS_IO_SOFTWARE
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
