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
 * @file	   Midi.cpp
 * @brief	   This file defines the API for the low-level manipulation of the
 *             MIDI deviceS.
 * @details
 *			   The MIDI devices are primarily intended for transmitting and
 *			   receiving MIDI data, although nothing in this layer of the
 *			   interface precludes other uses.
 *
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "Midi.h"

#define STR_MODULENAME "MIDI: "

/*****************************************************************************
 * Defines
 */
#define MIDI_MAGIC  0xABACAB

/*!
 * @brief
 * BLOCK_ALIGN is kind of a weird thing.  It turns out that WDM wants
 * the driver to always transmit data in 4-byte atomic units.  Since none
 * of the other drivers really care, we insure that this constraint is
 * met in WriteBuffer by setting the BLOCK_ALIGN parameter.
 */
#define BLOCK_ALIGN 4

#define GET_NUM_AVAIL() (BUFFER_SIZE - GetNumQueuedPackets())

static 
UCHAR SIZEOF_MIDI[] = {0, 0, 2, 3, 3, 1, 2, 3, 3, 3, 3, 3, 2, 2, 3, 1 };

/*****************************************************************************
 * Referenced forward
 */

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiClient::Init()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Initialize the MIDI client.
 * @param
 * Cable Pointer to the MIDI cable object.
 * @param
 * Direction MIDI_INPUT or MIDI_OUTPUT.
 * @param
 * CallbackRoutine The function to call when the MIDI transitions to an empty
 * state (on output) or a character becomes available (on input).
 * @param
 * CallbackData A user-specified handle which is passed as a parameter when
 * the callback is called.
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiClient::
Init
(
	IN		CMidiCable *			Cable,
	IN		MIDI_DIRECTION			Direction,
	IN		MIDI_CALLBACK_ROUTINE	CallbackRoutine,
	IN		PVOID					CallbackData
)
{
	PAGED_CODE();

	m_Cable = Cable;
	m_Direction = Direction;

	m_CableNumber = Cable->CableNumber();

	m_ReadPosition = 0;
    m_WritePosition = 1;

	BUFFER_SIZE = (m_Direction == MIDI_OUTPUT) ? OUTPUT_BUFFER_SIZE : INPUT_BUFFER_SIZE;

	m_CallbackData = CallbackData;
    m_CallbackRoutine = CallbackRoutine;
    m_CallbackRequired = FALSE;

	m_MidiParser.ResetParser();

	m_SysExTimeOutPeriod = 1000;

	m_EndOfSysEx = TRUE;

	KeInitializeSpinLock(&m_Lock);

	KeInitializeEvent(&m_PacketCompletionEvent, NotificationEvent, FALSE);

	return MIDIERR_SUCCESS;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiClient::Cable()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Returns the MIDI cable object which this client is attached to.
 * @param
 * <None>
 * @return
 * Returns the pointer to the MIDI cable object.
 */
CMidiCable *
CMidiClient::
Cable
(	void
)
{
	return m_Cable;
}

/*****************************************************************************
 * CMidiClient::Lock()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
VOID
CMidiClient::
Lock
(	void
)
{
	KeAcquireSpinLock(&m_Lock, &m_LockIrql);
}

/*****************************************************************************
 * CMidiClient::Unlock()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
VOID
CMidiClient::
Unlock
(	void
)
{
	KeReleaseSpinLock(&m_Lock, m_LockIrql);
}

/*
 * Important note: If you look at how m_ReadPosition and m_WritePosition are
 * handled, you'll notice that the ring buffer never gets completely full
 * (there always is one byte left open).
 *
 * To see this, consider the following case:
 *
 *
 *    0    1    2    3    4
 * +----+----+----+----+----+
 * |    |    |    |    |    |
 * +----+----+----+----+----+
 *
 * In the beginning, with nothing in the buffer, m_ReadPosition = 0,
 * m_WritePosition = 1. In this case, the ring buffer is 5 bytes in size
 * (BUFFER_SIZE == 4). If we call AddByte() four times, m_WritePosition
 * will be equal to 0 , while m_ReadPosition == 0.  At this point,
 * m_WritePosition == m_ReadPosition, so we consider the ring buffer to be
 * full.  Note, however, that we never wrote a byte to buffer[0], since
 * m_WritePosition started out being initialized to 1.
 *
 * This scheme is a little weird, but it has the advantage of making it
 * really easy to determine when the buffer is full or empty.
 */
/*****************************************************************************
 * CMidiClient::AddPacket()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Add a MIDI packet to the FIFO.
 * @param
 * Packet USB-MIDI event packet to be added.
 * @param
 * TimeDeltaMs Time stamp (in ms).
 * @return
 * Returns TRUE if successful, otherwise FALSE.
 */
BOOL
CMidiClient::
AddPacket
(
	IN		USB_MIDI_EVENT_PACKET	Packet,
	IN		LONGLONG				TimeStampCounter
)
{
    if (m_ReadPosition == m_WritePosition)
	{
        /* Buffer is completely FULL */
        return FALSE;
	}
    else
	{
        /* There's at least one byte left */
        m_DataBuffer[m_WritePosition].Packet = Packet;
        m_DataBuffer[m_WritePosition].TimeStampCounter = TimeStampCounter;

		m_WritePosition++;
        m_WritePosition %= (BUFFER_SIZE+1);

        return TRUE;
    }
}

/*****************************************************************************
 * CMidiClient::RemovePacket()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Remove a MIDI packet from the FIFO.
 * @param
 * OutPacket Pointer to the location to store the USB-MIDI event packet to 
 * be removed.
 * @param
 * OutTimeDeltaMs Pointer to the location to store the time stamp (in ms) of
 * the USB-MIDI event packet.
 * @return
 * Returns TRUE if successful, otherwise FALSE.
 */
BOOL
CMidiClient::
RemovePacket
(
	OUT		USB_MIDI_EVENT_PACKET *	OutPacket,
	OUT		LONGLONG *				OutTimeStampCounter	OPTIONAL
)
{
    USHORT NewReadPosition = (m_ReadPosition + 1) % (BUFFER_SIZE+1);

    if (NewReadPosition == m_WritePosition)
	{
        /* Buffer empty */
        return FALSE;
    }
	else
	{
        /* There's at least one character available */
        *OutPacket = m_DataBuffer[NewReadPosition].Packet;

		if (OutTimeStampCounter)
		{
			*OutTimeStampCounter = m_DataBuffer[NewReadPosition].TimeStampCounter;
		}

		m_ReadPosition = NewReadPosition;

        return TRUE;
    }
}

/*****************************************************************************
 * CMidiClient::PeekPacket()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Take a peek at the first MIDI packet in the FIFO.
 * @param
 * OutPacket Pointer to the location to store the USB-MIDI event packet.
 * @param
 * OutTimeDeltaMs Pointer to the location to store the time stamp (in ms) of
 * the USB-MIDI event packet.
 * @return
 * Returns TRUE if successful, otherwise FALSE.
 */
BOOL
CMidiClient::
PeekPacket
(
	OUT		USB_MIDI_EVENT_PACKET *	OutPacket,
	OUT		LONGLONG *				OutTimeStampCounter	OPTIONAL
)
{
    USHORT NewReadPosition = (m_ReadPosition + 1) % (BUFFER_SIZE+1);

    if (NewReadPosition == m_WritePosition)
	{
        /* Buffer empty */
        return FALSE;
    }
	else
	{
        /* There's at least one character available */
        *OutPacket = m_DataBuffer[NewReadPosition].Packet;

		if (OutTimeStampCounter)
		{
			*OutTimeStampCounter = m_DataBuffer[NewReadPosition].TimeStampCounter;
		}

        return TRUE;
    }
}

#if DBG
/*****************************************************************************
 * DumpUsbMidiPacket()
 *****************************************************************************
 */
VOID
DumpUsbMidiPacket
(
	IN		CHAR *					Title,
	IN		USB_MIDI_EVENT_PACKET	Packet
)
{
	_DbgPrintF(DEBUGLVL_TERSE,("%s", Title));
	_DbgPrintF(DEBUGLVL_TERSE,("----------------------"));
	_DbgPrintF(DEBUGLVL_TERSE,("CodeIndexNumber: %x", Packet.CodeIndexNumber));
	_DbgPrintF(DEBUGLVL_TERSE,("CableNumber: %x", Packet.CableNumber));
	_DbgPrintF(DEBUGLVL_TERSE,("MIDI[0]: %x", Packet.MIDI[0]));
	_DbgPrintF(DEBUGLVL_TERSE,("MIDI[1]: %x", Packet.MIDI[1]));
	_DbgPrintF(DEBUGLVL_TERSE,("MIDI[2]: %x", Packet.MIDI[2]));
}
#endif // DBG

/*****************************************************************************
 * CMidiClient::AssemblePacket()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Process a byte, and sent the reconstructed event packet to the cable.
 * @param
 * byte Data byte to be processed.
 * @return
 * Returns TRUE if successful, otherwise FALSE.
 */
BOOL
CMidiClient::
AssemblePacket
(
	IN		UCHAR					Byte,
	OUT		USB_MIDI_EVENT_PACKET *	OutPacket
)
{
	ASSERT(OutPacket);

	BOOL Success = FALSE;

	UCHAR Status = m_MidiParser.Parse(Byte);

	switch (Status)
	{
		case NOTE_OFF:
		{
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_NOTE_OFF;
			OutPacket->MIDI[0] = m_MidiParser.GetChannelStatus();
			OutPacket->MIDI[1] = m_MidiParser.GetData0();
			OutPacket->MIDI[2] = m_MidiParser.GetData1();

			Success = TRUE;
		}
		break;

		case NOTE_ON:
		{
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_NOTE_ON;
			OutPacket->MIDI[0] = m_MidiParser.GetChannelStatus();
			OutPacket->MIDI[1] = m_MidiParser.GetData0();
			OutPacket->MIDI[2] = m_MidiParser.GetData1();

			Success = TRUE;
		}
		break;

		case POLYKEY_PRESSURE:
		{
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_POLYKEY_PRESSURE;
			OutPacket->MIDI[0] = m_MidiParser.GetChannelStatus();
			OutPacket->MIDI[1] = m_MidiParser.GetData0();
			OutPacket->MIDI[2] = m_MidiParser.GetData1();

			Success = TRUE;
		}
		break;

		case CONTROL_CHANGE:
		{
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_CONTROL_CHANGE;
			OutPacket->MIDI[0] = m_MidiParser.GetChannelStatus();
			OutPacket->MIDI[1] = m_MidiParser.GetData0();
			OutPacket->MIDI[2] = m_MidiParser.GetData1();

			Success = TRUE;
		}
		break;

		case PROGRAM_CHANGE:
		{
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_PROGRAM_CHANGE;
			OutPacket->MIDI[0] = m_MidiParser.GetChannelStatus();
			OutPacket->MIDI[1] = m_MidiParser.GetData0();
			OutPacket->MIDI[2] = 0;

			Success = TRUE;
		}
		break;

		case CHANNEL_PRESSURE:
		{
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_CHANNEL_PRESSURE;
			OutPacket->MIDI[0] = m_MidiParser.GetChannelStatus();
			OutPacket->MIDI[1] = m_MidiParser.GetData0();
			OutPacket->MIDI[2] = 0;

			Success = TRUE;
		}
		break;

		case PITCH_WHEEL:
		{
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_PITCH_BEND_CHANGE;
			OutPacket->MIDI[0] = m_MidiParser.GetChannelStatus();
			OutPacket->MIDI[1] = m_MidiParser.GetData0();
			OutPacket->MIDI[2] = m_MidiParser.GetData1();

			Success = TRUE;
		}
		break;

		case SYSEX:
		{
			// This status is returned at the end of the SysEx message.
			m_EndOfSysEx = TRUE;

			ASSERT(m_MidiParser.GetChannelStatus() == SYSEX);

			CMidiQueue * MidiQueue = m_MidiParser.GetMidiQueue();

			USHORT BytesInQueue = MidiQueue->CanGetQueue(); // including EOX

			ASSERT((BytesInQueue > 0) && BytesInQueue <=3);

			OutPacket->CableNumber = m_CableNumber;

			static UCHAR CIN[] = {0, CODE_INDEX_NUMBER_1_BYTE_SYSEX_END,
									 CODE_INDEX_NUMBER_2_BYTE_SYSEX_END,
									 CODE_INDEX_NUMBER_3_BYTE_SYSEX_END};

			OutPacket->CodeIndexNumber = CIN[BytesInQueue];

			switch (BytesInQueue)
			{
				case 1:
				{

					MidiQueue->GetQueue(&OutPacket->MIDI[0]);
					OutPacket->MIDI[1] = 0;
					OutPacket->MIDI[2] = 0;
				}
				break;

				case 2:
				{

					MidiQueue->GetQueue(&OutPacket->MIDI[0]);
					MidiQueue->GetQueue(&OutPacket->MIDI[1]);
					OutPacket->MIDI[2] = 0;
				}
				break;

				case 3:
				{
					MidiQueue->GetQueue(&OutPacket->MIDI[0]);
					MidiQueue->GetQueue(&OutPacket->MIDI[1]);
					MidiQueue->GetQueue(&OutPacket->MIDI[2]);
				}
				break;
			}

			Success = TRUE;
		}
		break;

		case MTC_QUARTER_FRAME:
		case SONG_SELECT:
		{
			// 2 byte system common message.
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_2_BYTE_SYSTEM_COMMON;
			OutPacket->MIDI[0] = m_MidiParser.GetChannelStatus();
			OutPacket->MIDI[1] = m_MidiParser.GetData0();
			OutPacket->MIDI[2] = 0;

			Success = TRUE;
		}
		break;

		case SONG_POS_POINTER:
		{
			// 3 byte system common message.
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_3_BYTE_SYSTEM_COMMON;
			OutPacket->MIDI[0] = m_MidiParser.GetChannelStatus();
			OutPacket->MIDI[1] = m_MidiParser.GetData0();
			OutPacket->MIDI[2] = m_MidiParser.GetData1();

			Success = TRUE;
		}
		break;

		case F4: /* ED-MIDI command. */
		{
			ULONG DataCount = m_MidiParser.GetDataCount();

			if (DataCount == 2)
			{
				// 3 byte system common message.
				OutPacket->CableNumber = m_CableNumber;
				OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_3_BYTE_SYSTEM_COMMON;
				OutPacket->MIDI[0] = m_MidiParser.GetChannelStatus();
				OutPacket->MIDI[1] = m_MidiParser.GetData0();
				OutPacket->MIDI[2] = m_MidiParser.GetData1();

				Success = TRUE;
			}
			else if (DataCount > 2)
			{
				// 1 byte data only, no message.
				OutPacket->CableNumber = m_CableNumber;
				OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_1_BYTE;
				OutPacket->MIDI[0] = m_MidiParser.GetData0();
				OutPacket->MIDI[1] = 0;
				OutPacket->MIDI[2] = 0;

				Success = TRUE;
			}
		}
		break;

		//case 0xF5: /* Undefined data bytes. */
		case TUNE_REQUEST:
		{
			// Single byte system common message.
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_1_BYTE_SYSTEM_COMMON;
			OutPacket->MIDI[0] = Status;
			OutPacket->MIDI[1] = 0;
			OutPacket->MIDI[2] = 0;

			Success = TRUE;
		}
		break;

		case TIMING_CLOCK:
		case 0xF9:
		case START:
		case CONTINUE:
		case STOP:
		case 0xFD:
		case ACTIVE_SENSING:
		case SYSTEM_RESET:
		{
			// System realtime message.
			OutPacket->CableNumber = m_CableNumber;
			OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_1_BYTE;
			OutPacket->MIDI[0] = Status;
			OutPacket->MIDI[1] = 0;
			OutPacket->MIDI[2] = 0;

			Success = TRUE;
		}
		break;

		case NOOP:
		{
			if (m_MidiParser.GetChannelStatus() == SYSEX)
			{
				// Start or running SysEx message.
				m_EndOfSysEx = FALSE;

				CMidiQueue * MidiQueue = m_MidiParser.GetMidiQueue();

				if (MidiQueue->CanGetQueue() >= 3)
				{
					OutPacket->CableNumber = m_CableNumber;
					OutPacket->CodeIndexNumber = CODE_INDEX_NUMBER_SYSEX_START_OR_CONTINUE;

					MidiQueue->GetQueue(&OutPacket->MIDI[0]);
					MidiQueue->GetQueue(&OutPacket->MIDI[1]);
					MidiQueue->GetQueue(&OutPacket->MIDI[2]);

					Success = TRUE;
				}
			}
		}
		break;
	}

	return Success;
}

/*****************************************************************************
 * CMidiClient::DisassemblePacket()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Deconstructed a MIDI event packet into MIDI bytes for the client.
 * @param
 * Packet USB-MIDI event packet to be processed.
 * @return
 * <None>
 */
ULONG
CMidiClient::
DisassemblePacket
(
	IN		USB_MIDI_EVENT_PACKET	Packet,
	IN		CMidiQueue *			BytesQueue
)
{
	ULONG Size = 0;

	// If this is a SysEx packet, then we may have to add the 0xF0
	// byte back into the MIDI byte stream.
	switch (Packet.CodeIndexNumber)
	{
		// Start or continuation of SysEx message: <F0> nn nn nn nn nn ...
		case CODE_INDEX_NUMBER_SYSEX_START_OR_CONTINUE:
		// 3 byte SysEx message: <F0> nn nn nn F7
		case CODE_INDEX_NUMBER_3_BYTE_SYSEX_END:
		// 2 byte SysEx message: <F0> nn nn F7
		case CODE_INDEX_NUMBER_2_BYTE_SYSEX_END:
		{
			if (m_EndOfSysEx)
			{
				if (Packet.MIDI[0] != SYSEX)
				{
					// Now where did that SYSEX byte go ??
					BytesQueue->PutQueue(SYSEX); Size++;
				}
			}
		}
		break;

		case CODE_INDEX_NUMBER_1_BYTE_SYSEX_END:
		{
			// 1 byte SysEx message: <F0> nn F7 or <F0> F7 or
			// System Common message.

			// Check if this is a 1-byte System Common message.
			// They share the same CIN : 0x5.
			if ((!STATUS_BYTE(Packet.MIDI[0])) || (Packet.MIDI[0] == EOX))
			{
				if (m_EndOfSysEx)
				{
					// Someone send a F0 nn F7 or F0 F7.
					BytesQueue->PutQueue(SYSEX); Size++;
				}
			}
		}
		break;
	}

	// Copy the MIDI data bytes.
	for (ULONG j=0; j<SIZEOF_MIDI[Packet.CodeIndexNumber]; j++)
	{
		BytesQueue->PutQueue(Packet.MIDI[j]); Size++;
	}

	// If this is a SysEx packet, then we may have to add the 0xF7
	// byte back into the MIDI byte stream.
	switch (Packet.CodeIndexNumber)
	{
		case CODE_INDEX_NUMBER_SYSEX_START_OR_CONTINUE:
		{
			// SysEx message continues...
			m_EndOfSysEx = FALSE;
		}
		break;

		// End of SysEx message: ... nn nn nn <F7>
		case CODE_INDEX_NUMBER_3_BYTE_SYSEX_END:
		{
			m_EndOfSysEx = TRUE;

			if (Packet.MIDI[2] != EOX)
			{
				BytesQueue->PutQueue(EOX); Size++;
			}
		}
		break;

		// End of SysEx message: ... nn nn <F7>
		case CODE_INDEX_NUMBER_2_BYTE_SYSEX_END:
		{
			m_EndOfSysEx = TRUE;

			if (Packet.MIDI[1] != EOX)
			{
				BytesQueue->PutQueue(EOX); Size++;
			}
		}
		break;

		case CODE_INDEX_NUMBER_1_BYTE_SYSEX_END:
		{
			// End of SysEx message: ... nn <F7> or F0 <F7> or
			// System Common message.
			m_EndOfSysEx = TRUE;

			// Check if this is a 1-byte System Common message.
			// They share the same CIN : 0x5.
			if ((!STATUS_BYTE(Packet.MIDI[0]))  || (Packet.MIDI[0] == SYSEX))
			{
				// Someone send a F0 nn F7 or F0 F7.
				BytesQueue->PutQueue(EOX); Size++;
			}
		}
		break;

		case CODE_INDEX_NUMBER_1_BYTE:
		{
			// If this is a System RealTime message, then keep
			// the current EndOfSysEx status.
			if (!((Packet.MIDI[0] >= TIMING_CLOCK) && (Packet.MIDI[0] <= SYSTEM_RESET)))
			{
				// Not a realtime message. Premature abortion...
				m_EndOfSysEx = TRUE;
			}
		}
		break;

		default:
		{
			// All other packet type ends the SysEx message. Premature abortion...
			m_EndOfSysEx = TRUE;
		}
		break;
	}

	return Size;
}

/*****************************************************************************
 * CMidiClient::PackageMidiEvent()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
ULONG
CMidiClient::
PackageMidiEvent
(
	IN		CMidiQueue *	BytesQueue,
	IN		PUCHAR			Buffer,
	IN		ULONG			BufferLength,
	IN	OUT	LONGLONG *		TimeStampCounter,
	OUT		BOOL *			OutStructured
)
{
	// If the message is structured.
	BOOL Structured = TRUE;

	// Parse the MIDI bytes to form messages.
	ULONG Success = 0;

	for (ULONG i=0; BytesQueue->CanGetQueue(); i++)
	{
		UCHAR Byte;  BytesQueue->GetQueue(&Byte);

		UCHAR Status = m_MidiParser.Parse(Byte);

		switch (Status)
		{
			case NOTE_OFF:
			case NOTE_ON:
			case POLYKEY_PRESSURE:
			case CONTROL_CHANGE:
			case PITCH_WHEEL:
			case SONG_POS_POINTER:
			{
				Buffer[0] = m_MidiParser.GetChannelStatus();
				Buffer[1] = m_MidiParser.GetData0();
				Buffer[2] = m_MidiParser.GetData1();

				Success = 3;
			}
			break;

			case PROGRAM_CHANGE:
			case CHANNEL_PRESSURE:
			case MTC_QUARTER_FRAME:
			case SONG_SELECT:
			{
				Buffer[0] = m_MidiParser.GetChannelStatus();
				Buffer[1] = m_MidiParser.GetData0();

				Success = 2;
			}
			break;

			case SYSEX:
			{
				// This status is returned at the end of the SysEx message.
				ASSERT(m_MidiParser.GetChannelStatus() == SYSEX);

				CMidiQueue * MidiQueue = m_MidiParser.GetMidiQueue();

				USHORT BytesInQueue = MidiQueue->CanGetQueue(); // including EOX

				ASSERT((BytesInQueue > 0) && BytesInQueue <=BufferLength);

				for (ULONG j=0; j<BytesInQueue; j++)
				{
					MidiQueue->GetQueue(&Buffer[j]);
				}

				if (m_SysExTimeStampCounter)
				{
					*TimeStampCounter = m_SysExTimeStampCounter;
				}

				m_SysExTimeStampCounter = 0;

				Success = BytesInQueue;

				Structured = FALSE;
			}
			break;


			case F4: /* ED-MIDI command. */
			{
				ULONG DataCount = m_MidiParser.GetDataCount();

				if (DataCount == 2)
				{
					// 3 byte system common message.
					Buffer[0] = m_MidiParser.GetChannelStatus();
					Buffer[1] = m_MidiParser.GetData0();
					Buffer[2] = m_MidiParser.GetData1();

					Success = 3;
				}
				else if (DataCount > 2)
				{
					// 1 byte data only, no message.
					Buffer[0] = m_MidiParser.GetData0();

					Success = 1;
				}
			}
			break;

			//case 0xF5: /* Undefined data bytes. */
			case TUNE_REQUEST:
			case TIMING_CLOCK:
			case 0xF9:
			case START:
			case CONTINUE:
			case STOP:
			case 0xFD:
			case ACTIVE_SENSING:
			case SYSTEM_RESET:
			{
				// Single byte system common message.
				Buffer[0] = Status;

				Success = 1;
			}
			break;

			case NOOP:
			{
				if (m_MidiParser.GetChannelStatus() == SYSEX)
				{
					// Start or running SysEx message.
					CMidiQueue * MidiQueue = m_MidiParser.GetMidiQueue();

					USHORT BytesInQueue = MidiQueue->CanGetQueue();

					if (m_SysExTimeStampCounter == 0)
					{
						m_SysExTimeStampCounter = *TimeStampCounter;
					}

					if (BytesInQueue >= BufferLength)
					{
						for (ULONG j=0; j<BufferLength; j++)
						{
							MidiQueue->GetQueue(&Buffer[j]);
						}

						LONGLONG NewSysExTimeStampCounter = *TimeStampCounter;

						*TimeStampCounter = m_SysExTimeStampCounter;

						m_SysExTimeStampCounter = NewSysExTimeStampCounter;

						Success = BufferLength;
					}

					Structured = FALSE;
				}
			}
			break;
		}

		if (Success) break;
	}

	if (OutStructured)
	{
		*OutStructured = Structured;
	}

	return Success;
}

