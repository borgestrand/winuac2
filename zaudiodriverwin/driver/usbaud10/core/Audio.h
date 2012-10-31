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
 * @file	   Audio.h
 * @brief	   This file defines the API for the low-level manipulation of the
 *             audio devices.
 *
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __AUDIO_H__
#define __AUDIO_H__

#include "Common.h"
#include "UsbDev.h"
#include "usbaudio.h"

#include "Entity.h"
#include "Unit.h"
#include "Terminal.h"

#include "AudioFifo.h"


/*!
 * @defgroup AUDIO_GROUP Audio Module
 */

/*****************************************************************************
 * Defines
 */
/*! Audio status type definition. */
typedef LONG	AUDIOSTATUS;

#define AUDIO_SUCCESS(Status) ((AUDIOSTATUS)(Status) >= 0)

//@{
/*! @brief Audio error codes. */
#define AUDIOERR_SUCCESS					STATUS_SUCCESS
#define AUDIOERR_NO_MEMORY					STATUS_NO_MEMORY
#define AUDIOERR_FULL						STATUS_PIPE_BUSY
#define AUDIOERR_EMPTY						STATUS_PIPE_EMPTY
#define AUDIOERR_BAD_HANDLE					STATUS_INVALD_HANDLE
#define AUDIOERR_BAD_ID						STATUS_INVALD_HANDLE
#define AUDIOERR_BAD_PARAM					STATUS_INVALID_PARAMETER
#define AUDIOERR_DEVICE_CONFIGURATION_ERROR	STATUS_DEVICE_CONFIGURATION_ERROR
#define AUDIOERR_BAD_REQUEST				STATUS_INVALID_DEVICE_REQUEST
#define AUDIOERR_INSUFFICIENT_RESOURCES		STATUS_INSUFFICIENT_RESOURCES
#define AUDIOERR_NOT_SUPPORTED				STATUS_NOT_SUPPORTED
//@}

/*!
 * @brief
 * AUDIO_CALLBACK_ROUTINE specifies the form of a callback function
 * which gets invoked when data are available (in the
 * case of input) or the transmit buffer is empty (in the case
 * of output.
 */
typedef VOID (*AUDIO_CALLBACK_ROUTINE)(PVOID Context, ULONG Reason, PAUDIO_FIFO_WORK_ITEM FifoWorkItem);

/*!
 * @brief
 * AUDIO_CONVERSION_ROUTINE specifies the form of a copy-conversion function
 * which gets invoked when copying/converting data from one format to another.
 */
typedef VOID (*AUDIO_CONVERSION_ROUTINE)(PUCHAR Destination, PUCHAR Source, ULONG NumberOfFrames);

/*!
 * @brief
 * AUDIO_DIRECTION indicates whether we're attempting to create a handle
 * or output.
 */
typedef enum
{
   AUDIO_OUTPUT,
   AUDIO_INPUT
} AUDIO_DIRECTION;

//@{
/*! @brief The priority of the audio stream. */
#define AUDIO_PRIORITY_NONE		0
#define AUDIO_PRIORITY_LOW		1
#define AUDIO_PRIORITY_NORMAL	2
#define AUDIO_PRIORITY_HIGH		3
//@}

/*****************************************************************************
 * Classes
 */
class CAudioDataPipe;
class CAudioSynchPipe;
class CAudioInterface;
class CAudioDevice;

typedef enum
{
	AUDIO_INTERRUPT_ORIGINATOR_UNKNOWN,
	AUDIO_INTERRUPT_ORIGINATOR_AC_TERMINAL,
	AUDIO_INTERRUPT_ORIGINATOR_AC_UNIT,
	AUDIO_INTERRUPT_ORIGINATOR_AC_FUNCTION,
	AUDIO_INTERRUPT_ORIGINATOR_AS_INTERFACE,
	AUDIO_INTERRUPT_ORIGINATOR_AS_ENDPOINT
} AUDIO_INTERRUPT_ORIGINATOR;

/*!
 * @brief
 * AUDIO_INTERRUPT_HANDLER_ROUTINE specifies the form of a callback function
 * which gets invoked when the device issued an interrupt.
 */
typedef VOID (*AUDIO_INTERRUPT_HANDLER_ROUTINE)(PVOID Context, AUDIO_INTERRUPT_ORIGINATOR Originator, UCHAR OriginatorID, PVOID InterruptFilter);

/*! @brief Maximum number of input IRPs. */
#define MAX_INTERRUPT_IRP	1

#define AUDIO_INTERRUPT_PIPE_STATE_STOP		0
#define AUDIO_INTERRUPT_PIPE_STATE_RUN		1

/*****************************************************************************
 *//*! @class CAudioInterruptPipe
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Interrupt pipe object.
 */
