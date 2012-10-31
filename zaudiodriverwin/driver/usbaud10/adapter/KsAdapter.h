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
 * @file       KsAdapter.h
 * @brief      Device adapter level definitions of the hardware. .
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _KS_ADAPTER_H_
#define _KS_ADAPTER_H_

#include "IKsAdapter.h"
#include "UsbDev.h"
#include "usbaudio.h"

/*****************************************************************************
 * Defines
 */
//@{
/*! @brief PnP states. */
#define PNP_STATE_INITIAL			0x0000		// initial state
#define PNP_STATE_STARTED			0x0001		// received START
#define PNP_STATE_STOPPED			0x0002		// received STOP
#define PNP_STATE_REMOVED			0x0003		// received REMOVE
#define PNP_STATE_SURPRISE_REMOVAL	0x0004		// received SURPRISE_REMOVAL
#define PNP_STATE_PENDING			0x8000		// received QUERY_STOP/QUERY_REMOVE, waiting for STOP/REMOVE or CANCEL
//@}

#define DEVICE_READY_TO_START(state)	((state == PNP_STATE_INITIAL) || (state == PNP_STATE_STOPPED))
#define DEVICE_READY_FOR_SUPRISE_REMOVAL(state) ((state == PNP_STATE_INITIAL) || (state == PNP_STATE_STARTED) || (state == PNP_STATE_STOPPED) || (state & PNP_STATE_PENDING))
#define DEVICE_READY_FOR_REMOVAL(state) ((state == PNP_STATE_INITIAL) || (state == PNP_STATE_STARTED) || (state == PNP_STATE_STOPPED) || (state == PNP_STATE_SURPRISE_REMOVAL) || (state & PNP_STATE_PENDING))
#define DEVICE_READY_FOR_IO(state)	((state == PNP_STATE_STARTED) || (state == PNP_STATE_STOPPED))

//@{
/*! @brief Device status. */
#define DEVICE_STATUS_READY             0x0
#define DEVICE_STATUS_NOT_CONNECTED     0x1
#define DEVICE_STATUS_FIRMWARE_UPGRADE  0x2
//@}

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CKsAdapter
 *****************************************************************************
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * KS adapter object.
 */
class CKsAdapter 
:	public IKsAdapter,
    public CUnknown
{
private:
    PKSDEVICE			m_KsDevice;			/*!< @brief The AVStream device we're associated with. */
	PUSB_DEVICE			m_UsbDevice;		/*!< @brief Pointer to the USB device object. */

	PAUDIO_DEVICE		m_AudioDevice;		/*!< @brief Pointer to the audio device object. */
	PMIDI_DEVICE		m_MidiDevice;		/*!< @brief Pointer to the MIDI device object. */

	WCHAR				m_RefString[64][16];/*!< @brief Device reference string. Reason this is here is
											     because it need an unique pointer to the string, 
												 otherwise the constructed filter factory wouldn't be 
												 recognized. */

	USHORT				m_LanguageId;		/*!< @brief Current Language Identifier. */

    ULONG               m_PnpState;			/*!< @brief PnP state. */
	ULONG				m_DeviceStatus;		/*!< @brief Device state. */
	LONG				m_DeviceUsageCount;	/*!< @brief Device usage count. */

	BOOL				m_ShutdownNotification;	/*!< @brief Whether shutdown notification is registered. */

	NTSTATUS StartDevice
	(
        IN		PCM_RESOURCE_LIST	TranslatedResourceList,
        IN		PCM_RESOURCE_LIST	UntranslatedResourceList
	);

	NTSTATUS QueryStopDevice
	(	void
	);

	VOID CancelStopDevice
	(	void
	);

	VOID StopDevice
	(	void
	);

	NTSTATUS QueryRemoveDevice
	(	void
	);

	VOID CancelRemoveDevice
	(	void
	);

	VOID RemoveDevice
	(	void
	);

	VOID SupriseRemoval
	(	void
	);

	NTSTATUS QueryCapabilities
	(
		IN	OUT	PDEVICE_CAPABILITIES	Capabilities	
	);

	VOID Shutdown
	(	void
	);

	NTSTATUS ConfigureDevice
	(	void
    );

	NTSTATUS InitializeAudio
	(	void
	);

	NTSTATUS InitializeMidi
	(	void
	);

	NTSTATUS
	InitializeLanguageSupport
	(	void
	);

	NTSTATUS SaveSettingsToRegistry
	(	void
	);

	NTSTATUS RestoreSettingsFromRegistry
	(	void
	);

public:
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CKsAdapter);
    ~CKsAdapter();


    /*****************************************************************************
     * IKsAdapter implementation
     *
     * This macro is from IKSADAPTER.H.  It lists all the interface's functions.
     */
    IMP_IKsAdapter;

    /*****************************************************************************
     * CKsAdapter methods
     */

	/*************************************************************************
     * Static
     */
    static
    NTSTATUS DispatchCreate 
	(
        IN		PKSDEVICE	KsDevice
    );

    static
    NTSTATUS DispatchPnpStart 
	(
        IN		PKSDEVICE			KsDevice,
        IN		PIRP				Irp,
        IN		PCM_RESOURCE_LIST	TranslatedResourceList,
        IN		PCM_RESOURCE_LIST	UntranslatedResourceList
    );

    static
    NTSTATUS DispatchPnpQueryStop 
	(
        IN		PKSDEVICE	KsDevice,
        IN		PIRP		Irp
    );

    static
    VOID DispatchPnpCancelStop 
	(
        IN		PKSDEVICE	KsDevice,
        IN		PIRP		Irp
    );

    static
    VOID DispatchPnpStop 
	(
        IN		PKSDEVICE	KsDevice,
        IN		PIRP		Irp
    );

    static
    NTSTATUS DispatchPnpQueryRemove 
	(
        IN		PKSDEVICE	KsDevice,
        IN		PIRP		Irp
    );

    static
    VOID DispatchPnpCancelRemove 
	(
        IN		PKSDEVICE	KsDevice,
        IN		PIRP		Irp
    );

    static
    VOID DispatchPnpRemove 
	(
        IN		PKSDEVICE	KsDevice,
        IN		PIRP		Irp
    );

    static
    VOID DispatchPnpSupriseRemoval
	(
        IN		PKSDEVICE	KsDevice,
        IN		PIRP		Irp
    );

    static
    NTSTATUS DispatchPnpQueryCapabilities
	(
        IN		PKSDEVICE				KsDevice,
        IN		PIRP					Irp,
		IN	OUT	PDEVICE_CAPABILITIES	Capabilities	
    );

    static
    NTSTATUS DispatchShutdown
	(
        IN		PDEVICE_OBJECT	DeviceObject,
        IN		PIRP			Irp
    );

	static
    VOID Destruct 
	(
		IN		PVOID	Self
	);

	static
	VOID UsbDeviceCallbackRoutine
	(
		IN		ULONG	Reason,
		IN		PVOID	Parameter,
		IN		PVOID	Context
	);

	/*************************************************************************
     * Friends
     */
};

#endif  //_KS_ADAPTER_H_