/*****************************************************************************
 * CMidiClient::TransmitPacket()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Send a USB-MIDI event packet to the cable.
 * @param
 * Packet USB-MIDI event packet to be sent.
 * @param
 * Flush Whether to flush the USB-MIDI event packet out immediately.
 * @return
 * <None>
 */
VOID
CMidiClient::
TransmitPacket
(
	IN		USB_MIDI_EVENT_PACKET	Packet,
	IN		BOOL					Flush
)
{
	m_NumberOfPacketsTransmitted += 1;

	m_Cable->TransmitPacket(Packet, this);

	if (Flush)
	{
		m_Cable->FlushFifo();
	}
}

/*****************************************************************************
 * CMidiClient::ReceivePacket()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Receive an USB-MIDI event packet, and deconstructed the event packet into
 * MIDI bytes to client.
 * @param
 * Packet USB-MIDI event packet to be processed.
 * @return
 * <None>
 */
VOID
CMidiClient::
ReceivePacket
(
	IN		USB_MIDI_EVENT_PACKET	Packet,
	IN		LONGLONG				TimeStampCounter
)
{
	Lock();

	AddPacket(Packet, TimeStampCounter);

	Unlock();
}

/*****************************************************************************
 * CMidiClient::GetNumQueuedPackets()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Get number of packets queued in the FIFO.
 * @param
 * <None>
 * @return
 * Returns number of bytes queued in the FIFO.
 */
ULONG
CMidiClient::
GetNumQueuedPackets
(	void
)
{
    if (m_WritePosition > m_ReadPosition)
	{
        return m_WritePosition - m_ReadPosition - 1;
	}
    else
	{
        return (BUFFER_SIZE+1) - m_ReadPosition + m_WritePosition - 1;
	}
}

/*****************************************************************************
 * CMidiClient::FlushBuffer()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Flush the MIDI packets queued to the FIFO.
 * @param
 * <None>
 * @return
 * Returns TRUE if it is the end of the message, otherwise FALSE.
 */
BOOL
CMidiClient::
FlushBuffer
(
	IN		BOOL	SysExMode,
	IN		BOOL	Synchronize
)
{
	/* Take a time stamp to indicate when is the last time the client performed this operation. */
	KeQuerySystemTime(&m_ActivityTimeStamp);

	BOOL EndOfMessage = FALSE;

	if (Synchronize) Lock();

	if (SysExMode)
	{
		while (m_Cable->IsFifoReady())
		{
			USB_MIDI_EVENT_PACKET Packet;

			if (RemovePacket(&Packet, NULL))
			{
				/* If this is a realtime message, flush it out immediately. */
				BOOL Flush = (Packet.CodeIndexNumber == CODE_INDEX_NUMBER_1_BYTE) && 
					         (Packet.MIDI[0] >= TIMING_CLOCK) && (Packet.MIDI[0] <= SYSTEM_RESET);

				/* Just write the byte into the hardware FIFO */
				TransmitPacket(Packet, Flush);

				if ((Packet.CodeIndexNumber == CODE_INDEX_NUMBER_3_BYTE_SYSEX_END) ||
					(Packet.CodeIndexNumber == CODE_INDEX_NUMBER_2_BYTE_SYSEX_END) ||
					((Packet.CodeIndexNumber == CODE_INDEX_NUMBER_1_BYTE_SYSEX_END) && 
					 !STATUS_BYTE(Packet.MIDI[0])))
				{
					EndOfMessage = TRUE;
					break;
				}
			}
			else
			{
				EndOfMessage = TRUE;
				break;
			}
		}
	}
	else
	{
		while (m_Cable->IsFifoReady())
		{
			USB_MIDI_EVENT_PACKET Packet;

			if (RemovePacket(&Packet, NULL))
			{
				/* If this is a realtime message, flush it out immediately. */
				BOOL Flush = (Packet.CodeIndexNumber == CODE_INDEX_NUMBER_1_BYTE) && 
					         (Packet.MIDI[0] >= TIMING_CLOCK) && (Packet.MIDI[0] <= SYSTEM_RESET);

				/* Just write the byte into the hardware FIFO */
				TransmitPacket(Packet, Flush);
			}
			else
			{
				EndOfMessage = TRUE;
				break;
			}
		}
	}

	if (Synchronize) Unlock();

	return EndOfMessage;
}

