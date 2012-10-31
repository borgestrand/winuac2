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
 * @file	   Midi.h
 * @brief	   This file defines the API for the low-level manipulation of the
 *             MIDI devices.
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
#ifndef __MIDI_H__
#define __MIDI_H__

#include "Common.h"
#include "UsbDev.h"
#include "usbaudio.h"

#include "Entity.h"
#include "Element.h"
#include "Jack.h"

#include "MidiParser.h"

/*!
 * @defgroup MIDI_GROUP MIDI Module
 */

/*****************************************************************************
 * Defines
 */
/*! @brief Size of the client data buffer. */
#define INPUT_BUFFER_SIZE			4096
#define OUTPUT_BUFFER_SIZE			4096

/*! MIDI status type definition. */
typedef LONG	MIDISTATUS;

#define MIDI_SUCCESS(Status) ((MIDISTATUS)(Status) >= 0)

//@{
/*! @brief MIDI error codes. */
#define MIDIERR_SUCCESS						STATUS_SUCCESS
#define MIDIERR_NO_MEMORY					STATUS_NO_MEMORY
#define MIDIERR_FULL						STATUS_PIPE_BUSY
#define MIDIERR_EMPTY						STATUS_PIPE_EMPTY
#define MIDIERR_BAD_HANDLE					STATUS_INVALD_HANDLE
#define MIDIERR_BAD_ID						STATUS_INVALD_HANDLE
#define MIDIERR_BAD_PARAM					STATUS_INVALID_PARAMETER
#define MIDIERR_DEVICE_CONFIGURATION_ERROR	STATUS_DEVICE_CONFIGURATION_ERROR
#define MIDIERR_BAD_REQUEST					STATUS_INVALID_DEVICE_REQUEST
#define MIDIERR_INSUFFICIENT_RESOURCES		STATUS_INSUFFICIENT_RESOURCES;
//@}

/*!
 * @brief
 * MIDI_CALLBACK_ROUTINE specifies the form of a callback function
 * which gets invoked when characters are available (in the
 * case of input) or the transmit FIFO is empty (in the case
 * of output.
 */
typedef VOID (*MIDI_CALLBACK_ROUTINE)(PVOID Context, ULONG BytesCount);

/*!
 * @brief
 * MIDI_DIRECTION indicates whether we're attempting to create a handle
 * or output.
 */
typedef enum
{
   MIDI_OUTPUT,
   MIDI_INPUT
} MIDI_DIRECTION;

/*!
 * @brief
 * Extended USB MIDI event packet.
 */
typedef struct
{
	USB_MIDI_EVENT_PACKET	Packet;
	LONGLONG				TimeStampCounter;
} USB_MIDI_EVENT_PACKET_EX, *PUSB_MIDI_EVENT_PACKET_EX;

/*!
 * @brief
 * MIDI message status.
 */
#define MESSAGE_STATUS_STRUCTURED	0x00000001

/*****************************************************************************
 * Classes
 */
class CMidiCable;
class CMidiDataPipe;
class CMidiDevice;

/*****************************************************************************
 *//*! @class CMidiClient
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * MIDI client object.
 */
class CMidiClient
{
private:
    CMidiClient *				m_Next;					/*!< @brief The next client in the linked list. */
    CMidiClient *				m_Prev;					/*!< @brief The previous client in the linked list. */
    PVOID						m_Owner;				/*!< @brief The link list owner. */

	UCHAR						m_CableNumber;			/*!< @brief Number assignment of the Embedded MIDI Jack associated
											 * with the endpoint that is transferring data. */

    CMidiCable *				m_Cable;				/*!< @brief Pointer to the MIDI cable object. */

	MIDI_DIRECTION				m_Direction;			/*!< @brief Input or output. */

	KSPIN_LOCK					m_Lock;						/*!< @brief Lock to synchronize access to the client. */
	KIRQL						m_LockIrql;				/*!< @brief Lock IRQL. */