class CAudioInterruptPipe
{
private:
    CAudioInterruptPipe *		m_Next;			/*!< @brief The next handler in the linked list. */
    CAudioInterruptPipe *		m_Prev;			/*!< @brief The previous handler in the linked list. */
    PVOID						m_Owner;		/*!< @brief The link list owner. */

	DEVICE_POWER_STATE			m_PowerState;	/*!< @brief Device power state. */

	ULONG						m_PipeState;
	KMUTEX						m_PipeStateLock;

	PUSB_DEVICE					m_UsbDevice;	/*!< @brief Pointer to the USB device object. */

	CAudioDevice *				m_AudioDevice;

	UCHAR						m_InterfaceNumber;
	USBD_PIPE_INFORMATION		m_PipeInformation;
	ULONG						m_MaximumTransferSize;

	INTERRUPT_FIFO_WORK_ITEM      	m_FifoWorkItem[MAX_INTERRUPT_IRP];
    CList<INTERRUPT_FIFO_WORK_ITEM>	m_FifoWorkItemList;
	ULONG							m_MaximumIrpCount;
	KEVENT							m_NoPendingIrpEvent;

    /*************************************************************************
     * CInterruptPipe private methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	AUDIOSTATUS AcquireResources
	(	void
	);

	AUDIOSTATUS FreeResources
	(	void
	);

public:
    /*************************************************************************
     * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
     * defines inline IUnknown implementations that use CUnknown's aggregation
     * support.  NonDelegatingQueryInterface() is declared, but it cannot be
     * implemented generically.  Its definition appears in AUDIO.CPP.
     * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
     * only the outer unknown, which is used for aggregation.  The standard
     * create macro (in AUDIO.CPP) uses this constructor.
     */
    /*! @brief Constructor. */
    CAudioInterruptPipe()  { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~CAudioInterruptPipe();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CAudioInterruptPipe public methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	AUDIOSTATUS Init
	(
		IN		PUSB_DEVICE				UsbDevice,
		IN		UCHAR					InterfaceNumber,
		IN		USBD_PIPE_INFORMATION	PipeInformation,
		IN		CAudioDevice *			AudioDevice
	);

	AUDIOSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	AUDIOSTATUS Start
	(	void
	);

	AUDIOSTATUS Stop
	(	void
	);

	AUDIOSTATUS RegisterInterruptHandler
	(
		IN		AUDIO_INTERRUPT_HANDLER_ROUTINE	InterruptHandlerRoutine,
		IN		PVOID							InterruptHandlerContext,
		OUT		PVOID *							OutHandle
	);

	AUDIOSTATUS UnregisterInterruptHandler
	(
		IN		PVOID	Handle
	);

	VOID Service
	(
		IN		PINTERRUPT_FIFO_WORK_ITEM	FifoWorkItem
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
	friend class CList<CAudioInterruptPipe>;
};

typedef CAudioInterruptPipe * PAUDIO_INTERRUPT_PIPE;

#define AUDIO_CLIENT_INPUT_BUFFERSIZE	20
#define AUDIO_CLIENT_OUTPUT_BUFFERSIZE	100

//Define number of channels supported for all products here. See AUDIO_CLIENT_MAX_CHANNEL definition too. Beware!
#define AUDIO_EMU0202_CHANNEL			2
#define AUDIO_EMU0404_CHANNEL			4
#define AUDIO_EMU1616_CHANNEL			16
// AUDIO_CLIENT_MAX_CHANNEL must be updated to the largest value of the actual definition of all products supported(see above definitions)! Beware!
#define AUDIO_CLIENT_MAX_CHANNEL        AUDIO_EMU1616_CHANNEL 

#define KSAUDIO_VOLUME_CHAN_MASTER      -1

#define MASTERVOL_MAX_DB                0           // 0 dB
#define MASTERVOL_MIN_DB                (-6291456)  // -96 dB
#define MASTERVOL_0_DB                  0           // 0 dB
#define MASTERVOL_1_DB                  65536
#define MASTERVOL_STEP_SIZE_DB          32768       // 0.5 dB
/*****************************************************************************
 *//*! @class CAudioClient
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Audio client object.
 */
class CAudioClient
{
private:
    CAudioClient *			m_Next;				/*!< @brief The next client in the linked list. */
    CAudioClient *			m_Prev;				/*!< @brief The previous client in the linked list. */
    PVOID					m_Owner;			/*!< @brief The link list owner. */

	CAudioDevice *			m_AudioDevice;		/*!< @brief Pointer to the audio device object. */

	CAudioInterface *		m_Interface;

	CAudioDataPipe *		m_DataPipe;				/*!< @brief Pointer to the audio pipe object. */
	CAudioSynchPipe *		m_SynchPipe;			/*!< @brief Pointer to the audio pipe object. */

	ULONG					m_InterfaceNumber;
	UCHAR					m_AlternateSetting;

	UCHAR					m_BitResolution;
	ULONG					m_SampleRate;
	ULONG					m_FormatChannels;
	ULONG					m_SampleSize;
	ULONG					m_NumberOfFifoBuffers;


	AUDIO_DIRECTION			m_Direction;		/*!< @brief Input or output. */

	ULONG					m_Priority;			/*!< @brief Priority of the audio stream. */

	KSPIN_LOCK				m_Lock;				/*!< @brief Lock to synchronize access to the client. */
	KIRQL					m_LockIrql;			/*!< @brief Lock IRQL. */

	ULONG					m_ReadPosition;		/*!< @brief Currrent read position in the ring buffer. */
    ULONG					m_WritePosition;	/*!< @brief Current write position in the ring buffer. */

    PUCHAR					m_FifoBuffer;		/*!< @brief Pointer to the ring buffer. Allocate one additional
												 * byte to account for an eccentricity in the ring buffer
												 * code which leads to one byte always being empty. */

	ULONG					m_FifoBufferSize;	/*!< @brief Size of the ring buffer in frames. */

	ULONG					m_FifoFrameSize;	/*!< @brief Size of each audio frame in the ring buffer. */

	ULONG					m_ClientFrameSize;

	BOOL						m_BitConversion;
	AUDIO_CONVERSION_ROUTINE	m_ConversionRoutine;

	BOOL					m_IsActive;			/*!< @brief Indicates the client state: TRUE for running, FALSE for stopped. */

	ULONGLONG				m_TotalBytesQueued;	/*!< @brief Total number of queued. */

	PVOID					m_CallbackData;		/*!< @brief Client's user data, if any */
    AUDIO_CALLBACK_ROUTINE	m_CallbackRoutine;	/*!< @brief Client's callback routine */

	PEXTENSION_UNIT			m_ClockRateExtension;	/*!< @brief Clock rate extension unit. */
	ULONG					m_ClockRate;			/*!< @brief Clock rate. */

	PEXTENSION_UNIT			m_DriverResyncExtension;	/*!< @brief driver resync extension unit. */

    LONG                    m_MasterVolumeStep[AUDIO_CLIENT_MAX_CHANNEL];   /*!< @brief The step size of the master volume in dB */
    LONG                    m_MasterVolumeMin[AUDIO_CLIENT_MAX_CHANNEL];    /*!< @brief The minimum master volume in dB */
    LONG                    m_MasterVolumeMax[AUDIO_CLIENT_MAX_CHANNEL];    /*!< @brief The maximum master volume in dB */
	/*************************************************************************
     * CAudioClient private methods
     *
     * These are private member functions used internally by the object.  See
     * AUDIO.CPP for specific descriptions.
     */

	PEXTENSION_UNIT _FindExtensionUnit
	(
		IN		USHORT	ExtensionCode
	);

	VOID _SetClockRate
	(
		IN		ULONG	ClockRate
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
    CAudioClient();
    /*! @brief Destructor. */
    ~CAudioClient();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }
    BOOL                    m_requireSoftMaster;                            /*!< @brief Whether this audio client requires a software master vol/mute */

    /*************************************************************************
     * CAudioClient public methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	AUDIOSTATUS Init
	(
		IN		CAudioDevice *			AudioDevice,
		IN		AUDIO_CALLBACK_ROUTINE	CallbackRoutine,
		IN		PVOID					CallbackData
	);

	AUDIOSTATUS SetInterfaceParameter
	(
		IN		UCHAR	InterfaceNumber,
		IN		UCHAR	AlternateSetting,
		IN		ULONG	Priority,
		IN		ULONG	ClockRate,
		IN		BOOL	ForceSelection = FALSE
	);

	VOID OnResourcesRipOff
	(	void
	);

	VOID OnResourcesAvailability
	(	void
	);

	VOID RequestDriverResync
	(	void
	);

	VOID Lock
	(	void
	);

	VOID Unlock
	(	void
	);

	VOID Reset
	(	void
	);

	ULONG AddFramesToFifo
	(
		IN		PUCHAR	Buffer,
		IN		ULONG	NumberOfFrames,
		IN		BOOL	BitConversion = FALSE
	);

	ULONG RemoveFramesFromFifo
	(
		IN		PUCHAR	Buffer,
		IN		ULONG	NumberOfFrames,
		IN		BOOL	BitConversion = FALSE
	);

	ULONG GetNumQueuedFrames
	(	void
	);

	ULONG GetNumAvailableFrames
	(	void
	);

	BOOL IsFull
	(	void
	);

	BOOL IsEmpty
	(	void
	);

	NTSTATUS SetupBuffer
	(
		IN		ULONG	SampleRate,
		IN		ULONG	FormatChannels,
		IN		ULONG	SampleSize,
		IN		BOOL	Capture,
		IN		ULONG	NumberOfFifoBuffers
	);

	ULONG WriteBuffer
	(
		IN		PUCHAR	Buffer,
		IN		ULONG	BufferLength
	);

	ULONG QueueBuffer
	(
		IN		PUCHAR	Buffer,
		IN		ULONG	BufferLength
	);

	ULONG ReadBuffer
	(
		IN		PUCHAR	Buffer,
		IN		ULONG	BufferLength
	);

	AUDIOSTATUS Start
	(
		IN		BOOL	SynchronizeStart,
		IN		ULONG	StartFrameNumber
	);

	AUDIOSTATUS Pause
	(	void
	);

	AUDIOSTATUS Stop
	(	void
	);

	AUDIOSTATUS GetPosition
	(
		OUT		ULONGLONG *	OutTransferPosition,
		OUT		ULONGLONG *	OutQueuePosition
	);

	AUDIOSTATUS SetPosition
	(
		IN		ULONGLONG 	TransferPosition,
		IN		ULONGLONG 	QueuePosition
	);

	AUDIOSTATUS QueryControlSupport
	(
		IN		UCHAR	ControlSelector
	);

	AUDIOSTATUS WriteControl
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize
	);

	AUDIOSTATUS ReadControl
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	BOOL IsActive
	(	void
	);

	VOID Service
	(
		IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
	);

    void
    VolumeMuteAdjustment
    (
	    IN  PUCHAR	pBuffer, 
	    IN	ULONG	SamplesPerChannel
    ); 

    NTSTATUS
    ConvertIntData2Float
    (
        IN  BYTE*   pBufferInteger,
        IN  BYTE*   pBufferFloat,
        IN  ULONG   numTotalSamples,
        IN  ULONG   bitsPerSample
    );

    NTSTATUS
    ConvertFloatData2Int
    (
        IN  BYTE*   pBufferFloat,
        IN  BYTE*   pBufferInteger,
        IN  ULONG   numTotalSamples,
        IN  ULONG   bitsPerSample
    );
    /*************************************************************************
     * Friends
     */
	friend class CList<CAudioClient>;
};

typedef CAudioClient * PAUDIO_CLIENT;

/*! @brief Maximum number of audio I/O IRPs. */
#define MAX_SYNCH_IRP			2
#define MAX_AUDIO_OUTPUT_IRP	8
#define MAX_AUDIO_INPUT_IRP		8
#define MAX_AUDIO_IRP			max(MAX_AUDIO_OUTPUT_IRP, MAX_AUDIO_INPUT_IRP)

#define MIN_AUDIO_START_FRAME_OFFSET	3

#define AUDIO_SYNCH_PIPE_STATE_STOP		0
#define AUDIO_SYNCH_PIPE_STATE_RUN		1

/*****************************************************************************
 *//*! @class CAudioSynchPipe
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Synch pipe object.
 */
class CAudioSynchPipe
{
private:
    CAudioSynchPipe *			m_Next;			/*!< @brief The next handler in the linked list. */
    CAudioSynchPipe *			m_Prev;			/*!< @brief The previous handler in the linked list. */
    PVOID						m_Owner;		/*!< @brief The link list owner. */

	DEVICE_POWER_STATE			m_PowerState;	/*!< @brief Device power state. */

	AUDIO_DIRECTION				m_Direction;    /*!< @brief Input or output. */

	ULONG						m_PipeState;
	KMUTEX						m_PipeStateLock;

	PUSB_DEVICE					m_UsbDevice;	/*!< @brief Pointer to the USB device object. */

	BOOL						m_IsDeviceHighSpeed;

	UCHAR						m_InterfaceNumber;
	UCHAR						m_AlternateSetting;
    
	USBD_PIPE_INFORMATION		m_PipeInformation;

	ULONG						m_StartFrameNumber;

	ULONG						m_RefreshRate;

	CAudioDataPipe *			m_DataPipe;

	AUDIO_FIFO_WORK_ITEM *     	m_FifoWorkItem[MAX_SYNCH_IRP];
	CList<AUDIO_FIFO_WORK_ITEM>	m_FreeFifoWorkItemList;
    CList<AUDIO_FIFO_WORK_ITEM>	m_QueuedFifoWorkItemList;
    CList<AUDIO_FIFO_WORK_ITEM>	m_PendingFifoWorkItemList;

	LONG						m_PendingIrps;
	KEVENT						m_NoPendingIrpEvent;

	/*************************************************************************
     * CAudioSynchPipe private methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	NTSTATUS PrepareFifoWorkItems
	(
		IN		ULONG   NumFifoWorkItems,
		IN		ULONG	Interval,
		IN		BOOL	Read,
		IN		BOOL	TransferAsap
	);

	NTSTATUS InitializeFifoWorkItemUrb
	(
		IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
	);

	NTSTATUS ProcessFifoWorkItem
	(
		IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
	);

	NTSTATUS StartTransfer
	(	void
	);

	VOID CancelTransfer
	(	void
	);

	AUDIOSTATUS ClearFifo
	(	void
	);

public:
    /*************************************************************************
     * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
     * defines inline IUnknown implementations that use CUnknown's aggregation
     * support.  NonDelegatingQueryInterface() is declared, but it cannot be
     * implemented generically.  Its definition appears in AUDIO.CPP.
     * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
     * only the outer unknown, which is used for aggregation.  The standard
     * create macro (in AUDIO.CPP) uses this constructor.
     */
    /*! @brief Constructor. */
    CAudioSynchPipe()  { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~CAudioSynchPipe();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CAudioSynchPipe public methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	AUDIOSTATUS Init
	(
		IN		PUSB_DEVICE				UsbDevice,
		IN		UCHAR					InterfaceNumber,
		IN		UCHAR					AlternateSetting,
		IN		USBD_PIPE_INFORMATION	PipeInformation
	);

	AUDIOSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	AUDIOSTATUS SetDataPipe
	(
		IN		CAudioDataPipe *	DataPipe
	);

	AUDIOSTATUS SetTransferParameters
	(
		IN		ULONG	SampleRate,
		IN		ULONG	FormatChannels,
		IN		ULONG	SampleSize
	);

	AUDIOSTATUS AcquireResources
	(	void
	);

	AUDIOSTATUS FreeResources
	(	void
	);

	AUDIOSTATUS Start
	(
		IN		BOOL	SynchronizeStart,
		IN		ULONG	StartFrameNumber
	);

	AUDIOSTATUS Stop
	(	void
	);

	VOID Service
	(
		IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
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
	friend class CList<CAudioSynchPipe>;
};

typedef CAudioSynchPipe * PAUDIO_SYNCH_PIPE;

/*****************************************************************************
 * AUDIO_DATA_PIPE_PARAMETER_BLOCK
 */
typedef struct
{
	struct 
	{
		BOOL	Support;
		ULONG	Current;
	}						SamplingFrequency;
	struct 
	{
		BOOL	Support;
		BOOL	Current;
	}						PitchControl;
} AUDIO_DATA_PIPE_PARAMETER_BLOCK, *PAUDIO_DATA_PIPE_PARAMETER_BLOCK;

#define AUDIO_DATA_PIPE_STATE_STOP		0
#define AUDIO_DATA_PIPE_STATE_PAUSE		1
#define AUDIO_DATA_PIPE_STATE_RUN		2

/*****************************************************************************
 *//*! @class CAudioDataPipe
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Audio pipe object.
 */
class CAudioDataPipe
{
private:
    CAudioDataPipe *			m_Next;			/*!< @brief The next pipe in the linked list. */
    CAudioDataPipe *			m_Prev;			/*!< @brief The previous pipe in the linked list. */
    PVOID						m_Owner;		/*!< @brief The link list owner. */

	DEVICE_POWER_STATE			m_PowerState;	/*!< @brief Device power state. */
	AUDIO_DIRECTION				m_Direction;    /*!< @brief Input or output. */

	ULONG						m_PipeState;
	KMUTEX						m_PipeStateLock;

	PUSB_DEVICE					m_UsbDevice;	/*!< @brief Pointer to the USB device object. */

	BOOL						m_IsDeviceHighSpeed;

	UCHAR						m_InterfaceNumber;
	UCHAR						m_AlternateSetting;
    
	USBD_PIPE_INFORMATION		m_PipeInformation;

	UCHAR						m_SynchronizationType;

	UCHAR						m_Attributes; // CS AS attributes

	ULONG						m_NumberOfPacketsToSkip;

	BOOL						m_UseEmbeddedPacketLength;

	ULONG						m_StartFrameNumber;

	BOOL						m_SynchronizeStart;

	ULONG						m_SynchronizationDelay;

	BOOL						m_ResyncRequested;

	ULONG						m_AutoResyncCount;

	ULONG						m_NumberOfPacketsPerMs;

	ULONGLONG					m_TotalBytesTransfered;

	CAudioClient *				m_Client;

	AUDIO_FIFO_WORK_ITEM *     	m_FifoWorkItem[MAX_AUDIO_IRP];
    CList<AUDIO_FIFO_WORK_ITEM>	m_FreeFifoWorkItemList;
    CList<AUDIO_FIFO_WORK_ITEM>	m_QueuedFifoWorkItemList;
    CList<AUDIO_FIFO_WORK_ITEM>	m_PendingFifoWorkItemList;

	LONG						m_PendingIrps;
	KEVENT						m_NoPendingIrpEvent;

	ULONG						m_SampleRate;
	ULONG						m_SampleFrameSize;
	struct 
	{
		ULONG	Fraction;
		ULONG	Whole;  
	}							m_FfPerPacketInterval;

	ULONG						m_RunningFfFraction;

	ULONG						m_PacketDeficitInBytes;

	// These are to workaround Microsoft's USB OHCI bug.
	PVOID						m_LastRecordBuffer;
	ULONG						m_LastRecordBufferSize;
	ULONG						m_LastRecordFrameNumber;
	////

	AUDIO_DATA_PIPE_PARAMETER_BLOCK	m_ParameterBlock;

	BOOL _FindControl
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutControlSelector
	);

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR								ControlSelector,
		IN		BOOL								Support,
		IN		PAUDIO_DATA_PIPE_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL								Read
	);

	NTSTATUS PrepareFifoWorkItems
	(
		IN		ULONG   NumFifoWorkItems,
		IN		BOOL	Read,
		IN		BOOL	TransferAsap
	);

	NTSTATUS InitializeFifoWorkItemUrb
	(
		IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
	);

	NTSTATUS PrepareFullSpeedFifoWorkItems
	(
		IN		ULONG   NumFifoWorkItems,
		IN		BOOL	Read,
		IN		BOOL	TransferAsap
	);

	NTSTATUS PrepareHighSpeedFifoWorkItems
	(
		IN		ULONG   NumFifoWorkItems,
		IN		BOOL	Read,
		IN		BOOL	TransferAsap
	);

	NTSTATUS ProcessFifoWorkItem
	(
		IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
	);

	NTSTATUS StartTransfer
	(	void
	);

	VOID CancelTransfer
	(	void
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CAudioDataPipe() { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~CAudioDataPipe();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CAudioDataPipe public methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	AUDIOSTATUS Init
	(
		IN		PUSB_DEVICE				UsbDevice,
		IN		UCHAR					InterfaceNumber,
		IN		UCHAR					AlternateSetting,
		IN		USBD_PIPE_INFORMATION	PipeInformation
	);

	UCHAR InterfaceNumber
	(	void
	);

	UCHAR AlternateSetting
	(	void
	);

	PUSBD_PIPE_INFORMATION PipeInformation
	(	void
	);

	UCHAR SynchronizationType
	(	void
	);

	AUDIOSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	AUDIOSTATUS SetTransferParameters
	(
		IN		ULONG	SampleRate,
		IN		ULONG	FormatChannels,
		IN		ULONG	SampleSize,
		IN		ULONG	NumberOfFifoBuffers
	);

	ULONG GetTransferSizeInFrames
	(
		IN		ULONG	NumberOfTransfers,
		IN		BOOL	UpdateRunningFfFraction
	);

	VOID OnSampleRateSynchronization
	(
		IN		ULONG	FfWhole,
		IN		ULONG	FfFraction
	);

	AUDIOSTATUS SetCallbackClient
	(
		IN		CAudioClient *	Client
	);

	AUDIOSTATUS AcquireResources
	(
		IN		ULONG	NumberOfIrps
	);

	AUDIOSTATUS FreeResources
	(	void
	);

	AUDIOSTATUS FlushBuffer
	(	void
	);

	AUDIOSTATUS Start
	(
		IN		BOOL	SynchronizeStart,
		IN		ULONG	StartFrameNumber
	);

	AUDIOSTATUS Pause
	(	void
	);

	AUDIOSTATUS Stop
	(	void
	);

	AUDIOSTATUS GetPosition
	(
		OUT		ULONGLONG *	OutTransferPosition
	);

	AUDIOSTATUS SetPosition
	(
		IN		ULONGLONG 	TransferPosition
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

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	VOID Service
	(
		IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
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
	friend class CList<CAudioDataPipe>;
};

typedef CAudioDataPipe * PAUDIO_DATA_PIPE;

/*****************************************************************************
 *//*! @class CAudioInterface
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Audio pin object.
 */
class CAudioInterface
{
private:
    CAudioInterface *		m_Next;			/*!< @brief The next interface in the linked list. */
    CAudioInterface *		m_Prev;			/*!< @brief The previous interface in the linked list. */
    PVOID					m_Owner;		/*!< @brief The link list owner. */

	DEVICE_POWER_STATE		m_PowerState;	/*!< @brief Current device power state. */

	PUSB_DEVICE				m_UsbDevice;	/*!< @brief Pointer to the USB device object. */

	UCHAR					m_InterfaceNumber;
	UCHAR					m_AlternateSetting;

    CList<CAudioDataPipe>	m_DataPipeList;		/*!< @brief List of data pipes. */
    CList<CAudioSynchPipe>	m_SynchPipeList;	/*!< @brief List of synch pipes. */

	KMUTEX					m_InterfaceAcquireLock;	/*!< @brief Interface acquire lock. */
	PVOID					m_InterfaceTag;			/*!< @brief Interface tag. */
	ULONG					m_InterfaceTagPriority;	/*!< @brief Interface tag priority. */

	PVOID					m_InterfaceAvailabilityCallbackTag;	/*!< @brief Interface availability callback tag. */

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CAudioInterface() { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~CAudioInterface();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CAudioInterface public methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	AUDIOSTATUS Init
	(
		IN		PUSB_DEVICE	UsbDevice,
		IN		UCHAR		InterfaceNumber
	);

	AUDIOSTATUS	AcquireInterface
	(
		IN		PVOID	Tag,
		IN		ULONG	TagPriority
	);

	AUDIOSTATUS	ReleaseInterface
	(
		IN		PVOID	Tag
	);

	AUDIOSTATUS SetInterfaceAvailabilityCallback
	(
		IN		PVOID	Tag,
		IN		BOOL	Enable
	);

	AUDIOSTATUS	SelectAlternateSetting
	(
		IN		UCHAR	AlternateSetting
	);

	UCHAR InterfaceNumber
	(	void
	);

	UCHAR TerminalLink
	(
		IN		UCHAR	AlternateSetting
	);
	
	BOOL HasBadDescriptors
	(	void
	);

	BOOL ParseSupportedFormat
	(
		IN		UCHAR		AlternateSetting,
		OUT		USHORT *	FormatTag
	);
	
	PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR GetFormatTypeDescriptor
	(
		IN		UCHAR		AlternateSetting
	);

	PUSB_AUDIO_COMMON_FORMAT_SPECIFIC_DESCRIPTOR GetFormatSpecificDescriptor
	(
		IN		UCHAR		AlternateSetting
	);

	PUSB_ENDPOINT_DESCRIPTOR GetDataEndpointDescriptor
	(
		IN		UCHAR		AlternateSetting
	);

	AUDIOSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	CAudioDataPipe * FindDataPipe
	(	void
	);

	CAudioSynchPipe * FindSynchPipe
	(	void
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
	friend class CList<CAudioInterface>;
};

typedef CAudioInterface * PAUDIO_INTERFACE;

/*****************************************************************************
 *//*! @class CAudioTopology
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Audio topology object.
 */
class CAudioTopology
{
private:
    CAudioTopology *			m_Next;			/*!< @brief The next topology in the linked list. */
    CAudioTopology *			m_Prev;			/*!< @brief The previous topology in the linked list. */
    PVOID						m_Owner;		/*!< @brief The link list owner. */

	DEVICE_POWER_STATE			m_PowerState;	/*!< @brief Current device power state. */

	PUSB_DEVICE					m_UsbDevice;	/*!< @brief Pointer to the USB device object. */

	UCHAR						m_InterfaceNumber;	/*! @brief AudioControl interface number */

    CList<CEntity>				m_EntityList;	/*!< @brief List of topology entities. */

	/*************************************************************************
     * CAudioTopology private methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	AUDIOSTATUS ParseCsAcInterfaceDescriptor
	(
		IN		UCHAR	InterfaceNumber
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
    CAudioTopology()  { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
    ~CAudioTopology();
	/*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CAudioTopology public methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	AUDIOSTATUS Init
	(
		IN		PUSB_DEVICE	UsbDevice
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

	AUDIOSTATUS SaveParameterBlocks
	(
		IN		PVOID	ParameterBlocks,
		IN		ULONG	SizeOfParameterBlocks,
		OUT		ULONG *	OutSizeOfParameterBlocks
	);

	AUDIOSTATUS RestoreParameterBlocks
	(
		IN		PVOID	ParameterBlocks,
		IN		ULONG	SizeOfParameterBlocks
	);

	ULONG GetSizeOfParameterBlocks
	(	void
	);

	AUDIOSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

    /*************************************************************************
     * Friends
     */
	friend class CList<CAudioTopology>;
};

typedef CAudioTopology * PAUDIO_TOPOLOGY;

/*****************************************************************************
 *//*! @class CAudioDevice
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Audio device object.
 */
class CAudioDevice
:	public CUnknown
{
private:
    ULONG					m_MagicNumber;		/*!< @brief Magic number. */
	DEVICE_POWER_STATE		m_PowerState;		/*!< @brief Current device power state. */

//	PUSB_DEVICE				m_UsbDevice;		/*!< @brief Pointer to the USB device object. */

	CAudioInterruptPipe *	m_InterruptPipe;	/*!< @brief Pointer to the audio interrupt pipe object. */
	CAudioTopology *		m_Topology;			/*!< @brief Pointer to the audio topology object. */
    CList<CAudioInterface>	m_InterfaceList;	/*!< @brief List of audio interfaces. */

	CList<CAudioClient>		m_ClientList;

    AUDIO_INTERRUPT_HANDLER_ROUTINE	m_InterruptHandlerRoutine;	/*!< @brief Interrupt handler routine */
	PVOID							m_InterruptHandlerContext;	/*!< @brief Interrupt user context, if any */

    LONG                    m_MasterVolumeStep[AUDIO_CLIENT_MAX_CHANNEL];   /*!< @brief The step size of the master volume in dB */
    LONG                    m_MasterVolumeMin[AUDIO_CLIENT_MAX_CHANNEL];    /*!< @brief The minimum master volume in dB */
    LONG                    m_MasterVolumeMax[AUDIO_CLIENT_MAX_CHANNEL];    /*!< @brief The maximum master volume in dB */
    LONG                    m_MasterVolume[AUDIO_CLIENT_MAX_CHANNEL];       /*!< @brief The running master volume in dB */
    BOOL                    m_MasterMute;                                   /*!< @brief The running master mute */
	LONG					m_NumOfClientChannel; /*!< @brief The actual number of chnnels for software master volume control */

	/*************************************************************************
     * CAudioDevice private methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	CAudioInterface * FindInterface
	(
		IN		UCHAR			InterfaceNumber
	);

public:
    /*************************************************************************
     * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
     * defines inline IUnknown implementations that use CUnknown's aggregation
     * support.  NonDelegatingQueryInterface() is declared, but it cannot be
     * implemented generically.  Its definition appears in USBDEV.CPP.
     * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
     * only the outer unknown, which is used for aggregation.  The standard
     * create macro (in AUDIO.CPP) uses this constructor.
     */
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CAudioDevice);

    ~CAudioDevice();
    BOOL                    m_requireSoftMaster;                            /*!< @brief Whether this audio client requires a software master vol/mute */
    ULONG                   m_NoOfSoftNode;                                 /*!< @brief No of software node this audio client requires a software master vol/mute */
	//edit yuanfen
	PUSB_DEVICE				m_UsbDevice;		                            /*!< @brief Pointer to the USB device object. */

    /*************************************************************************
     * CAudioDevice public methods
     *
     * These are public member functions.  See AUDIO.CPP for specific
	 * descriptions.
     */
	AUDIOSTATUS Init
	(
		IN		PUSB_DEVICE	UsbDevice
	);

	BOOL ParseInterfaces
	(
		IN		ULONG				Index,
		OUT		PAUDIO_INTERFACE *	OutInterface
	);

	BOOL ParseTopology
	(
		IN		ULONG				Index,
		OUT		PAUDIO_TOPOLOGY	*	OutTopology
	);

	AUDIOSTATUS GetDeviceDescriptor
	(
		OUT		PUSB_DEVICE_DESCRIPTOR *	OutDeviceDescriptor
	);

	AUDIOSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	AUDIOSTATUS SetInterruptHandler
	(
		IN		AUDIO_INTERRUPT_HANDLER_ROUTINE	InterruptHandlerRoutine,
		IN		PVOID							InterruptHandlerContext
	);

	VOID OnStatusInterrupt
	(
		IN		ULONG					NumberOfStatusWord,
		IN		PUSB_AUDIO_STATUS_WORD	StatusWord,
		IN		PVOID					InterruptFilter
	);

	AUDIOSTATUS SaveParameterBlocks
	(
		IN		PVOID	ParameterBlocks,
		IN		ULONG	SizeOfParameterBlocks,
		OUT		ULONG *	OutSizeOfParameterBlocks
	);

	AUDIOSTATUS RestoreParameterBlocks
	(
		IN		PVOID	ParameterBlocks,
		IN		ULONG	SizeOfParameterBlocks
	);

	ULONG GetSizeOfParameterBlocks
	(	void
	);

	AUDIOSTATUS AttachClient
	(
		IN		CAudioClient *	Client
	);

	AUDIOSTATUS DetachClient
	(
		IN		CAudioClient *	Client
	);

	AUDIOSTATUS Open
	(
		IN		AUDIO_CALLBACK_ROUTINE	CallbackRoutine,
		IN		PVOID					CallbackData,
		OUT		PAUDIO_CLIENT *			OutClient
	);

	AUDIOSTATUS Close
	(
		IN		PAUDIO_CLIENT	Client
	);

    NTSTATUS
    GetMasterVolumeRange(
        IN  LONG    channel,
        OUT LONG*   pVolumeMinDB,
        OUT LONG*   pVolumeMaxDB,
        OUT LONG*   pVolumeStepDB
    );

    NTSTATUS
    GetMasterVolume(
        IN  LONG    channel,
        OUT LONG*   pVolumeDB
    );

    NTSTATUS
    SetMasterVolume(
        IN  LONG    channel,
        IN  LONG    volumeDB
    );

    NTSTATUS
    GetMasterMute(
        OUT BOOL*   pMute
    );

    NTSTATUS
    SetMasterMute(
        IN  BOOL    mute
    );

    BOOL
    IsSoftwareMasterVolMute(void);

    void
    GetNoOfSupportedChannel(OUT ULONG* pChNumber);
    /*************************************************************************
     * Friends
     */
	friend CAudioClient;
};

typedef CAudioDevice * PAUDIO_DEVICE;

#endif /* __AUDIO_H__ */