/*****************************************************************************
 * CMidiClient::WriteBuffer()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Writes a buffer of data to the MIDI FIFO.
 * @details
 * This routine does not block if the MIDI FIFO is full. If the entire buffer
 * couldn't be queued,the MIDI FIFO will call back when space is available.
 * @param
 * Buffer Pointer to the buffer that contains the MIDI data for the client.
 * @param
 * BytesLength Length in bytes of the MIDI data stream buffer at Buffer.
 * @param
 * Synchronous Indicates whether the write needs to occur synchronously. If TRUE,
 * this routine will wait till all the data is sent out before returning to the
 * caller.
 * @return
 * Returns the actual number of bytes successfully written to the FIFO.
 * This value must be either the same value as BufferLength or the next
 * greater multiple of four.
 */
ULONG
CMidiClient::
WriteBuffer
(
	IN		PUCHAR		Buffer,
	IN		ULONG		BufferLength,
	IN		LONGLONG	TimeStampCounter,
	IN		BOOL		Synchronous
)
{
	ASSERT(m_Cable);
	ASSERT(m_Direction == MIDI_OUTPUT);

	/* Take a time stamp to indicate when is the last time the client performed this write operation. */
	KeQuerySystemTime(&m_ActivityTimeStamp);

	ULONG BytesWritten = 0;

    /* First of all, before we can write to the Midi cable. We need to find
       out if anyone else is in SysExMode that is on the same cable number. */
	m_Cable->LockClientList();

	CMidiClient * SysExClient = m_Cable->FindSysExClient();

	m_Cable->UnlockClientList();

	if (SysExClient && (SysExClient != this))
	{
		/* Not this one. */
	}
	else
	{
		m_Cable->LockFifo();

		Lock();

		/* Copy some bytes into the ring buffer */
		if (BytesWritten < BufferLength)
		{
			while ( (BytesWritten < BufferLength) /* Write up to data length */ &&
					(((GET_NUM_AVAIL()/BLOCK_ALIGN)*BLOCK_ALIGN) /* Any space(in block align) available */ ||
						(BytesWritten % BLOCK_ALIGN)) ) /* Block aligned */
			{
				if (!IsFull()) /* If the ring buffer is not filled up. */
				{
					USB_MIDI_EVENT_PACKET Packet;

					if (AssemblePacket(*Buffer, &Packet))
					{
						AddPacket(Packet, TimeStampCounter);
					}

					Buffer++;
					BytesWritten++;
				}
			}
		}

		/* Flush the ring buffer. */
		if (FlushBuffer(SysExClient == this, FALSE))
		{
			/* Flush the FIFO. */
			m_Cable->FlushFifo();
		}

		Unlock();

		m_Cable->UnlockFifo();
	}

	if (Synchronous)
	{	
		LARGE_INTEGER TimeOut; TimeOut.QuadPart = -1*10000; // 1ms

		// Wait till all the data have been sent out...
		while (!IsEmpty() || (m_NumberOfPacketsCompleted < m_NumberOfPacketsTransmitted))
		{
			/* Take a time stamp to indicate when is the last time the client performed this operation. */
			KeQuerySystemTime(&m_ActivityTimeStamp);

			KeClearEvent(&m_PacketCompletionEvent);

			KeWaitForSingleObject(&m_PacketCompletionEvent, Executive, KernelMode, FALSE, &TimeOut);
		}
	}

	/* Take a time stamp to indicate when is the last time the client performed this operation. */
	KeQuerySystemTime(&m_ActivityTimeStamp);

	return BytesWritten;
}

/*****************************************************************************
 * CMidiClient::ReadBuffer()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Reads a buffer of data from the MIDI FIFO.
 * @details
 * This routine does not block if no data is available.
 * @param
 * Buffer Buffer address of the incoming MIDI stream.
 * @param
 * BufferLength Length in bytes of the buffer pointed to by Buffer.
 * Must be multiples of 4.
 * @return
 * Returns the actual number of bytes successfully read from the MIDI FIFO.
 */
ULONG
CMidiClient::
ReadBuffer
(
	IN		PUCHAR		Buffer,
	IN		ULONG		BufferLength,
	OUT		LONGLONG *	OutTimeStampCounter	OPTIONAL,
	OUT		ULONG *		OutStatus	OPTIONAL
)
{
	//ASSERT((BufferLength >= 4) && ((BufferLength % 4) == 0));
	ASSERT(m_Direction == MIDI_INPUT);

	ULONG BytesRead = 0;

	Lock();

	ULONG Status = 0;

	/* First read data out of the buffer, if any. */
	if ((BytesRead + 3) <= BufferLength)
	{
		BOOL Continue = TRUE;

		if (m_MidiBytesQueue.CanGetQueue())
		{
			LONGLONG TimeStampCounter = m_SysExTimeStampCounter;

			BOOL Structured = TRUE;

			BytesRead = PackageMidiEvent(&m_MidiBytesQueue, Buffer, BufferLength, &TimeStampCounter, &Structured);

			if (BytesRead)
			{
				if (OutTimeStampCounter)
				{
					*OutTimeStampCounter = TimeStampCounter;
				}

				Status |= (Structured) ? MESSAGE_STATUS_STRUCTURED : 0;

				Continue = FALSE;
			}
		}

		if (Continue)
		{
			USB_MIDI_EVENT_PACKET Packet;

			LONGLONG TimeStampCounter;

			while (RemovePacket(&Packet, &TimeStampCounter))
			{
				if (DisassemblePacket(Packet, &m_MidiBytesQueue))
				{
					BOOL Structured = TRUE;

					BytesRead = PackageMidiEvent(&m_MidiBytesQueue, Buffer, BufferLength, &TimeStampCounter, &Structured);

					if (BytesRead)
					{
						if (OutTimeStampCounter)
						{
							*OutTimeStampCounter = TimeStampCounter;
						}

						Status |= (Structured) ? MESSAGE_STATUS_STRUCTURED : 0;

						break;
					}
				}
			}
		}
    }

	if (OutStatus)
	{
		*OutStatus = Status;
	}

	Unlock();

    return BytesRead;
}

/*****************************************************************************
 * CMidiClient::IsFull()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Check to see whether the MIDI FIFO is FULL.
 * @param
 * <None>
 * @return
 * Returns TRUE if the MIDI FIFO is FULL, otherwise FALSE.
 */
BOOL
CMidiClient::
IsFull
(	void
)
{
    return (GetNumQueuedPackets() == BUFFER_SIZE);
}

/*****************************************************************************
 * CMidiClient::IsEmpty()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Check to see whether the MIDI FIFO is EMPTY.
 * @param
 * <None>
 * @return
 * Returns TRUE if the MIDI FIFO is EMPTY, otherwise FALSE.
 */
BOOL
CMidiClient::
IsEmpty
(	void
)
{
    return (GetNumQueuedPackets() == 0);
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiClient::Start()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Start the client.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiClient::
Start
(
	OUT		LONGLONG *	OutStartTimeStampCounter	OPTIONAL,
	OUT		LONGLONG *	OutTimeStampFrequency	OPTIONAL
)
{
	PAGED_CODE();

	MIDISTATUS midiStatus = m_Cable->Start();

	if (MIDI_SUCCESS(midiStatus))
	{
		LARGE_INTEGER PerformanceFrequency;

		LARGE_INTEGER PerformanceCounter = KeQueryPerformanceCounter(&PerformanceFrequency);

		if (OutStartTimeStampCounter)
		{
			*OutStartTimeStampCounter = PerformanceCounter.QuadPart;
		}

		if (OutTimeStampFrequency)
		{
			*OutTimeStampFrequency = PerformanceFrequency.QuadPart;
		}

		m_IsActive = TRUE;
	}

	return midiStatus;
}

/*****************************************************************************
 * CMidiClient::Pause()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Pause the client.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiClient::
Pause
(	void
)
{
	PAGED_CODE();

	m_IsActive = FALSE;

	MIDISTATUS midiStatus = m_Cable->Pause();

	return midiStatus;
}

/*****************************************************************************
 * CMidiClient::Stop()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Stop the client.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiClient::
Stop
(	void
)
{
	PAGED_CODE();

	m_IsActive = FALSE;

	MIDISTATUS midiStatus = m_Cable->Stop();

	return midiStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiClient::Reset()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Clears any data from the FIFO.
 * @param
 * <None>
 * @return
 * <None>
 */
VOID
CMidiClient::
Reset
(	void
)
{
	m_Cable->Reset();

	Lock();

	m_MidiBytesQueue.Reset();

	m_MidiParser.ResetParser();

	m_EndOfSysEx = TRUE;

	m_ReadPosition = 0;
    m_WritePosition = 1;

	m_NumberOfPacketsTransmitted = 0;
	m_NumberOfPacketsCompleted = 0;

	Unlock();
}

/*****************************************************************************
 * CMidiClient::IsActive()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Determine if the client is active.
 * @param
 * <None>
 * @return
 * Returns TRUE if client is active, otherwise FALSE.
 */
BOOL
CMidiClient::
IsActive
(	void
)
{
	return m_IsActive;
}

/*****************************************************************************
 * CMidiClient::InSysExMode()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Determine if the client is in SysEx mode.
 * @param
 * <None>
 * @return
 * Returns TRUE if client is in SysEx mode, otherwise FALSE.
 */
BOOL
CMidiClient::
InSysExMode
(	void
)
{
	if (m_Direction == MIDI_OUTPUT)
	{
		if ((!m_EndOfSysEx) && (m_MidiParser.GetChannelStatus() == SYSEX))
		{
			// Determine if the client has improperly aborted the SysEx
			// message by looking at the last activity time stamp. If it
			// exceeds the predetermined timeout value, then reset the
			// MIDI parser to unblock the shared virtual cable.			
			LARGE_INTEGER CurrentTime; KeQuerySystemTime(&CurrentTime);

			if ((CurrentTime.QuadPart - m_ActivityTimeStamp.QuadPart) > LONGLONG(GTI_MILLISECONDS(m_SysExTimeOutPeriod)))
			{
				// The client has timed out.
				m_MidiParser.ResetParser();

				m_EndOfSysEx = TRUE;

				return FALSE;
			}
			else
			{
				// Still within the timeout period, so it is given the rights
				// to the virtual cable.
				return TRUE;
			}
		}
		else
		{
			// Not in SysEx mode.
			return FALSE;
		}
	}
	else
	{
		return (!m_EndOfSysEx);
	}
}

/*****************************************************************************
 * CMidiClient::SetSysExTimeOutPeriod()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Set the SysEx message inactivity time out.
 * @param
 * TimeOutPeriod Time out period in ms.
 * @return
 * <None>
 */
VOID
CMidiClient::
SetSysExTimeOutPeriod
(
	IN		ULONG	TimeOutPeriod
)
{
	m_SysExTimeOutPeriod = TimeOutPeriod;
}

/*****************************************************************************
 * CMidiClient::RequestCallback()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Set the callback flag.
 * @param
 * <None>
 * @return
 * <None>
 */
VOID
CMidiClient::
RequestCallback
(
	IN		ULONG	PacketsCompleted
)
{
	m_NumberOfPacketsCompleted += PacketsCompleted;

	m_CallbackRequired = TRUE;
}

/*****************************************************************************
 * CMidiClient::ClearCallback()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Clear the callback flag.
 * @param
 * <None>
 * @return
 * <None>
 */
VOID
CMidiClient::
ClearCallback
(	void
)
{
	m_CallbackRequired = FALSE;
}

/*****************************************************************************
 * CMidiClient::IsCallbackRequired()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Determine if callback is required by the client.
 * @param
 * <None>
 * @return
 * Returns TRUE if callback is required by the client, otherwise FALSE.
 */
BOOL
CMidiClient::
IsCallbackRequired
(	void
)
{
	return m_CallbackRequired;
}

/*****************************************************************************
 * CMidiClient::Service()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Service the MIDI interrupts.
 * @param
 * <None>
 * @return
 * <None>
 */
VOID
CMidiClient::
Service
(	void
)
{
	if (m_Direction == MIDI_OUTPUT)
	{
		KeSetEvent(&m_PacketCompletionEvent, IO_SOUND_INCREMENT, FALSE);

		if (m_CallbackRoutine)
		{
			m_CallbackRoutine(m_CallbackData, GET_NUM_AVAIL());
		}
	}
	else
	{
		if (m_CallbackRoutine)
		{
			m_CallbackRoutine(m_CallbackData, GetNumQueuedPackets());
		}
	}
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiCable::~CMidiCable()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Destructor.
 */
CMidiCable::
~CMidiCable
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiCable::~CMidiCable]"));

	if (m_Direction == MIDI_OUTPUT)
	{
		FreeResources();
	}

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CMidiCable::Init()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
MIDISTATUS
CMidiCable::
Init
(
	IN		UCHAR				CableNumber,
	IN		UCHAR				AssociatedJackID,
	IN		PUSB_DEVICE			UsbDevice,
	IN		CMidiDataPipe *		DataPipe
)
{
	PAGED_CODE();

	m_CableNumber = CableNumber;

	m_AssociatedJackID = AssociatedJackID;

	m_InterfaceNumber = DataPipe->InterfaceNumber();

	m_EndpointAddress = DataPipe->EndpointAddress();

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_DataPipe = DataPipe;

	m_PipeHandle = m_DataPipe->PipeHandle();

	m_Direction = USB_ENDPOINT_DIRECTION_IN(m_EndpointAddress) ? MIDI_INPUT : MIDI_OUTPUT;

	m_MaximumTransferSize = (m_DataPipe->MaximumTransferSize() < MIDI_FIFO_BUFFER_SIZE) ? m_DataPipe->MaximumTransferSize() : MIDI_FIFO_BUFFER_SIZE;

	//BEGIN_HACK
	// Due to internal double buffering in the HulaPod USB-MIDI implementation, it is necessary to limit
	// the packet size to 32 to avoid data being sent out too "fast" for certain WHQL tests.
	PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor; m_UsbDevice->GetDeviceDescriptor(&UsbDeviceDescriptor);

	if ((UsbDeviceDescriptor->idVendor == 0x41E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F04/*HulaPod*/))
	{
		m_MaximumTransferSize = (m_MaximumTransferSize > 32) ? 32 : m_MaximumTransferSize;
	}
	//END_HACK

	m_PowerState = PowerDeviceD0;

	m_CableState = MIDI_CABLE_STATE_STOP;

	KeInitializeMutex(&m_CableStateLock, 0);

	KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	if (m_Direction == MIDI_OUTPUT)
	{
		AcquireResources();
	}

	return MIDIERR_SUCCESS;
}

/*****************************************************************************
 * CMidiCable::PowerStateChange()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns MIDIERR_SUCCESS if the power state changed.
 */
MIDISTATUS
CMidiCable::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (m_PowerState == PowerDeviceD3)
		{
			/* TODO: Reinitialize the MIDI */
		}

		m_PowerState = NewState;
	}

    return MIDIERR_SUCCESS;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiCable::AttachClient()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
