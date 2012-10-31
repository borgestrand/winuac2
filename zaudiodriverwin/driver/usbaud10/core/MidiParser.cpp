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
 * @file        MidiParser.cpp
 * 
 * @brief       Implements the CMidiParser object, a general-purpose MIDI parser
 * 
 * This is, perhaps, the oldest piece of (largely) unmodified code in the UDA codebase
 *
 * @author      Mike Guzewicz, mikeg@atc.creative.com.
 *				Robert S. Crawford
 * 
 * $Date: 2006/01/24 03:34:55 $
 * $Revision: 1.3 $
 * 
 * 
 *****************************************************************************
 * Copyright (C) Creative Technology, Ltd., 1994-2002. All rights reserved.
 *****************************************************************************
 */
#include "MidiParser.h"

#define NO_MIDI_CHANNEL MIDICHANNELS+1

/*****************************************************************************
* Defines
*/
//#define DEBUG
//#define DEBUG_SYSEX
//#define DEBUG_STATUS
//#define DEBUG_SYSCOMMON
//#define DEBUG_SYSREAL

#define VELCURVE(x) x 


/******************************************************************************
 * CMidiParser::CMidiParser()
 ******************************************************************************/
/**
 * @brief   Constructor
 *
 * Simply resets state through ResetParser()
 *
 */
CMidiParser::
CMidiParser
(	void
)
{
	ResetParser();
}

/******************************************************************************
 * CMidiParser::CMidiParser()
 ******************************************************************************/
/**
 * @brief   Destructor
 *
 * Simply resets state through ResetParser()
 *
 */
CMidiParser::
~CMidiParser
(	void
)
{
	ResetParser();
}

/******************************************************************************
 * CMidiParser::ResetParser()
 ******************************************************************************/
/**
 * @brief   Reset all MIDI parser state
 *
 */
void 
CMidiParser::
ResetParser
(	void
)
{
	m_MidiPacket.byStatus  = NOOP;
	m_MidiPacket.byData[0] = 0;
	m_MidiPacket.byData[1] = 0;

	m_byRTStatus   = 0;
	m_bySysExStatus = NOOP;

	//m_uiSysExCmd = 0;
	m_ErrorIndex   = 0;

	m_OmniOn = TRUE;
	m_BasicChannel = m_MaxChannel = emcEndChannel;
	SetBasicChannel(emcChannel1);

	m_NoteFlag        = TRUE;
	m_PolyPressFlag   = TRUE;
	m_ProgChangeFlag  = TRUE;
	m_PitchWheelFlag  = TRUE;
	m_ChanPressFlag   = TRUE;
	m_CtrlChangeFlag  = TRUE;
	m_SysExFlag       = TRUE;

	for (UCHAR i=0; i<MIDICHANNELS; i++)
	{
		m_ChanFlag[i] = TRUE;
	}

	m_OmniFlag		  = FALSE; // Never turn this on!!!
}

/******************************************************************************
 * CMidiParser::UpdateStatus()
 ******************************************************************************/
/**
 * @brief   Update the MIDI status on internal MIDI state, compute the return value
 *
 * Used by Parse()
 *
 * @param byMIDI
 * The current MIDI byte
 *
 */
