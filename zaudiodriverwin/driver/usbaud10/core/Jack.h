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
 * @file	   Jack.h
 * @brief	   MIDI jack definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __JACK_H__
#define __JACK_H__

#include "Common.h"
#include "UsbDev.h"
#include "usbaudio.h"
#include "Entity.h"

/*****************************************************************************
 * Classes
 */
class CMidiTopology;

/*****************************************************************************
 *//*! @class CMidiJack
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * MIDI jack object.
 */
class CMidiJack
:	public CEntity
{
private:

protected:
	PUSB_AUDIO_MIDI_COMMON_JACK_DESCRIPTOR	m_MidiJackDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CMidiJack() {}
    /*! @brief Destructor. */
	~CMidiJack() {}

    /*************************************************************************
     * CMidiJack public methods
     *
     * These are public member functions.  See JACK.CPP for specific
	 * descriptions.
     */
	UCHAR JackID
	(	void
	);

	UCHAR JackType
	(	void
	);

	virtual BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID,
		OUT		UCHAR * OutSourcePin
	) = 0;

	virtual UCHAR iJack
	(	void
	) = 0;

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
	friend class CList<CMidiJack>;
};

typedef CMidiJack * PMIDI_JACK;

/*****************************************************************************
 *//*! @class CMidiInJack
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * MIDI input jack object.
 */
class CMidiInJack
:	public CMidiJack
{
private:
	CMidiTopology *		m_MidiTopology;

	PUSB_AUDIO_MIDI_IN_JACK_DESCRIPTOR	m_MidiInJackDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CMidiInJack() : CMidiJack() {}
    /*! @brief Destructor. */
    ~CMidiInJack();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CMidiInJack public methods
     *
     * These are public member functions.  See JACK.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CMidiTopology *							MidiTopology,
		IN		PUSB_DEVICE								UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_MIDI_COMMON_JACK_DESCRIPTOR	MidiJackDescriptor
	);

	BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID,
		OUT		UCHAR * OutSourcePin
	);

	UCHAR iJack
	(	void
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CMidiInJack * PMIDI_IN_JACK;

/*****************************************************************************
 *//*! @class CMidiOutJack
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * MIDI output jack object.
 */
class CMidiOutJack
:	public CMidiJack
{
private:
	CMidiTopology *		m_MidiTopology;

	PUSB_AUDIO_MIDI_OUT_JACK_DESCRIPTOR	m_MidiOutJackDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CMidiOutJack() : CMidiJack() {}
    /*! @brief Destructor. */
    ~CMidiOutJack();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CMidiOutJack public methods
     *
     * These are public member functions.  See JACK.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CMidiTopology *							MidiTopology,
		IN		PUSB_DEVICE								UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_MIDI_COMMON_JACK_DESCRIPTOR	MidiJackDescriptor
	);

	BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID,
		OUT		UCHAR * OutSourcePin
	);

	UCHAR iJack
	(	void
	);

	ULONG NumberOfInputPins
	(	void
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CMidiOutJack * PMIDI_OUT_JACK;

#endif // __JACK_H__