MIDISTATUS
CMidiCable::
AttachClient
(
	IN		CMidiClient *	Client
)
{
	KeWaitForMutexObject(&m_CableStateLock, Executive, KernelMode, FALSE, NULL);

	m_ClientList.Lock();

    /* Add it to the device list.  Since we're just inserting onto the
     * head, we don't have to worry about any readers. */
	m_ClientList.Put(Client);

	m_ClientList.Unlock();

	KeReleaseMutex(&m_CableStateLock, FALSE);

	return MIDIERR_SUCCESS;
}

/*****************************************************************************
 * CMidiCable::DetachClient()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
MIDISTATUS
CMidiCable::
DetachClient
(
	IN		CMidiClient *	Client
)
{
	MIDISTATUS midiStatus = MIDIERR_BAD_PARAM;

	KeWaitForMutexObject(&m_CableStateLock, Executive, KernelMode, FALSE, NULL);

	m_ClientList.Lock();

	if (m_ClientList.IsItemInList(Client))
	{
        /* Dequeue the info structure from the open list */
		m_ClientList.Remove(Client);

		midiStatus = MIDIERR_SUCCESS;
    }

	m_ClientList.Unlock();

	KeReleaseMutex(&m_CableStateLock, FALSE);

	return midiStatus;
}

/*****************************************************************************
 * CMidiCable::LockClientList()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
VOID
CMidiCable::
LockClientList
(	void
)
{
	m_ClientList.Lock();
}

/*****************************************************************************
 * CMidiCable::UnlockClientList()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
VOID
CMidiCable::
UnlockClientList
(	void
)
{
	m_ClientList.Unlock();
}

/*****************************************************************************
 * CMidiCable::FindSysExClient()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
CMidiClient *
CMidiCable::
FindSysExClient
(	void
)
{
	CMidiClient * Client;

	for (Client = m_ClientList.First(); Client; Client = m_ClientList.Next(Client))
	{
		if (Client->InSysExMode()) break;
	}

	return Client;
}

/*****************************************************************************
 * CMidiCable::FindActiveClient()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
CMidiClient *
CMidiCable::
FindActiveClient
(	void
)
{
	CMidiClient * Client;

	for (Client = m_ClientList.First(); Client; Client = m_ClientList.Next(Client))
	{
		if (Client->IsActive()) break;
	}

	return Client;
}

/*****************************************************************************
 * CMidiCable::InterfaceNumber()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Returns the interface which this cable is on.
 * @param
 * <None>
 * @return
 * Returns the interface which this cable is on.
 */
UCHAR
CMidiCable::
InterfaceNumber
(	void
)
{
	return m_InterfaceNumber;
}

/*****************************************************************************
 * CMidiCable::EndpointAddress()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Returns the endpoint address that is transferring data.
 * @param
 * <None>
 * @return
 * Returns the endpoint address that is transferring data.
 */
UCHAR
CMidiCable::
EndpointAddress
(	void
)
{
	return m_EndpointAddress;
}

/*****************************************************************************
 * CMidiCable::CableNumber()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Returns the number assignment of the Embedded MIDI Jack associated with the
 * endpoint that is transferring data.
 * @param
 * <None>
 * @return
 * Returns the number assignment of the Embedded MIDI Jack associated with the
 * endpoint that is transferring data.
 */
UCHAR
CMidiCable::
CableNumber
(	void
)
{
	return m_CableNumber;
}

/*****************************************************************************
 * CMidiCable::AssociatedJackID()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Returns the Embedded MIDI Jack ID associated with the endpoint that is 
 * transferring data.
 * @param
 * <None>
 * @return
 * Returns the Embedded MIDI Jack ID associated with the endpoint that is 
 * transferring data.
 */