void   
CMidiParser::
UpdateStatus
(
	IN		UCHAR	byMIDI
)
{
	if (CHANNEL_STATUS_LIMIT(byMIDI))
	{
		#ifdef DEBUG_STATUS
		DbgPrintF(DEBUGLVL_VERBOSE, "ChanStatus: %X ", byMIDI));
		#endif

		m_bySysExStatus = NOOP;

		m_MidiPacket.byStatus  = STATUS_MASK(byMIDI);
		m_MidiPacket.byChannel = CHANNEL_MASK(byMIDI);

		// Check for OMNI mode
		if ((m_OmniFlag) && (ChannelGetsVoiceMessages(m_MidiPacket.byChannel) == FALSE) &&
			(m_MidiPacket.byStatus != CONTROL_CHANGE))
		{
			m_MidiPacket.byStatus = NOOP;
			return;
		}

		m_DataCount  = 0;
	}
	else if (SYSEX_MSG(byMIDI))
	{
		#ifdef DEBUG_SYSEX
		DbgPrintF(DEBUGLVL_VERBOSE, ("Sysex: %X ", byMIDI));
		#endif
		
		m_bySysExStatus = SYSEX;

		m_MidiPacket.byStatus  = STATUS_MASK(byMIDI);
		m_MidiPacket.byChannel = CHANNEL_MASK(byMIDI);

		m_DataCount  = 0;
	}
	else if (END_SYSEX_MSG(byMIDI))
	{
		; // No operation, handled by the Parser.
	}
	else if (SYSTEM_COMMON_LIMIT(byMIDI))
	{
		#ifdef DEBUG_SYSCOMMON
		DbgPrintF(DEBUGLVL_VERBOSE, ("SysCommon: %X ", byMIDI));
		#endif

		m_bySysExStatus = NOOP;

		m_MidiPacket.byStatus  = STATUS_MASK(byMIDI);
		m_MidiPacket.byChannel = CHANNEL_MASK(byMIDI);

		m_DataCount  = 0;
	}
	//// byMIDI must be a System Real-time UCHAR ////
	else
	{
		#ifdef DEBUG_SYSREAL
		DbgPrintF(DEBUGLVL_VERBOSE, ("SysReal: %X ", byMIDI));
		#endif
        
		m_bySysExStatus = NOOP;

		m_byRTStatus = byMIDI;
	}
}

/******************************************************************************
 * CMidiParser::Parse()
 ******************************************************************************/
/**
 * @brief   Given a single MIDI byte, update the internal channel status and
 *          return a valid code if enough data bytes are collected
 *
 * Note this function is designed to work on a single complete message on a 
 * single MIDI channel, it does not maintain state for 16 independent MIDI
 * channels
 *
 * @param byMIDI
 * The MIDI byte
 * @return 
 * A CMidiParser return code
 *
 */
