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
 * @file       Filter.h
 * @brief      Audio filter definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _AUDIO_FILTER_PRIVATE_H_
#define _AUDIO_FILTER_PRIVATE_H_

#include "IKsAdapter.h"

#include "Descriptor.h"
#include "Formats.h"
#include "ExtProp.h"
#include "PrvProp.h"

using namespace AUDIO_TOPOLOGY;

/*****************************************************************************
 * Defines
 */
/*****************************************************************************
 * Classes
 */
class CAudioFilterFactory;

/*****************************************************************************
 *//*! @class CAudioFilter
 *****************************************************************************
 * @brief
 * Audio filter.
 * @details
 * This object is associated with the device and is created when the device
 * is started.  The class inherits CUnknown so it automatically gets reference 
 * counting and aggregation support.
 */
class CAudioFilter
:	public CUnknown
{
private:
    PKSFILTER				m_KsFilter;					/*!< @brief The AVStream filter we're associated with. */
    PKSADAPTER				m_KsAdapter;				/*!< @brief Pointer to the IKsAdapter interface. */
	PKSFILTER_DESCRIPTOR	m_KsFilterDescriptor;		/*!< @brief KS filter descriptor. */

	PUSB_DEVICE				m_UsbDevice;				/*!< @brief Pointer to the USB device object. */
	PAUDIO_DEVICE			m_AudioDevice;				/*!< @brief Pointer to the audio device object. */

	LONG                    m_ActiveSpeakerPositions;   /*!< @brief Active speaker positions. */
    LONG                    m_StereoSpeakerGeometry;    /*!< @brief Stereo speaker geometry. */

	CAudioFilterFactory * 	m_FilterFactory;

	PVOID					m_EventHandle;

	BOOL					m_UsePreferredSampleRate;

	PNODE_DESCRIPTOR		m_ClockRateExtension;

	struct 
	{
		ULONG				Input;
		ULONG				Output;
	}						m_NumberOfFifoBuffers;

	BOOL					m_SynchronizeStart;
	
	ULONG					m_StartFrameNumber;

	PNODE_DESCRIPTOR _FindClockRateExtension
	(	void
	);

	ULONG _FindPreferredFormatChannels
	(
		IN      ULONG           PinId,
		IN      PKSDATARANGE	MatchingDataRange,
		IN		ULONG			SampleFrequency
	);

	ULONG _FindPreferredFormatBitResolution
	(
		IN      ULONG           PinId,
		IN      PKSDATARANGE	MatchingDataRange,
		IN		ULONG			SampleFrequency,
		IN		ULONG			MaximumChannels
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
	DEFINE_STD_CONSTRUCTOR(CAudioFilter);
    ~CAudioFilter();

    /*************************************************************************
     * CAudioFilter methods
     */
	NTSTATUS Init
	(
		IN		PKSFILTER	KsFilter
	);

	NTSTATUS GetDescription
	(
		OUT		PKSFILTER_DESCRIPTOR *	OutKsFilterDescriptor
	);

	NTSTATUS DataRangeIntersection
	(
		IN      ULONG           PinId,
		IN      PKSDATARANGE    DataRange,
		IN      PKSDATARANGE    MatchingDataRange,
		IN      ULONG           OutputBufferLength,
		OUT     PVOID           ResultantFormat,
		OUT     PULONG          ResultantFormatLength
	);

    NTSTATUS ValidateFormat
    (   
		IN      ULONG           PinId,
        IN      BOOLEAN         Capture,
        IN      PKSDATAFORMAT   Format
    );

	NTSTATUS GetLatency
	(
		IN		ULONG	PinId,
		IN		ULONG	SampleRate,
	    OUT     PKSTIME	OutLatency
	);

	ULONG GetPreferredSampleRate
	(	void
	);

	VOID TriggerEvent
	(
		IN		ULONG	EventType
	);

	PFILTER_PIN_DESCRIPTOR FindPin
	(
		IN		ULONG	PinId
	);

	PNODE_DESCRIPTOR FindNode
	(
		IN		ULONG	NodeId
	);

	VOID SimulateStatusInterrupt
	(
		IN		UCHAR	StatusType,
		IN		UCHAR	Originator
	);

	/*************************************************************************
     * Static
     */
	static
	KSFILTER_DISPATCH DispatchTable;

    static
    NTSTATUS DispatchCreate 
	(
        IN		PKSFILTER	KsFilter,
		IN		PIRP		Irp
    );

	static
    VOID Destruct 
	(
		IN		PVOID	Self
	);

	static const
	KSPROPERTY_ITEM PinPropertyTable[];

	static const
	KSPROPERTY_ITEM AudioPropertyTable[];

	static const
	KSPROPERTY_ITEM ControlPropertyTable[];

	static const
	KSPROPERTY_SET PropertySetTable[];

	static const
	KSAUTOMATION_TABLE AutomationTable;

	static
	GUID Categories[4];

	static
	NTSTATUS SupportPinName
	(
		IN		PIRP		Irp,
		IN		PKSP_PIN	Request,
		IN OUT	PVOID		Value
	);

	static
	NTSTATUS GetPinName
	(
		IN		PIRP		Irp,
		IN		PKSP_PIN	Request,
		IN OUT	PVOID		Value
	);

	static
	NTSTATUS GetGlobalPinCInstances
	(
		IN		PIRP		Irp,
		IN		PKSP_PIN	Request,
		IN OUT	PVOID		Value
	);

	static
	NTSTATUS GetCopyProtectControl
	(
		IN		PIRP		Irp,
		IN		PKSP_PIN	Request,
		IN OUT	PVOID		Value
	);

	static
	NTSTATUS SetCopyProtectControl
	(
		IN		PIRP		Irp,
		IN		PKSP_PIN	Request,
		IN OUT	PVOID		Value
	);

	static
	NTSTATUS GetLatencyControl
	(
		IN		PIRP		Irp,
		IN		PKSP_PIN	Request,
		IN OUT	PVOID		Value
	);

	static
	NTSTATUS GetDeviceControl
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetDeviceControl
	(
		IN		PIRP			Irp,
		IN		PKSPROPERTY		Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS AddControlEvent
	(
		IN		PIRP			Irp,
		IN		PKSEVENTDATA	EventData,
		IN		PKSEVENT_ENTRY	EventEntry
	);

	static
	VOID RemoveControlEvent
	(
		IN		PFILE_OBJECT	FileObject,
		IN		PKSEVENT_ENTRY	EventEntry
	);

	static
	VOID EventCallbackRoutine
	(
		IN		PVOID	Context,
		IN		GUID *	Set,
		IN		ULONG	EventId,
		IN		BOOL	PinEvent,
		IN		ULONG	PinId,
		IN		BOOL	NodeEvent,
		IN		ULONG	NodeId
	);

	static
	NTSTATUS SupportLevelControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS GetLevelControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SetLevelControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SupportOnOffControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS GetOnOffControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SetOnOffControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SupportEqControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS GetEqControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SetEqControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SupportMixControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetMixControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetMixControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SupportMuxControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetMuxControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetMuxControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetDelayControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SetDelayControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SupportEnableProcessingControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetEnableProcessingControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetEnableProcessingControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SupportModeSelectControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetModeSelectControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetModeSelectControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SupportUnsignedProcessingControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetUnsignedProcessingControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetUnsignedProcessingControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SupportSignedProcessingControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetSignedProcessingControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetSignedProcessingControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SupportEnableExtensionControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetEnableExtensionControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetEnableExtensionControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetExtensionControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SetExtensionControl
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SupportCpuResources
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS GetCpuResources
	(
		IN		PIRP			Irp,
		IN		PKSNODEPROPERTY	Request,
		IN OUT	PVOID			Value
	);

	static
	NTSTATUS SwSupportLevelControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SwGetLevelControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SwSetLevelControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);
	static
	NTSTATUS SwSupportOnOffControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SwGetOnOffControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);

	static
	NTSTATUS SwSetOnOffControl
	(
		IN		PIRP							Irp,
		IN		PKSNODEPROPERTY_AUDIO_CHANNEL	Request,
		IN OUT	PVOID							Value
	);
	/*************************************************************************
     * Friends
     */
    friend class CAudioPin;
};

#endif // _AUDIO_FILTER_PRIVATE_H_