UCHAR
CMidiCable::
AssociatedJackID
(	void
)
{
	return m_AssociatedJackID;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiCable::AcquireResources()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Acquire the resouces used by the cable.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiCable::
AcquireResources
(	void
)
{
	PAGED_CODE();

	ASSERT(m_Direction == MIDI_OUTPUT);

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	m_MaximumIrpCount = MAX_OUTPUT_IRP;

	for (ULONG i=0; i<m_MaximumIrpCount; i++)
	{
		if (m_UsbDevice->CreateIrp(&m_FifoWorkItem[i].Irp) != STATUS_SUCCESS)
		{
			midiStatus = MIDIERR_INSUFFICIENT_RESOURCES;
			break;
		}
	}

	return midiStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiCable::FreeResources()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Free the resouces used by the cable.
 * @details
 * Runs at IRQL < DISPATCH_LEVEL.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiCable::
FreeResources
(	void
)
{
	ASSERT(m_Direction == MIDI_OUTPUT);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiCable::FreeResources]"));

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	for (ULONG i=0; i<m_MaximumIrpCount; i++)
	{
		if (m_FifoWorkItem[i].Irp)
		{
			IoFreeIrp(m_FifoWorkItem[i].Irp);

			m_FifoWorkItem[i].Irp = NULL;
		}
	}

	return midiStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiCable::Start()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Start the cable.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiCable::
Start
(	void
)
{
	PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiCable::Start]"));

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	KeWaitForMutexObject(&m_CableStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_CableState != MIDI_CABLE_STATE_RUN)
	{
		if (m_CableState == MIDI_CABLE_STATE_PAUSE)
		{
			m_CableState = MIDI_CABLE_STATE_RUN;
		}
		else // MIDI_CABLE_STATE_STOP
		{
			m_CableState = MIDI_CABLE_STATE_RUN;

			if (m_Direction == MIDI_OUTPUT)
			{
				for (ULONG i=0; i<m_MaximumIrpCount; i++)
				{
					m_FifoWorkItem[i].BytesInFifoBuffer = 0;

					m_FifoWorkItemList.Put(&m_FifoWorkItem[i]);
				}
			}
		}
	}

	KeReleaseMutex(&m_CableStateLock, FALSE);

	return midiStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiCable::Pause()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Pause the cable.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiCable::
Pause
(	void	
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiCable::Pause]"));

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	KeWaitForMutexObject(&m_CableStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_CableState != MIDI_CABLE_STATE_PAUSE)
	{		
		BOOL DisableCable;

		m_ClientList.Lock();

		DisableCable = (FindActiveClient() == NULL);

		m_ClientList.Unlock();

		if (DisableCable)
		{
			m_CableState = MIDI_CABLE_STATE_PAUSE;

			if (m_Direction == MIDI_OUTPUT)
			{
				_DbgPrintF(DEBUGLVL_BLAB,("IRPs count [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

				m_FifoWorkItemList.Lock();
				
				BOOL WaitForIrpToFinish = (m_FifoWorkItemList.Count() < m_MaximumIrpCount);

				if (WaitForIrpToFinish)
				{
					KeClearEvent(&m_NoPendingIrpEvent);
				}

				m_FifoWorkItemList.Unlock();

				if (WaitForIrpToFinish)
				{
					_DbgPrintF(DEBUGLVL_BLAB,("Waiting for IRPs to finish... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

					KeWaitForSingleObject(&m_NoPendingIrpEvent, Executive, KernelMode, TRUE, NULL);

					_DbgPrintF(DEBUGLVL_BLAB,("IRPs finished... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));
				}
			}
		}
	}

	KeReleaseMutex(&m_CableStateLock, FALSE);

	return midiStatus;
}

/*****************************************************************************
 * CMidiCable::Stop()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Stop the cable.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiCable::
Stop
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiCable::Stop]"));

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	KeWaitForMutexObject(&m_CableStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_CableState != MIDI_CABLE_STATE_STOP)
	{
		BOOL DisableCable;

		m_ClientList.Lock();

		DisableCable = (FindActiveClient() == NULL);

		m_ClientList.Unlock();

		if (DisableCable)
		{
			m_CableState = MIDI_CABLE_STATE_STOP;

			if (m_Direction == MIDI_OUTPUT)
			{
				_DbgPrintF(DEBUGLVL_BLAB,("IRPs count [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

				m_FifoWorkItemList.Lock();
				
				BOOL WaitForIrpToFinish = (m_FifoWorkItemList.Count() < m_MaximumIrpCount);

				if (WaitForIrpToFinish)
				{
					KeClearEvent(&m_NoPendingIrpEvent);
				}

				m_FifoWorkItemList.Unlock();

				if (WaitForIrpToFinish)
				{
					_DbgPrintF(DEBUGLVL_BLAB,("Waiting for IRPs to finish... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

					KeWaitForSingleObject(&m_NoPendingIrpEvent, Executive, KernelMode, TRUE, NULL);

					_DbgPrintF(DEBUGLVL_BLAB,("IRPs finished... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));
				}

				ASSERT(m_FifoWorkItemList.Count() == m_MaximumIrpCount);
				
				while (m_FifoWorkItemList.Pop()) {} // Remove all items from list.
			}
		}
	}

	KeReleaseMutex(&m_CableStateLock, FALSE);

	return midiStatus;
}

/*****************************************************************************
 * CMidiCable::Reset()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Reset the virtual cable.
 * @param
 * CableNumber The virtual cable to be to reset.
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiCable::
Reset
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiCable::Reset]"));

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	if (m_Direction == MIDI_OUTPUT)
	{
		KeWaitForMutexObject(&m_CableStateLock, Executive, KernelMode, FALSE, NULL);

		m_ClientList.Lock();

		ULONG NumberOfClients = m_ClientList.Count();

		m_ClientList.Unlock();

		if (NumberOfClients <= 1)
		{
			if (m_CableState != MIDI_CABLE_STATE_STOP)
			{
				m_CableState = MIDI_CABLE_STATE_RESET;

				// The client that call this is the only one left on the virtual
				// cable, so it is ok to send a reset request to the cable.
				for (UCHAR i = 0; i < 16; i++)
				{
					USB_MIDI_EVENT_PACKET Packet[3]; // 3 messages

					Packet[0].CableNumber = m_CableNumber;
					Packet[0].CodeIndexNumber = CODE_INDEX_NUMBER_CONTROL_CHANGE;
					Packet[0].MIDI[0] = 0xB0 | i; Packet[0].MIDI[1] = 0x40;	Packet[0].MIDI[2] = 0x00;

					Packet[1].CableNumber = m_CableNumber;
					Packet[1].CodeIndexNumber = CODE_INDEX_NUMBER_CONTROL_CHANGE;
					Packet[1].MIDI[0] = 0xB0 | i; Packet[1].MIDI[1] = 0x78;	Packet[1].MIDI[2] = 0x7F;

					Packet[2].CableNumber = m_CableNumber;
					Packet[2].CodeIndexNumber = CODE_INDEX_NUMBER_CONTROL_CHANGE;
					Packet[2].MIDI[0] = 0xB0 | i; Packet[2].MIDI[1] = 0x7B;	Packet[2].MIDI[2] = 0x7F;

					// Remove the Local Control Off message, which was a hack for the Korg MIDI gear. 
					// It caused conflicts with Emulator2X.
					/*Packet[3].CableNumber = m_CableNumber;
					Packet[3].CodeIndexNumber = CODE_INDEX_NUMBER_CONTROL_CHANGE;
					Packet[3].MIDI[0] = 0xB0 | i; Packet[3].MIDI[1] = 0x7A;	Packet[3].MIDI[2] = 0x00;/**/

					for (ULONG j=0; j<SIZEOF_ARRAY(Packet); )
					{
						LockFifo();

						BOOL FlushPacket = IsFifoReady();

						while (IsFifoReady() && (j<SIZEOF_ARRAY(Packet)))
						{
							TransmitPacket(Packet[j++], NULL);
						}

						if (FlushPacket)
						{
							FlushFifo();
						}

						UnlockFifo();
					}
				}
			}
		}

		KeReleaseMutex(&m_CableStateLock, FALSE);
	}

	return midiStatus;
}

/*****************************************************************************
 * CMidiCable::IsActive()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
BOOL
CMidiCable::
IsActive
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiCable::IsActive]"));

	return (m_CableState == MIDI_CABLE_STATE_RUN);
}

/*****************************************************************************
 * CMidiCable::Lock()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
VOID
CMidiCable::
LockFifo
(	void
)
{
	m_FifoWorkItemList.Lock();
}

/*****************************************************************************
 * CMidiCable::Unlock()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
VOID
CMidiCable::
UnlockFifo
(	void
)
{
	m_FifoWorkItemList.Unlock();
}

/*****************************************************************************
 * CMidiCable::IsFifoReady()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
BOOL
CMidiCable::
IsFifoReady
(	void
)
{
	BOOL Ready = FALSE;

	if (m_FifoWorkItemList.Count() > 0)
	{
	    PMIDI_FIFO_WORK_ITEM FifoWorkItem = m_FifoWorkItemList.First();

		ASSERT(FifoWorkItem);

		if ((m_CableState == MIDI_CABLE_STATE_RUN) || (m_CableState == MIDI_CABLE_STATE_RESET))
		{
			Ready = (FifoWorkItem->BytesInFifoBuffer <= (m_MaximumTransferSize - sizeof(USB_MIDI_EVENT_PACKET)));
		}
	}

	return Ready;
}

/*****************************************************************************
 * CMidiCable::ReceivePacket()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Receive an USB-MIDI event packet, and deconstructed the event packet into
 * MIDI bytes to client.
 * @param
 * Packet USB-MIDI event packet to be processed.
 * @return
 * <None>
 */
VOID
CMidiCable::
ReceivePacket
(
	IN		USB_MIDI_EVENT_PACKET	Packet,
	IN		LONGLONG				TimeStampCounter
)
{
	ASSERT(m_CableNumber == Packet.CableNumber);

	CMidiClient * client = m_ClientList.First();

	/* Add the byte to all of the clients */
	while (client)
	{
		if (client->IsActive())
		{
			/* Tell the client that there are new data available. */
			client->ReceivePacket(Packet, TimeStampCounter);

			client->RequestCallback(0);
		}

		client = m_ClientList.Next(client);
	}
}

/*****************************************************************************
 * CMidiCable::TransmitPacket()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
VOID
CMidiCable::
TransmitPacket
(
	IN		USB_MIDI_EVENT_PACKET	Packet,
	IN		PVOID					Tag
)
{
    ASSERT(m_FifoWorkItemList.Count() > 0);

	PMIDI_FIFO_WORK_ITEM FifoWorkItem = m_FifoWorkItemList.First();

	ASSERT(FifoWorkItem);

	ASSERT(FifoWorkItem->BytesInFifoBuffer <= (m_MaximumTransferSize - sizeof(USB_MIDI_EVENT_PACKET)));

    RtlCopyMemory(&FifoWorkItem->FifoBuffer[FifoWorkItem->BytesInFifoBuffer], &Packet, sizeof(USB_MIDI_EVENT_PACKET));

	FifoWorkItem->Tags[FifoWorkItem->BytesInFifoBuffer/sizeof(USB_MIDI_EVENT_PACKET)] = Tag; // Tag it for reference later.

	FifoWorkItem->BytesInFifoBuffer += sizeof(USB_MIDI_EVENT_PACKET);

	//_DbgPrintF(DEBUGLVL_BLAB,("TX Packet"));
	//_DbgPrintF(DEBUGLVL_BLAB,("----------------------"));
	//_DbgPrintF(DEBUGLVL_BLAB,("CodeIndexNumber: %x", Packet.CodeIndexNumber));
	//_DbgPrintF(DEBUGLVL_BLAB,("CableNumber: %x", Packet.CableNumber));
	//_DbgPrintF(DEBUGLVL_BLAB,("MIDI[0]: %x", Packet.MIDI[0]));
	//_DbgPrintF(DEBUGLVL_BLAB,("MIDI[1]: %x", Packet.MIDI[1]));
	//_DbgPrintF(DEBUGLVL_BLAB,("MIDI[2]: %x", Packet.MIDI[2]));

	if (FifoWorkItem->BytesInFifoBuffer == m_MaximumTransferSize)
    {
        FlushFifo();
    }
}

/*****************************************************************************
 * CMidiCable::FlushFifo()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
VOID
CMidiCable::
FlushFifo
(	void
)
{
	//_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiCable::Flush]"));

	if (m_Direction == MIDI_OUTPUT)
	{
		if ((m_FifoWorkItemList.Count() > 0) && (m_FifoWorkItemList.First()->BytesInFifoBuffer > 0))
		{
			PMIDI_FIFO_WORK_ITEM FifoWorkItem = m_FifoWorkItemList.Pop();

			ASSERT(FifoWorkItem);

			FifoWorkItem->Context = this;

			UsbBuildInterruptOrBulkTransferRequest
			(
				&FifoWorkItem->Urb,
				sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
				m_PipeHandle,
				FifoWorkItem->FifoBuffer,
				NULL,
				FifoWorkItem->BytesInFifoBuffer,
				USBD_TRANSFER_DIRECTION_OUT,
				NULL
			);

			m_UsbDevice->RecycleIrp(&FifoWorkItem->Urb, FifoWorkItem->Irp, IoCompletionRoutine, FifoWorkItem);
		}
	}
}

#pragma code_seg()

/*****************************************************************************
 * CMidiCable::ProcessFifoWorkItem()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Process the FIFO work item.
 * @param
 * FifoWorkItem FIFO work item to process.
 * @return
 * <None>
 */
VOID
CMidiCable::
ProcessFifoWorkItem
(
	IN		PMIDI_FIFO_WORK_ITEM	FifoWorkItem
)
{
	PUSB_MIDI_EVENT_PACKET Packet = PUSB_MIDI_EVENT_PACKET(FifoWorkItem->FifoBuffer);

	ULONG PacketCount = (FifoWorkItem->BytesInFifoBuffer/sizeof(USB_MIDI_EVENT_PACKET));

	for (ULONG i=0; i<PacketCount; i++)
	{
		CMidiClient * Client = (CMidiClient*)FifoWorkItem->Tags[i];

		if (Client)
		{
			Client->RequestCallback(1);
		}
	}/**/
}

/*****************************************************************************
 * CMidiCable::Service()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Service the MIDI interrupts.
 * @param
 * FifoWorkItem FIFO work item to service.
 * @return
 * <None>
 */
VOID
CMidiCable::
Service
(
	IN		PMIDI_FIFO_WORK_ITEM	FifoWorkItem
)
{
	BOOL SkipCallbackProcessing = FALSE;

	if (m_Direction == MIDI_OUTPUT)
	{
		if ((m_CableState == MIDI_CABLE_STATE_STOP) || (m_CableState == MIDI_CABLE_STATE_PAUSE))
		{
			m_FifoWorkItemList.Lock();

			FifoWorkItem->BytesInFifoBuffer = 0; // Empty.

			m_FifoWorkItemList.Put(FifoWorkItem);

			_DbgPrintF(DEBUGLVL_BLAB,("Current IRPs count... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

			if (m_FifoWorkItemList.Count() == m_MaximumIrpCount)
			{
				_DbgPrintF(DEBUGLVL_BLAB,("Setting Event..."));

				KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
			}

			m_FifoWorkItemList.Unlock();

			SkipCallbackProcessing = TRUE;
		}
		else
		{
			m_ClientList.Lock();

			// Service the Client that is in SysEx Mode first.
			CMidiClient * SysExClient =	FindSysExClient();

			// If there a SysEx Client, that client will be the first to handle.
			if (SysExClient)
			{
				m_FifoWorkItemList.Lock();

				ProcessFifoWorkItem(FifoWorkItem);

				FifoWorkItem->BytesInFifoBuffer = 0; // Empty.

				m_FifoWorkItemList.Put(FifoWorkItem);

				if (SysExClient->FlushBuffer(TRUE))
				{
					if (SysExClient->InSysExMode())
					{
						// Still in SysEx mode, more SysEx data to process...
						SysExClient->RequestCallback(0);
					}
					else
					{
						// To be fair to other clients, put the SysExClient at the end of the list
						// once it is done.
						if (m_ClientList.Count() > 1)
						{
							m_ClientList.Remove(SysExClient);
							m_ClientList.Put(SysExClient);
						}
					}

					FlushFifo();
				}

				m_FifoWorkItemList.Unlock();
			}
			else
			{
				m_FifoWorkItemList.Lock();

				ProcessFifoWorkItem(FifoWorkItem);

				FifoWorkItem->BytesInFifoBuffer = 0; // Empty.

				m_FifoWorkItemList.Put(FifoWorkItem);

				CMidiClient * client = m_ClientList.First();

				if (client)
				{
					BOOL RotateClientPriority = IsFifoReady();

					while (IsFifoReady())
					{
						if (client->FlushBuffer(FALSE))
						{
							/* To tell client that all queued data are already consumed. */
							client->RequestCallback(0);

							client = m_ClientList.Next(client);

							if (client == NULL)
							{
								/* No more data to transmit; flush the FIFO */
								FlushFifo();
								break;
							}
						}
					}

					if (RotateClientPriority)
					{
						// To be fair to all clients, use a round-robin scheme to allocate access priority
						// to the cable.
						if (m_ClientList.Count() > 1)
						{
							CMidiClient * FirstClient = m_ClientList.First();

							m_ClientList.Remove(FirstClient);
							m_ClientList.Put(FirstClient);
						}
					}
				}

				m_FifoWorkItemList.Unlock();
			}

			m_ClientList.Unlock();
		}
	}

	if (!SkipCallbackProcessing)
	{
		// Do callbacks while holding the locks. We can do this
		// because in the callback routine, we only perform a notify
		// to the upper layer, so there is no risk of deadlock.
		m_ClientList.Lock();

		CMidiClient * client = m_ClientList.First();

		while (client)
		{
			if (client->IsCallbackRequired())
			{
				/* Service the client. */
				client->Service();

				/* Clear the callback. */
				client->ClearCallback();
			}

			client = m_ClientList.Next(client);
		}

		m_ClientList.Unlock();
	}
}

/*****************************************************************************
 * CMidiCable::IoCompletionRoutine()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
NTSTATUS
CMidiCable::
IoCompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp,
    IN		PVOID			Context
)
{
    //_DbgPrintF(DEBUGLVL_BLAB,("[CMidiCable::IoCompletionRoutine]"));

	PMIDI_FIFO_WORK_ITEM FifoWorkItem = (PMIDI_FIFO_WORK_ITEM)Context;

	CMidiCable * that = (CMidiCable*)FifoWorkItem->Context;

	that->Service(FifoWorkItem);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiDataPipe::~CMidiDataPipe()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Destructor.
 */
CMidiDataPipe::
~CMidiDataPipe
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiDataPipe::~CMidiDataPipe]"));

	if (m_Direction == MIDI_INPUT)
	{
		Stop();

		FreeResources();
	}

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CMidiDataPipe::Init()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
MIDISTATUS
CMidiDataPipe::
Init
(
	IN		PUSB_DEVICE									UsbDevice,
	IN		UCHAR										InterfaceNumber,
	IN		USBD_PIPE_INFORMATION						PipeInformation,
	IN		PUSB_AUDIO_CS_MS_DATA_ENDPOINT_DESCRIPTOR	CsMsEndpointDescriptor
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_PipeInformation = PipeInformation;

	m_Direction = USB_ENDPOINT_DIRECTION_IN(m_PipeInformation.EndpointAddress) ? MIDI_INPUT : MIDI_OUTPUT;

	m_MaximumTransferSize = (m_PipeInformation.MaximumPacketSize < MIDI_FIFO_BUFFER_SIZE) ? m_PipeInformation.MaximumPacketSize : MIDI_FIFO_BUFFER_SIZE;

	m_NumberOfCables = CsMsEndpointDescriptor->bNumEmbMIDIJacks;

	m_PowerState = PowerDeviceD0;

	m_PipeState = MIDI_DATA_PIPE_STATE_STOP;

	KeInitializeMutex(&m_PipeStateLock, 0);

	KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	for (UCHAR i=0; i<m_NumberOfCables; i++)
	{
		m_CableList[i].Init(i, CsMsEndpointDescriptor->baAssocJackID[i], UsbDevice, this);
	}

	if (m_Direction == MIDI_INPUT)
	{
		AcquireResources();

		Start();
	}

	return MIDIERR_SUCCESS;
}

/*****************************************************************************
 * CMidiDataPipe::InterfaceNumber()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
UCHAR
CMidiDataPipe::
InterfaceNumber
(	void
)
{
	return m_InterfaceNumber;
}

/*****************************************************************************
 * CMidiDataPipe::EndpointAddress()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
UCHAR
CMidiDataPipe::
EndpointAddress
(	void
)
{
	return m_PipeInformation.EndpointAddress;
}

/*****************************************************************************
 * CMidiDataPipe::MaximumTransferSize()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
ULONG
CMidiDataPipe::
MaximumTransferSize
(	void
)
{
	return m_MaximumTransferSize;
}

/*****************************************************************************
 * CMidiDataPipe::PipeHandle()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
USBD_PIPE_HANDLE
CMidiDataPipe::
PipeHandle
(	void
)
{
	return m_PipeInformation.PipeHandle;
}

/*****************************************************************************
 * CMidiDataPipe::FindCable()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Find the cable that matches the specified criteria.
 * @return
 * Returns the MIDI cable that matches the specified criteria.
 */
CMidiCable *
CMidiDataPipe::
FindCable
(
	IN		UCHAR	CableNumber
)
{
    PAGED_CODE();

	CMidiCable * Cable = NULL;

	if (CableNumber < m_NumberOfCables)
	{
		Cable = &m_CableList[CableNumber];
	}

	return Cable;
}

/*****************************************************************************
 * CMidiDataPipe::NumberOfCables()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Returns the number of MIDI cables that this pipe has.
 * @param
 * None
 * @return
 * Returns the number of MIDI cables that this pipe has.
 */
UCHAR
CMidiDataPipe::
NumberOfCables
(	void
)
{
    PAGED_CODE();

	return m_NumberOfCables;
}

/*****************************************************************************
 * CMidiDataPipe::PowerStateChange()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns MIDIERR_SUCCESS if the power state changed.
 */
MIDISTATUS
CMidiDataPipe::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		for (UCHAR i=0; i<m_NumberOfCables; i++)
		{
			m_CableList[i].PowerStateChange(NewState);
		}

		if (m_Direction == MIDI_INPUT)
		{
			if (NewState == PowerDeviceD0)
			{
				Start();
			}
			else
			{
				Stop();
			}
		}

		m_PowerState = NewState;
	}

    return MIDIERR_SUCCESS;
}

/*****************************************************************************
 * CMidiDataPipe::AcquireResources()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Acquire the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiDataPipe::
AcquireResources
(	void
)
{
	PAGED_CODE();

	ASSERT(m_Direction == MIDI_INPUT);

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	m_MaximumIrpCount = MAX_INPUT_IRP;

    for (ULONG i=0; i<m_MaximumIrpCount; i++)
    {
        if (m_UsbDevice->CreateIrp(&m_FifoWorkItem[i].Irp) != STATUS_SUCCESS)
		{
            midiStatus = MIDIERR_INSUFFICIENT_RESOURCES;
			break;
		}
    }

	if (!MIDI_SUCCESS(midiStatus))
	{
		for (ULONG i=0; i<m_MaximumIrpCount; i++)
		{
			if (m_FifoWorkItem[i].Irp)
			{
				IoFreeIrp(m_FifoWorkItem[i].Irp);

				m_FifoWorkItem[i].Irp = NULL;
			}
		}

		m_MaximumIrpCount = 0;
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return midiStatus;
}

/*****************************************************************************
 * CMidiDataPipe::FreeResources()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Free the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiDataPipe::
FreeResources
(	void
)
{
	PAGED_CODE();

	ASSERT(m_Direction == MIDI_INPUT);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiDataPipe::FreeResources]"));

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	KeClearEvent(&m_NoPendingIrpEvent);

	for (ULONG i=0; i<m_MaximumIrpCount; i++)
	{
		if (m_FifoWorkItem[i].Irp)
		{
			IoFreeIrp(m_FifoWorkItem[i].Irp);

			m_FifoWorkItem[i].Irp = NULL;
		}
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return midiStatus;
}

/*****************************************************************************
 * CMidiDataPipe::Start()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Start the pipe.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiDataPipe::
Start
(	void
)
{
	PAGED_CODE();

	ASSERT(m_Direction == MIDI_INPUT);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiDataPipe::Start]"));

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_PipeState != MIDI_DATA_PIPE_STATE_RUN)
	{
		KeClearEvent(&m_NoPendingIrpEvent);

		m_PipeState = MIDI_DATA_PIPE_STATE_RUN;

		if (m_Direction == MIDI_INPUT)
		{
			for (ULONG i=0; i<m_MaximumIrpCount; i++)
			{
				UsbBuildInterruptOrBulkTransferRequest
				(
					&m_FifoWorkItem[i].Urb,
					sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
					m_PipeInformation.PipeHandle,
					m_FifoWorkItem[i].FifoBuffer,
					NULL,
					m_MaximumTransferSize,
					USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK,
					NULL
				);

				m_FifoWorkItem[i].Context = this;

				_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiDataPipe::Start] - Recycle IRP - %d", i));

				m_UsbDevice->RecycleIrp(&m_FifoWorkItem[i].Urb, m_FifoWorkItem[i].Irp, IoCompletionRoutine, (PVOID)&m_FifoWorkItem[i]);
			}
		}
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return midiStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiDataPipe::Stop()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Start the pipe.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiDataPipe::
Stop
(	void
)
{
	ASSERT(m_Direction == MIDI_INPUT);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiDataPipe::Stop]"));

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_PipeState != MIDI_DATA_PIPE_STATE_STOP)
	{
		m_PipeState = MIDI_DATA_PIPE_STATE_STOP;

		if (m_Direction == MIDI_INPUT)
		{
			if (m_PipeInformation.PipeHandle)
			{
				_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiDataPipe::Stop] - Abort pipe %p", m_PipeInformation.PipeHandle));

				// Abort the outstanding asynchronous transactions on the pipe (if any).
				m_UsbDevice->AbortPipe(m_PipeInformation.PipeHandle);

				// Cancel the FIFO work item. Safe to touch these Irps because the completion 
				// routine always returns STATUS_MORE_PRCESSING_REQUIRED.	
				for (ULONG i=0; i<m_MaximumIrpCount; i++)
				{
					if (m_FifoWorkItem[i].Irp)
					{
						IoCancelIrp(m_FifoWorkItem[i].Irp);
					}
				}
			}
		}

		_DbgPrintF(DEBUGLVL_BLAB,("IRPs count [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

		if (m_FifoWorkItemList.Count() < m_MaximumIrpCount)
		{
			_DbgPrintF(DEBUGLVL_BLAB,("Waiting for IRPs to finish... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

			KeWaitForSingleObject(&m_NoPendingIrpEvent, Executive, KernelMode, TRUE, NULL);

			_DbgPrintF(DEBUGLVL_BLAB,("IRPs finished... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));
		}

		ASSERT(m_FifoWorkItemList.Count() == m_MaximumIrpCount);

		while (m_FifoWorkItemList.Pop()) {} // Remove all items from list.
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return midiStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiDataPipe::Service()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Service the MIDI interrupts.
 * @param
 * FifoWorkItem FIFO work item to service.
 * @return
 * <None>
 */
VOID
CMidiDataPipe::
Service
(
	IN		PMIDI_FIFO_WORK_ITEM	FifoWorkItem
)
{
	ASSERT(m_Direction == MIDI_INPUT);

	BOOL SkipCallbackProcessing = FALSE;

	if (m_PipeState == MIDI_DATA_PIPE_STATE_STOP)
	{
		m_FifoWorkItemList.Lock();

		m_FifoWorkItemList.Put(FifoWorkItem);

		_DbgPrintF(DEBUGLVL_BLAB,("Current IRPs count... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

		if (m_FifoWorkItemList.Count() == m_MaximumIrpCount)
		{
			_DbgPrintF(DEBUGLVL_BLAB,("Setting Event..."));

			KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
		}

		m_FifoWorkItemList.Unlock();

		SkipCallbackProcessing = TRUE;
	}
	else if (FifoWorkItem->Irp->IoStatus.Status != STATUS_SUCCESS)
	{
		m_FifoWorkItemList.Lock();

		_DbgPrintF(DEBUGLVL_BLAB,("Current IRPs count... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

		m_FifoWorkItemList.Put(FifoWorkItem);

		if (m_FifoWorkItemList.Count() == m_MaximumIrpCount)
		{
			_DbgPrintF(DEBUGLVL_BLAB,("Setting Event..."));

			KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
		}

		m_FifoWorkItemList.Unlock();

		SkipCallbackProcessing = TRUE;
	}
	else
	{
		LARGE_INTEGER TimeStampCounter = KeQueryPerformanceCounter(NULL);

		// Lock all cable lists here instead in the packet loop to minimize lock/unlock calls.
		for (UCHAR CableNumber = 0; CableNumber < m_NumberOfCables; CableNumber++)
		{
			m_CableList[CableNumber].LockClientList();
		}

		// Crack the MIDI event packets.
		PUSB_MIDI_EVENT_PACKET Packet = PUSB_MIDI_EVENT_PACKET(FifoWorkItem->FifoBuffer);

		for (ULONG i=0; i<FifoWorkItem->Urb.UrbBulkOrInterruptTransfer.TransferBufferLength/sizeof(USB_MIDI_EVENT_PACKET); i++)
		{
			//_DbgPrintF(DEBUGLVL_BLAB,("RX Packet"));
			//_DbgPrintF(DEBUGLVL_BLAB,("----------------------"));
			//_DbgPrintF(DEBUGLVL_BLAB,("CodeIndexNumber: %x", Packet[i].CodeIndexNumber));
			//_DbgPrintF(DEBUGLVL_BLAB,("CableNumber: %x", Packet[i].CableNumber));
			//_DbgPrintF(DEBUGLVL_BLAB,("MIDI[0]: %x", Packet[i].MIDI[0]));
			//_DbgPrintF(DEBUGLVL_BLAB,("MIDI[1]: %x", Packet[i].MIDI[1]));
			//_DbgPrintF(DEBUGLVL_BLAB,("MIDI[2]: %x", Packet[i].MIDI[2]));

			if (SIZEOF_MIDI[Packet[i].CodeIndexNumber])
			{
				m_CableList[Packet[i].CableNumber].ReceivePacket(Packet[i], TimeStampCounter.QuadPart);
			}
		}

		// Unlock all cable lists.
		for (UCHAR CableNumber = m_NumberOfCables; CableNumber > 0; CableNumber--)
		{
			m_CableList[CableNumber-1].UnlockClientList();
		}

		UsbBuildInterruptOrBulkTransferRequest
		(
			&FifoWorkItem->Urb,
			sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
			m_PipeInformation.PipeHandle,
			FifoWorkItem->FifoBuffer,
			NULL,
			m_MaximumTransferSize,
			USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK,
			NULL
		);

		NTSTATUS ntStatus = m_UsbDevice->RecycleIrp(&FifoWorkItem->Urb, FifoWorkItem->Irp, IoCompletionRoutine, (PVOID)FifoWorkItem);

		if (!NT_SUCCESS(ntStatus))
		{
			// Clean it up...
			m_FifoWorkItemList.Lock();

			m_FifoWorkItemList.Put(FifoWorkItem);

			m_FifoWorkItemList.Unlock();
		}
	}

	if (!SkipCallbackProcessing)
	{
		for (UCHAR i=0; i<m_NumberOfCables; i++)
		{
			m_CableList[i].Service(NULL);
		}
	}
}

/*****************************************************************************
 * CMidiDataPipe::IoCompletionRoutine()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
NTSTATUS
CMidiDataPipe::
IoCompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp,
    IN		PVOID			Context
)
{
    //_DbgPrintF(DEBUGLVL_BLAB,("[CMidiDataPipe::IoCompletionRoutine]"));

	PMIDI_FIFO_WORK_ITEM FifoWorkItem = (PMIDI_FIFO_WORK_ITEM)Context;

	CMidiDataPipe * that = (CMidiDataPipe*)FifoWorkItem->Context;

	that->Service(FifoWorkItem);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiTransferPipe::~CMidiTransferPipe()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Destructor.
 */
CMidiTransferPipe::
~CMidiTransferPipe
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiTransferPipe::~CMidiTransferPipe]"));

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CMidiTransferPipe::Init()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
MIDISTATUS
CMidiTransferPipe::
Init
(
	IN		PUSB_DEVICE				UsbDevice,
	IN		UCHAR					InterfaceNumber,
	IN		USBD_PIPE_INFORMATION	PipeInformation
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_PipeInformation = PipeInformation;

	m_Direction = USB_ENDPOINT_DIRECTION_IN(m_PipeInformation.EndpointAddress) ? MIDI_INPUT : MIDI_OUTPUT;

	m_MaximumTransferSize = (m_PipeInformation.MaximumPacketSize < MIDI_FIFO_BUFFER_SIZE) ? m_PipeInformation.MaximumPacketSize : MIDI_FIFO_BUFFER_SIZE;

	m_PowerState = PowerDeviceD0;

	m_PipeState = MIDI_TRANSFER_PIPE_STATE_STOP;

	KeInitializeMutex(&m_PipeStateLock, 0);

	KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	_RestoreParameterBlock(USB_AUDIO_MIDI_EP_CONTROL_ASSOCIATION, &m_ParameterBlock);

	return MIDIERR_SUCCESS;
}

/*****************************************************************************
 * CMidiTransferPipe::InterfaceNumber()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
UCHAR
CMidiTransferPipe::
InterfaceNumber
(	void
)
{
	return m_InterfaceNumber;
}

/*****************************************************************************
 * CMidiTransferPipe::EndpointAddress()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
UCHAR
CMidiTransferPipe::
EndpointAddress
(	void
)
{
	return m_PipeInformation.EndpointAddress;
}

/*****************************************************************************
 * CMidiTransferPipe::MaximumTransferSize()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
ULONG
CMidiTransferPipe::
MaximumTransferSize
(	void
)
{
	return m_MaximumTransferSize;
}

/*****************************************************************************
 * CMidiTransferPipe::PipeHandle()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
USBD_PIPE_HANDLE
CMidiTransferPipe::
PipeHandle
(	void
)
{
	return m_PipeInformation.PipeHandle;
}

/*****************************************************************************
 * CMidiTransferPipe::PowerStateChange()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns MIDIERR_SUCCESS if the power state changed.
 */
MIDISTATUS
CMidiTransferPipe::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		m_PowerState = NewState;
	}

    return MIDIERR_SUCCESS;
}

/*****************************************************************************
 * CMidiTransferPipe::AcquireResources()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Acquire the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiTransferPipe::
AcquireResources
(	void
)
{
	PAGED_CODE();

	ASSERT(m_Direction == MIDI_INPUT);

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

    KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	m_MaximumIrpCount = MAX_TRANSFER_IRP;

	m_UsbDevice->ResetPipe(m_PipeInformation.PipeHandle);

	for (ULONG i=0; i<m_MaximumIrpCount; i++)
    {
        if (m_UsbDevice->CreateIrp(&m_FifoWorkItem[i].Irp) != STATUS_SUCCESS)
		{
            midiStatus = MIDIERR_INSUFFICIENT_RESOURCES;
			break;
		}
    }

	if (MIDI_SUCCESS(midiStatus))
	{
		if (m_Direction == MIDI_INPUT)
		{
			for (ULONG i=0; i<m_MaximumIrpCount; i++)
			{
				UsbBuildInterruptOrBulkTransferRequest
				(
					&m_FifoWorkItem[i].Urb,
					sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER),
					m_PipeInformation.PipeHandle,
					m_FifoWorkItem[i].FifoBuffer,
					NULL,
					m_MaximumTransferSize,
					USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK,
					NULL
				);

				m_FifoWorkItem[i].Context = this;

				_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiTransferPipe::AcquireResources] - Recycle IRP - %d", i));

				m_UsbDevice->RecycleIrp(&m_FifoWorkItem[i].Urb, m_FifoWorkItem[i].Irp, IoCompletionRoutine, (PVOID)&m_FifoWorkItem[i]);
			}
		}
	}

	if (!MIDI_SUCCESS(midiStatus))
	{
		for (ULONG i=0; i<m_MaximumIrpCount; i++)
		{
			if (m_FifoWorkItem[i].Irp)
			{
				IoFreeIrp(m_FifoWorkItem[i].Irp);

				m_FifoWorkItem[i].Irp = NULL;
			}
		}

		m_MaximumIrpCount = 0;
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return midiStatus;
}

/*****************************************************************************
 * CMidiTransferPipe::FreeResources()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Free the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiTransferPipe::
FreeResources
(	void
)
{
	PAGED_CODE();

	ASSERT(m_Direction == MIDI_INPUT);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiTransferPipe::FreeResources]"));

	MIDISTATUS midiStatus = MIDIERR_BAD_REQUEST;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_Direction == MIDI_INPUT)
	{
		if (m_PipeInformation.PipeHandle)
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiDataPipe::Stop] - Abort pipe %p", m_PipeInformation.PipeHandle));

			// Abort the outstanding asynchronous transactions on the pipe (if any).
			m_UsbDevice->AbortPipe(m_PipeInformation.PipeHandle);

			// Cancel the FIFO work item. Safe to touch these Irps because the completion 
			// routine always returns STATUS_MORE_PRCESSING_REQUIRED.	
			for (ULONG i=0; i<m_MaximumIrpCount; i++)
			{
				if (m_FifoWorkItem[i].Irp)
				{
					IoCancelIrp(m_FifoWorkItem[i].Irp);
				}
			}
		}
	}

	_DbgPrintF(DEBUGLVL_BLAB,("IRPs count [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

	if (m_FifoWorkItemList.Count() < m_MaximumIrpCount)
	{
		_DbgPrintF(DEBUGLVL_BLAB,("Waiting for IRPs to finish... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

		KeWaitForSingleObject(&m_NoPendingIrpEvent, Executive, KernelMode, TRUE, NULL);

		_DbgPrintF(DEBUGLVL_BLAB,("IRPs finished... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));
	}

	KeClearEvent(&m_NoPendingIrpEvent);

	ASSERT(m_FifoWorkItemList.Count() == m_MaximumIrpCount);

	while (m_FifoWorkItemList.Pop()) {} // Remove all items from list.

	for (ULONG i=0; i<m_MaximumIrpCount; i++)
	{
		if (m_FifoWorkItem[i].Irp)
		{
			IoFreeIrp(m_FifoWorkItem[i].Irp);

			m_FifoWorkItem[i].Irp = NULL;
		}
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return midiStatus;
}

/*****************************************************************************
 * CMidiTransferPipe::Start()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Start the pipe.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiTransferPipe::
Start
(	void
)
{
	PAGED_CODE();

	ASSERT(m_Direction == MIDI_INPUT);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiTransferPipe::Start]"));

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_PipeState != MIDI_TRANSFER_PIPE_STATE_RUN)
	{
		m_PipeState = MIDI_TRANSFER_PIPE_STATE_RUN;

		midiStatus = AcquireResources();
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return midiStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiTransferPipe::Stop()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Start the pipe.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiTransferPipe::
Stop
(	void
)
{
	ASSERT(m_Direction == MIDI_INPUT);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiTransferPipe::Stop]"));

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_PipeState != MIDI_TRANSFER_PIPE_STATE_STOP)
	{
		m_PipeState = MIDI_TRANSFER_PIPE_STATE_STOP;

		FreeResources();
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return midiStatus;
}

/*****************************************************************************
 * CMidiTransferPipe::SetRequest()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS
CMidiTransferPipe::
SetRequest
(
	IN		UCHAR	RequestCode,
	IN		USHORT	Value,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	NTSTATUS ntStatus = m_UsbDevice->ControlClassEndpointCommand
						(
							RequestCode,
							Value,
							m_PipeInformation.EndpointAddress,
							ParameterBlock,
							ParameterBlockSize,
							NULL,
							FALSE
						);

	return ntStatus;
}

/*****************************************************************************
 * CMidiTransferPipe::GetRequest()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS
CMidiTransferPipe::
GetRequest
(
	IN		UCHAR	RequestCode,
	IN		USHORT	Value,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
	NTSTATUS ntStatus = m_UsbDevice->ControlClassEndpointCommand
						(
							RequestCode | 0x80,
							Value,
							m_PipeInformation.EndpointAddress,
							ParameterBlock,
							ParameterBlockSize,
							OutParameterBlockSize,
							TRUE
						);

	return ntStatus;
}

/*****************************************************************************
 * CMidiTransferPipe::QueryControlSupport()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CMidiTransferPipe::
QueryControlSupport
(
	IN		UCHAR	ControlSelector
)
{
	NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;
	
	switch (ControlSelector)
	{
		case USB_AUDIO_MIDI_EP_CONTROL_ASSOCIATION:
		{
			if (m_ParameterBlock.Association.Support)
			{
				ntStatus = STATUS_SUCCESS;
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;
	}

	return ntStatus;
}

/*****************************************************************************
 * CMidiTransferPipe::WriteParameterBlock()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CMidiTransferPipe::
WriteParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	IN 		ULONG	Flags
)
{
	NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_MIDI_EP_CONTROL_ASSOCIATION:
		{
			if (m_ParameterBlock.Association.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(UCHAR))
					{
						UCHAR Association = *(PUCHAR(ParameterBlock));

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Association, 1);
						}
						else					
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock.Association.Current = Association;
							}
						}
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		default:
		{
			ASSERT(0);
			ntStatus = STATUS_INVALID_PARAMETER;
		}
		break;
	}

	return ntStatus;
}

/*****************************************************************************
 * CMidiTransferPipe::ReadParameterBlock()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CMidiTransferPipe::
ReadParameterBlock
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize,
	IN 		ULONG	Flags
)
{
	NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_MIDI_EP_CONTROL_ASSOCIATION:
		{
			if (m_ParameterBlock.Association.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(UCHAR))
					{
						PUCHAR Association = PUCHAR(ParameterBlock);
						
						*Association = m_ParameterBlock.Association.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(UCHAR);
						}

						ntStatus = STATUS_SUCCESS;
					}
				}
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		default:
		{
			ASSERT(0);
			ntStatus = STATUS_INVALID_PARAMETER;
		}
		break;
	}

	return ntStatus;
}

/*****************************************************************************
 * CMidiTransferPipe::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CMidiTransferPipe::
_RestoreParameterBlock
(
	IN		UCHAR								ControlSelector,
	IN		PMIDI_TRANSFER_PIPE_PARAMETER_BLOCK	ParameterBlock
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_MIDI_EP_CONTROL_ASSOCIATION:
		{
			UCHAR Current = 0;					

			ParameterBlock->Association.Support = GetRequest(REQUEST_CUR, Control, &Current, 1, NULL) == STATUS_SUCCESS;

			ParameterBlock->Association.Current = Current;
		}
		break;

		default:
		{
			ntStatus = STATUS_INVALID_PARAMETER;
		}
		break;
	}

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CMidiTransferPipe::Service()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Service the MIDI interrupts.
 * @param
 * FifoWorkItem FIFO work item to service.
 * @return
 * <None>
 */
VOID
CMidiTransferPipe::
Service
(
	IN		PMIDI_FIFO_WORK_ITEM	FifoWorkItem
)
{
	ASSERT(m_Direction == MIDI_INPUT);

	m_FifoWorkItemList.Lock();

	m_FifoWorkItemList.Put(FifoWorkItem);

	_DbgPrintF(DEBUGLVL_BLAB,("Current IRPs count... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

	if (m_FifoWorkItemList.Count() == m_MaximumIrpCount)
	{
		_DbgPrintF(DEBUGLVL_BLAB,("Setting Event..."));

		KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
	}

	m_FifoWorkItemList.Unlock();
}

/*****************************************************************************
 * CMidiTransferPipe::IoCompletionRoutine()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
NTSTATUS
CMidiTransferPipe::
IoCompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp,
    IN		PVOID			Context
)
{
    //_DbgPrintF(DEBUGLVL_BLAB,("[CMidiTransferPipe::IoCompletionRoutine]"));

	PMIDI_FIFO_WORK_ITEM FifoWorkItem = (PMIDI_FIFO_WORK_ITEM)Context;

	CMidiTransferPipe * that = (CMidiTransferPipe*)FifoWorkItem->Context;

	that->Service(FifoWorkItem);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CMidiTopology::~CMidiTopology()
 *****************************************************************************
 *//*!
 * @ingroup AUDIO_GROUP
 * @brief
 * Destructor.
 */
CMidiTopology::
~CMidiTopology
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiTopology::~CMidiTopology]"));

	m_EntityList.DeleteAllItems();

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CMidiTopology::Init()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Initialize the MIDI topology.
 * @return
 * Returns AUDIOERR_SUCCESS if successful, AUDIOERR_NO_MEMORY if the audio 
 * topology couldn't be created.
 */
MIDISTATUS
CMidiTopology::
Init
(
	IN		PUSB_DEVICE	UsbDevice,
	IN		UCHAR		InterfaceNumber
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_PowerState = PowerDeviceD0;

	MIDISTATUS midiStatus = ParseCsMsInterfaceDescriptor(InterfaceNumber);

	if (!MIDI_SUCCESS(midiStatus))
	{
		// Cleanup mess...
		if (m_UsbDevice)
		{
			m_UsbDevice->Release();
			m_UsbDevice = NULL;
		}
	}

	return midiStatus;
}

/*****************************************************************************
 * CMidiTopology::PowerStateChange()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns AUDIOERR_SUCCESS if the power state changed.
 */
MIDISTATUS
CMidiTopology::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

	m_PowerState = NewState;

    return MIDIERR_SUCCESS;
}

/*****************************************************************************
 * CMidiTopology::ParseCsMsInterfaceDescriptor()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 */
MIDISTATUS
CMidiTopology::
ParseCsMsInterfaceDescriptor
(
	IN		UCHAR	InterfaceNumber
)
{
    PAGED_CODE();

	PUSB_AUDIO_CS_MS_INTERFACE_DESCRIPTOR CsMsInterfaceDescriptor = NULL;

	MIDISTATUS midiStatus = m_UsbDevice->GetClassInterfaceDescriptor(InterfaceNumber, 0, USB_AUDIO_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsMsInterfaceDescriptor);

	if (MIDI_SUCCESS(midiStatus))
	{
		// Parse the input/output terminals and units...
		PUCHAR DescriptorEnd = PUCHAR(CsMsInterfaceDescriptor) + CsMsInterfaceDescriptor->wTotalLength;

		PUSB_AUDIO_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_AUDIO_COMMON_DESCRIPTOR)(PUCHAR(CsMsInterfaceDescriptor)+CsMsInterfaceDescriptor->bLength);

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_AUDIO_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  				((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			// ESI-ROMIO firmware has a bug in its CS MS interface descriptor. The total length it specify 
			// for the CS MS interface is 0x4D00, but its configuration length is only 0x71. Go figure!!
			// This will break the infinite loop, but driver verifier will catch the access beyond allocated
			// memory, so don't run this routine if you have the verifier on.
			if (CommonDescriptor->bLength == 0) break;

			if (CommonDescriptor->bDescriptorType == USB_AUDIO_CS_INTERFACE)
			{
				switch (CommonDescriptor->bDescriptorSubtype)
				{
					case USB_AUDIO_MS_DESCRIPTOR_MIDI_IN_JACK:
					{
						CMidiInJack * Jack = new(NonPagedPool) CMidiInJack();

						if (Jack)
						{
							if (MIDI_SUCCESS(Jack->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_MIDI_COMMON_JACK_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Jack);
							}
							else
							{
								delete Jack;
							}
						}
					}
					break;

					case USB_AUDIO_MS_DESCRIPTOR_MIDI_OUT_JACK:
					{
						CMidiOutJack * Jack = new(NonPagedPool) CMidiOutJack();

						if (Jack)
						{
							if (MIDI_SUCCESS(Jack->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_MIDI_COMMON_JACK_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Jack);
							}
							else
							{
								delete Jack;
							}
						}
					}
					break;

					case USB_AUDIO_MS_DESCRIPTOR_ELEMENT:
					{
						CMidiElement * Element = new(NonPagedPool) CMidiElement();

						if (Element)
						{
							if (MIDI_SUCCESS(Element->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_MIDI_ELEMENT_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Element);
							}
							else
							{
								delete Element;
							}
						}
					}
					break;

					default:
						_DbgPrintF(DEBUGLVL_VERBOSE,("Unknown/unsupport descriptor subtype: 0x%x", CommonDescriptor->bDescriptorSubtype));
						break;
				}
			}

			CommonDescriptor = (PUSB_AUDIO_COMMON_DESCRIPTOR)(PUCHAR(CommonDescriptor) + CommonDescriptor->bLength);
		} // while
	}

	return midiStatus;
}

/*****************************************************************************
 * CMidiTopology::FindEntity()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Find the entity with the specified ID.
 * @param
 * EntityID Entity identifier.
 * @param
 * OutEntity A pointer to the PENTITY which will receive the entity.
 * @return
 * TRUE if the specified ID matches one of the entity, otherwise FALSE.
 */
BOOL
CMidiTopology::
FindEntity
(
	IN		UCHAR		EntityID,
	OUT		PENTITY *	OutEntity
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		if (Entity->EntityID() == EntityID)
		{
			*OutEntity = Entity;

			Found = TRUE;
			break;
		}
	}

	return Found;
}

/*****************************************************************************
 * CMidiTopology::FindConnection()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Find if the specified entities are connected.
 * @return
 * TRUE if the specified entities are connected, otherwise FALSE.
 */
BOOL
CMidiTopology::
FindConnection
(
	IN		UCHAR	FromEntityID,
	IN		UCHAR	ToEntityID
)
{
	BOOL Connected = FALSE;

	PENTITY FromEntity = NULL; FindEntity(FromEntityID, &FromEntity);

	PENTITY ToEntity = NULL; FindEntity(ToEntityID, &ToEntity);
	
	if (FromEntity && ToEntity)
	{
		for (UCHAR i=0; ; i++)
		{
			UCHAR SourceID = 0;
			UCHAR SourcePin = 0;

			BOOL Found = (FromEntity->DescriptorSubtype() == USB_AUDIO_MS_DESCRIPTOR_ELEMENT) ? PMIDI_ELEMENT(FromEntity)->ParseSources(i, &SourceID, &SourcePin) : PMIDI_JACK(FromEntity)->ParseSources(i, &SourceID, &SourcePin);

			if (Found)
			{
				if (SourceID == ToEntity->EntityID())
				{
					Connected = TRUE;
					break;
				}
				else
				{
					Connected = FindConnection(SourceID, ToEntityID);
				}
			}
			else
			{
				break;
			}
		}
	}

	return Connected;
}

/*****************************************************************************
 * CMidiTopology::ParseJacks()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Enumerate the jacks that are available on this device.
 * @param
 * Index Enumeration index.
 * @param
 * OutTerminal A pointer to the PMIDI_JACK which will receive the terminal.
 * @return
 * TRUE if the specified index matches one of the jacks, otherwise FALSE.
 */
BOOL
CMidiTopology::
ParseJacks
(
	IN		ULONG			Index,
	OUT		PMIDI_JACK *	OutJack
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	ULONG idx = 0;

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

		if ((DescriptorSubtype == USB_AUDIO_MS_DESCRIPTOR_MIDI_IN_JACK) ||
			(DescriptorSubtype == USB_AUDIO_MS_DESCRIPTOR_MIDI_OUT_JACK))
		{
			if (idx == Index)
			{
				*OutJack = PMIDI_JACK(Entity);

				Found = TRUE;
				break;
			}

			idx++;
		}
	}

	return Found;
}