	USHORT						m_ReadPosition;			/*!< @brief Currrent read position in the ring buffer. */
    USHORT						m_WritePosition;		/*!< @brief Current write position in the ring buffer. */
    USB_MIDI_EVENT_PACKET_EX	m_DataBuffer[max(INPUT_BUFFER_SIZE, OUTPUT_BUFFER_SIZE)+1];
														/*!< @brief Actual ring buffer array. Allocate one additional
														 * byte to account for an eccentricity in the ring buffer
														 * code which leads to one byte always being empty. */
	USHORT						BUFFER_SIZE;

	BOOL						m_IsActive;				/*!< @brief Indicates the client state: TRUE for running, FALSE for stopped. */

	LARGE_INTEGER				m_ActivityTimeStamp;	/*!< @brief The time stamp indicating the last data transfer activity performed by the client. */
	ULONG						m_SysExTimeOutPeriod;	/*!< @brief The SysEx message time out in ms. */

	BOOL						m_EndOfSysEx;			/*!< @brief Whether an end of SysEx message byte 0xF7 is detected in the MIDI byte stream. */

	LONGLONG					m_SysExTimeStampCounter;/*!< @brief Time stamp counter for the SysEx message. */

	CMidiQueue					m_MidiBytesQueue;		/*!< @brief Queue for holding input MIDI bytes not processed yet. */

	PVOID						m_CallbackData;			/*!< @brief Client's user data, if any */
    MIDI_CALLBACK_ROUTINE		m_CallbackRoutine;		/*!< @brief Client's callback routine */
    BOOL						m_CallbackRequired;		/*!< @brief Indicates that a previous read or write attempt
														 * could not be completed and that the client's
														 * callback function should be invoked the next time
														 * space is available in the ring buffer */

	CMidiParser					m_MidiParser;			/*!< @brief Helper MIDI event parser. */

	KEVENT						m_PacketCompletionEvent;

	LONG						m_NumberOfPacketsTransmitted;
	LONG						m_NumberOfPacketsCompleted;

	/*************************************************************************
     * CMidiClient private methods
     *
     * These are private member functions used internally by the object.  See
     * MIDI.CPP for specific descriptions.
     */
	BOOL AddPacket
	(
		IN		USB_MIDI_EVENT_PACKET	Packet,
		IN		LONGLONG				TimeStampCounter
	);

	BOOL RemovePacket
	(
		OUT		USB_MIDI_EVENT_PACKET *	OutPacket,
		OUT		LONGLONG *				OutTimeStampCounter	OPTIONAL
	);

	BOOL PeekPacket
	(
		OUT		USB_MIDI_EVENT_PACKET *	OutPacket,
		OUT		LONGLONG *				OutTimeStampCounter	OPTIONAL
	);

	BOOL AssemblePacket
	(
		IN		UCHAR					Byte,
		OUT		USB_MIDI_EVENT_PACKET *	OutPacket
	);

	ULONG DisassemblePacket
	(
		IN		USB_MIDI_EVENT_PACKET	Packet,
		IN		CMidiQueue *			BytesQueue
	);

	ULONG PackageMidiEvent
	(
		IN		CMidiQueue *	BytesQueue,
		IN		PUCHAR			Buffer,
		IN		ULONG			BufferLength,
		IN	OUT	LONGLONG *		TimeStampCounter,
		OUT		BOOL *			OutStructured
	);

