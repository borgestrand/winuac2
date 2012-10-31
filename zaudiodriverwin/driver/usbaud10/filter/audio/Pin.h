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
*//*
 *****************************************************************************
 *//*!
 * @file       Pin.h
 * @brief      AUdio pin definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _AUDIO_PIN_PRIVATE_H_
#define _AUDIO_PIN_PRIVATE_H_

#include "Filter.h"

using namespace AUDIO_TOPOLOGY;

/*****************************************************************************
 * Defines
 */
/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CAudioPin
 *****************************************************************************
 * @brief
 * Audio pin.
 * @details
 * This object is associated with a streaming pin and is created when a pin
 * is created on the filter.  The class inherits CUnknown so it automatically 
 * gets reference counting and aggregation support.
 */
class CAudioPin
:	public CUnknown
{
private:
	PKSPIN						m_KsPin;

    CAudioFilter *				m_AudioFilter;			/*!< @brief Filter that created us. */
    ULONG						m_PinId;				/*!< @brief Pin identifier. */
    KSSTATE						m_State;				/*!< @brief State (RUN/PAUSE/ACQUIRE/STOP). */
    BOOLEAN						m_Capture;				/*!< @brief TRUE for capture, FALSE for render. */
    DEVICE_POWER_STATE			m_DevicePowerState;		/*!< @brief Device power state. */

	PAUDIO_DEVICE				m_AudioDevice;			/*!< @brief Pointer to the audio device object. */
	PAUDIO_CLIENT				m_AudioClient;			/*!< @brief Pointer to the audio client instance. */

	UCHAR						m_InterfaceNumber;		/*!< @brief USB interface number. */
	UCHAR						m_AlternateSetting;		/*!< @brief USB interface alternate setting. */

    ULONG                       m_FormatChannels;       /*!< @brief Indicate number of channels. */
    ULONG                       m_SamplingFrequency;    /*!< @brief Frames per second. */
    ULONG                       m_SampleSize;           /*!< @brief Sample size. */
    BOOL                        m_NonPcmFormat;         /*!< @brief Non PCM format. */

    ULONG                       m_NotificationInterval; /*!< @brief Notification interval in milliseconds. */

	PIKSREFERENCECLOCK			m_ReferenceClock;

	PKSSTREAM_POINTER			m_PreviousClonePointer;

	KSPIN_LOCK					m_ProcessingLock;

	ULONG						m_LoopedBufferSize;

	BOOL						m_PendingIo;

	DRMRIGHTS					m_DrmRights;

    /*************************************************************************
     * CAudioPin methods
     *
     * These are private member functions used internally by the object.
     */
	NTSTATUS _FindAudioBitResolution
	(
		IN      ULONG           PinId,
		IN      BOOLEAN         Capture,
		IN      PKSDATAFORMAT   Format,
		OUT		ULONG *			OutBitResolution
	);
	NTSTATUS _FindAudioInterface
	(
		IN      ULONG           PinId,
		IN      BOOLEAN         Capture,
		IN      PKSDATAFORMAT   Format,
		IN		ULONG 			BitResolution
	);
	NTSTATUS _AcquireAudioInterface
	(
		IN		UCHAR	InterfaceNumber,
		IN		UCHAR	AlternateSetting,
		IN		ULONG	SampleRate,
		IN		ULONG	FormatChannels,
		IN		ULONG	SampleSize,
		IN		ULONG	Priority,
		IN		BOOL	ChangeClockRate
	);
	NTSTATUS _ReleaseAudioInterface
	(
		IN		UCHAR	InterfaceNumber
	);
    NTSTATUS _AllocateResources
    (   void
    );
    NTSTATUS _FreeResources
    (   void
    );
	NTSTATUS _FreeClonePointers
	(   void
	);
    NTSTATUS _Run
    (   void
    );
    NTSTATUS _Pause
    (   void
    );
    NTSTATUS _Stop
    (   void
    );
	NTSTATUS _GetPosition
	(
		OUT		ULONGLONG *	OutTransferPosition,
		OUT		ULONGLONG *	OutQueuePosition
	); 
	NTSTATUS _SetPosition
	(
		IN		ULONGLONG 	TransferPosition,
		IN		ULONGLONG 	QueuePosition
	);
	BOOL _IsPinsConnected
	(
		IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
		IN		ULONG					FromNode,
		IN		ULONG					FromNodePin,
		IN		ULONG					ToNode,
		IN		ULONG					ToNodePin
	);
	NTSTATUS _EnforceDrmRights
	(
		IN		PKSFILTER	KsFilter,
		IN		DRMRIGHTS	OldDrmRights,
		IN		DRMRIGHTS	NewDrmRights
	);
	BOOL _EnforceDrmCopyProtect
	(
		IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
		IN		ULONG					FromNode,
		IN		ULONG					FromNodePin,
		IN		ULONG					ToNode,
		IN		ULONG					ToNodePin,
		IN		BOOL					CopyProtect,
		IN		BOOL					FoundSuperMixNode,
		OUT		NTSTATUS *				OutStatus
	);
	BOOL _EnforceDrmDigitalOutputDisable
	(
		IN		PKSFILTER_DESCRIPTOR	KsFilterDescriptor,
		IN		ULONG					FromNode,
		IN		ULONG					FromNodePin,
		IN		ULONG					ToNode,
		IN		ULONG					ToNodePin,
		IN		BOOL					DigitalOutputDisable,
		IN		BOOL					FoundMuteNode,
		OUT		NTSTATUS *				OutStatus
	);

public:
    /*************************************************************************
     * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
     * defines inline IUnknown implementations that use CUnknown's aggregation
     * support.  NonDelegatingQueryInterface() is declared, but it cannot be
     * implemented generically.  Its definition appears in FILTER.CPP.
     * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
     * only the outer unknown, which is used for aggregation.  The standard
     * create macro (in FILTER.CPP) uses this constructor.
     */
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CAudioPin);
    ~CAudioPin();

    /*************************************************************************
     * CAudioPin methods
     */
    NTSTATUS Init
    (
		IN		PKSPIN	KsPin
    );

	NTSTATUS SetFormat
	(
		IN      PKSDATAFORMAT   Format
	);

	VOID PowerChangeNotify
	(
		IN      DEVICE_POWER_STATE	NewPowerState
	);

	NTSTATUS GetAudioPosition
	(
		OUT     PKSAUDIO_POSITION	OutPosition
	);

	NTSTATUS SetAudioPosition
	(
		OUT     KSAUDIO_POSITION	Position
	);

	NTSTATUS GetLatency
	(
	    OUT     PKSTIME	OutLatency
	);

	NTSTATUS SetDrmContentId
	(
		IN		ULONG		ContentId,
		IN		DRMRIGHTS	DrmRights
	);

	NTSTATUS SetState
	(
		IN      KSSTATE     NewState
	);

	NTSTATUS Process
	(	void
	);

	void IoCompletion 
	(
		IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
	);

	NTSTATUS QueryControlSupport
	(
		IN		UCHAR	ControlSelector
	);

	NTSTATUS WriteControl
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize
	);

	NTSTATUS ReadControl
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	/*************************************************************************
     * Static methods
     */
	static
	KSPIN_DISPATCH DispatchTable; 

    static
    NTSTATUS DispatchCreate 
	(
		IN		PKSPIN		KsPin,
		IN		PIRP		Irp
    );

    static
    NTSTATUS DispatchSetFormat 
	(
		IN		PKSPIN						KsPin,
		IN		PKSDATAFORMAT				OldFormat			OPTIONAL,
		IN		PKSMULTIPLE_ITEM			OldAttributeList	OPTIONAL,
		IN		const KSDATARANGE *			DataRange,
		IN		const KSATTRIBUTE_LIST *	AttributeRange		OPTIONAL
    );

    static
    NTSTATUS DispatchSetState 
	(
		IN		PKSPIN		KsPin,
		IN		KSSTATE		ToState,
		IN		KSSTATE		FromState
    );

    static 
    NTSTATUS DispatchProcess 
	(
        IN		PKSPIN	KsPin
    );

	static
    NTSTATUS IntersectHandler 
	(
	    IN		PKSFILTER		KsFilter,
		IN		PIRP			Irp,
		IN		PKSP_PIN		PinInstance,
		IN		PKSDATARANGE	CallerDataRange,
		IN		PKSDATARANGE	DescriptorDataRange,
		IN		ULONG			BufferSize,
		OUT		PVOID			Data OPTIONAL,
		OUT		PULONG			DataSize
    );

	static
	VOID SleepCallback 
	(
		IN		PKSPIN				KsPin,
		IN		DEVICE_POWER_STATE	PowerState
	);

	static
	VOID WakeCallback 
	(
		IN		PKSPIN				KsPin,
		IN		DEVICE_POWER_STATE	PowerState
	);

	static const
	KSALLOCATOR_FRAMING_EX AllocatorFraming;

	static 
	KSPIN_INTERFACE Interfaces[2];

	static
    VOID Destruct 
	(
		IN		PVOID	Self
	);

	static
    void NotificationRoutine
    (
		IN      PVOID					Context,
		IN		ULONG					Reason,
		IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
    );

	static const
	KSPROPERTY_ITEM AudioPropertyTable[];

	static const
	KSPROPERTY_ITEM DrmPropertyTable[];

	static const
	KSPROPERTY_SET PropertySetTable[];

	static const
	KSAUTOMATION_TABLE AutomationTable;

	static
	NTSTATUS GetLatencyControl
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetLatencyControl
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetAudioPositionControl
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetAudioPositionControl
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SupportSrcControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetSrcControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetSrcControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetDrmControl
	(
		IN		PIRP							Irp,
		IN		PKSP_DRMAUDIOSTREAM_CONTENTID	Request,
		IN OUT	PVOID							Value
	);
};

#endif // _AUDIO_PIN_PRIVATE_H_