/*****************************************************************************
 * CMidiTopology::ParseElements()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Enumerate the elements that are available on this device.
 * @param
 * Index Enumeration index.
 * @param
 * OutTerminal A pointer to the PMIDI_ELEMENT which will receive the unit.
 * @return
 * TRUE if the specified index matches one of the elements, otherwise FALSE.
 */
BOOL
CMidiTopology::
ParseElements
(
	IN		ULONG			Index,
	OUT		PMIDI_ELEMENT *	OutElement
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	ULONG idx = 0;

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

		if (DescriptorSubtype == USB_AUDIO_MS_DESCRIPTOR_ELEMENT)
		{
			if (idx == Index)
			{
				*OutElement = PMIDI_ELEMENT(Entity);

				Found = TRUE;
				break;
			}

			idx++;
		}
	}

	return Found;
}

/*****************************************************************************
 * CMidiTopology::InterfaceNumber()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Returns the interface which this topology is on.
 * @param
 * <None>
 * @return
 * Returns the interface which this topology is on.
 */
UCHAR
CMidiTopology::
InterfaceNumber
(	void
)
{
	return m_InterfaceNumber;
}

/*****************************************************************************
 * CMidiDevice::NonDelegatingQueryInterface()
 *****************************************************************************
 *//*!
 * @brief
 * Obtains an interface.
 * @details
 * This function works just like a COM QueryInterface call and is used if
 * the object is not being aggregated.
 * @param
 * Interface The GUID of the interface to be retrieved.
 * @param
 * Object Pointer to the location to store the retrieved interface object.
 * @return
 * Returns STATUS_SUCCESS if the interface is found. Otherwise, returns
 * STATUS_INVALID_PARAMETER.
 */