UCHAR 
CMidiParser::
Parse
(
	IN		UCHAR	byMIDI
)
{
	m_byRTStatus = 0;

	if (STATUS_BYTE(byMIDI))
	{
		UpdateStatus(byMIDI);

		if (m_byRTStatus != 0)
		{
			return m_byRTStatus;
		}
		else if ((byMIDI > SONG_SELECT) && (byMIDI <= TUNE_REQUEST))
		{
			return (m_MidiPacket.byStatus | m_MidiPacket.byChannel);
		}
		else if (!(m_ChanFlag[CHANNEL_MASK(byMIDI)]) && (byMIDI < SYSEX))
		{
			return (m_MidiPacket.byStatus = NOOP);
		}
		else if (!END_SYSEX_MSG(byMIDI))
		{
			return NOOP;
		}
	}
	
	if (m_MidiPacket.byStatus == NOOP)
	{
		return NOOP;
	}

	switch (m_MidiPacket.byStatus)
	{
		case NOTE_OFF:
		{
			if (m_NoteFlag)
			{
				m_MidiPacket.byData[m_DataCount++] = byMIDI;

				if (m_DataCount == 2)
				{
					m_DataCount = 0;

					return m_MidiPacket.byStatus;
				}
			}
		}
		break;

		case NOTE_ON:
		{
			if (m_NoteFlag)
			{
				if (m_DataCount == 1)
				{	          
					// MG I think this is not correct because it breaks running status	
					//if (byMIDI==0)
					//  m_MidiPacket.byStatus=NOTE_OFF;

					m_MidiPacket.byData[1] = VELCURVE(byMIDI);
					m_DataCount = 0;

					return m_MidiPacket.byStatus;
				}
				else
				{
					m_MidiPacket.byData[m_DataCount++] = byMIDI;
				}
			}
		}
		break;

		case POLYKEY_PRESSURE:
		{
			if (m_PolyPressFlag)
			{
				m_MidiPacket.byData[m_DataCount++] = byMIDI;

				if (m_DataCount == 2)
				{
					m_DataCount = 0;

					return m_MidiPacket.byStatus;
				}
			}
		}
		break;

		case CONTROL_CHANGE:
		{
			if (m_CtrlChangeFlag)
			{
				m_MidiPacket.byData[m_DataCount++] = byMIDI;

				if (m_DataCount == 2)
				{
					m_DataCount = 0;

					if (m_OmniFlag)
					{
						UCHAR Control = m_MidiPacket.byData[0];
						
						if (Control == OMNI_ON) 
						{
							if (m_MidiPacket.byChannel == m_BasicChannel)
							{
								SetOmniOffChannel(emcEndChannel);
							}
							break;
						}
						else if (Control == OMNI_OFF)
						{
							if (m_MidiPacket.byChannel == m_BasicChannel)
							{
								SetOmniOffChannel(CHANNEL_MASK(byMIDI));
							}
							break;
						}
						else if ((m_OmniOn == FALSE) &&
								(ChannelGetsVoiceMessages(m_MidiPacket.byChannel) == FALSE))
						{
							m_MidiPacket.byStatus  = NOOP;
							m_MidiPacket.byChannel = emcEndChannel;
						}
					}

					return m_MidiPacket.byStatus;
				}
			}
		}
		break;

		case PROGRAM_CHANGE:
		{
			if (m_ProgChangeFlag)
			{
				m_MidiPacket.byData[0] = byMIDI;

				return m_MidiPacket.byStatus;
			}
		}
		break;
		
		case CHANNEL_PRESSURE:
		{
			if (m_ChanPressFlag)
			{
				m_MidiPacket.byData[0] = byMIDI;

				return m_MidiPacket.byStatus;
			}
		}
		break;

		case PITCH_WHEEL:
		{
			if (m_PitchWheelFlag)
			{
				m_MidiPacket.byData[m_DataCount++] = byMIDI;

				if (m_DataCount == 2)
				{
					m_DataCount = 0;
					
					return m_MidiPacket.byStatus;
				}
			}
		}
		break;

		case 0xF0:
		{
			if (m_bySysExStatus == EOX)
			{
				// A data byte followed an EOX, ignore it.
				m_MidiPacket.byStatus = NOOP;
				m_MidiPacket.byChannel = emcEndChannel;
				
				return NOOP;
			}
			else
			{
				UCHAR byChanStatus = GetChannelStatus();

				if (byChanStatus == SYSEX)
				{
					if (m_SysExFlag)
					{
						if (m_DataCount == 0)
						{
							m_MidiQueue.Reset();
							m_MidiQueue.PutQueue(SYSEX);
							m_MidiQueue.PutQueue(byMIDI);
							
							if ((byMIDI == EOX)||(byMIDI == SYSEX_ESC))
							{
								// Flag to say that data bytes following this byte do 
								// not honor running status, they need to be ignored.
								m_bySysExStatus = EOX;

								return m_MidiPacket.byStatus;
							}
							else
							{
								m_DataCount++;

								return NOOP;
							}
						}
						else if ((byMIDI == EOX)||(byMIDI == SYSEX_ESC))
						{
							// Flag to say that data bytes following this byte do 
							// not honor running status, they need to be ignored.
							m_bySysExStatus = EOX;

							m_MidiQueue.PutQueue(byMIDI);

							m_DataCount = (UCHAR)m_MidiQueue.CanGetQueue();
							
							return m_MidiPacket.byStatus;
						}
						else
						{
							/* SysEx UCHAR... toss it in the queue... */
							m_MidiQueue.PutQueue(byMIDI);

							return NOOP;
						}
					}
				}
				else
				{
					/* Must be system common messages. */
					switch (byChanStatus)
					{
						case MTC_QUARTER_FRAME:
						case SONG_SELECT:
						{
							m_MidiPacket.byData[0] = byMIDI;

							return m_MidiPacket.byStatus | m_MidiPacket.byChannel;
						}
						break;

						case SONG_POS_POINTER:
						{
							m_MidiPacket.byData[m_DataCount++] = byMIDI;

							if (m_DataCount == 2)
							{
								m_DataCount = 0;
								
								return m_MidiPacket.byStatus | m_MidiPacket.byChannel;
							}
						}
						break;

						case F4: /* ED-MIDI command. */
						{
							if (m_DataCount < 2)
							{
								// First packet is 2 byte data.
								m_MidiPacket.byData[m_DataCount++] = byMIDI;

								if (m_DataCount == 2)
								{
									return m_MidiPacket.byStatus | m_MidiPacket.byChannel;
								}
								else
								{
									return NOOP;
								}
							}
							else
							{
								// Subsequent packet is only 1 byte data.
								m_MidiPacket.byData[0] = byMIDI;

								m_DataCount++;

								return m_MidiPacket.byStatus | m_MidiPacket.byChannel;
							}
						}
						break;					

						default: 
						{
							/* 0xF5-0xF6 are single byte status.*/
							m_MidiPacket.byStatus = NOOP;
							m_MidiPacket.byChannel = emcEndChannel;

							return NOOP;
						}
						break;
					}
				}
			}
		}
		break;

		default:
			break;
	}

	return NOOP;
}

