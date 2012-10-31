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
/**
 *****************************************************************************
 * @file        MidiParser.h
 * 
 * @brief       Defines the MIDIParser object, a general-purpose MIDI parser
 * 
 * TODO: insert detailed description here if needed, else delete line
 * 
 * @author      Mike Guzewicz, mikeg@atc.creative.com.
 *				Robert S. Crawford
 * 
 * $Date: 2005/06/06 22:32:29 $
 * $Revision: 1.1 $
 * 
 * 
 *****************************************************************************
 * Copyright (C) Creative Technology, Ltd., 1994-2002. All rights reserved.
 *****************************************************************************
 */

#ifndef __MIDIPRSR_H
#define __MIDIPRSR_H

#include "Common.h"
#include "MidiEnum.h"
#include "MidiQueue.h"

/*****************************************************************************
* Defines
*/
#define DEBUG_PARSER

#define NOOP                    0
#define STATUS_BYTE(x)          ((x) & 0x80)
#define CHANNEL_STATUS_LIMIT(x) ((x) < SYSEX)
#define SYSEX_MSG(x)            ((x) == SYSEX)
#define SYSTEM_COMMON_LIMIT(x)  ((x) <= EOX)
#define END_SYSEX_MSG(x)        ((x) == SYSEX_ESC)


#define STATUS_MASK(x)          ((x) & 0xF0)
#define CHANNEL_MASK(x)         (enMIDIChannel)((x) & 0x0F)
#define HINIBBLE(x)             (((x) & 0xF0) >> 4)
#define LONIBBLE(x)             ((x) & 0x0F)

// Sysex Defines
#define NO_SYSEX_COMMAND 0x0000
#define MIDI_SRC_FLAG    0x0100 // I should not use this
#define EMU_DIAG_FUNC    0x0100
#define EMU_EDIT_FUNC    0x0200
#define EMU_SYNTHMODE    0x0300
#define EMU_EDIT_SF_FUNC 0x0400
#define EMU_UNIV_MASTER  0x0600
#define EMU_UNIV_GM      0x0700

#define EMU_DNLD_FUNC    0x0400
#define EMU_RLTM_FUNC    0x0200

/*****************************************************************************
* Structures
*/
/** @brief Structure used to store MIDI state for any message other than a SysEx.
 */
typedef struct 
{
	UCHAR byStatus;				///< MIDI Status
	enMIDIChannel byChannel;	///< MIDI Channel
	UCHAR byData[2];			///< Message specific data
} MIDI_PACKET, *PMIDI_PACKET; 

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CMidiParser
 *****************************************************************************
 * @ingroup MIDIUART_GROUP
 * @brief This class is a general purpose MIDI parser
 *
 * Note this object cannot be used to parse an interleaved stream of MIDI bytes in
 * different channels. Streams must be either pre-parsed such that complete MIDI
 * messages on a single channel pass serially, or multiple MIDI parser instances
 * must be instantiated for each channel.
 *
 * Default in multi-mode.
 *
 */
class CMidiParser
{
private:
    CMidiQueue		m_MidiQueue;		///< The queue for processing SysEx messages

    MIDI_PACKET		m_MidiPacket;		///< The packet of MIDI information
    UCHAR			m_byRTStatus;		///< The Real-time status UCHAR
	UCHAR			m_bySysExStatus;	///< SysEx is an exception to running status, status byte must follow EOX
    UCHAR			m_DataCount;		///< The number of data items left to receive

    enMIDIChannel	m_BasicChannel;		///< The BASIC channel to which OMNI/POLY is sent
    enMIDIChannel   m_OmniChannel;		///< If OMNI is OFF, ignore all BUT this channel
    enMIDIChannel   m_MaxChannel;		///< If OMNI is OFF, and Poly is OFF, parse OmniChannel up to THIS channel and receive controllers for all of those channels only on OmniChannel (not yet implemented)
    BOOL			m_OmniOn;			///< Is OMNI On or Off?

	BOOL			m_NoteFlag;					///< Note filter
    BOOL			m_PolyPressFlag;			///< Poly pressure filter
    BOOL			m_ProgChangeFlag;			///< Prog control filter
    BOOL			m_PitchWheelFlag;			///< Pitch wheel filter
    BOOL			m_ChanPressFlag;			///< Channel Pressure filter
    BOOL			m_CtrlChangeFlag;			///< Control Change filter
    BOOL			m_SysExFlag;				///< System Exclusive filter
    BOOL			m_ChanFlag[MIDICHANNELS];	///< Channel filters
	BOOL			m_OmniFlag;					///< Omni filter

    USHORT			m_ErrorIndex;              ///< Index of the last error

protected:

	/*************************************************************************
     * CMidiParser methods
     *
     * These are protected member functions used internally by the object.  See
     * MidiParser.cpp for specific descriptions.
     */
    void UpdateStatus
	(
		IN		UCHAR	byMIDI
	);
    void SetOmniOffChannel
	(
		IN		enMIDIChannel Channel
	);

public:
    /*************************************************************************
     * Constructor/destructor
     */
    CMidiParser(void);
    ~CMidiParser(void);

    /*************************************************************************
     * CMidiParser methods
     */
    void ResetParser
	(	void
	);

    UCHAR Parse
	(
		IN		UCHAR	byMIDI
	);
	
    void GetPacket
	(
		OUT		MIDI_PACKET *	OutMidiPacket
	);
    
	UCHAR GetStatus
	(	void
	) const;

    enMIDIChannel GetChannel
	(	void
	) const;
    
	UCHAR GetData0
	(	void
	) const;

    UCHAR GetData1
	(	void
	) const;
	
	UCHAR GetDataCount
	(	void
	) const;
	
	UCHAR GetChannelStatus
	(	void
	) const;

	UCHAR GetSysRealTimeStatus
	(	void
	) const;
    
	CMidiQueue * GetMidiQueue
	(	void
	);

    // Omni on/off implementation
    enMIDIChannel GetBasicChannel
	(	void
	);
    
	void SetBasicChannel
	(
		IN		enMIDIChannel	Channel
	);
    
	enMIDIChannel GetMaxChannel
	(	void
	);
    
	void SetMaxChannel
	(
		IN		enMIDIChannel	Channel
	);

    BOOL IsOmniOff
	(	void
	);
    
	BOOL ChannelGetsVoiceMessages
	(
		IN		enMIDIChannel	Channel
	);

	#ifdef DEBUG_PARSER
    void Display(void);
    #endif

	/*************************************************************************
     * Static
     */
	static 
	enMIDIChannel GetChannel
	(
		IN		UCHAR byMIDI
	);
};


#endif  // __MIDIPRSR_HPP
//////////////////////////// MIDIPRSR.HPP ///////////////////////////