STDMETHODIMP
CMidiDevice::
NonDelegatingQueryInterface
(
    IN		REFIID  Interface,
    OUT		PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_BLAB,("[CMidiDevice::NonDelegatingQueryInterface]"));

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PMIDI_DEVICE(this)));
    }
    else
    {
        *Object = NULL;
    }

    if (*Object)
    {
        //
        // We reference the interface for the caller.
        //
        PUNKNOWN(*Object)->AddRef();
        return MIDIERR_SUCCESS;
    }

    return MIDIERR_BAD_PARAM;
}

/*****************************************************************************
 * CMidiDevice::~CMidiDevice()
 *****************************************************************************
 *//*!
 * @ingroup MIDI_GROUP
 * @brief
 * Destructor.
 */
CMidiDevice::
~CMidiDevice
(	void
)
{
    PAGED_CODE();

	ASSERT(m_MagicNumber == MIDI_MAGIC);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiDevice::~CMidiDevice]"));

	m_TopologyList.DeleteAllItems();

	m_DataPipeList.DeleteAllItems();

	m_TransferPipeList.DeleteAllItems();

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CMidiDevice::Init()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Initialize the MIDI device.
 * @param
 * <None>
 * @return
 * Returns MIDIERR_SUCCESS if successful, MIDIERR_NO_MEMORY if the MIDI device
 * couldn't be created.
 */