/******************************************************************************
* CMidiParser::GetPacket()
******************************************************************************/
/**
* @brief   Returns the current MIDI packet
*
* @param OutMidiPacket
* Returned MIDI Packet
*
*/
void 
CMidiParser::
GetPacket
(
	OUT		MIDI_PACKET *	OutMidiPacket
) 
{ 
	*OutMidiPacket = m_MidiPacket;	
}

/******************************************************************************
* CMidiParser::GetStatus()
******************************************************************************/
/**
* @brief   Returns the current MIDI status
*
* @return 
* One of the MIDI Status, without the channel information.
* 0x90=NOTE ON, 0x80 = NOTE OFF, etc
*
*/
UCHAR 
CMidiParser::
GetStatus
(	void
) const 
{ 
	return m_MidiPacket.byStatus; 
}

/******************************************************************************
* CMidiParser::GetChannel()
******************************************************************************/
/**
* @brief   Returns the current MIDI channel
*
* @return 
* One of enMIDIChannel
*
*/
enMIDIChannel 
CMidiParser::
GetChannel
(	void
) const 
{ 
	return m_MidiPacket.byChannel; 
}

/******************************************************************************
* CMidiParser::GetData0()
******************************************************************************/
/**
* @brief   Returns the first data byte
*
* Should be called when Parse() returns a return code with a proper status.
* Not useful if the status indicates a SysEx message.
*
* @return 
* A valid MIDI data byte
*
*/
UCHAR 
CMidiParser::
GetData0
(	void
) const 
{ 
	return m_MidiPacket.byData[0]; 
}

/******************************************************************************
* CMidiParser::GetData1()
******************************************************************************/
/**
* @brief   Returns the second data byte
*
* Should be called when Parse() returns a return code with a proper status
* Not useful if the status indicates a SysEx message.
*
* @return 
* A valid MIDI data byte
*
*/
UCHAR 
CMidiParser::
GetData1
(	void
) const 
{ 
	return m_MidiPacket.byData[1]; 
}

/******************************************************************************
* CMidiParser::GetDataCount()
******************************************************************************/
/**
* @brief   Returns the data byte count
*
* Should be called when Parse() returns a return code with a proper status
* Not useful if the status indicates a SysEx message.
*
* @return 
* A value between 0 and 2, indicating whether GetData0() and/or GetData1() 
* need be called
*
*/
UCHAR 
CMidiParser::
GetDataCount
(	void
) const 
{
	return m_DataCount;
}

/******************************************************************************
* CMidiParser::GetChannelStatus()
******************************************************************************/
/**
* @brief   Returns the current MIDI channel and status as a proper MIDI byte
*
* @return 
* A valid MIDI status byte
*
*/
UCHAR 
CMidiParser::
GetChannelStatus
(	void
) const 
{ 
	return (m_MidiPacket.byStatus | CHANNEL_MASK(m_MidiPacket.byChannel)); 
}

/******************************************************************************
* CMidiParser::GetSysRealTimeStatus()
******************************************************************************/
/**
* @brief   Returns the realtime status as a proper MIDI byte
*
* @return 
* A valid MIDI realtime status byte
*
*/
UCHAR 
CMidiParser::
GetSysRealTimeStatus
(	void
) const 
{ 
	return m_byRTStatus; 
}