	ULONG GetNumQueuedPackets
	(	void
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
    CMidiClient()  { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~CMidiClient() {}
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CMidiClient public methods
     *
     * These are public member functions.  See MIDI.CPP for specific
	 * descriptions.
     */
	MIDISTATUS Init
	(
		IN		CMidiCable *			Cable,
		IN		MIDI_DIRECTION			Direction,
		IN		MIDI_CALLBACK_ROUTINE	CallbackRoutine,
		IN		PVOID					CallbackData
	);

	CMidiCable * Cable
	(	void
	);

	VOID Lock
	(	void
	);

	VOID Unlock
	(	void
	);

	VOID TransmitPacket
	(
		IN		USB_MIDI_EVENT_PACKET	Packet,
		IN		BOOL					Flush
	);

	VOID ReceivePacket
	(
		IN		USB_MIDI_EVENT_PACKET	Packet,
		IN		LONGLONG				TimeStampCounter
	);

	BOOL FlushBuffer
	(
		IN		BOOL	SysExMode,
		IN		BOOL	Synchronize = TRUE
	);

	ULONG WriteBuffer
	(
		IN		PUCHAR			Buffer,
		IN		ULONG			BufferLength,
		IN		LONGLONG		TimeStampCounter,
		IN		BOOL			Synchronous
	);

	ULONG ReadBuffer
	(
		IN		PUCHAR			Buffer,
		IN		ULONG			BufferLength,
		OUT		LONGLONG *		OutTimeStampCounter	OPTIONAL,
		OUT		ULONG *			OutStatus	OPTIONAL
	);

	BOOL IsFull
	(	void
	);

	BOOL IsEmpty
	(	void
	);

	MIDISTATUS Start
	(	
		OUT		LONGLONG *	OutStartTimeStampCounter	OPTIONAL,
		OUT		LONGLONG *	OutTimeStampFrequency	OPTIONAL
	);

	MIDISTATUS Pause
	(	void
	);

	MIDISTATUS Stop
	(	void
	);

	VOID Reset
	(	void
	);

	BOOL InSysExMode
	(	void
	);

	BOOL IsActive
	(	void
	);

	VOID SetSysExTimeOutPeriod
	(
		IN		ULONG	TimeOutPeriod
	);

	VOID RequestCallback
	(
		IN		ULONG	PacketsCompleted
	);

	VOID ClearCallback
	(	void
	);

	BOOL IsCallbackRequired
	(	void
	);

	VOID Service
	(	void
	);

    /*************************************************************************
     * Friends
     */
	friend class CList<CMidiClient>;
};

typedef CMidiClient * PMIDI_CLIENT;

/*! @brief Maximum number of input IRPs. */
#define MAX_INPUT_IRP           16

/*! @brief Maximum number of output IRPs. */
#define MAX_OUTPUT_IRP          8

#include "MidiFifo.h"

#define MIDI_CABLE_STATE_STOP		0
#define MIDI_CABLE_STATE_RESET		1
#define MIDI_CABLE_STATE_PAUSE		2
#define MIDI_CABLE_STATE_RUN		3

/*****************************************************************************
 *//*! @class CMidiCable
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * MIDI cable object.
 */
class CMidiCable
{
private:
	DEVICE_POWER_STATE	m_PowerState;	/*!< @brief Device power state. */
	MIDI_DIRECTION		m_Direction;    /*!< @brief Input or output. */

	UCHAR				m_CableNumber;	/*!< @brief Number assignment of the Embedded MIDI Jack associated
										 * with the endpoint that is transferring data. */

	UCHAR				m_AssociatedJackID;

	UCHAR				m_InterfaceNumber;
	UCHAR				m_EndpointAddress;

    CList<CMidiClient>	m_ClientList;	/*!< @brief The per-cable open client linked list. */

	ULONG				m_CableState;
	KMUTEX				m_CableStateLock;

	PUSB_DEVICE			m_UsbDevice;	/*!< @brief Pointer to the USB device object. */
	CMidiDataPipe *		m_DataPipe;		/*!< @brief Pointer to the MIDI data pipe object. */

    USBD_PIPE_HANDLE    m_PipeHandle;
	ULONG				m_MaximumTransferSize;

	MIDI_FIFO_WORK_ITEM			m_FifoWorkItem[MAX_OUTPUT_IRP];
    CList<MIDI_FIFO_WORK_ITEM>	m_FifoWorkItemList;
	ULONG						m_MaximumIrpCount;
	KEVENT						m_NoPendingIrpEvent;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CMidiCable() {}
    /*! @brief Destructor. */
    ~CMidiCable();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

	/*************************************************************************
     * CMidiCable public methods
     *
     * These are public member functions.  See MIDI.CPP for specific
	 * descriptions.
     */
	MIDISTATUS Init
	(
		IN		UCHAR				CableNumber,
		IN		UCHAR				AssociatedJackID,
		IN		PUSB_DEVICE			UsbDevice,
		IN		CMidiDataPipe *		DataPipe
	);

	MIDISTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	MIDISTATUS AttachClient
	(
		IN		CMidiClient *	Client
	);

	MIDISTATUS DetachClient
	(
		IN		CMidiClient *	Client
	);

	VOID LockClientList
	(	void
	);

	VOID UnlockClientList
	(	void
	);

	CMidiClient * FindSysExClient
	(	void
	);

	CMidiClient * FindActiveClient
	(	void
	);

	UCHAR InterfaceNumber
	(	void
	);

	UCHAR EndpointAddress
	(	void
	);

	UCHAR CableNumber
	(	void
	);

	UCHAR AssociatedJackID
	(	void
	);

	MIDISTATUS AcquireResources
	(	void
	);

	MIDISTATUS FreeResources
	(	void
	);

	MIDISTATUS Start
	(	void
	);

	MIDISTATUS Pause
	(	void
	);

	MIDISTATUS Stop
	(	void
	);

	MIDISTATUS Reset
	(	void
	);

	BOOL IsActive
	(	void
	);

	VOID LockFifo
	(	void
	);

	VOID UnlockFifo
	(	void
	);

	BOOL IsFifoReady
	(	void
	);

	VOID ReceivePacket
	(
		IN		USB_MIDI_EVENT_PACKET	Packet,
		IN		LONGLONG				TimeStampCounter
	);

	VOID TransmitPacket
	(
		IN		USB_MIDI_EVENT_PACKET	Packet,
		IN		PVOID					Tag
	);

	VOID FlushFifo
	(	void
	);

	VOID ProcessFifoWorkItem
	(
		IN		PMIDI_FIFO_WORK_ITEM	FifoWorkItem
	);

	VOID Service
	(
		IN		PMIDI_FIFO_WORK_ITEM	FifoWorkItem
	);

    /*************************************************************************
     * Static
     */
	static
	NTSTATUS IoCompletionRoutine
	(
		IN		PDEVICE_OBJECT	DeviceObject,
		IN		PIRP			Irp,
		IN		PVOID			Context
	);

    /*************************************************************************
     * Friends
     */
};

typedef CMidiCable * PMIDI_CABLE;

#define MIDI_DATA_PIPE_STATE_STOP		0
#define MIDI_DATA_PIPE_STATE_RUN		1

/*****************************************************************************
 *//*! @class CMidiDataPipe
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * MIDI data pipe object.
 */
class CMidiDataPipe
{
private:
    CMidiDataPipe *		m_Next;			/*!< @brief The next pipe in the linked list. */
    CMidiDataPipe *		m_Prev;			/*!< @brief The previous pipe in the linked list. */
    PVOID				m_Owner;		/*!< @brief The link list owner. */

	DEVICE_POWER_STATE	m_PowerState;	/*!< @brief Device power state. */
	MIDI_DIRECTION		m_Direction;    /*!< @brief Input or output. */

    CMidiCable			m_CableList[16];		/*!< @brief List of cables. There can only be a maximum of 16 cables per pipe. */
	UCHAR				m_NumberOfCables;		/*!< @brief Number of cables that this pipe support. */

	ULONG				m_PipeState;
	KMUTEX				m_PipeStateLock;

	PUSB_DEVICE			m_UsbDevice;	/*!< @brief Pointer to the USB device object. */

	UCHAR					m_InterfaceNumber;
	USBD_PIPE_INFORMATION	m_PipeInformation;

	ULONG					m_MaximumTransferSize;

	MIDI_FIFO_WORK_ITEM			m_FifoWorkItem[MAX_INPUT_IRP];
    CList<MIDI_FIFO_WORK_ITEM>	m_FifoWorkItemList;
	ULONG						m_MaximumIrpCount;
	KEVENT						m_NoPendingIrpEvent;

	/*************************************************************************
     * CMidiDataPipe private methods
     *
     * These are private member functions.  See MIDI.CPP for specific
	 * descriptions.
     */

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CMidiDataPipe() { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~CMidiDataPipe();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

	/*************************************************************************
     * CMidiDataPipe public methods
     *
     * These are public member functions.  See MIDI.CPP for specific
	 * descriptions.
     */
	MIDISTATUS Init
	(
		IN		PUSB_DEVICE									UsbDevice,
		IN		UCHAR										InterfaceNumber,
		IN		USBD_PIPE_INFORMATION						PipeInformation,
		IN		PUSB_AUDIO_CS_MS_DATA_ENDPOINT_DESCRIPTOR	CsMsEndpointDescriptor
	);

	UCHAR InterfaceNumber
	(	void
	);

	UCHAR EndpointAddress
	(	void
	);

	ULONG MaximumTransferSize
	(	void
	);

	USBD_PIPE_HANDLE PipeHandle
	(	void
	);

	CMidiCable * FindCable
	(
		IN		UCHAR	CableNumber
	);

	UCHAR NumberOfCables
	(	void
	);

	MIDISTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	MIDISTATUS AcquireResources
	(	void
	);

	MIDISTATUS FreeResources
	(	void
	);

	MIDISTATUS Start
	(	void
	);

	MIDISTATUS Stop
	(	void
	);

	VOID Service
	(
		IN		PMIDI_FIFO_WORK_ITEM	FifoWorkItem
	);

    /*************************************************************************
     * Static
     */
	static
	NTSTATUS IoCompletionRoutine
	(
		IN		PDEVICE_OBJECT	DeviceObject,
		IN		PIRP			Irp,
		IN		PVOID			Context
	);

    /*************************************************************************
     * Friends
     */
	friend class CList<CMidiDataPipe>;
};

typedef CMidiDataPipe * PMIDI_DATA_PIPE;

/*****************************************************************************
 * MIDI_TRANSFER_PIPE_PARAMETER_BLOCK
 */
typedef struct
{
	struct 
	{
		BOOL	Support;
		UCHAR	Current;
	}						Association;
} MIDI_TRANSFER_PIPE_PARAMETER_BLOCK, *PMIDI_TRANSFER_PIPE_PARAMETER_BLOCK;

#define MAX_TRANSFER_IRP				16

#define MIDI_TRANSFER_PIPE_STATE_STOP	0
#define MIDI_TRANSFER_PIPE_STATE_RUN	1

/*****************************************************************************
 *//*! @class CMidiTransferPipe
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * MIDI transfer pipe object.
 */
class CMidiTransferPipe
{
private:
    CMidiTransferPipe *			m_Next;			/*!< @brief The next pipe in the linked list. */
    CMidiTransferPipe *			m_Prev;			/*!< @brief The previous pipe in the linked list. */
    PVOID						m_Owner;		/*!< @brief The link list owner. */

	DEVICE_POWER_STATE			m_PowerState;	/*!< @brief Device power state. */
	MIDI_DIRECTION				m_Direction;    /*!< @brief Input or output. */

	ULONG						m_PipeState;
	KMUTEX						m_PipeStateLock;

	PUSB_DEVICE					m_UsbDevice;	/*!< @brief Pointer to the USB device object. */

	UCHAR						m_InterfaceNumber;
	USBD_PIPE_INFORMATION		m_PipeInformation;

	ULONG						m_MaximumTransferSize;

	MIDI_FIFO_WORK_ITEM			m_FifoWorkItem[MAX_TRANSFER_IRP];
    CList<MIDI_FIFO_WORK_ITEM>	m_FifoWorkItemList;
	ULONG						m_MaximumIrpCount;
	KEVENT						m_NoPendingIrpEvent;

	MIDI_TRANSFER_PIPE_PARAMETER_BLOCK	m_ParameterBlock;

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR								ControlSelector,
		IN		PMIDI_TRANSFER_PIPE_PARAMETER_BLOCK	ParameterBlock
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CMidiTransferPipe() { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~CMidiTransferPipe();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

	/*************************************************************************
     * CMidiDataPipe public methods
     *
     * These are public member functions.  See MIDI.CPP for specific
	 * descriptions.
     */
	MIDISTATUS Init
	(
		IN		PUSB_DEVICE				UsbDevice,
		IN		UCHAR					InterfaceNumber,
		IN		USBD_PIPE_INFORMATION	PipeInformation
	);

	UCHAR InterfaceNumber
	(	void
	);

	UCHAR EndpointAddress
	(	void
	);

	ULONG MaximumTransferSize
	(	void
	);

	USBD_PIPE_HANDLE PipeHandle
	(	void
	);

	MIDISTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	MIDISTATUS AcquireResources
	(	void
	);

	MIDISTATUS FreeResources
	(	void
	);

	MIDISTATUS Start
	(	void
	);

	MIDISTATUS Stop
	(	void
	);

	NTSTATUS SetRequest
	(
		IN		UCHAR	RequestCode,
		IN		USHORT	Value,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize
	);

	NTSTATUS GetRequest
	(
		IN		UCHAR	RequestCode,
		IN		USHORT	Value,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	NTSTATUS QueryControlSupport
	(
		IN		UCHAR	ControlSelector
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		IN 		ULONG	Flags = PARAMETER_BLOCK_FLAGS_IO_BOTH
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags = PARAMETER_BLOCK_FLAGS_IO_SOFTWARE
	);

	VOID Service
	(
		IN		PMIDI_FIFO_WORK_ITEM	FifoWorkItem
	);

    /*************************************************************************
     * Static
     */
	static
	NTSTATUS IoCompletionRoutine
	(
		IN		PDEVICE_OBJECT	DeviceObject,
		IN		PIRP			Irp,
		IN		PVOID			Context
	);

    /*************************************************************************
     * Friends
     */
	friend class CList<CMidiTransferPipe>;
};

typedef CMidiTransferPipe * PMIDI_TRANSFER_PIPE;

/*****************************************************************************
 *//*! @class CMidiTopology
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * MIDI topology object.
 */
class CMidiTopology
{
private:
    CMidiTopology *				m_Next;			/*!< @brief The next topology in the linked list. */
    CMidiTopology *				m_Prev;			/*!< @brief The previous topology in the linked list. */
    PVOID						m_Owner;		/*!< @brief The link list owner. */

	DEVICE_POWER_STATE			m_PowerState;	/*!< @brief Current device power state. */

	PUSB_DEVICE					m_UsbDevice;	/*!< @brief Pointer to the USB device object. */

	UCHAR						m_InterfaceNumber;	/*! @brief AudioControl interface number */

    CList<CEntity>				m_EntityList;	/*!< @brief List of topology entities. */

	/*************************************************************************
     * CMidiTopology private methods
     *
     * These are public member functions.  See MIDI.CPP for specific
	 * descriptions.
     */
	MIDISTATUS ParseCsMsInterfaceDescriptor
	(
		IN		UCHAR	InterfaceNumber
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
    CMidiTopology()  { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~CMidiTopology();
	/*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CMidiTopology public methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	MIDISTATUS Init
	(
		IN		PUSB_DEVICE	UsbDevice,
		IN		UCHAR		InterfaceNumber
	);

	BOOL FindEntity
	(
		IN		UCHAR		EntityID,
		OUT		PENTITY *	OutEntity
	);

	BOOL FindConnection
	(
		IN		UCHAR	FromEntityID,
		IN		UCHAR	ToEntityID
	);

	BOOL ParseJacks
	(
		IN		ULONG			Index,
		OUT		PMIDI_JACK *	OutJack
	);

	BOOL ParseElements
	(
		IN		ULONG			Index,
		OUT		PMIDI_ELEMENT *	OutElement
	);

	UCHAR InterfaceNumber
	(	void
	);

	MIDISTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

    /*************************************************************************
     * Friends
     */
	friend class CList<CMidiTopology>;
};

typedef CMidiTopology * PMIDI_TOPOLOGY;

/*****************************************************************************
 *//*! @class CMidiDevice
 *****************************************************************************
 * @ingroup MIDI_GROUP
 * @brief
 * MIDI device object.
 */
class CMidiDevice
:	public CUnknown
{
private:
    ULONG						m_MagicNumber;		/*!< @brief Magic number. */
	DEVICE_POWER_STATE			m_PowerState;		/*!< @brief Current device power state. */

	PUSB_DEVICE					m_UsbDevice;		/*!< @brief Pointer to the USB device object. */

	CList<CMidiTopology>		m_TopologyList;		/*!< @brief List of topologies. */

    CList<CMidiDataPipe>		m_DataPipeList;		/*!< @brief List of data pipes. */

	CList<CMidiTransferPipe>	m_TransferPipeList;	/*!< @brief List of transfer pipes. */

	/*************************************************************************
     * CMidiDevice private methods
     *
     * These are private member functions used internally by the object.  See
     * MIDI.CPP for specific descriptions.
     */
	CMidiDataPipe * FindDataPipe
	(
		IN		UCHAR			InterfaceNumber,
		IN		UCHAR			EndpointAddress
	);

	CMidiTransferPipe * FindTransferPipe
	(
		IN		UCHAR			InterfaceNumber,
		IN		UCHAR			EndpointAddress
	);

public:
    /*************************************************************************
     * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
     * defines inline IUnknown implementations that use CUnknown's aggregation
     * support.  NonDelegatingQueryInterface() is declared, but it cannot be
     * implemented generically.  Its definition appears in USBDEV.CPP.
     * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
     * only the outer unknown, which is used for aggregation.  The standard
     * create macro (in MIDI.CPP) uses this constructor.
     */
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CMidiDevice);

    ~CMidiDevice();

    /*************************************************************************
     * CMidiDevice public methods
     *
     * These are public member functions.  See MIDI.CPP for specific
	 * descriptions.
     */
	MIDISTATUS Init
	(
		IN		PUSB_DEVICE	UsbDevice
	);

	BOOL ParseCables
	(
		IN		ULONG			Index,
		OUT		PMIDI_CABLE *	OutCable
	);

	BOOL ParseTopology
	(
		IN		ULONG				Index,
		OUT		PMIDI_TOPOLOGY	*	OutTopology
	);

	MIDISTATUS GetDeviceDescriptor
	(
		OUT		PUSB_DEVICE_DESCRIPTOR *	OutDeviceDescriptor
	);

	MIDISTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	MIDISTATUS Open
	(
		IN		UCHAR					InterfaceNumber,
		IN		UCHAR					EndpointAddress,
		IN		UCHAR					CableNumber,
		IN		MIDI_CALLBACK_ROUTINE	CallbackRoutine,
		IN		PVOID					CallbackData,
		OUT		PMIDI_CLIENT *			OutClient
	);

	MIDISTATUS Close
	(
		IN		CMidiClient *	Client
	);

    /*************************************************************************
     * Friends
     */
};

typedef CMidiDevice * PMIDI_DEVICE;

#endif /* __MIDI_H__ */