MIDISTATUS
CMidiDevice::
Init
(
	IN		PUSB_DEVICE	UsbDevice
)
{
	PAGED_CODE();

    m_MagicNumber = MIDI_MAGIC;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_PowerState = PowerDeviceD0;

	//BEGIN_HACK
	LONG ClassCode = USB_CLASS_CODE_AUDIO;
	PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor; m_UsbDevice->GetDeviceDescriptor(&UsbDeviceDescriptor);
	if ((UsbDeviceDescriptor->idVendor == 0x41E/*Creative*/) &&
		((UsbDeviceDescriptor->idProduct == 0x3F02/*MicroPod*/) || 
		(UsbDeviceDescriptor->idProduct == 0x3F04/*HulaPod*/) ||
		(UsbDeviceDescriptor->idProduct == 0x3F0B/*Itey*/) ||
		(UsbDeviceDescriptor->idProduct == 0x3F0A/*MicroPre*/)))
	{
		ClassCode = -1; // don't care
	}
	//END_HACK

	PUSB_INTERFACE_DESCRIPTOR AcInterfaceDescriptor = NULL;

	MIDISTATUS midiStatus = m_UsbDevice->GetInterfaceDescriptor(-1, -1, ClassCode, USB_AUDIO_SUBCLASS_AUDIOCONTROL, -1, &AcInterfaceDescriptor);

	if (MIDI_SUCCESS(midiStatus))
	{
		PUSB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR CsAcInterfaceDescriptor = NULL;

		midiStatus = m_UsbDevice->GetClassInterfaceDescriptor(AcInterfaceDescriptor->bInterfaceNumber, 0, USB_AUDIO_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsAcInterfaceDescriptor);

		if (MIDI_SUCCESS(midiStatus))
		{
			ASSERT(CsAcInterfaceDescriptor->bDescriptorType == USB_AUDIO_CS_INTERFACE);
			ASSERT(CsAcInterfaceDescriptor->bDescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_HEADER);

			for (UCHAR i=0; i<CsAcInterfaceDescriptor->bInCollection; i++)
			{		
				PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

				midiStatus = m_UsbDevice->GetInterfaceDescriptor(CsAcInterfaceDescriptor->baInterfaceNr[i], 0, -1, -1, -1, &InterfaceDescriptor);

				if (MIDI_SUCCESS(midiStatus))
				{
					if (((ClassCode == -1) || (InterfaceDescriptor->bInterfaceClass == ClassCode)) &&
						(InterfaceDescriptor->bInterfaceSubClass == USB_AUDIO_SUBCLASS_MIDISTREAMING))
					{
						UCHAR InterfaceNumber = InterfaceDescriptor->bInterfaceNumber;

						_DbgPrintF(DEBUGLVL_VERBOSE,("[CMidiDevice::Init]- InterfaceNumber = %d", InterfaceNumber));

						CMidiTopology * Topology = new(NonPagedPool) CMidiTopology();

						if (Topology)
						{
							midiStatus = Topology->Init(m_UsbDevice, InterfaceNumber);

							if (MIDI_SUCCESS(midiStatus))
							{
								m_TopologyList.Put(Topology);
							}
							else
							{
								delete Topology;
							}
						}
						else
						{
							midiStatus = MIDIERR_NO_MEMORY;
						}

						if (MIDI_SUCCESS(midiStatus))
						{
							// Select alternate setting 0 for MIDI streaming.
							m_UsbDevice->SelectAlternateInterface(InterfaceNumber, 0);

							PUSBD_INTERFACE_INFORMATION InterfaceInfo = NULL;
							
							midiStatus = m_UsbDevice->GetInterfaceInformation(InterfaceNumber, 0, &InterfaceInfo);

							if (MIDI_SUCCESS(midiStatus))
							{
								for (UCHAR j=0; j<InterfaceInfo->NumberOfPipes; j++)
								{
									// Check if this endpoint is a MS Bulk MIDI IN/OUT endpoint descriptor, and not a
									// MS XFER endpoint descriptor. This can be achieved by looking up the
									// class-specific MS endpoint descriptor. There is no class-specifc MS XFER
									// endpoint descriptor.
									PUSB_AUDIO_CS_MS_DATA_ENDPOINT_DESCRIPTOR CsEndpointDescriptor = NULL;

									if (MIDI_SUCCESS(m_UsbDevice->GetClassEndpointDescriptor(InterfaceNumber, 0, InterfaceInfo->Pipes[j].EndpointAddress, USB_AUDIO_CS_ENDPOINT, (PUSB_ENDPOINT_DESCRIPTOR *)&CsEndpointDescriptor)))
									{
										ASSERT(CsEndpointDescriptor->bNumEmbMIDIJacks > 0);

										CMidiDataPipe * DataPipe = new(NonPagedPool) CMidiDataPipe();

										if (DataPipe)
										{
											midiStatus = DataPipe->Init
															(
																UsbDevice,
																InterfaceNumber,
																InterfaceInfo->Pipes[j],
																CsEndpointDescriptor
															);

											if (MIDI_SUCCESS(midiStatus))
											{
												m_DataPipeList.Put(DataPipe);
											}
											else
											{
												delete DataPipe;
												break;
											}
										}
										else
										{
											midiStatus = MIDIERR_NO_MEMORY;
											break;
										}
									}
									else
									{
										CMidiTransferPipe * TransferPipe = new(NonPagedPool) CMidiTransferPipe();

										if (TransferPipe)
										{
											midiStatus = TransferPipe->Init
															(
																UsbDevice,
																InterfaceNumber,
																InterfaceInfo->Pipes[j]
															);

											if (MIDI_SUCCESS(midiStatus))
											{
												m_TransferPipeList.Put(TransferPipe);
											}
											else
											{
												delete TransferPipe;
												break;
											}
										}
										else
										{
											midiStatus = MIDIERR_NO_MEMORY;
											break;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (!MIDI_SUCCESS(midiStatus))
	{
		// Cleanup mess...
		m_TopologyList.DeleteAllItems();

		m_DataPipeList.DeleteAllItems();

		m_TransferPipeList.DeleteAllItems();

		if (m_UsbDevice)
		{
			m_UsbDevice->Release();
			m_UsbDevice = NULL;
		}
	}

	return midiStatus;
}

/*****************************************************************************
 * CMidiDevice::PowerStateChange()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns MIDIERR_SUCCESS if the power state changed.
 */
MIDISTATUS
CMidiDevice::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

	ASSERT(m_MagicNumber == MIDI_MAGIC);

	for (CMidiTopology * Topology = m_TopologyList.First(); Topology; Topology = m_TopologyList.Next(Topology))
	{
		Topology->PowerStateChange(NewState);
	}

	for (CMidiDataPipe * DataPipe = m_DataPipeList.First(); DataPipe; DataPipe = m_DataPipeList.Next(DataPipe))
	{
		DataPipe->PowerStateChange(NewState);
	}

	for (CMidiTransferPipe * TransferPipe = m_TransferPipeList.First(); TransferPipe; TransferPipe = m_TransferPipeList.Next(TransferPipe))
	{
		TransferPipe->PowerStateChange(NewState);
	}

	m_PowerState = NewState;

    return MIDIERR_SUCCESS;
}

/*****************************************************************************
 * CMidiDevice::FindDataPipe()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Find the data pipe that matches the specified criteria.
 * @return
 * Returns the MIDI data pipe that matches the specified criteria.
 */
CMidiDataPipe *
CMidiDevice::
FindDataPipe
(
	IN		UCHAR			InterfaceNumber,
	IN		UCHAR			EndpointAddress
)
{
    PAGED_CODE();

	CMidiDataPipe * DataPipe;

	for (DataPipe = m_DataPipeList.First(); DataPipe; DataPipe = m_DataPipeList.Next(DataPipe))
	{
		if ((DataPipe->InterfaceNumber() == InterfaceNumber) &&
			(DataPipe->EndpointAddress() == EndpointAddress))
		{
			break;
		}
	}

	return DataPipe;
}

/*****************************************************************************
 * CMidiDevice::FindTransferPipe()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Find the transfer pipe that matches the specified criteria.
 * @return
 * Returns the MIDI transfer pipe that matches the specified criteria.
 */
CMidiTransferPipe *
CMidiDevice::
FindTransferPipe
(
	IN		UCHAR			InterfaceNumber,
	IN		UCHAR			EndpointAddress
)
{
    PAGED_CODE();

	CMidiTransferPipe * TransferPipe;

	for (TransferPipe = m_TransferPipeList.First(); TransferPipe; TransferPipe = m_TransferPipeList.Next(TransferPipe))
	{
		if ((TransferPipe->InterfaceNumber() == InterfaceNumber) &&
			(TransferPipe->EndpointAddress() == EndpointAddress))
		{
			break;
		}
	}

	return TransferPipe;
}

/*****************************************************************************
 * CMidiDevice::ParseCables()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Enumerate the cables that are available on this device.
 * @param
 * OutCable Pointer to the PMIDI_CABLE which will receive the enumerated
 * MIDI cable.
 * @return
 * TRUE if the specified index matches one of the MIDI cables, otherwise
 * FALSE.
 */
BOOL
CMidiDevice::
ParseCables
(
	IN		ULONG			Index,
	OUT		PMIDI_CABLE *	OutCable
)
{
    PAGED_CODE();

	BOOL Found = FALSE;

	ULONG idx = 0;

	for (CMidiDataPipe * DataPipe = m_DataPipeList.First(); DataPipe; DataPipe = m_DataPipeList.Next(DataPipe))
	{
		for (UCHAR i=0; i<DataPipe->NumberOfCables(); i++)
		{
			CMidiCable * Cable = DataPipe->FindCable(i);

			if (Cable)
			{
				if (Index == idx)
				{
					*OutCable = Cable;

					Found = TRUE;
					break;
				}
			}

			idx++;
		}

		if (Found) break;
	}

	return Found;
}

/*****************************************************************************
 * CMidiDevice::ParseTopology()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Enumerate the MIDI topology that are available on this device.
 * @param
 * Index Enumeration index.
 * @return
 * Returns the MIDI topology that matches the specified criteria.
 */
BOOL
CMidiDevice::
ParseTopology
(
	IN		ULONG				Index,
	OUT		PMIDI_TOPOLOGY	*	OutTopology
)
{
    PAGED_CODE();

	BOOL Found = FALSE;

	ULONG idx = 0;

	for (CMidiTopology * Topology = m_TopologyList.First(); Topology; Topology = m_TopologyList.Next(Topology))
	{
		if (Index == idx)
		{
			*OutTopology = Topology;

			Found = TRUE;
			break;
		}

		idx++;
	}

	return Found;
}

/*****************************************************************************
 * CMidiDevice::GetDeviceDescriptor()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 */
MIDISTATUS
CMidiDevice::
GetDeviceDescriptor
(
	OUT		PUSB_DEVICE_DESCRIPTOR *	OutDeviceDescriptor
)
{
    PAGED_CODE();

	return m_UsbDevice->GetDeviceDescriptor(OutDeviceDescriptor);
}

/*****************************************************************************
 * CMidiDevice::Open()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Open a particular MIDI channel for input or output.
 * @param
 * Direction MIDI_INPUT or MIDI_OUTPUT.
 * @param
 * CallbackRoutine The function to call when the MIDI transitions to an empty
 * state (on output) or a character becomes available (on input).
 * @param
 * CallbackData A user-specified handle which is passed as a parameter when
 * the callback is called.
 * @param
 * OutMidiClient A pointer to the CMidiClient* which will receive the newly
 * opened MIDI client.
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiDevice::
Open
(
	IN		UCHAR					InterfaceNumber,
	IN		UCHAR					EndpointAddress,
	IN		UCHAR					CableNumber,
    IN		MIDI_CALLBACK_ROUTINE	CallbackRoutine,
	IN		PVOID					CallbackData,
	OUT		PMIDI_CLIENT *			OutClient
)
{
    PAGED_CODE();

	ASSERT(m_MagicNumber == MIDI_MAGIC);

	MIDISTATUS midiStatus = MIDIERR_SUCCESS;

    CMidiClient * Client = new(NonPagedPool) CMidiClient();

	if (Client)
	{
		// Use InterfaceNumber & EndpointAddress to find the correct pipe to use.
		CMidiDataPipe * DataPipe = FindDataPipe(InterfaceNumber, EndpointAddress);

		if (DataPipe)
		{
			CMidiCable * Cable = DataPipe->FindCable(CableNumber);

			if (Cable)
			{
				MIDI_DIRECTION Direction = USB_ENDPOINT_DIRECTION_IN(EndpointAddress) ? MIDI_INPUT : MIDI_OUTPUT;

				midiStatus = Client->Init(Cable, Direction, CallbackRoutine, CallbackData);

				if (MIDIERR_SUCCESS == midiStatus)
				{
					Cable->AttachClient(Client);
				}
			}
			else
			{
				midiStatus = MIDIERR_DEVICE_CONFIGURATION_ERROR;
			}
		}
		else
		{
			midiStatus = MIDIERR_DEVICE_CONFIGURATION_ERROR;
		}

		if (!MIDI_SUCCESS(midiStatus))
		{
			Client->Destruct();
			Client = NULL;
		}
	}
	else
	{
		midiStatus = MIDIERR_NO_MEMORY;
	}

    *OutClient = Client;

	return midiStatus;
}

/*****************************************************************************
 * CMidiDevice::Close()
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * Close a previously opened MIDI client instance.
 * @param
 * Client The client to close.
 * @return
 * Returns MIDIERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
MIDISTATUS
CMidiDevice::
Close
(
	IN		PMIDI_CLIENT	Client
)
{
    PAGED_CODE();

	ASSERT(Client);
	ASSERT(m_MagicNumber == MIDI_MAGIC);

	CMidiCable * Cable = Client->Cable();

	MIDISTATUS midiStatus = Cable->DetachClient(Client);

	if (MIDI_SUCCESS(midiStatus))
	{
		Client->Destruct();
	}

    return midiStatus;
}

#pragma code_seg()