/******************************************************************************
* CMidiParser::GetMidiQueue()
******************************************************************************/
/**
* @brief   Returns a pointer to a queue of MIDI data bytes
*
* Should be called when Parse() returns a return code with a proper status
* Not useful unless the status indicates a SysEx message.
*
* @param void
* TODO: Describe void here...
* @return 
* TODO: Add QueueClass *  return details here
*
*/
CMidiQueue * 
CMidiParser::
GetMidiQueue
(	void
) 
{ 
	return &m_MidiQueue; 
}

/******************************************************************************
* CMidiParser::GetChannel() 
******************************************************************************/
/**
* @brief  Given a MIDI status byte, return the MIDI channel
*
* @param byMIDI
* A valid MIDI status byte
* @return 
* The MIDI channel
*
*/
enMIDIChannel 
CMidiParser::
GetChannel
(
	IN		UCHAR	byMIDI
) 
{
	return CHANNEL_MASK(byMIDI);
}

// Omni on/off implementation

/******************************************************************************
* CMidiParser::GetBasicChannel() 
******************************************************************************/
/**
* @brief   Get the MIDI Basic Channel, if not in multi-mode
*
* Not useful if parser is in multi-mode, which is the default
*
* @return 
* The MIDI channel
*
*/
enMIDIChannel 
CMidiParser::
GetBasicChannel
(	void
) 
{ 
	return m_BasicChannel; 
}

/******************************************************************************
 * CMidiParser::SetBasicChannel()
 ******************************************************************************/
/**
 * @brief   Sets the MIDI Basic channel, used to turn off multi-mode
 *
 * @param Channel
 * The basic MIDI channel
 *
 */
void 
CMidiParser::
SetBasicChannel
(
	IN		enMIDIChannel	Channel
)
{
	if (Channel >= REALMIDICHANNELS)
	{
		m_BasicChannel = emcChannel1;
	}
	else
	{
		m_BasicChannel = Channel;
	}
}

/******************************************************************************
 * CMidiParser::SetOmniOffChannel()
 ******************************************************************************/
/**
 * @brief   Set the Omni Off channel
 *
 * @param enMIDIChannel Channel
 * The MIDI channel
 *
 */
void 
CMidiParser::
SetOmniOffChannel
(
	IN		enMIDIChannel	Channel
)
{
	Channel = (enMIDIChannel)((USHORT)(Channel)-1); // Input is 1-16, internal data is 0-15

	if (Channel >= REALMIDICHANNELS)
	{
		m_OmniChannel = m_MaxChannel = emcEndChannel;
		m_OmniOn = TRUE;
	}
	else
	{
		m_OmniChannel = m_MaxChannel = Channel;
		m_OmniOn = FALSE;
		/*
		Waiting for feedback from customers.
		Special case OmniOff/Mono thing...
		if ((m_MaxChannel >= MIDICHANNELS)  || (m_MaxChannel < Channel))
			m_MaxChannel = Channel;
		*/
	}
}

/******************************************************************************
 * CMidiParser::IsOmniOff()
 ******************************************************************************/
/**
 * @brief   Says if the parser is in Omni Off or On mode
 *
 * @return 
 * FALSE=OmniOff, TRUE=OmniOn
 *
 */

BOOL 
CMidiParser::
IsOmniOff
(	void
)
{
	return m_OmniOn;
}

/******************************************************************************
 * CMidiParser::ChannelGetsVoiceMessages()
 ******************************************************************************/
/**
 * @brief   When in Poly mode, says whether the given channel 
 *          controller data should apply to the basic channel
 *
 * @param Channel
 * The channel
 * @return 
 * TRUE=yes, FALSE=no
 *
 */
BOOL 
CMidiParser::
ChannelGetsVoiceMessages
(
	IN		enMIDIChannel	Channel
)
{
	if (m_OmniOn == TRUE)
	{
		return TRUE;
	}
	else if ((Channel >= m_OmniChannel) && (Channel <= m_MaxChannel)) 
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}
