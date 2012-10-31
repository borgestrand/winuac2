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
 * @file	   Audio.cpp
 * @brief	   This file defines the API for the low-level manipulation of the
 *             audio devices.
 *
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "Audio.h"

#include "ExtProp.h"
#include "DataProcessingConstants.h"

#define STR_MODULENAME "Audio: "

/*****************************************************************************
 * Defines
 */
#define AUDIO_MAGIC	0xABBADEAF

// This is to workaround a bug in the Microsoft's USB EHCI driver where it
// would return a packet with length 0 even though there is valid data in the
// USB packet. It occurs when the endpoint specified an interval other than 1
// or 8 micro-frames. In our case, the data endpoint specify interval of 4
// micro-frames. The first packet is returned correctly, but the second is
// zero in length (although there is actually valid data in it). 
// The algorithm used to work around this issue:
// 1. Fill the packet with a known pattern for each audio frame before
//    passing it to the EHCI driver.
// 2. Upon receiving it back from the EHCI driver, check if the packet length
//    is zero. If it is zero, then check the following:
// (a) If the first audio frame matches the pattern, then the packet length is
//     indeed zero.
// (b) If the first audio frame doesn't match the pattern, then walk the packet
//     backwards to find an audio frame which doesn't match the pattern. This
//     will determine the packet length.
// 3. Update the packet length based on determination in (2).
#define MICROSOFT_USB_EHCI_BUG_WORKAROUND

// This is to workaround a bug in the Microsoft's USB OHCI driver where it
// would return IRPs late and out of order. IRP with an earlier StartFrame = N 
// is returned before IRP with StartFrame = N-1. This causes the glitches to 
// appear in the recorded audio stream.
// The algorithm used to work around this issue:
// 1. Keep the last record frame number. 
// 2. Compare it to the current record frame number.
// (a) If the last record frame number is smaller by more than 1 as compared to
//     the current record frame number, then copy the data in the FIFO work item
//     to temporary buffer.
// (b) If the last record frame number is larger than the current record frame
//     number, a flip is detected. Copy the current FIFO work item's data to
//     the client first, then followed by data kept in the temporary buffer.
#define MICROSOFT_USB_OHCI_BUG_WORKAROUND


/*****************************************************************************
 * Referenced forward
 */

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioInterruptPipe::~CAudioInterruptPipe()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Destructor.
 */
CAudioInterruptPipe::
~CAudioInterruptPipe
(	void
)
{
	PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioInterruptPipe::~CAudioInterruptPipe]"));

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CAudioInterruptPipe::Init()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioInterruptPipe::
Init
(
	IN		PUSB_DEVICE				UsbDevice,
	IN		UCHAR					InterfaceNumber,
	IN		USBD_PIPE_INFORMATION	PipeInformation,
	IN		CAudioDevice *			AudioDevice
)
{
    PAGED_CODE();

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_InterfaceNumber = InterfaceNumber;

	m_PipeInformation = PipeInformation;

	m_MaximumTransferSize = (m_PipeInformation.MaximumPacketSize < INTERRUPT_FIFO_BUFFER_SIZE) ? m_PipeInformation.MaximumPacketSize : INTERRUPT_FIFO_BUFFER_SIZE;

	m_AudioDevice = AudioDevice;

	m_PowerState = PowerDeviceD0;

	m_PipeState = AUDIO_INTERRUPT_PIPE_STATE_STOP;

	KeInitializeMutex(&m_PipeStateLock, 0);

	KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioInterruptPipe::PowerStateChange()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns AUDIOERR_SUCCESS if the power state changed.
 */
AUDIOSTATUS
CAudioInterruptPipe::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		m_PowerState = NewState;

		// Stop or restart transfers as needed.
		if (NewState == PowerDeviceD0)
		{
			Start();
		}
		else
		{
			Stop();
		}
	}

    return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioInterruptPipe::AcquireResources()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Acquire the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioInterruptPipe::
AcquireResources
(	void
)
{
	PAGED_CODE();

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_PipeInformation.PipeHandle)
	{
		m_MaximumIrpCount = MAX_INTERRUPT_IRP;

		for (ULONG i=0; i<m_MaximumIrpCount; i++)
		{
			if (m_UsbDevice->CreateIrp(&m_FifoWorkItem[i].Irp) != STATUS_SUCCESS)
			{
				audioStatus = AUDIOERR_INSUFFICIENT_RESOURCES;
				break;
			}
		}

		if (AUDIO_SUCCESS(audioStatus))
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

				_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioInterruptPipe::AcquireResources] - Recycle IRP - %d", i));

				m_UsbDevice->RecycleIrp(&m_FifoWorkItem[i].Urb, m_FifoWorkItem[i].Irp, IoCompletionRoutine, (PVOID)&m_FifoWorkItem[i]);
			}
		}
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioInterruptPipe::FreeResources()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Free the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioInterruptPipe::
FreeResources
(	void
)
{
	PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioInterruptPipe::FreeResources]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_PipeInformation.PipeHandle)
	{
		_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioInterruptPipe::FreeResources] - Abort pipe %p", m_PipeInformation.PipeHandle));

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
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioInterruptPipe::Start()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Start the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioInterruptPipe::
Start
(	void
)
{
	PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioInterruptPipe::Start]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_PipeState != AUDIO_INTERRUPT_PIPE_STATE_RUN)
	{
		m_PipeState = AUDIO_INTERRUPT_PIPE_STATE_RUN;

		audioStatus = AcquireResources();
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioInterruptPipe::Stop()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Start the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioInterruptPipe::
Stop
(	void
)
{
	PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioInterruptPipe::Stop]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_PipeState != AUDIO_INTERRUPT_PIPE_STATE_STOP)
	{
		m_PipeState = AUDIO_INTERRUPT_PIPE_STATE_STOP;

		FreeResources();
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioInterruptPipe::Service()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Service the interrupts.
 * @param
 * FifoWorkItem FIFO work item to service.
 * @return
 * <None>
 */
VOID
CAudioInterruptPipe::
Service
(
	IN		PINTERRUPT_FIFO_WORK_ITEM	FifoWorkItem
)
{
	if (m_PipeState == AUDIO_INTERRUPT_PIPE_STATE_STOP)
	{
		m_FifoWorkItemList.Lock();

		m_FifoWorkItemList.Put(FifoWorkItem);

		_DbgPrintF(DEBUGLVL_BLAB,("A:Current IRPs count... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

		if (m_FifoWorkItemList.Count() == m_MaximumIrpCount)
		{
			_DbgPrintF(DEBUGLVL_BLAB,("Setting Event..."));

			KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
		}

		m_FifoWorkItemList.Unlock();
	}
	else if (FifoWorkItem->Irp->IoStatus.Status != STATUS_SUCCESS)
	{
		m_FifoWorkItemList.Lock();

		m_FifoWorkItemList.Put(FifoWorkItem);

		_DbgPrintF(DEBUGLVL_BLAB,("B:Current IRPs count... [%d, %d]", m_FifoWorkItemList.Count(), m_MaximumIrpCount));

		m_FifoWorkItemList.Unlock();
	}
	else
	{
		PUSB_AUDIO_STATUS_WORD StatusWord = PUSB_AUDIO_STATUS_WORD(FifoWorkItem->Urb.UrbBulkOrInterruptTransfer.TransferBuffer);

		ULONG NumberOfStatusWord = FifoWorkItem->Urb.UrbBulkOrInterruptTransfer.TransferBufferLength/sizeof(USB_AUDIO_STATUS_WORD);

		if (NumberOfStatusWord)
		{
			m_AudioDevice->OnStatusInterrupt(NumberOfStatusWord, StatusWord, NULL);
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
}

/*****************************************************************************
 * CAudioInterruptPipe::IoCompletionRoutine()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
NTSTATUS
CAudioInterruptPipe::
IoCompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp,
    IN		PVOID			Context
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioInterruptPipe::IoCompletionRoutine]"));

	PINTERRUPT_FIFO_WORK_ITEM FifoWorkItem = (PINTERRUPT_FIFO_WORK_ITEM)Context;

	CAudioInterruptPipe * that = (CAudioInterruptPipe*)FifoWorkItem->Context;

	that->Service(FifoWorkItem);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioClient::CAudioClient()
 *****************************************************************************
 *//*!
 * @ingroup AUDIO_GROUP
 * @brief
 * Constructor.
 */
CAudioClient::
CAudioClient
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioClient::CAudioClient]"));

	m_Next = m_Prev = NULL;
    m_Owner = NULL;

    for(ULONG count = 0; count < AUDIO_CLIENT_MAX_CHANNEL; count++)
    {
        m_MasterVolumeStep[count]   = MASTERVOL_STEP_SIZE_DB;
        m_MasterVolumeMin[count]    = MASTERVOL_MIN_DB;
        m_MasterVolumeMax[count]    = MASTERVOL_MAX_DB;
    }

    m_requireSoftMaster = TRUE; // default assume software master vol/mute is not required
}

/*****************************************************************************
 * CAudioClient::~CAudioClient()
 *****************************************************************************
 *//*!
 * @ingroup AUDIO_GROUP
 * @brief
 * Destructor.
 */
CAudioClient::
~CAudioClient
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioClient::~CAudioClient]"));

	if (m_FifoBuffer)
	{
		ExFreePool(m_FifoBuffer);
	}

	if (m_Interface)
	{
		if (AUDIO_SUCCESS(m_Interface->AcquireInterface(this, m_Priority)))
		{
			// Back to zero-bandwidth setting.
			m_Interface->SelectAlternateSetting(0);

			// Remove callback.
			m_Interface->SetInterfaceAvailabilityCallback(this, FALSE);

			// Free the interface.
			m_Interface->ReleaseInterface(this);
		}
		else
		{
			// Remove callback.
			m_Interface->SetInterfaceAvailabilityCallback(this, FALSE);
		}
	}
}

/*****************************************************************************
 * CAudioClient::Init()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Initialize the Audio client.
 * @param
 * Stream Pointer to the audio stream object.
 * @param
 * Direction AUDIO_INPUT or AUDIO_OUTPUT.
 * @param
 * CallbackRoutine The function to call when the audio transitions to an empty
 * state (on output) or data becomes available (on input).
 * @param
 * CallbackData A user-specified handle which is passed as a parameter when
 * the callback is called.
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioClient::
Init
(
	IN		CAudioDevice *			AudioDevice,
	IN		AUDIO_CALLBACK_ROUTINE	CallbackRoutine,
	IN		PVOID					CallbackData
)
{
	PAGED_CODE();

	m_AudioDevice = AudioDevice;

	m_ReadPosition = 0;  m_WritePosition = 1;

	m_CallbackData = CallbackData;
    m_CallbackRoutine = CallbackRoutine;

	m_InterfaceNumber = ULONG(-1); // uninitialized.

	KeInitializeSpinLock(&m_Lock);

	m_ClockRateExtension = _FindExtensionUnit(XU_CODE_CLOCK_RATE);

	m_DriverResyncExtension = _FindExtensionUnit(XU_CODE_DRIVER_RESYNC);
    // Init variables for software master vol/mute
    for(ULONG count = 0; count < AUDIO_CLIENT_MAX_CHANNEL; count++)
    {
        // Currently caching the range value.
        // If range need to be dynamic, then need to remove the caching and call
        // m_AudioDevice to get when required
        m_AudioDevice->GetMasterVolumeRange(
            count,
            &m_MasterVolumeMin[count],   
            &m_MasterVolumeMax[count],  
            &m_MasterVolumeStep[count]
            );

        m_requireSoftMaster = m_AudioDevice->IsSoftwareMasterVolMute();
    }

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioClient::_FindExtensionUnit()
 *****************************************************************************
 *//*!
 * @brief
 */
PEXTENSION_UNIT 
CAudioClient::
_FindExtensionUnit
(
	IN		USHORT	ExtensionCode
)
{
    PAGED_CODE();

	PEXTENSION_UNIT ExtensionUnit = NULL;

	PAUDIO_TOPOLOGY Topology = NULL;

	if (m_AudioDevice->ParseTopology(0, &Topology))
	{
		for (ULONG i=0; ; i++)
		{
			PUNIT Unit = NULL;

			if (Topology->ParseUnits(i, &Unit))
			{
				if (Unit->DescriptorSubtype() == USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT)
				{
					PEXTENSION_UNIT XU = PEXTENSION_UNIT(Unit);

					EXTENSION_UNIT_DETAILS Details;

					XU->ExtensionDetails(&Details);

					if (Details.ExtensionCode == ExtensionCode)
					{
						ExtensionUnit = XU;
						break;
					}
				}
			}
			else
			{
				break;
			}
		}
	}

	return ExtensionUnit;
}

/*****************************************************************************
 * CAudioClient::_SetClockRate()
 *****************************************************************************
 *//*!
 * @brief
 */
VOID 
CAudioClient::
_SetClockRate
(
	IN		ULONG	ClockRate
)
{
    PAGED_CODE();

	if (m_ClockRateExtension)
	{
		if (ClockRate)
		{
			#define XU_CLOCK_RATE_SR_44kHz			0x0
			#define XU_CLOCK_RATE_SR_48kHz			0x1
			#define XU_CLOCK_RATE_SR_88kHz			0x2
			#define XU_CLOCK_RATE_SR_96kHz			0x3
			#define XU_CLOCK_RATE_SR_176kHz			0x4
			#define XU_CLOCK_RATE_SR_192kHz			0x5
			#define XU_CLOCK_RATE_SR_UNSPECIFIED	0xFF

			UCHAR Rate = (ClockRate == 44100) ?  XU_CLOCK_RATE_SR_44kHz :
						 (ClockRate == 48000) ?  XU_CLOCK_RATE_SR_48kHz :
						 (ClockRate == 88200) ?  XU_CLOCK_RATE_SR_88kHz :
						 (ClockRate == 96000) ?  XU_CLOCK_RATE_SR_96kHz :
						 (ClockRate == 176400) ? XU_CLOCK_RATE_SR_176kHz :
						 (ClockRate == 192000) ? XU_CLOCK_RATE_SR_192kHz :
												 XU_CLOCK_RATE_SR_UNSPECIFIED;

			m_ClockRateExtension->WriteParameterBlock(REQUEST_CUR, 3/*Rate selector*/, 0, &Rate, sizeof(UCHAR), PARAMETER_BLOCK_FLAGS_IO_BOTH);
		}
	}
}

/*****************************************************************************
 * CAudioClient::SetInterfaceParameter()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioClient::
SetInterfaceParameter
(	
	IN		UCHAR	InterfaceNumber,
	IN		UCHAR	AlternateSetting,
	IN		ULONG	Priority,
	IN		ULONG	ClockRate,
	IN		BOOL	ForceSelection
)
{
	PAGED_CODE();

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioClient::SetInterfaceParameter]"));

	AUDIOSTATUS audioStatus = AUDIOERR_DEVICE_CONFIGURATION_ERROR;

	CAudioInterface * Interface = m_AudioDevice->FindInterface(InterfaceNumber);

	if (Interface)
	{
		audioStatus = Interface->AcquireInterface(this, Priority);

		if (Interface != m_Interface)
		{
			if (AUDIO_SUCCESS(audioStatus))
			{
				_SetClockRate(ClockRate);

				audioStatus = Interface->SelectAlternateSetting(AlternateSetting);
			}

			if (AUDIO_SUCCESS(audioStatus))
			{
				if (m_Interface)
				{
					// Back to zero-bandwidth setting.
					m_Interface->SelectAlternateSetting(0);

					// Remove callback.
					m_Interface->SetInterfaceAvailabilityCallback(this, FALSE);

					m_Interface->ReleaseInterface(this);

					m_Interface = NULL;
				}
			}
		}
		else
		{
			if ((m_AlternateSetting != AlternateSetting) || (ForceSelection))
			{
				// Switch to alternate setting 0 first.
				Interface->SelectAlternateSetting(0);

				// Change the internal clock rate.
				_SetClockRate(ClockRate);

				// Select the new setting.
				audioStatus = Interface->SelectAlternateSetting(AlternateSetting);

				if (!AUDIO_SUCCESS(audioStatus))
				{
					// Reset the clock back to its original setting.
					_SetClockRate(m_ClockRate);

					// Set it back to whatever it was before...
					Interface->SelectAlternateSetting(m_AlternateSetting);	

					m_SynchPipe = NULL;

					m_DataPipe = Interface->FindDataPipe();

					if (m_DataPipe)
					{
						_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioClient::SetInterfaceParameter] - Data Synchronization Type: 0x%x, EndpointAddress: 0x%x", m_DataPipe->SynchronizationType(), m_DataPipe->PipeInformation()->EndpointAddress));

						m_DataPipe->SetCallbackClient(this);

						m_SynchPipe = Interface->FindSynchPipe();

						if (m_SynchPipe)
						{
							m_SynchPipe->SetDataPipe(m_DataPipe);
						}
					}
				}
			}
			else
			{
				audioStatus = AUDIOERR_SUCCESS;
			}
		}
	}

	if (AUDIO_SUCCESS(audioStatus))
	{
		m_Interface = Interface;

		m_InterfaceNumber = InterfaceNumber;

		m_AlternateSetting = AlternateSetting;

		m_Priority = Priority;

		m_ClockRate = ClockRate;

		PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR FormatTypeDescriptor_ = Interface->GetFormatTypeDescriptor(AlternateSetting);

		if ((FormatTypeDescriptor_->bFormatType == USB_AUDIO_FORMAT_TYPE_I) || (FormatTypeDescriptor_->bFormatType == USB_AUDIO_FORMAT_TYPE_III))
		{
			// The format type descriptor is the same for TYPE_I and TYPE_III.
			PUSB_AUDIO_TYPE_I_FORMAT_DESCRIPTOR FormatTypeDescriptor = PUSB_AUDIO_TYPE_I_FORMAT_DESCRIPTOR(FormatTypeDescriptor_);

			m_BitResolution = FormatTypeDescriptor->bBitResolution;
		}
		else
		{
			// Type II format. No bit resolution specified.
			m_BitResolution = 0;
		}

		m_SynchPipe = NULL;

		m_DataPipe = Interface->FindDataPipe();

		if (m_DataPipe)
		{
			_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioClient::SetInterfaceParameter] - Data Synchronization Type: 0x%x, EndpointAddress: 0x%x", m_DataPipe->SynchronizationType(), m_DataPipe->PipeInformation()->EndpointAddress));

			m_DataPipe->SetCallbackClient(this);

			m_SynchPipe = Interface->FindSynchPipe();

			if (m_SynchPipe)
			{
				m_SynchPipe->SetDataPipe(m_DataPipe);
			}
		}
	}

	if (!AUDIO_SUCCESS(audioStatus))
	{
		if (Interface)
		{
			Interface->ReleaseInterface(this);
		}
	}

	return audioStatus;
}

/*****************************************************************************
 * CAudioClient::OnResourcesRipOff()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
VOID
CAudioClient::
OnResourcesRipOff
(	void
)
{
	PAGED_CODE();
	
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioClient::OnResourcesRipOff]"));

	if (m_Interface)
	{
		if (AUDIO_SUCCESS(m_Interface->AcquireInterface(this, m_Priority)))
		{
			// Cleanup the pipes...
			if (m_DataPipe)
			{
				if (m_IsActive)
				{
					m_DataPipe->Stop();
				}

				m_DataPipe = NULL;
			}

			if (m_SynchPipe)
			{
				if (m_IsActive)
				{
					m_SynchPipe->Stop();
				}

				m_SynchPipe = NULL;
			}

			// Back to zero-bandwidth setting.
			m_Interface->SelectAlternateSetting(0);

			// Free the interface.
			m_Interface->ReleaseInterface(this);

			// Add interface callback to get resources arrival message.
			m_Interface->SetInterfaceAvailabilityCallback(this, TRUE);
		}
	}
}

/*****************************************************************************
 * CAudioClient::OnResourcesAvailability()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
VOID
CAudioClient::
OnResourcesAvailability
(	void
)
{
	PAGED_CODE();
	
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioClient::OnResourcesAvailability]"));

	if (m_Interface)
	{
		if (AUDIO_SUCCESS(SetInterfaceParameter((UCHAR)m_InterfaceNumber, m_AlternateSetting, m_Priority, m_ClockRate, TRUE)))
		{
			// Remove interface callback.
			m_Interface->SetInterfaceAvailabilityCallback(this, FALSE);

			// Restart the pipes...
			if (m_SynchPipe)
			{
				ULONG BitResolution = m_BitResolution ? m_BitResolution : m_SampleSize;

				m_SynchPipe->SetTransferParameters(m_SampleRate, m_FormatChannels, BitResolution);
			}

			if (m_DataPipe)
			{
				ULONG BitResolution = m_BitResolution ? m_BitResolution : m_SampleSize;

				m_DataPipe->SetTransferParameters(m_SampleRate, m_FormatChannels, BitResolution, m_NumberOfFifoBuffers);
			}

			if (m_IsActive)
			{
				if (m_SynchPipe)
				{
					m_SynchPipe->Start(FALSE, 0);
				}
				
				if (m_DataPipe)
				{
					m_DataPipe->Start(FALSE, 0);
				}
			}
		}
	}
}

#pragma code_seg()

/*****************************************************************************
 * CAudioClient::RequestDriverResync()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
VOID
CAudioClient::
RequestDriverResync
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioClient::RequestDriverResync]"));

	if (m_DriverResyncExtension)
	{
		USB_AUDIO_STATUS_WORD StatusWord;

		StatusWord.bmStatusType = USB_AUDIO_STATUS_TYPE_ORIGINATOR_AC_INTERFACE | USB_AUDIO_STATUS_TYPE_INTERRUPT_PENDING;
		StatusWord.bOriginator = m_DriverResyncExtension->UnitID();

		m_AudioDevice->OnStatusInterrupt(1, &StatusWord, NULL);
	}
}

/*****************************************************************************
 * CAudioClient::Lock()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
VOID
CAudioClient::
Lock
(	void
)
{
	KeAcquireSpinLock(&m_Lock, &m_LockIrql);
}

/*****************************************************************************
 * CAudioClient::Unlock()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
VOID
CAudioClient::
Unlock
(	void
)
{
	KeReleaseSpinLock(&m_Lock, m_LockIrql);
}

/*****************************************************************************
 * CAudioClient::Reset()
 *****************************************************************************
 * @brief
 * Reset the auto pointer.
 * @param
 * <None>
 * @return
 * <None>
 */
VOID
CAudioClient::
Reset
(	void
)
{
	Lock();

	m_ReadPosition = 0;
    m_WritePosition = 1;

	Unlock();
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
 * m_WritePosition = 1. In this case, the ring buffer is 5 frames in size
 * (BUFFER_SIZE == 4). If we call AddFrames() four times, m_WritePosition
 * will be equal to 0 , while m_ReadPosition == 0.  At this point,
 * m_WritePosition == m_ReadPosition, so we consider the ring buffer to be
 * full.  Note, however, that we never wrote a frame to buffer[0], since
 * m_WritePosition started out being initialized to 1.
 *
 * This scheme is a little weird, but it has the advantage of making it
 * really easy to determine when the buffer is full or empty.
 */

/*****************************************************************************
 * CAudioClient::AddFramesToFifo()
 *****************************************************************************
 * @brief
 * Adds data in buffer to the FIFO.
 * @param
 * Buffer Pointer to the buffer that contains the data for the client.
 * @param
 * NumberOfFrames Number of audio frames in Buffer.
 * @return
 * Returns the actual number of frames successfully written to the FIFO.
 */
ULONG
CAudioClient::
AddFramesToFifo
(
	IN		PUCHAR	Buffer,
	IN		ULONG	NumberOfFrames,
	IN		BOOL	BitConversion
)
{
	ULONG FramesWritten = 0;

	if (NumberOfFrames)
	{
		if (m_WritePosition < m_ReadPosition)
		{
			ULONG FramesAvailable = m_ReadPosition - m_WritePosition;

			ULONG FramesToWrite = (NumberOfFrames > FramesAvailable) ? FramesAvailable : NumberOfFrames;

			if (BitConversion)
			{
				m_ConversionRoutine(m_FifoBuffer + m_WritePosition * m_FifoFrameSize, Buffer, FramesToWrite * m_FormatChannels);
			}
			else
			{
				RtlCopyMemory(m_FifoBuffer + m_WritePosition * m_FifoFrameSize, Buffer, FramesToWrite * m_FifoFrameSize);
			}
            VolumeMuteAdjustment(m_FifoBuffer + m_WritePosition * m_FifoFrameSize, FramesToWrite);

			m_WritePosition += FramesToWrite;

			m_WritePosition %= (m_FifoBufferSize+1);

			FramesWritten += FramesToWrite;
		}
		else if (m_WritePosition > m_ReadPosition)
		{
			ULONG FramesAvailable = m_FifoBufferSize - m_WritePosition + 1;

			if (NumberOfFrames > FramesAvailable)
			{
				ULONG FramesToWrite0 = FramesAvailable;

				if (BitConversion)
				{
					m_ConversionRoutine(m_FifoBuffer + m_WritePosition * m_FifoFrameSize, Buffer, FramesToWrite0 * m_FormatChannels);
				}
				else
				{
					RtlCopyMemory(m_FifoBuffer + m_WritePosition * m_FifoFrameSize, Buffer, FramesToWrite0 * m_FifoFrameSize);
				}
                VolumeMuteAdjustment(m_FifoBuffer + m_WritePosition * m_FifoFrameSize, FramesToWrite0);

				ULONG FramesLeftToWrite = NumberOfFrames - FramesToWrite0;

				ULONG FramesToWrite1 = (FramesLeftToWrite > m_ReadPosition) ? m_ReadPosition : FramesLeftToWrite;

				if (FramesToWrite1) 
				{
					if (BitConversion)
					{
						m_ConversionRoutine(m_FifoBuffer, Buffer + FramesToWrite0 * m_ClientFrameSize, FramesToWrite1 * m_FormatChannels);
					}
					else
					{
						RtlCopyMemory(m_FifoBuffer, Buffer + FramesToWrite0 * m_FifoFrameSize, FramesToWrite1 * m_FifoFrameSize);
					}
                    VolumeMuteAdjustment(m_FifoBuffer, FramesToWrite1);
				}

				m_WritePosition = FramesToWrite1;

				FramesWritten += (FramesToWrite0 + FramesToWrite1);
			}
			else
			{
				ULONG FramesToWrite = NumberOfFrames;

				if (BitConversion)
				{
					m_ConversionRoutine(m_FifoBuffer + m_WritePosition * m_FifoFrameSize, Buffer, FramesToWrite * m_FormatChannels);
				}
				else
				{
					RtlCopyMemory(m_FifoBuffer + m_WritePosition * m_FifoFrameSize, Buffer, FramesToWrite * m_FifoFrameSize);
				}
                VolumeMuteAdjustment(m_FifoBuffer + m_WritePosition * m_FifoFrameSize, FramesToWrite);

				m_WritePosition += FramesToWrite;

				m_WritePosition %= (m_FifoBufferSize+1);

				FramesWritten += FramesToWrite;
			}
		}
		else
		{
			/* Buffer is completely FULL */
		}
	}

	return FramesWritten;
}

/*****************************************************************************
 * CAudioClient::RemoveFramesFromFifo()
 *****************************************************************************
 * @brief
 * Removes data in the FIFO to a buffer.
 * @details
 * This routine does not block if no data is available.
 * @param
 * Buffer Buffer address of the incoming stream.
 * @param
 * NumberOfFrames Length in frames of the buffer pointed to by Buffer.
 * @return
 * Returns the actual number of frames successfully read from the FIFO.
 */
ULONG
CAudioClient::
RemoveFramesFromFifo
(
	IN		PUCHAR	Buffer,
	IN		ULONG	NumberOfFrames,
	IN		BOOL	BitConversion
)
{
	ULONG FramesRead = 0;

    ULONG NewReadPosition = (m_ReadPosition + 1) % (m_FifoBufferSize+1);

	if (m_WritePosition > NewReadPosition)
	{
		ULONG FramesAvailable = m_WritePosition - NewReadPosition;

		ULONG FramesToRead = (NumberOfFrames > FramesAvailable) ? FramesAvailable : NumberOfFrames;

		if (Buffer)
		{
			if (BitConversion)
			{
				m_ConversionRoutine(Buffer, m_FifoBuffer + NewReadPosition * m_FifoFrameSize, FramesToRead * m_FormatChannels);
			}
			else
			{
				RtlCopyMemory(Buffer, m_FifoBuffer + NewReadPosition * m_FifoFrameSize, FramesToRead * m_FifoFrameSize);
			}
		}

		m_ReadPosition = NewReadPosition + FramesToRead - 1;

		FramesRead += FramesToRead;
	}
	else if (m_WritePosition < NewReadPosition)
	{
		ULONG FramesAvailable = m_FifoBufferSize - NewReadPosition + 1;

		if (NumberOfFrames > FramesAvailable)
		{
			ULONG FramesToRead0 = FramesAvailable;

			if (Buffer)
			{
				if (BitConversion)
				{
					m_ConversionRoutine(Buffer, m_FifoBuffer + NewReadPosition * m_FifoFrameSize, FramesToRead0 * m_FormatChannels);
				}
				else
				{
					RtlCopyMemory(Buffer, m_FifoBuffer + NewReadPosition * m_FifoFrameSize, FramesToRead0 * m_FifoFrameSize);
				}
			}

			m_ReadPosition = NewReadPosition + FramesToRead0 - 1;

			ULONG FramesLeftToRead = NumberOfFrames - FramesToRead0;

			ULONG FramesToRead1 = (FramesLeftToRead > m_WritePosition) ? m_WritePosition : FramesLeftToRead;

			if (FramesToRead1) 
			{
				if (Buffer)
				{
					if (BitConversion)
					{
						m_ConversionRoutine(Buffer + FramesToRead0 * m_ClientFrameSize, m_FifoBuffer, FramesToRead1 * m_FormatChannels);
					}
					else
					{
						RtlCopyMemory(Buffer + FramesToRead0 * m_FifoFrameSize, m_FifoBuffer, FramesToRead1 * m_FifoFrameSize);
					}
				}

				m_ReadPosition = FramesToRead1 - 1;
			}

			FramesRead += (FramesToRead0 + FramesToRead1);
		}
		else
		{
			ULONG FramesToRead = NumberOfFrames;

			if (Buffer)
			{
				if (BitConversion)
				{
					m_ConversionRoutine(Buffer, m_FifoBuffer + NewReadPosition * m_FifoFrameSize, FramesToRead * m_FormatChannels);
				}
				else
				{
					RtlCopyMemory(Buffer, m_FifoBuffer + NewReadPosition * m_FifoFrameSize, FramesToRead * m_FifoFrameSize);
				}
			}

			m_ReadPosition = NewReadPosition + FramesToRead - 1;

			FramesRead += FramesToRead;
		}
	}
	else
	{
        /* Buffer is EMPTY */
	}

    return FramesRead;
}

/*****************************************************************************
 * CAudioClient::GetNumQueuedFrames()
 *****************************************************************************
 * @brief
 * Get number of frames queued in the FIFO.
 * @param
 * <None>
 * @return
 * Returns number of frames queued in the FIFO.
 */
ULONG
CAudioClient::
GetNumQueuedFrames
(	void
)
{
    if (m_WritePosition > m_ReadPosition)
	{
        return m_WritePosition - m_ReadPosition - 1;
	}
    else
	{
        return (m_FifoBufferSize+1) - m_ReadPosition + m_WritePosition - 1;
	}
}

/*****************************************************************************
 * CAudioClient::GetNumAvailableFrames()
 *****************************************************************************
 * @brief
 * Get number of frames available in the FIFO.
 * @param
 * <None>
 * @return
 * Returns number of frames available in the FIFO.
 */
ULONG
CAudioClient::
GetNumAvailableFrames
(	void
)
{
	return m_FifoBufferSize - GetNumQueuedFrames();
}

/*****************************************************************************
 * CAudioClient::IsFull()
 *****************************************************************************
 * @brief
 * Check to see whether the FIFO is FULL.
 * @param
 * <None>
 * @return
 * Returns TRUE if the FIFO is FULL, otherwise FALSE.
 */
BOOL
CAudioClient::
IsFull
(	void
)
{
    return (GetNumQueuedFrames() == m_FifoBufferSize);
}

/*****************************************************************************
 * CAudioClient::IsEmpty()
 *****************************************************************************
 * @brief
 * Check to see whether the FIFO is EMPTY.
 * @param
 * <None>
 * @return
 * Returns TRUE if the FIFO is EMPTY, otherwise FALSE.
 */
BOOL
CAudioClient::
IsEmpty
(	void
)
{
    return (GetNumQueuedFrames() == 0);
}

#include "Convert.h"

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioClient::SetupBuffer()
 *****************************************************************************
 * @brief
 * Setup the FIFO buffer.
 * @return
 * Returns the STATUS_SUCCESS if successful, otherwise appropriate error code.
 */
NTSTATUS
CAudioClient::
SetupBuffer
(
	IN		ULONG	SampleRate,
	IN		ULONG	FormatChannels,
	IN		ULONG	SampleSize,
	IN		BOOL	Capture,
	IN		ULONG	NumberOfFifoBuffers
)
{
	PAGED_CODE();

	NTSTATUS ntStatus = STATUS_INSUFFICIENT_RESOURCES;

	ULONG BitResolution = m_BitResolution ? m_BitResolution : SampleSize;

	m_BitConversion = BitResolution != SampleSize;

	m_SampleRate = SampleRate;

	m_FormatChannels = FormatChannels;

	m_SampleSize = SampleSize;

	m_NumberOfFifoBuffers = NumberOfFifoBuffers;

	if (m_BitConversion)
	{
		if (Capture)
		{
			m_ConversionRoutine = FindConversionRoutine(m_BitResolution, SampleSize);
		}
		else
		{
			m_ConversionRoutine = FindConversionRoutine(SampleSize, m_BitResolution);
		}
	}

	// Allocate a 100ms buffer.
	ULONG FifoBufferSizeInFrames = (Capture) ? (SampleRate * AUDIO_CLIENT_INPUT_BUFFERSIZE / 1000) :
											   (SampleRate * AUDIO_CLIENT_OUTPUT_BUFFERSIZE / 1000);
	
	ULONG FifoFrameSize = FormatChannels * (BitResolution / 8);

	PUCHAR FifoBuffer = PUCHAR(ExAllocatePoolWithTag(NonPagedPool, (FifoBufferSizeInFrames + 1) * FifoFrameSize, 'mdW'));

	if (FifoBuffer)
	{
		if (m_FifoBuffer)
		{
			ExFreePool(m_FifoBuffer);
		}

		m_FifoBuffer = FifoBuffer;

		m_FifoBufferSize = FifoBufferSizeInFrames;

		m_FifoFrameSize = FifoFrameSize;

		m_ClientFrameSize = FormatChannels * SampleSize / 8;

		if (m_DataPipe)
		{
			m_DataPipe->SetTransferParameters(SampleRate, FormatChannels, BitResolution, NumberOfFifoBuffers);
		}

		if (m_SynchPipe)
		{
			m_SynchPipe->SetTransferParameters(SampleRate, FormatChannels, BitResolution);
		}

		ntStatus = STATUS_SUCCESS;
	}

	return ntStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioClient::WriteBuffer()
 *****************************************************************************
 * @brief
 * Writes a buffer of data to the FIFO.
 * @param
 * Buffer Pointer to the buffer that contains the data for the client.
 * @param
 * BytesLength Length in bytes of the data stream buffer at Buffer.
 * @return
 * Returns the actual number of bytes successfully written to the FIFO.
 */
ULONG
CAudioClient::
WriteBuffer
(
	IN		PUCHAR	Buffer,
	IN		ULONG	BufferLength
)
{
	Lock();

	ULONG BytesWritten = AddFramesToFifo(Buffer, BufferLength / m_ClientFrameSize, m_BitConversion) * m_ClientFrameSize;

	Unlock();

	if (BytesWritten)
	{
		// Increment the total number of bytes queued.
		m_TotalBytesQueued += BytesWritten;

		// Kickstart the process of queueing buffer to the data pipe.
		if (m_DataPipe)
		{
			m_DataPipe->FlushBuffer();
		}
	}

	return BytesWritten;
}

/*****************************************************************************
 * CAudioClient::QueueBuffer()
 *****************************************************************************
 * @brief
 * Queue a buffer for reading.
 * @details
 * Note that this method is there just to keep track of the queue position.
 * It doesn't actually reads the data.
 * @param
 * Buffer Buffer address of the incoming stream.
 * @param
 * BufferLength Length in bytes of the buffer pointed to by Buffer.
 * @return
 * Returns the actual number of bytes successfully queeued.
 */
ULONG
CAudioClient::
QueueBuffer
(
	IN		PUCHAR	Buffer,
	IN		ULONG	BufferLength
)
{
	// Increment the total number of bytes queued.
	m_TotalBytesQueued += BufferLength;

    return BufferLength;
}

/*****************************************************************************
 * CAudioClient::ReadBuffer()
 *****************************************************************************
 * @brief
 * Reads a buffer of data from the FIFO.
 * @details
 * This routine does not block if no data is available.
 * @param
 * Buffer Buffer address of the incoming stream.
 * @param
 * BufferLength Length in bytes of the buffer pointed to by Buffer.
 * @return
 * Returns the actual number of bytes successfully read from the FIFO.
 */
ULONG
CAudioClient::
ReadBuffer
(
	IN		PUCHAR	Buffer,
	IN		ULONG	BufferLength
)
{
	Lock();

	ULONG BytesRead = RemoveFramesFromFifo(Buffer, BufferLength / m_ClientFrameSize, m_BitConversion) * m_ClientFrameSize;

	Unlock();

    return BytesRead;
}

/*****************************************************************************
 * CAudioClient::Start()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Start the client.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioClient::
Start
(
	IN		BOOL	SynchronizeStart,
	IN		ULONG	StartFrameNumber
)
{
	PAGED_CODE();

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;
	
	if (m_SynchPipe)
	{
		audioStatus = m_SynchPipe->Start(SynchronizeStart, StartFrameNumber);
	}

	if (AUDIO_SUCCESS(audioStatus))
	{
		if (m_DataPipe)
		{
			audioStatus = m_DataPipe->Start(SynchronizeStart, StartFrameNumber);
		}

		if (!AUDIO_SUCCESS(audioStatus))
		{
			if (m_SynchPipe)
			{
				m_SynchPipe->Stop();
			}
		}
	}

	if (AUDIO_SUCCESS(audioStatus))
	{
		m_IsActive = TRUE;
	}

	return audioStatus;
}

/*****************************************************************************
 * CAudioClient::Pause()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * "Pause" the client.
 * @param
 * <None>
 * @return
 * <None>
 */
AUDIOSTATUS
CAudioClient::
Pause
(	void
)
{
	PAGED_CODE();

	m_IsActive = FALSE;

	AUDIOSTATUS audioStatus = m_DataPipe ? m_DataPipe->Pause() : AUDIOERR_SUCCESS;

	if (AUDIO_SUCCESS(audioStatus))
	{
		if (m_SynchPipe)
		{
			m_SynchPipe->Stop();
		}
	}

	return audioStatus;
}

/*****************************************************************************
 * CAudioClient::Stop()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Stop the client.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioClient::
Stop
(	void
)
{
	PAGED_CODE();

	m_IsActive = FALSE;

	AUDIOSTATUS audioStatus = m_DataPipe ? m_DataPipe->Stop() : AUDIOERR_SUCCESS;

	if (AUDIO_SUCCESS(audioStatus))
	{
		Reset();

		m_TotalBytesQueued = 0;

		if (m_SynchPipe)
		{
			m_SynchPipe->Stop();
		}
	}

	return audioStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioClient::GetPosition()
 *****************************************************************************
 *//*!
 * @brief
 * Get the current transfer position.
 */
AUDIOSTATUS 
CAudioClient::
GetPosition
(
	OUT		ULONGLONG *	OutTransferPosition,
	OUT		ULONGLONG *	OutQueuePosition
)
{
	if (OutQueuePosition)
	{
		*OutQueuePosition = m_TotalBytesQueued;
	}

	ULONGLONG TransferPosition = 0;

    if (m_DataPipe)
	{
		m_DataPipe->GetPosition(&TransferPosition);

		TransferPosition *= m_ClientFrameSize;

		TransferPosition /= m_FifoFrameSize;
	}

	if (TransferPosition > m_TotalBytesQueued)
	{
		TransferPosition = m_TotalBytesQueued;
	}

	if (OutTransferPosition)
	{
		*OutTransferPosition = TransferPosition;
	}

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioClient::SetPosition()
 *****************************************************************************
 *//*!
 * @brief
 * Get the current transfer position.
 */
AUDIOSTATUS 
CAudioClient::
SetPosition
(
	IN		ULONGLONG 	TransferPosition,
	IN		ULONGLONG 	QueuePosition
)
{
	m_TotalBytesQueued = QueuePosition;

	TransferPosition *= m_FifoFrameSize;

	TransferPosition /= m_ClientFrameSize;

	return m_DataPipe ? m_DataPipe->SetPosition(TransferPosition) : AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioClient::QueryControlSupport()
 *****************************************************************************
 *//*!
 * @brief
 */
AUDIOSTATUS 
CAudioClient::
QueryControlSupport
(
	IN		UCHAR	ControlSelector
)
{
	return m_DataPipe ? m_DataPipe->QueryControlSupport(ControlSelector) : AUDIOERR_BAD_REQUEST;
}

/*****************************************************************************
 * CAudioClient::WriteControl()
 *****************************************************************************
 *//*!
 * @brief
 */
AUDIOSTATUS 
CAudioClient::
WriteControl
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize
)
{
	return m_DataPipe ? m_DataPipe->WriteParameterBlock(RequestCode, ControlSelector, 0, ParameterBlock, ParameterBlockSize) : AUDIOERR_BAD_REQUEST;
}

/*****************************************************************************
 * CAudioClient::ReadControl()
 *****************************************************************************
 *//*!
 * @brief
 */
AUDIOSTATUS 
CAudioClient::
ReadControl
(
	IN		UCHAR	RequestCode,
	IN		UCHAR	ControlSelector,
	IN		UCHAR,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
	return m_DataPipe ? m_DataPipe->ReadParameterBlock(RequestCode, ControlSelector, 0, ParameterBlock, ParameterBlockSize, OutParameterBlockSize) : AUDIOERR_BAD_REQUEST;
}

/*****************************************************************************
 * CAudioClient::IsActive()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Determine if the client is active.
 * @param
 * <None>
 * @return
 * Returns TRUE if client is active, otherwise FALSE.
 */
BOOL
CAudioClient::
IsActive
(	void
)
{
	return m_IsActive;
}

/*****************************************************************************
 * CAudioClient::Service()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Service the audio interrupts.
 * @param
 * <None>
 * @return
 * <None>
 */
VOID
CAudioClient::
Service
(
	IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
)
{
	if (m_CallbackRoutine)
	{
		FifoWorkItem->BytesInFifoBuffer *= m_ClientFrameSize;

		FifoWorkItem->BytesInFifoBuffer /= m_FifoFrameSize;

		m_CallbackRoutine(m_CallbackData, 0, FifoWorkItem);
	}
}

void
CAudioClient::
VolumeMuteAdjustment
(
	IN  PUCHAR	pBuffer, 
	IN	ULONG	SamplesPerChannel
)    
{
	//edit yuanfen 
	//Not apply volume control to AC3 passthrough
//	if(m_requireSoftMaster)
    if(m_requireSoftMaster && m_Priority!= AUDIO_PRIORITY_HIGH)
    {
        BOOL volumeAdjust = FALSE;
        LONG masterVolume[AUDIO_CLIENT_MAX_CHANNEL];

        // Update cache values
        for(ULONG chCount = 0; chCount < m_FormatChannels; chCount++)
        {
            m_AudioDevice->GetMasterVolume(chCount, &masterVolume[chCount]);

            // Check if we need volume adjustment
            if(masterVolume[chCount] != MASTERVOL_0_DB)
            {
                volumeAdjust = TRUE;
            }
        }
        BOOL masterMute = FALSE;
        
        m_AudioDevice->GetMasterMute(&masterMute);

        if(masterMute)
        {
            // Muted
			//edit yuanfen
			//Remove distortion when mute
//			RtlZeroMemory(pBuffer, SamplesPerChannel * m_FormatChannels * m_SampleSize / 8);
            RtlZeroMemory(pBuffer, SamplesPerChannel * m_FormatChannels * m_BitResolution / 8);
        }
        else
        {
            // Only need to care about volume if not muted
            if(volumeAdjust)
            {
                KFLOATING_SAVE floatingSave;

                NTSTATUS ntStatus = KeSaveFloatingPointState(&floatingSave);
                if(NT_SUCCESS(ntStatus))
                {
                    ULONG totalSamples = m_FormatChannels * SamplesPerChannel;
                    float* pFloatBuf = static_cast<float*>(ExAllocatePoolWithTag(NonPagedPool, sizeof(float)*totalSamples, 'mdW'));

                    if(pFloatBuf)
                    {
                        // Convert samples to float for volume adjustment
					    //edit yuanfen
						//Remove distortion when adjusting volume
//                        ntStatus = ConvertIntData2Float(pBuffer, reinterpret_cast<BYTE*>(pFloatBuf), totalSamples, m_SampleSize);
						ntStatus = ConvertIntData2Float(pBuffer, reinterpret_cast<BYTE*>(pFloatBuf), totalSamples, m_BitResolution);

                        if(NT_SUCCESS(ntStatus))
                        {
                            float volumeAmp[AUDIO_CLIENT_MAX_CHANNEL];

                            // Convert the volume from dB to amplitude
                            for(ULONG chCount = 0; chCount < m_FormatChannels; chCount++)
                            {
                                if(masterVolume[chCount] == m_MasterVolumeMin[chCount])
                                {
                                    // Minimum. Set to 0.
                                    volumeAmp[chCount] = 0.0f;
                                }
                                else
                                {
                                    volumeAmp[chCount] = dB2Amp(masterVolume[chCount]);
							
                                }
                            }
                            
                            // The actual volume adjustment
                            for(ULONG sampleCount = 0; sampleCount < totalSamples; sampleCount = sampleCount + m_FormatChannels)
                            {
                                for(ULONG chCount = 0; chCount < m_FormatChannels; chCount++)
                                {
                                    pFloatBuf[sampleCount + chCount] = pFloatBuf[sampleCount + chCount] * volumeAmp[chCount];
                                }
                            }

                            // Convert the adjusted data and put back into original buffer
                            if(NT_SUCCESS(ntStatus))
                            {
								//edit yuanfen
								//Remove distortion when adjusting volume
//                                ntStatus = ConvertFloatData2Int(reinterpret_cast<BYTE*>(pFloatBuf), pBuffer, totalSamples, m_SampleSize);
								ntStatus = ConvertFloatData2Int(reinterpret_cast<BYTE*>(pFloatBuf), pBuffer, totalSamples, m_BitResolution);
                            }

						
                         
                        }

                        ExFreePool(pFloatBuf);
                    }
                }
                KeRestoreFloatingPointState(&floatingSave);
            }
        }
    }

    return;
}

NTSTATUS
CAudioClient::
ConvertIntData2Float
(
    IN  BYTE*   pBufferInteger,
    IN  BYTE*   pBufferFloat,
    IN  ULONG   numTotalSamples,
    IN  ULONG   bitsPerSample
)
{
    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;
    if(pBufferInteger && pBufferFloat && numTotalSamples && bitsPerSample)
    {
        if(bitsPerSample == 16 || bitsPerSample == 24 || bitsPerSample == 32)
        {
            switch(bitsPerSample)
            {
                case 16:
                {
                    //DbgPrint("ConvertIntData2Float - 16\n");
                    SHORT* pSrc = reinterpret_cast<SHORT*>(pBufferInteger);
                    float* pDst = reinterpret_cast<float*>(pBufferFloat);

                    for(ULONG idx = 0; idx < numTotalSamples; idx++)
                    {
                        pDst[idx] = (float)pSrc[idx];//Int16ToFloat16(pSrc[idx]);
                    }
                    ntStatus = STATUS_SUCCESS;
                    break;
                }
                case 24:
                case 32:              //NOTE:  32 bits have been converted to 24bit prior to this !!
                {
                    //DbgPrint("ConvertIntData2Float - 24\n");
                    // Headroom was reduce to optimized the conversion
                    LBYTE lByte;

                    BYTE* pSrc = pBufferInteger;
                    float* pDst = reinterpret_cast<float*>(pBufferFloat);

                    for(ULONG idx = 0; idx < numTotalSamples; idx++)
                    {
                        lByte.byteValue[0] = 0;
                        lByte.byteValue[1] = *pSrc; pSrc++;
                        lByte.byteValue[2] = *pSrc; pSrc++;
                        lByte.byteValue[3] = *pSrc; pSrc++;
                        pDst[idx] = (float)lByte.lValue;//Int24ToFloat16(lByte.lValue);

                    }
                    ntStatus = STATUS_SUCCESS;
                    break;
                }

                default:
                {
                    ntStatus = STATUS_INVALID_PARAMETER;
                    break;
                }
            }
        }
    }
    return ntStatus;
}

///////////////////////////////////////////////////////////////////////////////
// Protected Methods
///////////////////////////////////////////////////////////////////////////////
NTSTATUS
CAudioClient::
ConvertFloatData2Int
(
    IN  BYTE*   pBufferFloat,
    IN  BYTE*   pBufferInteger,
    IN  ULONG   numTotalSamples,
    IN  ULONG   bitsPerSample
)
{
    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;
    if(pBufferInteger && pBufferFloat && numTotalSamples && bitsPerSample)
    {
        if(bitsPerSample == 16 || bitsPerSample == 24 || bitsPerSample == 32)
        {
            switch(bitsPerSample)
            {
                case 16:
                {
                    //DbgPrint("ConvertFloatData2Int - 16\n");
                    float* pSrc = reinterpret_cast<float*>(pBufferFloat);
                    SHORT* pDst = reinterpret_cast<SHORT*>(pBufferInteger);

                    for(ULONG idx = 0; idx < numTotalSamples; idx++)
                    {
                        //pDst[idx] = (short)pSrc[idx];// Float16ToInt16(pSrc[idx]);
						pDst[idx] = (short)(__int64)pSrc[idx];// Float16ToInt16(pSrc[idx]);
                    }
                    ntStatus = STATUS_SUCCESS;
                    break;
                }
                case 24:
                case 32:             //NOTE: 32 bit has been converted to 24 bit prior to this
                {
                    //DbgPrint("ConvertFloatData2Int - 24\n");
                    LBYTE lByte;

                    float* pSrc = reinterpret_cast<float*>(pBufferFloat);
                    BYTE* pDst = pBufferInteger;
                    for(ULONG idx = 0; idx < numTotalSamples; idx++)
                    {
						//lByte.lValue = (long)pSrc[idx];//Float16ToInt24(pSrc[idx]);
                        lByte.lValue = (long)(__int64)pSrc[idx];//Float16ToInt24(pSrc[idx]);
                        *pDst = lByte.byteValue[1]; pDst++;
                        *pDst = lByte.byteValue[2]; pDst++;
                        *pDst = lByte.byteValue[3]; pDst++;
	                }
                    ntStatus = STATUS_SUCCESS;
                    break;
                }

            default:
                {
                    ntStatus = STATUS_INVALID_PARAMETER;
                    break;
                }
            }
        }
    }

    return ntStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioSynchPipe::~CAudioSynchPipe()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Destructor.
 */
CAudioSynchPipe::
~CAudioSynchPipe
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioSynchPipe::~CAudioSynchPipe]"));

	FreeResources();

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CAudioSynchPipe::Init()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioSynchPipe::
Init
(
	IN		PUSB_DEVICE				UsbDevice,
	IN		UCHAR					InterfaceNumber,
	IN		UCHAR					AlternateSetting,
	IN		USBD_PIPE_INFORMATION	PipeInformation
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_IsDeviceHighSpeed = m_UsbDevice->IsDeviceHighSpeed();

	m_InterfaceNumber = InterfaceNumber;

	m_AlternateSetting = AlternateSetting;

	m_PipeInformation = PipeInformation;

	m_Direction = USB_ENDPOINT_DIRECTION_IN(PipeInformation.EndpointAddress) ? AUDIO_INPUT : AUDIO_OUTPUT;

	m_PowerState = PowerDeviceD0;

	m_PipeState = AUDIO_SYNCH_PIPE_STATE_STOP;

	KeInitializeMutex(&m_PipeStateLock, 0);

	m_PendingIrps = 0;

	KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	PUSB_AUDIO_ENDPOINT_DESCRIPTOR EndpointDescriptor = NULL;

	m_UsbDevice->GetEndpointDescriptor(m_InterfaceNumber, m_AlternateSetting, m_PipeInformation.EndpointAddress, (PUSB_ENDPOINT_DESCRIPTOR *)&EndpointDescriptor);

	PRINT_USB_DESCRIPTOR(EndpointDescriptor);

	if (EndpointDescriptor->bLength >= sizeof(USB_AUDIO_ENDPOINT_DESCRIPTOR))
	{
		m_RefreshRate = 1 << EndpointDescriptor->bRefresh;
	}
	else
	{
		// BEGIN_HACK
		m_RefreshRate = 32; // Hardwired to 32 for Micro/HulaPod.
		// END_HACK
	}

	//DbgPrint("Refresh Rate = %d\n", m_RefreshRate);

	AcquireResources();

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioSynchPipe::PowerStateChange()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns AUDIOERR_SUCCESS if the power state changed.
 */
AUDIOSTATUS
CAudioSynchPipe::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up.
			PUSBD_INTERFACE_INFORMATION InterfaceInfo = NULL;
			
			m_UsbDevice->GetInterfaceInformation(m_InterfaceNumber, m_AlternateSetting, &InterfaceInfo);

			for (UCHAR i=0; i<InterfaceInfo->NumberOfPipes; i++)
			{
				if (m_PipeInformation.EndpointAddress == InterfaceInfo->Pipes[i].EndpointAddress)
				{
					m_PipeInformation = InterfaceInfo->Pipes[i];
				}
			}
		}
		else
		{
			// Power down.
		}

		m_PowerState = NewState;
	}

    return AUDIOERR_SUCCESS;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioSynchPipe::SetDataPipe()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioSynchPipe::
SetDataPipe
(
	IN		CAudioDataPipe *	DataPipe
)
{
	m_DataPipe = DataPipe;

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioSynchPipe::SetTransferParameters()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioSynchPipe::
SetTransferParameters
(
	IN		ULONG	SampleRate,
	IN		ULONG	FormatChannels,
	IN		ULONG	SampleSize
)
{
	if (m_Direction == AUDIO_OUTPUT)
	{
		// Queue the synch packet.
		KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

		m_FreeFifoWorkItemList.Lock();

		PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_FreeFifoWorkItemList.Pop(); 

		if (FifoWorkItem)
		{
			ULONG TransferSize = SampleRate / 1000;

			if (m_IsDeviceHighSpeed)
			{
				// 16.16 format
				TransferSize <<= 16;
				TransferSize |= ((SampleRate - ((SampleRate / 1000) * 1000)) << 16) / 1000;

				FifoWorkItem->TransferSize = 4;
			}
			else
			{
				// 10.14 format
				TransferSize <<= 14;
				TransferSize |= ((SampleRate - ((SampleRate / 1000) * 1000)) << 14) / 1000;

				FifoWorkItem->TransferSize = 3;
			}

			RtlCopyMemory(FifoWorkItem->FifoBuffer, &TransferSize, FifoWorkItem->TransferSize);

			InitializeFifoWorkItemUrb(FifoWorkItem);

			if (m_PipeState == AUDIO_SYNCH_PIPE_STATE_RUN)
			{
				InterlockedIncrement(&m_PendingIrps);

				m_PendingFifoWorkItemList.Lock();	

				m_PendingFifoWorkItemList.Put(FifoWorkItem);

				m_PendingFifoWorkItemList.Unlock();	

				NTSTATUS ntStatus = m_UsbDevice->RecycleIrp(FifoWorkItem->Urb, FifoWorkItem->Irp, IoCompletionRoutine, (PVOID)FifoWorkItem);
			}
			else
			{
				m_QueuedFifoWorkItemList.Lock();	

				m_QueuedFifoWorkItemList.Put(FifoWorkItem);

				m_QueuedFifoWorkItemList.Unlock();	
			}
		}

		m_FreeFifoWorkItemList.Unlock();
	}

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioSynchPipe::AcquireResources()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Acquire the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioSynchPipe::
AcquireResources
(	void
)
{
	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

    KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	if (m_Direction == AUDIO_OUTPUT)
	{
		audioStatus = PrepareFifoWorkItems(MAX_SYNCH_IRP, 1, FALSE, FALSE);
	}
	else
	{
		audioStatus = PrepareFifoWorkItems(MAX_SYNCH_IRP, 1, TRUE, FALSE);
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioSynchPipe::FreeResources()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Free the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioSynchPipe::
FreeResources
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioSynchPipe::FreeResources]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	m_FreeFifoWorkItemList.Lock();

	for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_FreeFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_FreeFifoWorkItemList.Pop())
	{
		FifoWorkItem->Destruct();
	}

	m_FreeFifoWorkItemList.Unlock();

	RtlZeroMemory(m_FifoWorkItem, sizeof(m_FifoWorkItem));

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioSynchPipe::PrepareFifoWorkItems()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Prepare FIFO work item to perform full speed isochronous transfer.
 * @details
 * This routine splits up a main isoch transfer request into one or
 * more sub requests as necessary.  Each isoch irp/urb pair can span at
 * most 255 packets.
 * 1. It creates a SUB_REQUEST_CONTEXT for each IRP/URB pair and
 *    attaches it to the main request irp.
 * 2. It intializes all of the sub request IRP/URB pairs.
 */
NTSTATUS
CAudioSynchPipe::
PrepareFifoWorkItems
(
    IN		ULONG   NumFifoWorkItems,
	IN		ULONG	Interval,
	IN		BOOL	Read,
	IN		BOOL	TransferAsap
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioSynchPipe::PrepareFifoWorkItems]"));

	// Each packet (frame) can hold this much info.
	ULONG PacketSize = m_PipeInformation.MaximumPacketSize;
	
    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioSynchPipe::PrepareBuffer] - PacketSize: %d", PacketSize));
	
    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioSynchPipe::PrepareBuffer] - Interval: %d", m_PipeInformation.Interval));

	// There is an inherent limit on the number of packets that can be
    // passed down the stack with each irp/urb pair (255)
    //
    // If the number of required packets is > 255, we shall create
    // "(required-packets / NotificationInterval) [+ 1]" number of irp/urb pairs.
    //
    // Each irp/urb pair transfer is also called a stage transfer.
    //
	ULONG StageSize = PacketSize * Interval;

    //_DbgPrintF(DEBUGLVL_BLAB,("[CAudioSynchPipe::PrepareBuffer] - StageSize: %d, NumFifoWorkItems: %d", StageSize, NumFifoWorkItems));
	
	// Allocate the sub requests.
	for (ULONG i = 0; i < NumFifoWorkItems; i++)
	{
		// The following outer scope variables are updated during each
		// iteration of the loop:  virtualAddress, TotalLength, stageSize

		// For every stage of transfer we need to do the following
		// tasks:
		//
		// 1. Allocate a FIFO work item.
		// 2. Allocate a sub request IRP.
		// 3. Allocate a sub request URB.
		// 4. Initialize the above allocations.
		//

		// 1. Allocate a FIFO work item.
		PAUDIO_FIFO_WORK_ITEM FifoWorkItem = new(NonPagedPool) AUDIO_FIFO_WORK_ITEM();

		if (FifoWorkItem == NULL)
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}

		if (NT_SUCCESS(ntStatus))
		{
			m_FifoWorkItem[i] = FifoWorkItem;

			m_FreeFifoWorkItemList.Put(FifoWorkItem);

			FifoWorkItem->Context = this;
			FifoWorkItem->Read = Read;

			// 2. Allocate a sub request irp
			ntStatus = m_UsbDevice->CreateIrp(&FifoWorkItem->Irp);
		}

		if (NT_SUCCESS(ntStatus))
		{
			// 3. Allocate a sub request URB.

			ULONG NumberOfPackets = (StageSize + PacketSize - 1) / PacketSize;

			//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareBuffer] - NumberOfPackets: %d for IRP/URB pair %d.", NumberOfPackets, i));

			ASSERT(NumberOfPackets <= Interval);

			FifoWorkItem->NumberOfPackets = NumberOfPackets;
			FifoWorkItem->PacketSize = PacketSize;

			ULONG UrbSize = GET_ISO_URB_SIZE(NumberOfPackets);

			FifoWorkItem->Urb = (PURB)ExAllocatePoolWithTag(NonPagedPool, UrbSize, 'mdW');

			if (FifoWorkItem->Urb == NULL)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			}
		}

		if (NT_SUCCESS(ntStatus))
		{
			// 4. The buffer.
			FifoWorkItem->FifoBuffer = PUCHAR(ExAllocatePoolWithTag(NonPagedPool, StageSize, 'mdW'));
			FifoWorkItem->FifoBufferSize = StageSize;
			FifoWorkItem->BytesInFifoBuffer = 0;
			FifoWorkItem->TransferSize = 0;
			FifoWorkItem->TransferAsap = TransferAsap;
			FifoWorkItem->Flags = 0x1;
		}

		if (!NT_SUCCESS(ntStatus))
		{
			break;
		}
	}

	if (!NT_SUCCESS(ntStatus))
	{
		for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_FreeFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_FreeFifoWorkItemList.Pop())
		{
			FifoWorkItem->Destruct();
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CAudioSynchPipe::InitializeFifoWorkItemUrb()
 *****************************************************************************
 *//*!
 * @brief
 * This routine initializes the FIFO work item URB.
 */
NTSTATUS 
CAudioSynchPipe::
InitializeFifoWorkItemUrb
(
	IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
)
{
	PURB Urb = FifoWorkItem->Urb;

	ULONG UrbSize = GET_ISO_URB_SIZE(FifoWorkItem->NumberOfPackets);

 	// Initialize the sub request urb.
	RtlZeroMemory(Urb, UrbSize);

    Urb->UrbIsochronousTransfer.Hdr.Length = (USHORT)UrbSize;
    Urb->UrbIsochronousTransfer.Hdr.Function = URB_FUNCTION_ISOCH_TRANSFER;
    Urb->UrbIsochronousTransfer.PipeHandle = m_PipeInformation.PipeHandle;

	if (FifoWorkItem->Read)
	{
		Urb->UrbIsochronousTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
	}
	else
	{
		Urb->UrbIsochronousTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_OUT;
	}

	Urb->UrbIsochronousTransfer.TransferBufferLength = FifoWorkItem->TransferSize;
	Urb->UrbIsochronousTransfer.TransferBuffer = FifoWorkItem->FifoBuffer;

	if (!FifoWorkItem->TransferAsap)
	{
		Urb->UrbIsochronousTransfer.TransferFlags &= ~USBD_START_ISO_TRANSFER_ASAP;

		//This is a way to set the start frame and NOT specify ASAP flag.
		ULONG FrameNumber; m_UsbDevice->GetCurrentFrameNumber(&FrameNumber);
		
		if (m_StartFrameNumber <= FrameNumber)
		{
			m_StartFrameNumber = FrameNumber + MIN_AUDIO_START_FRAME_OFFSET;
		}

		Urb->UrbIsochronousTransfer.StartFrame = m_StartFrameNumber + m_RefreshRate;

		m_StartFrameNumber += m_RefreshRate;
	}
	else
	/**/
	//
	// when the client driver sets the ASAP flag, it basically
	// guarantees that it will make data available to the HC
	// and that the HC should transfer it in the next transfer frame
	// for the endpoint.(The HC maintains a next transfer frame
	// state variable for each endpoint). By resetting the pipe,
	// we make the pipe as virgin. If the data does not get to the HC
	// fast enough, the USBD_ISO_PACKET_DESCRIPTOR - Status is
	// USBD_STATUS_BAD_START_FRAME on uhci. On ohci it is 0xC000000E.
	//
	{
		Urb->UrbIsochronousTransfer.TransferFlags |= USBD_START_ISO_TRANSFER_ASAP;
	}

	Urb->UrbIsochronousTransfer.NumberOfPackets = FifoWorkItem->NumberOfPackets;

	// Set the offsets for every packet for reads/writes
	if (FifoWorkItem->Read)
	{
		FifoWorkItem->BytesInFifoBuffer = 0;

		ULONG Offset = 0, StageSize = FifoWorkItem->TransferSize;

		for (ULONG i=0; i<FifoWorkItem->NumberOfPackets; i++)
		{
			Urb->UrbIsochronousTransfer.IsoPacket[i].Offset = Offset;

			// For input operation, length is set to whatever the device supplies.
			Urb->UrbIsochronousTransfer.IsoPacket[i].Length = 0;

			if (StageSize > FifoWorkItem->PacketSize)
			{
				Offset += FifoWorkItem->PacketSize;
				StageSize -= FifoWorkItem->PacketSize;
			}
			else
			{
				Offset += StageSize;
				StageSize  = 0;
			}
		}
	}
	else
	{
		ULONG Offset = 0, StageSize = FifoWorkItem->TransferSize;
		
		FifoWorkItem->BytesInFifoBuffer = FifoWorkItem->TransferSize;

		for (ULONG i=0; i<FifoWorkItem->NumberOfPackets; i++)
		{
			Urb->UrbIsochronousTransfer.IsoPacket[i].Offset = Offset;

			if (StageSize > FifoWorkItem->PacketSize)
			{
				Urb->UrbIsochronousTransfer.IsoPacket[i].Length = FifoWorkItem->PacketSize;

				Offset += FifoWorkItem->PacketSize;
				StageSize -= FifoWorkItem->PacketSize;
			}
			else
			{
				FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length = StageSize;
				
				Offset += StageSize;
				StageSize  = 0;

				ASSERT(Offset == (Urb->UrbIsochronousTransfer.IsoPacket[i].Length + Urb->UrbIsochronousTransfer.IsoPacket[i].Offset));
			}
		}
	}

	FifoWorkItem->SkipPackets = 0;

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CAudioSynchPipe::Start()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Start the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioSynchPipe::
Start
(
	IN		BOOL	SynchronizeStart,
	IN		ULONG	StartFrameNumber
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioSynchPipe::Start]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_PipeState != AUDIO_SYNCH_PIPE_STATE_RUN)
	{
		m_PipeState = AUDIO_SYNCH_PIPE_STATE_RUN;

		if (SynchronizeStart)
		{
			m_StartFrameNumber = StartFrameNumber;
		}
		else
		{
			m_UsbDevice->GetCurrentFrameNumber(&m_StartFrameNumber);

			m_StartFrameNumber += MIN_AUDIO_START_FRAME_OFFSET;
		}

		if (m_Direction == AUDIO_INPUT)
		{
			m_FreeFifoWorkItemList.Lock();

			m_QueuedFifoWorkItemList.Lock();	

			for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_FreeFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_FreeFifoWorkItemList.Pop())
			{
				FifoWorkItem->TransferSize = FifoWorkItem->FifoBufferSize;

				InitializeFifoWorkItemUrb(FifoWorkItem);

				FifoWorkItem->SkipPackets = 1;

				m_QueuedFifoWorkItemList.Put(FifoWorkItem);
			}

			m_QueuedFifoWorkItemList.Unlock();	

			m_FreeFifoWorkItemList.Unlock();
		}

		audioStatus = StartTransfer();
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioSynchPipe::Stop()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Start the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioSynchPipe::
Stop
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioSynchPipe::Stop]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	if (m_PipeState == AUDIO_SYNCH_PIPE_STATE_RUN)
	{
		m_PipeState = AUDIO_SYNCH_PIPE_STATE_STOP;

		CancelTransfer();
	}
	else
	{
		m_PipeState = AUDIO_SYNCH_PIPE_STATE_STOP;

		m_FreeFifoWorkItemList.Lock();

		m_QueuedFifoWorkItemList.Lock();	

		for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_QueuedFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_QueuedFifoWorkItemList.Pop())
		{
			m_FreeFifoWorkItemList.Put(FifoWorkItem);
		}

		m_QueuedFifoWorkItemList.Unlock();	

		m_FreeFifoWorkItemList.Unlock();
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioSynchPipe::StartTransfer()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Performs isochronous transfer.
 */
NTSTATUS
CAudioSynchPipe::
StartTransfer
(	void
)
{
    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioSynchPipe::StartTransfer]"));

	m_UsbDevice->ResetPipe(m_PipeInformation.PipeHandle);

	m_PendingIrps = 0;

	KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	m_QueuedFifoWorkItemList.Lock();

	for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_QueuedFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_QueuedFifoWorkItemList.Pop())
	{
		InterlockedIncrement(&m_PendingIrps);

		m_PendingFifoWorkItemList.Lock();	

		m_PendingFifoWorkItemList.Put(FifoWorkItem);

		m_PendingFifoWorkItemList.Unlock();	

		NTSTATUS ntStatus = m_UsbDevice->RecycleIrp(FifoWorkItem->Urb, FifoWorkItem->Irp, IoCompletionRoutine, (PVOID)FifoWorkItem);

		//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioSynchPipe::StartTransfer] - ntStatus: 0x%x", ntStatus));
	}

	//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioSynchPipe::StartTransfer] - Pending Irps: %d", m_PendingIrps));

	m_QueuedFifoWorkItemList.Unlock();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CAudioSynchPipe::CancelTransfer()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Free the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CAudioSynchPipe::
CancelTransfer
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioSynchPipe::CancelTransfer]"));

	m_UsbDevice->AbortPipe(m_PipeInformation.PipeHandle);

	m_PendingFifoWorkItemList.Lock();

	BOOL PendingIrp = m_PendingFifoWorkItemList.Count();

	m_PendingFifoWorkItemList.Unlock();/**/

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioSynchPipe::CancelTransfer] - Pending Irps: %d", m_PendingIrps));
	
	if (PendingIrp)
	{
		// Cancel the FIFO work item. Safe to touch these Irps because the completion 
		// routine always returns STATUS_MORE_PRCESSING_REQUIRED.	
		for (ULONG i=0; i<MAX_SYNCH_IRP; i++)
		{
			if (m_FifoWorkItem[i])
			{
				if (m_FifoWorkItem[i]->Irp)
				{
					IoCancelIrp(m_FifoWorkItem[i]->Irp);
				}
			}
		}

		// Wait for the queued FIFO work item Irps to complete.
		KeWaitForSingleObject(&m_NoPendingIrpEvent, Executive, KernelMode, FALSE, NULL);
	}

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioSynchPipe::CancelTransfer] - Done"));
}

/*****************************************************************************
 * CAudioSynchPipe::ClearFifo()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Clear the stale data from the FIFO.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioSynchPipe::
ClearFifo
(	void
)
{
	PAGED_CODE();

	ASSERT(m_Direction == AUDIO_INPUT);

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioSynchPipe::ClearFifo]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	if (m_PipeState != AUDIO_SYNCH_PIPE_STATE_RUN)
	{
		// Clear the FIFO...
		KeClearEvent(&m_NoPendingIrpEvent);

		audioStatus = Start(FALSE, 0);

		if (AUDIO_SUCCESS(audioStatus))
		{
			LARGE_INTEGER TimeOut; TimeOut.QuadPart = -10 * 10000; // 2ms

			KeWaitForSingleObject(&m_NoPendingIrpEvent, Executive, KernelMode, TRUE, &TimeOut);

			Stop();
		}
	}

	return audioStatus;
}

/*****************************************************************************
 * CAudioSynchPipe::ProcessFifoWorkItem()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Process the FIFO work item.
 * @param
 * FifoWorkItem FIFO work item to service.
 * @return
 * Returns the status of the FIFO work item.
 */
NTSTATUS
CAudioSynchPipe::
ProcessFifoWorkItem
(
	IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
)
{
	NTSTATUS ntStatus = FifoWorkItem->Irp->IoStatus.Status;

    if (!NT_SUCCESS(ntStatus)) 
	{
        _DbgPrintF(DEBUGLVL_BLAB, ("Isochronous read/write Irp failed with status = 0x%x", ntStatus));
    }

    USBD_STATUS usbdStatus = FifoWorkItem->Urb->UrbHeader.Status;

    if (!USBD_SUCCESS(usbdStatus)) 
	{
        _DbgPrintF(DEBUGLVL_BLAB, ("Urb failed with status = 0x%x", usbdStatus));
    }

	if (NT_SUCCESS(ntStatus))
	{
		if (FifoWorkItem->Read) 
		{
			// Check each of the urb packets.
			for (ULONG i = 0; i < FifoWorkItem->Urb->UrbIsochronousTransfer.NumberOfPackets; i++) 
			{
				FifoWorkItem->BytesInFifoBuffer += FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length;

				/*if (!USBD_SUCCESS(FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Status))
				_DbgPrintF(DEBUGLVL_BLAB, ("IsoPacket[%d].Length = %d IsoPacket[%d].Status = %X",
											i,
											FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length,
											i,
											FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Status));/**/
			}
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CAudioSynchPipe::Service()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Service the synch interrupts.
 * @param
 * FifoWorkItem FIFO work item to service.
 * @return
 * <None>
 */
VOID
CAudioSynchPipe::
Service
(
	IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
)
{
	NTSTATUS ntStatus = ProcessFifoWorkItem(FifoWorkItem);

	if ((ntStatus == STATUS_CANCELLED) || (ntStatus == STATUS_DEVICE_NOT_CONNECTED) || (ntStatus == STATUS_NO_SUCH_DEVICE))
	{
		//_DbgPrintF(DEBUGLVL_BLAB, ("Isoch Irp cancelled/device removed: %d\n", m_PendingIrps));
		m_PendingFifoWorkItemList.Lock();
		m_PendingFifoWorkItemList.Remove(FifoWorkItem);
		m_PendingFifoWorkItemList.Unlock();

		m_FreeFifoWorkItemList.Lock();
		m_FreeFifoWorkItemList.Put(FifoWorkItem);
		m_FreeFifoWorkItemList.Unlock();

		// This is the last irp to complete with this erroneous value. 
		// Signal an event.
        if (InterlockedDecrement(&m_PendingIrps) == 0) 
		{
			KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
        }
	}
	else if (m_PipeState == AUDIO_SYNCH_PIPE_STATE_STOP)
	{
		//_DbgPrintF(DEBUGLVL_BLAB, ("Pipe stopped: %d\n", m_PendingIrps));
		m_PendingFifoWorkItemList.Lock();
		m_PendingFifoWorkItemList.Remove(FifoWorkItem);
		m_PendingFifoWorkItemList.Unlock();

		m_FreeFifoWorkItemList.Lock();
		m_FreeFifoWorkItemList.Put(FifoWorkItem);
		m_FreeFifoWorkItemList.Unlock();

		// This is the last irp to complete with this erroneous value. 
		// Signal an event.
        if (InterlockedDecrement(&m_PendingIrps) == 0) 
		{
			KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
        }
	}
	else
	{
		if (FifoWorkItem->Read)
		{
			for (ULONG i = 0; i < FifoWorkItem->Urb->UrbIsochronousTransfer.NumberOfPackets; i++) 
			{
				//DbgPrint("IsoPacket[%d] Status = %x, Length = %d\n", i, FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Status, FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length);
				if (FifoWorkItem->SkipPackets) continue;

				if (FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length)
				{
					ULONG TransferRate = 0, Whole, Fraction;

					if (m_IsDeviceHighSpeed)
					{
						RtlCopyMemory(&TransferRate, FifoWorkItem->FifoBuffer + FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Offset, 4);
					
						Whole = TransferRate>>16; Fraction = ((TransferRate & 0xFFFF) * 1000);
					}
					else
					{
						RtlCopyMemory(&TransferRate, FifoWorkItem->FifoBuffer + FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Offset, 3);

						Whole = TransferRate>>14; Fraction = ((TransferRate & 0x3FFF) * 1000) << 2; // change to 16-bit base.
					}

					//DbgPrint("TransferRate: %d, Fraction: %d\n", Whole, Fraction);

					if (m_DataPipe)
					{
						m_DataPipe->OnSampleRateSynchronization(Whole, Fraction);
					}
				}
			}

			FifoWorkItem->TransferSize = FifoWorkItem->FifoBufferSize;

			InitializeFifoWorkItemUrb(FifoWorkItem);

			NTSTATUS ntStatus = m_UsbDevice->RecycleIrp(FifoWorkItem->Urb, FifoWorkItem->Irp, IoCompletionRoutine, (PVOID)FifoWorkItem);
		}
		else
		{
			m_PendingFifoWorkItemList.Lock();
			m_PendingFifoWorkItemList.Remove(FifoWorkItem);
			m_PendingFifoWorkItemList.Unlock();

			m_FreeFifoWorkItemList.Lock();
			m_FreeFifoWorkItemList.Put(FifoWorkItem);
			m_FreeFifoWorkItemList.Unlock();

			if (InterlockedDecrement(&m_PendingIrps) == 0) 
			{
				KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
			}
		}
	}
}

/*****************************************************************************
 * CAudioSynchPipe::IoCompletionRoutine()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
NTSTATUS
CAudioSynchPipe::
IoCompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp,
    IN		PVOID			Context
)
{
    //_DbgPrintF(DEBUGLVL_BLAB,("[CAudioSynchPipe::IoCompletionRoutine]"));

	PAUDIO_FIFO_WORK_ITEM FifoWorkItem = (PAUDIO_FIFO_WORK_ITEM)Context;

	CAudioSynchPipe * that = (CAudioSynchPipe*)FifoWorkItem->Context;

	that->Service(FifoWorkItem);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioDataPipe::~CAudioDataPipe()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Destructor.
 */
CAudioDataPipe::
~CAudioDataPipe
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioDataPipe::~CAudioDataPipe]"));


	#ifdef MICROSOFT_USB_OHCI_BUG_WORKAROUND
	if (m_LastRecordBuffer)
	{
		ExFreePool(m_LastRecordBuffer);
	}
	#endif // MICROSOFT_USB_OHCI_BUG_WORKAROUND

	FreeResources();

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CAudioDataPipe::Init()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioDataPipe::
Init
(
	IN		PUSB_DEVICE				UsbDevice,
	IN		UCHAR					InterfaceNumber,
	IN		UCHAR					AlternateSetting,
	IN		USBD_PIPE_INFORMATION	PipeInformation
)
{
	PAGED_CODE();

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_IsDeviceHighSpeed = m_UsbDevice->IsDeviceHighSpeed();

	m_InterfaceNumber = InterfaceNumber;

	m_AlternateSetting = AlternateSetting;

	m_PipeInformation = PipeInformation;

	m_Direction = USB_ENDPOINT_DIRECTION_IN(PipeInformation.EndpointAddress) ? AUDIO_INPUT : AUDIO_OUTPUT;

	m_PowerState = PowerDeviceD0;

	m_PipeState = AUDIO_DATA_PIPE_STATE_STOP;

	KeInitializeMutex(&m_PipeStateLock, 0);

	m_PendingIrps = 0;

	KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	PUSB_AUDIO_ENDPOINT_DESCRIPTOR EndpointDescriptor = NULL;

	m_UsbDevice->GetEndpointDescriptor(m_InterfaceNumber, m_AlternateSetting, m_PipeInformation.EndpointAddress, (PUSB_ENDPOINT_DESCRIPTOR *)&EndpointDescriptor);

	m_SynchronizationType = (EndpointDescriptor->bmAttributes & 0x0C)>>2;

	PRINT_USB_DESCRIPTOR(EndpointDescriptor);

	if (EndpointDescriptor->bLength >= sizeof(USB_AUDIO_ENDPOINT_DESCRIPTOR))
	{
		_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::Init] - bRefresh: 0x%x, bSynchAddress: 0x%x", EndpointDescriptor->bRefresh, EndpointDescriptor->bSynchAddress));
	}

	PUSB_AUDIO_CS_AS_AUDIO_ENDPOINT_DESCRIPTOR CsAsEndpointDescriptor = NULL;

	m_UsbDevice->GetClassEndpointDescriptor(m_InterfaceNumber, m_AlternateSetting, m_PipeInformation.EndpointAddress, USB_AUDIO_CS_ENDPOINT, (PUSB_ENDPOINT_DESCRIPTOR *)&CsAsEndpointDescriptor);

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::Init] - CS-AS: bmAttributes: 0x%x", CsAsEndpointDescriptor->bmAttributes));
	
	m_Attributes = CsAsEndpointDescriptor->bmAttributes;

	m_NumberOfPacketsPerMs = (m_IsDeviceHighSpeed) ? (8 / (1 << (m_PipeInformation.Interval - 1))) : m_PipeInformation.Interval;

	RestoreParameterBlock();

	#if DBG
	if (m_Attributes & USB_AUDIO_DATA_EP_ATTR_SAMPLING_FREQUENCY)
	{
		ULONG SamplingRate = 0;

		NTSTATUS ntStatus = m_UsbDevice->ControlClassEndpointCommand
							(
								REQUEST_CUR | 0x80,
								USB_AUDIO_DATA_EP_ATTR_SAMPLING_FREQUENCY<<8,
								m_PipeInformation.EndpointAddress,
								&SamplingRate,
								3,
								NULL,
								TRUE
							);

		_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::Init] - ntStatus: 0x%x, SamplingRate: %d", ntStatus, SamplingRate));
	}
	#endif // DBG

	AcquireResources((m_Direction == AUDIO_OUTPUT) ? MAX_AUDIO_OUTPUT_IRP : MAX_AUDIO_INPUT_IRP);

	// Determine if the input pipe of this device is broken or not. Those broken
	// devices require the driver to skip the first N initial packets returned
	// by the device because the packets contain stale data.
	if (m_Direction == AUDIO_INPUT)
	{
		PUSB_DEVICE_DESCRIPTOR UsbDeviceDescriptor = NULL;

		m_UsbDevice->GetDeviceDescriptor(&UsbDeviceDescriptor);

		if (UsbDeviceDescriptor)
		{
			if (((UsbDeviceDescriptor->idVendor == 0x041E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F02/*Micropod*/)) ||
				((UsbDeviceDescriptor->idVendor == 0x041E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F0A/*MicroPre*/)) ||
				((UsbDeviceDescriptor->idVendor == 0x041E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F0B/*Itey*/)) ||
			    ((UsbDeviceDescriptor->idVendor == 0x041E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F04/*Hulapod*/)))
			{
				m_NumberOfPacketsToSkip = 5; // 5 packets to skip for Micro/Hulapod.

				#ifdef MICROSOFT_USB_EHCI_BUG_WORKAROUND
				if ((m_NumberOfPacketsPerMs == 2) || (m_NumberOfPacketsPerMs == 4))
				{
					// Indicates that the packet length is embedded in the stream itself.
					m_UseEmbeddedPacketLength = TRUE;
				}
				#endif // MICROSOFT_USB_EHCI_BUG_WORKAROUND
			}
		}
	}

	#ifdef MICROSOFT_USB_OHCI_BUG_WORKAROUND
	if (!m_IsDeviceHighSpeed)
	{
		m_LastRecordBuffer = ExAllocatePoolWithTag(NonPagedPool, m_PipeInformation.MaximumPacketSize, 'mdW');
	}
	#endif // MICROSOFT_USB_OHCI_BUG_WORKAROUND

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioDataPipe::InterfaceNumber()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
UCHAR
CAudioDataPipe::
InterfaceNumber
(	void
)
{
	PAGED_CODE();

	return m_InterfaceNumber;
}

/*****************************************************************************
 * CAudioDataPipe::AlternateSetting()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
UCHAR
CAudioDataPipe::
AlternateSetting
(	void
)
{
	PAGED_CODE();

	return m_AlternateSetting;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioDataPipe::PipeInformation()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
PUSBD_PIPE_INFORMATION
CAudioDataPipe::
PipeInformation
(	void
)
{
	return &m_PipeInformation;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioDataPipe::SynchronizationType()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
UCHAR
CAudioDataPipe::
SynchronizationType
(	void
)
{
	return m_SynchronizationType;
}

/*****************************************************************************
 * CAudioDataPipe::PowerStateChange()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns AUDIOERR_SUCCESS if the power state changed.
 */
AUDIOSTATUS
CAudioDataPipe::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up.
			PUSBD_INTERFACE_INFORMATION InterfaceInfo = NULL;
			
			m_UsbDevice->GetInterfaceInformation(m_InterfaceNumber, m_AlternateSetting, &InterfaceInfo);

			for (UCHAR i=0; i<InterfaceInfo->NumberOfPipes; i++)
			{
				if (m_PipeInformation.EndpointAddress == InterfaceInfo->Pipes[i].EndpointAddress)
				{
					m_PipeInformation = InterfaceInfo->Pipes[i];
				}
			}

			RestoreParameterBlock(&m_ParameterBlock, sizeof(AUDIO_DATA_PIPE_PARAMETER_BLOCK));	
		}
		else
		{
			// Power down.
		}

		m_PowerState = NewState;
	}

    return AUDIOERR_SUCCESS;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioDataPipe::SetTransferParameters()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioDataPipe::
SetTransferParameters
(
	IN		ULONG	SampleRate,
	IN		ULONG	FormatChannels,
	IN		ULONG	SampleSize,
	IN		ULONG	NumberOfFifoBuffers
)
{
	// Sample rate.
	m_SampleRate = SampleRate;

	// Number of bytes per sample.
	m_SampleFrameSize = FormatChannels * (SampleSize / 8);

	// The average number of sample frames per ms.
	m_FfPerPacketInterval.Whole = SampleRate / 1000 / m_NumberOfPacketsPerMs;

	// Ff.Fraction = 65536 * SampleFraction * 1000
	m_FfPerPacketInterval.Fraction = ((SampleRate - ((SampleRate / 1000 ) * 1000)) / m_NumberOfPacketsPerMs) << 16;

	// Running sample frames fraction.
	m_RunningFfFraction = 0;

	// Adjust the number of fifo buffers.
	if (NumberOfFifoBuffers)
	{
		FreeResources();

		AcquireResources(NumberOfFifoBuffers);
	}

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioDataPipe::GetTransferSizeInFrames()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
ULONG
CAudioDataPipe::
GetTransferSizeInFrames
(
	IN		ULONG	NumberOfTransfers,
	IN		BOOL	UpdateRunningFfFraction
)
{
	ULONG RunningFfFraction = m_RunningFfFraction + (m_FfPerPacketInterval.Fraction * NumberOfTransfers);

	ULONG TransferSizeInFrames = (m_FfPerPacketInterval.Whole * NumberOfTransfers) + (RunningFfFraction / (1000 << 16));

	if (UpdateRunningFfFraction)
	{
		m_RunningFfFraction = RunningFfFraction % (1000 << 16);
	}

	return TransferSizeInFrames;
}

/*****************************************************************************
 * CAudioDataPipe::OnSampleRateSynchronization()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
VOID 
CAudioDataPipe::
OnSampleRateSynchronization
(
	IN		ULONG	FfWhole,
	IN		ULONG	FfFraction
)
{
	if ((FfWhole != m_FfPerPacketInterval.Whole) || (FfFraction != m_FfPerPacketInterval.Fraction))
	{
		//DbgPrint("%d.%03d ", FfWhole, FfFraction);
		//DbgPrint("%d.%04d ", FfWhole, (10*FfFraction)>>16);
		//DbgPrint("Old sample rate: %d.%04d\n", m_FfPerPacketInterval.Whole, (10*m_FfPerPacketInterval.Fraction)>>16);
		//DbgPrint("New sample rate: %d.%04d\n", FfWhole, (10*FfFraction)>>16);

		if ((FfWhole * m_SampleFrameSize) > m_PipeInformation.MaximumPacketSize)
		{
			//DbgPrint("---Bad sync packet--- ignoring it...\n");
			return;
		}
	}

	// The number of sample frames per packet interval.
	m_FfPerPacketInterval.Whole = FfWhole;

	m_FfPerPacketInterval.Fraction = FfFraction;
}

/*****************************************************************************
 * CAudioDataPipe::SetCallbackClient()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioDataPipe::
SetCallbackClient
(
	IN		CAudioClient *	Client
)
{
	m_Client = Client;

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioDataPipe::AcquireResources()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Acquire the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioDataPipe::
AcquireResources
(
	IN		ULONG	NumberOfIrps
)
{
	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

    KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	if (m_Direction == AUDIO_OUTPUT)
	{
		audioStatus = PrepareFifoWorkItems((NumberOfIrps >= MAX_AUDIO_OUTPUT_IRP) ? MAX_AUDIO_OUTPUT_IRP : NumberOfIrps, FALSE, FALSE);
	}
	else
	{
		audioStatus = PrepareFifoWorkItems((NumberOfIrps >= MAX_AUDIO_INPUT_IRP) ? MAX_AUDIO_INPUT_IRP : NumberOfIrps, TRUE, FALSE);
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioDataPipe::FreeResources()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Free the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioDataPipe::
FreeResources
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioDataPipe::FreeResources]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	m_FreeFifoWorkItemList.Lock();

	for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_FreeFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_FreeFifoWorkItemList.Pop())
	{
		FifoWorkItem->Destruct();
	}

	m_FreeFifoWorkItemList.Unlock();

	RtlZeroMemory(m_FifoWorkItem, sizeof(m_FifoWorkItem));

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioDataPipe::FlushBuffer()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioDataPipe::
FlushBuffer
(	void
)
{
	//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::FlushBuffer]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	if (m_Direction == AUDIO_OUTPUT)
	{
		m_Client->Lock();

		if (m_SynchronizeStart)
		{
			ULONG FrameNumber = m_UsbDevice->GetSyncFrameNumber();
		
			if (m_StartFrameNumber < FrameNumber)
			{
				ULONG Delta = FrameNumber - m_StartFrameNumber;

				// See if it is way too late..
				if (Delta >= 100)
				{
					//DbgPrint("[%d] Bingo..: %d, %d, %d", (m_Direction == AUDIO_INPUT), m_StartFrameNumber, FrameNumber, Delta);

					if (!m_ResyncRequested)
					{
						m_Client->RequestDriverResync();

						m_ResyncRequested = TRUE;
					}
				}

				// See if it is late..
				if (Delta > AUDIO_CLIENT_INPUT_BUFFERSIZE)
				{
					ULONG FramesToRemove = m_Client->GetNumQueuedFrames() % (m_SampleRate / 1000);
					
					// Remove the extra frames left over from the previous buffer.
					if (FramesToRemove) 
					{
						m_Client->RemoveFramesFromFifo(NULL, FramesToRemove);
					}

					//DbgPrint("[%d] Bingo..: %d, %d, %d, %d", (m_Direction == AUDIO_INPUT), m_StartFrameNumber, FrameNumber, Delta, FramesToRemove);

					// Adjust the start frame so that playback can catch up with record.
					m_StartFrameNumber = FrameNumber - AUDIO_CLIENT_INPUT_BUFFERSIZE;
				}
				else
				{
					m_AutoResyncCount++;

					// See how many times the driver tries to resync the playback with the record. 
					// If it is too many (50), then issue a resync request since the driver is 
					// starving, ie not enough data to feed the playback stream.
					if ((m_AutoResyncCount >= 50) && (Delta < (AUDIO_CLIENT_INPUT_BUFFERSIZE-5)))
					{
						m_Client->RequestDriverResync();
					}
				}
			}
			else
			{
				m_AutoResyncCount = 0;
			}
		}

		while (m_Client->GetNumQueuedFrames() >= GetTransferSizeInFrames(m_NumberOfPacketsPerMs, FALSE))
		{
			m_FreeFifoWorkItemList.Lock();

			PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_FreeFifoWorkItemList.Pop();

			m_FreeFifoWorkItemList.Unlock();

			if (FifoWorkItem)
			{
				InitializeFifoWorkItemUrb(FifoWorkItem);

				FifoWorkItem->BytesInFifoBuffer = 0;

				for (ULONG i = 0; i < FifoWorkItem->Urb->UrbIsochronousTransfer.NumberOfPackets; i++) 
				{
					// Calculate the packet size.
					ULONG PacketSize = (GetTransferSizeInFrames(1, TRUE) * m_SampleFrameSize) + m_PacketDeficitInBytes;

					//ASSERT(PacketSize <= FifoWorkItem->PacketSize);
					if (PacketSize > FifoWorkItem->PacketSize)
					{
						m_PacketDeficitInBytes = PacketSize - FifoWorkItem->PacketSize;

						PacketSize = FifoWorkItem->PacketSize;
					}
					else
					{
						m_PacketDeficitInBytes = 0;
					}

					// Setup the FIFO work item.
					FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Offset = (i) ? (FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i-1].Offset + FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i-1].Length) : 0;

					FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length = m_Client->RemoveFramesFromFifo(FifoWorkItem->FifoBuffer + FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Offset, PacketSize / m_SampleFrameSize) * m_SampleFrameSize;

					m_PacketDeficitInBytes += (FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length - PacketSize);

					//if (FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length != PacketSize)
					//{
					//	DbgPrint("Expected: %d, Actual: %d\n", PacketSize, FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length);
					//}

					FifoWorkItem->BytesInFifoBuffer += FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length;
				}

				FifoWorkItem->Urb->UrbIsochronousTransfer.TransferBufferLength = FifoWorkItem->BytesInFifoBuffer;

				if (m_PipeState == AUDIO_DATA_PIPE_STATE_RUN)
				{
					KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

					InterlockedIncrement(&m_PendingIrps);

					m_PendingFifoWorkItemList.Lock();	

					m_PendingFifoWorkItemList.Put(FifoWorkItem);

					m_PendingFifoWorkItemList.Unlock();	

					m_UsbDevice->RecycleIrp(FifoWorkItem->Urb, FifoWorkItem->Irp, IoCompletionRoutine, (PVOID)FifoWorkItem);
				}
				else
				{
					m_QueuedFifoWorkItemList.Lock();	

					m_QueuedFifoWorkItemList.Put(FifoWorkItem);

					m_QueuedFifoWorkItemList.Unlock();	
				}
			}
			else
			{
				// No more FIFO work items.
				break;
			}
		}

		m_Client->Unlock();
	}

	return audioStatus;
}

/*****************************************************************************
 * CAudioDataPipe::PrepareFifoWorkItems()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Prepare FIFO work item to perform full speed isochronous transfer.
 * @details
 * This routine splits up a main isoch transfer request into one or
 * more sub requests as necessary.  Each isoch irp/urb pair can span at
 * most 255 packets.
 * 1. It creates a SUB_REQUEST_CONTEXT for each IRP/URB pair and
 *    attaches it to the main request irp.
 * 2. It intializes all of the sub request IRP/URB pairs.
 */
NTSTATUS
CAudioDataPipe::
PrepareFifoWorkItems
(
    IN		ULONG   NumFifoWorkItems,
	IN		BOOL	Read,
	IN		BOOL	TransferAsap
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareFifoWorkItems]"));

	if (m_IsDeviceHighSpeed)
	{
		ntStatus = PrepareHighSpeedFifoWorkItems(NumFifoWorkItems, Read, TransferAsap);
	}
	else
	{
		ntStatus = PrepareFullSpeedFifoWorkItems(NumFifoWorkItems, Read, TransferAsap);
	}

	return ntStatus;
}

#ifdef MICROSOFT_USB_EHCI_BUG_WORKAROUND			
/*****************************************************************************
 * UnusedPattern[]
 *****************************************************************************
 *//*!
 * @brief
 * Pattern used to identify unused audio frames.
 */
static
UCHAR UnusedPattern[] = 
{ 'h', 'y', 'h', 'u', 'a', 'n', 'g', '@', 'a', 't', 'c', '.', 'c', 'r', 'e', 'a', 't', 'i', 'v', 'e', '.', 'c', 'o', 'm', ' ', '2', '0', '0', '5', '-', '2', '0', '0', '6'};
#endif // MICROSOFT_USB_EHCI_BUG_WORKAROUND			

/*****************************************************************************
 * CAudioDataPipe::InitializeFifoWorkItemUrb()
 *****************************************************************************
 *//*!
 * @brief
 * This routine initializes the FIFO work item URB.
 */
NTSTATUS 
CAudioDataPipe::
InitializeFifoWorkItemUrb
(
	IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
)
{
	PURB Urb = FifoWorkItem->Urb;

	ULONG UrbSize = GET_ISO_URB_SIZE(FifoWorkItem->NumberOfPackets);

 	// Initialize the sub request urb.
	RtlZeroMemory(Urb, UrbSize);

    Urb->UrbIsochronousTransfer.Hdr.Length = (USHORT)UrbSize;
    Urb->UrbIsochronousTransfer.Hdr.Function = URB_FUNCTION_ISOCH_TRANSFER;
    Urb->UrbIsochronousTransfer.PipeHandle = m_PipeInformation.PipeHandle;

	if (FifoWorkItem->Read)
	{
		Urb->UrbIsochronousTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
	}
	else
	{
		Urb->UrbIsochronousTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_OUT;
	}

	Urb->UrbIsochronousTransfer.TransferBufferLength = FifoWorkItem->FifoBufferSize;

	Urb->UrbIsochronousTransfer.TransferBuffer = FifoWorkItem->FifoBuffer;

	if (!FifoWorkItem->TransferAsap)
	{
		//This is a way to set the start frame and NOT specify ASAP flag.	
		Urb->UrbIsochronousTransfer.TransferFlags &= ~USBD_START_ISO_TRANSFER_ASAP;

		if (m_SynchronizeStart)
		{
			if (FifoWorkItem->Read)
			{
				// In case the frame number is out of sync. This should not happen
				// in normal operating situation, even under heavy CPU utilization,
				// unless the kernel itself is blocked.
				ULONG FrameNumber; m_UsbDevice->GetCurrentFrameNumber(&FrameNumber);

				if (m_StartFrameNumber <= FrameNumber)
				{
					m_StartFrameNumber = FrameNumber + MIN_AUDIO_START_FRAME_OFFSET;
				}

				// Save the sync frame number to be used for to resync the output to the input
				// when needed.
				m_UsbDevice->SetSyncFrameNumber((m_StartFrameNumber - (MAX_AUDIO_INPUT_IRP - 2)));
			}

			// Offset the start of playback by +3ms relative to the start of record.
			Urb->UrbIsochronousTransfer.StartFrame = m_StartFrameNumber + m_SynchronizationDelay;
		}
		else
		{
			ULONG FrameNumber; m_UsbDevice->GetCurrentFrameNumber(&FrameNumber);

			if (m_StartFrameNumber <= FrameNumber)
			{
				m_StartFrameNumber = FrameNumber + MIN_AUDIO_START_FRAME_OFFSET;
			}

			Urb->UrbIsochronousTransfer.StartFrame = m_StartFrameNumber;
		}
		
		m_StartFrameNumber += 1;
	}
	else
	/**/
	//
	// when the client driver sets the ASAP flag, it basically
	// guarantees that it will make data available to the HC
	// and that the HC should transfer it in the next transfer frame
	// for the endpoint.(The HC maintains a next transfer frame
	// state variable for each endpoint). By resetting the pipe,
	// we make the pipe as virgin. If the data does not get to the HC
	// fast enough, the USBD_ISO_PACKET_DESCRIPTOR - Status is
	// USBD_STATUS_BAD_START_FRAME on uhci. On ohci it is 0xC000000E.
	//
	{
		Urb->UrbIsochronousTransfer.TransferFlags |= USBD_START_ISO_TRANSFER_ASAP;
	}

	Urb->UrbIsochronousTransfer.NumberOfPackets = FifoWorkItem->NumberOfPackets;

	// Set the offsets for every packet for reads/writes
	if (FifoWorkItem->Read)
	{
		FifoWorkItem->BytesInFifoBuffer = 0;

		ULONG Offset = 0, StageSize = FifoWorkItem->FifoBufferSize;

		for (ULONG i=0; i<FifoWorkItem->NumberOfPackets; i++)
		{
			Urb->UrbIsochronousTransfer.IsoPacket[i].Offset = Offset;

			// For input operation, length is set to whatever the device supplies.
			Urb->UrbIsochronousTransfer.IsoPacket[i].Length = 0;

			#ifdef MICROSOFT_USB_EHCI_BUG_WORKAROUND
			if (!m_UseEmbeddedPacketLength)
			{
				if (i > 0)
				{
					for (ULONG k=0; k<FifoWorkItem->PacketSize/m_SampleFrameSize; k++)
					{
						RtlCopyMemory(FifoWorkItem->FifoBuffer + Offset + k * m_SampleFrameSize, UnusedPattern, m_SampleFrameSize);
					}
				}
			}
			#endif // MICROSOFT_USB_EHCI_BUG_WORKAROUND			

			if (StageSize > FifoWorkItem->PacketSize)
			{
				Offset += FifoWorkItem->PacketSize;
				StageSize -= FifoWorkItem->PacketSize;
			}
			else
			{
				Offset += StageSize;
				StageSize = 0;
			}
		}
	}
	else
	{
		// For output, this is initialized by the caller themselves since the output buffer
		// size can vary.
		#if 0
		FifoWorkItem->BytesInFifoBuffer = FifoWorkItem->FifoBufferSize;

		ULONG Offset = 0, StageSize = FifoWorkItem->FifoBufferSize;

		for (ULONG i=0; i<FifoWorkItem->NumberOfPackets; i++)
		{
			Urb->UrbIsochronousTransfer.IsoPacket[i].Offset = Offset;

			if (StageSize > FifoWorkItem->PacketSize)
			{
				Urb->UrbIsochronousTransfer.IsoPacket[i].Length = FifoWorkItem->PacketSize;

				Offset += FifoWorkItem->PacketSize;
				StageSize -= FifoWorkItem->PacketSize;
			}
			else
			{
				FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length = StageSize;
				
				Offset += StageSize;
				StageSize  = 0;

				ASSERT(Offset == (Urb->UrbIsochronousTransfer.IsoPacket[i].Length + Urb->UrbIsochronousTransfer.IsoPacket[i].Offset));
			}
		}
		#endif // 0
	}

	FifoWorkItem->SkipPackets = 0;

    return STATUS_SUCCESS;
}

/*****************************************************************************
 * CAudioDataPipe::PrepareFullSpeedFifoWorkItems()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Prepare FIFO work item to perform full speed isochronous transfer.
 * @details
 * This routine splits up a main isoch transfer request into one or
 * more sub requests as necessary.  Each isoch irp/urb pair can span at
 * most 255 packets.
 * 1. It creates a SUB_REQUEST_CONTEXT for each IRP/URB pair and
 *    attaches it to the main request irp.
 * 2. It intializes all of the sub request IRP/URB pairs.
 */
NTSTATUS
CAudioDataPipe::
PrepareFullSpeedFifoWorkItems
(
    IN		ULONG   NumFifoWorkItems,
	IN		BOOL	Read,
	IN		BOOL	TransferAsap
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareFullSpeedFifoWorkItems]"));

	// Each packet (frame) can hold this much info.
	ULONG PacketSize = m_PipeInformation.MaximumPacketSize;

	ULONG NumberOfPackets = m_NumberOfPacketsPerMs;

	ASSERT(NumberOfPackets == 1);
	
    //_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareFullSpeedFifoWorkItems] - TotalLength: %d, PacketSize: %d", TotalLength, PacketSize));
	
    // There is an inherent limit on the number of packets that can be
    // passed down the stack with each irp/urb pair (255)
    //
    // If the number of required packets is > 255, we shall create
    // "(required-packets / NotificationInterval) [+ 1]" number of irp/urb pairs.
    //
    // Each irp/urb pair transfer is also called a stage transfer.
    //
	ULONG StageSize = PacketSize;

    //_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareFullSpeedFifoWorkItems] - StageSize: %d, NumFifoWorkItems: %d", StageSize, NumFifoWorkItems));
	
	// Allocate the sub requests.
	for (ULONG i = 0; i < NumFifoWorkItems; i++)
	{
		// The following outer scope variables are updated during each
		// iteration of the loop:  virtualAddress, TotalLength, stageSize

		// For every stage of transfer we need to do the following
		// tasks:
		//
		// 1. Allocate a FIFO work item.
		// 2. Allocate a sub request IRP.
		// 3. Allocate a sub request URB.
		// 4. Initialize the above allocations.
		//

		// 1. Allocate a FIFO work item.
		PAUDIO_FIFO_WORK_ITEM FifoWorkItem = new(NonPagedPool) AUDIO_FIFO_WORK_ITEM();

		if (FifoWorkItem == NULL)
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}

		if (NT_SUCCESS(ntStatus))
		{
			m_FifoWorkItem[i] = FifoWorkItem;

			m_FreeFifoWorkItemList.Put(FifoWorkItem);

			FifoWorkItem->Context = this;
			FifoWorkItem->Read = Read;
			FifoWorkItem->Tag = NULL;

			// 2. Allocate a sub request irp
			ntStatus = m_UsbDevice->CreateIrp(&FifoWorkItem->Irp);
		}

		if (NT_SUCCESS(ntStatus))
		{
			// 3. Allocate a sub request URB.

			//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareFullSpeedFifoWorkItems] - NumberOfPackets: %d for IRP/URB pair %d.", NumberOfPackets, i));

			FifoWorkItem->NumberOfPackets = NumberOfPackets;
			FifoWorkItem->PacketSize = PacketSize;

			ULONG UrbSize = GET_ISO_URB_SIZE(NumberOfPackets);

			FifoWorkItem->Urb = (PURB)ExAllocatePoolWithTag(NonPagedPool, UrbSize, 'mdW');

			if (FifoWorkItem->Urb == NULL)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			}
		}

		if (NT_SUCCESS(ntStatus))
		{
			// 4. The buffer.
			FifoWorkItem->FifoBuffer = PUCHAR(ExAllocatePoolWithTag(NonPagedPool, StageSize, 'mdW'));
			FifoWorkItem->FifoBufferSize = StageSize;
			FifoWorkItem->BytesInFifoBuffer = 0;
			FifoWorkItem->TransferSize = 0;
			FifoWorkItem->TransferAsap = TransferAsap;
			FifoWorkItem->Flags = 0x1;
		}

		if (!NT_SUCCESS(ntStatus))
		{
			break;
		}
	}

	if (!NT_SUCCESS(ntStatus))
	{
		for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_FreeFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_FreeFifoWorkItemList.Pop())
		{
			FifoWorkItem->Destruct();
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CAudioDataPipe::PrepareHighSpeedFifoWorkItems()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Prepare FIFO work item to perform high speed isochronous transfer.
 * @details
 * This routine splits up a main isoch transfer request into one or
 * more sub requests as necessary.  Each isoch irp/urb pair can span at
 * most 1024 packets.
 * 1. It creates a SUB_REQUEST_CONTEXT for each IRP/URB pair and
 *    attaches it to the main request irp.
 * 2. It intializes all of the sub request IRP/URB pairs.
 */
NTSTATUS
CAudioDataPipe::
PrepareHighSpeedFifoWorkItems
(
    IN		ULONG   NumFifoWorkItems,
	IN		BOOL	Read,
	IN		BOOL	TransferAsap
)
{
	//TODO: //FIXME: Implement this and verify that this works correctly...
	NTSTATUS ntStatus = STATUS_SUCCESS;

    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareHighSpeedFifoWorkItems]"));

	// Each packet (micro-frame) can hold this much info.
	ULONG PacketSize = m_PipeInformation.MaximumPacketSize;
	
	//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareHighSpeedFifoWorkItems] - PacketSize: %d, Interval: %d", m_PipeInformation.MaximumPacketSize, m_PipeInformation.Interval));

	ULONG NumberOfPackets = m_NumberOfPacketsPerMs;

	//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareHighSpeedFifoWorkItems] - TotalLength: %d, PacketSize: %d", TotalLength, PacketSize));
	
    // There is an inherent limit on the number of packets that can be
    // passed down the stack with each irp/urb pair (255)
    //
    // If the number of required packets is > 255, we shall create
    // "(required-packets / NotificationInterval) [+ 1]" number of irp/urb pairs.
    //
    // Each irp/urb pair transfer is also called a stage transfer.
    //
	ULONG StageSize = PacketSize * NumberOfPackets;

    //_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareHighSpeedFifoWorkItems] - StageSize: %d, NumFifoWorkItems: %d", StageSize, NumFifoWorkItems));
	
	// Allocate the sub requests.
	for (ULONG i = 0; i < NumFifoWorkItems; i++)
	{
		// The following outer scope variables are updated during each
		// iteration of the loop:  virtualAddress, TotalLength, stageSize

		// For every stage of transfer we need to do the following
		// tasks:
		//
		// 1. Allocate a FIFO work item.
		// 2. Allocate a sub request IRP.
		// 3. Allocate a sub request URB.
		// 4. Initialize the above allocations.
		//

		// 1. Allocate a FIFO work item.
		PAUDIO_FIFO_WORK_ITEM FifoWorkItem = new(NonPagedPool) AUDIO_FIFO_WORK_ITEM();

		if (FifoWorkItem == NULL)
		{
			ntStatus = STATUS_INSUFFICIENT_RESOURCES;
		}

		if (NT_SUCCESS(ntStatus))
		{
			m_FifoWorkItem[i] = FifoWorkItem;

			m_FreeFifoWorkItemList.Put(FifoWorkItem);

			FifoWorkItem->Context = this;
			FifoWorkItem->Read = Read;
			FifoWorkItem->Tag = NULL;

			// 2. Allocate a sub request irp
			ntStatus = m_UsbDevice->CreateIrp(&FifoWorkItem->Irp);
		}

		if (NT_SUCCESS(ntStatus))
		{
			// 3. Allocate a sub request URB.

			//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::PrepareHighSpeedFifoWorkItems] - NumberOfPackets: %d for IRP/URB pair %d.", NumberOfPackets, i));

			FifoWorkItem->NumberOfPackets = NumberOfPackets;
			FifoWorkItem->PacketSize = PacketSize;

			ULONG UrbSize = GET_ISO_URB_SIZE(NumberOfPackets);

			FifoWorkItem->Urb = (PURB)ExAllocatePoolWithTag(NonPagedPool, UrbSize, 'mdW');

			if (FifoWorkItem->Urb == NULL)
			{
				ntStatus = STATUS_INSUFFICIENT_RESOURCES;
			}
		}

		if (NT_SUCCESS(ntStatus))
		{
			// 4. The buffer.
			FifoWorkItem->FifoBuffer = PUCHAR(ExAllocatePoolWithTag(NonPagedPool, StageSize, 'mdW'));
			FifoWorkItem->FifoBufferSize = StageSize;
			FifoWorkItem->BytesInFifoBuffer = 0;
			FifoWorkItem->TransferSize = 0;
			FifoWorkItem->TransferAsap = TransferAsap;
			FifoWorkItem->Flags = 0x1;
		}

		if (!NT_SUCCESS(ntStatus))
		{
			break;
		}
	}

	if (!NT_SUCCESS(ntStatus))
	{
		for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_FreeFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_FreeFifoWorkItemList.Pop())
		{
			FifoWorkItem->Destruct();
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CAudioDataPipe::Start()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Start the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioDataPipe::
Start
(
	IN		BOOL	SynchronizeStart,
	IN		ULONG	StartFrameNumber
)
{
	//_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioDataPipe::Start]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	m_SynchronizeStart = SynchronizeStart;

	m_ResyncRequested = FALSE;

	if (m_PipeState != AUDIO_DATA_PIPE_STATE_RUN)
	{
		m_PipeState = AUDIO_DATA_PIPE_STATE_RUN;

		if (SynchronizeStart)
		{
			m_StartFrameNumber = StartFrameNumber;

			if (m_Direction == AUDIO_OUTPUT)
			{
				if (m_IsDeviceHighSpeed)
				{
					m_SynchronizationDelay = MIN_AUDIO_START_FRAME_OFFSET;
				}
				else
				{
					m_SynchronizationDelay = MIN_AUDIO_START_FRAME_OFFSET + 3;
				}
			}
			else
			{
				m_SynchronizationDelay = 0;
			}
		}
		else
		{
			m_UsbDevice->GetCurrentFrameNumber(&m_StartFrameNumber);

			m_StartFrameNumber += MIN_AUDIO_START_FRAME_OFFSET;

			m_SynchronizationDelay = 0;
		}

		//DbgPrint("[%d] StartFrameNumber = %d, SynchronizationDelay = %d\n", (m_Direction==AUDIO_INPUT) ? 1:0, m_StartFrameNumber, m_SynchronizationDelay);

		if (m_Direction == AUDIO_INPUT)
		{
			#ifdef MICROSOFT_USB_OHCI_BUG_WORKAROUND
			m_LastRecordFrameNumber = m_StartFrameNumber;
			#endif // MICROSOFT_USB_OHCI_BUG_WORKAROUND

			ULONG NumberOfPacketsToSkip = SynchronizeStart ? 0 : m_NumberOfPacketsToSkip;

			m_FreeFifoWorkItemList.Lock();

			m_QueuedFifoWorkItemList.Lock();	

			for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_FreeFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_FreeFifoWorkItemList.Pop())
			{
				InitializeFifoWorkItemUrb(FifoWorkItem);

				if (NumberOfPacketsToSkip)
				{
					if (NumberOfPacketsToSkip >= m_NumberOfPacketsPerMs)
					{
						FifoWorkItem->SkipPackets = m_NumberOfPacketsPerMs;

						NumberOfPacketsToSkip -= m_NumberOfPacketsPerMs;
					}
					else
					{
						FifoWorkItem->SkipPackets = NumberOfPacketsToSkip;

						NumberOfPacketsToSkip = 0;
					}
				}

				m_QueuedFifoWorkItemList.Put(FifoWorkItem);
			}

			m_QueuedFifoWorkItemList.Unlock();	

			m_FreeFifoWorkItemList.Unlock();
		}
		else
		{
			// Used for non-ASAP transfer
			m_QueuedFifoWorkItemList.Lock();	

			for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_QueuedFifoWorkItemList.First(); FifoWorkItem; FifoWorkItem = m_QueuedFifoWorkItemList.Next(FifoWorkItem))
			{
				// In case of power down... See PowerStateChange(). 
				// Reinitialize these as it might have been changed by the USBD driver.
				FifoWorkItem->Urb->UrbIsochronousTransfer.Hdr.Status = 0;
				FifoWorkItem->Urb->UrbIsochronousTransfer.PipeHandle = m_PipeInformation.PipeHandle;
				FifoWorkItem->Urb->UrbIsochronousTransfer.TransferFlags = USBD_TRANSFER_DIRECTION_OUT;
				FifoWorkItem->Urb->UrbIsochronousTransfer.TransferBufferMDL = NULL;
				FifoWorkItem->Urb->UrbIsochronousTransfer.ErrorCount = 0;

				if (!FifoWorkItem->TransferAsap)
				{
					// For synchronized start, offset the start of playback by +3ms relative to the start of record.
					FifoWorkItem->Urb->UrbIsochronousTransfer.StartFrame = m_StartFrameNumber + m_SynchronizationDelay;

					m_StartFrameNumber += 1;
				}
			}

			m_QueuedFifoWorkItemList.Unlock();	
		}

		audioStatus = StartTransfer();
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioDataPipe::Pause()
 *****************************************************************************
 *//*!
 * @brief
 * "Pause" the USB pipe.
 * @details
 * This is exactly like Stop(), except that we don't reset the positions.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS 
CAudioDataPipe::
Pause
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioDataPipe::Pause]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	m_SynchronizeStart = FALSE;

	if (m_PipeState == AUDIO_DATA_PIPE_STATE_RUN)
	{
		m_PipeState = AUDIO_DATA_PIPE_STATE_PAUSE;

		CancelTransfer();
	}
	else
	{
		m_PipeState = AUDIO_DATA_PIPE_STATE_PAUSE;
	}

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return AUDIOERR_SUCCESS; 
}

/*****************************************************************************
 * CAudioDataPipe::Stop()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Start the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioDataPipe::
Stop
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioDataPipe::Stop]"));

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

	KeWaitForMutexObject(&m_PipeStateLock, Executive, KernelMode, FALSE, NULL);

	m_SynchronizeStart = FALSE;

	if (m_PipeState == AUDIO_DATA_PIPE_STATE_RUN)
	{
		m_PipeState = AUDIO_DATA_PIPE_STATE_STOP;

		CancelTransfer();
	}
	else
	{
		m_PipeState = AUDIO_DATA_PIPE_STATE_STOP;

		m_FreeFifoWorkItemList.Lock();

		m_QueuedFifoWorkItemList.Lock();	

		for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_QueuedFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_QueuedFifoWorkItemList.Pop())
		{
			m_FreeFifoWorkItemList.Put(FifoWorkItem);
		}

		m_QueuedFifoWorkItemList.Unlock();	

		m_FreeFifoWorkItemList.Unlock();
	}

	m_TotalBytesTransfered = 0;

	m_RunningFfFraction = 0;

	m_PacketDeficitInBytes = 0;

	KeReleaseMutex(&m_PipeStateLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioDataPipe::StartTransfer()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Performs isochronous transfer.
 */
NTSTATUS
CAudioDataPipe::
StartTransfer
(	void
)
{
    //_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::StartTransfer]"));
	m_UsbDevice->ResetPipe(m_PipeInformation.PipeHandle);

	m_PendingIrps = 0;

	KeInitializeEvent(&m_NoPendingIrpEvent, NotificationEvent, FALSE);

	m_QueuedFifoWorkItemList.Lock();

	for (PAUDIO_FIFO_WORK_ITEM FifoWorkItem = m_QueuedFifoWorkItemList.Pop(); FifoWorkItem; FifoWorkItem = m_QueuedFifoWorkItemList.Pop())
	{
		InterlockedIncrement(&m_PendingIrps);

		m_PendingFifoWorkItemList.Lock();	

		m_PendingFifoWorkItemList.Put(FifoWorkItem);

		m_PendingFifoWorkItemList.Unlock();	

		NTSTATUS ntStatus = m_UsbDevice->RecycleIrp(FifoWorkItem->Urb, FifoWorkItem->Irp, IoCompletionRoutine, (PVOID)FifoWorkItem);

		//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::StartTransfer] - ntStatus: 0x%x", ntStatus));
	}

	//_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::StartTransfer] - Pending Irps: %d", m_PendingIrps));

	m_QueuedFifoWorkItemList.Unlock();

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CAudioDataPipe::CancelTransfer()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Free the resouces used by the pipe.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
VOID
CAudioDataPipe::
CancelTransfer
(	void
)
{
	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioDataPipe::CancelTransfer]"));

	m_UsbDevice->AbortPipe(m_PipeInformation.PipeHandle);

	m_PendingFifoWorkItemList.Lock();

	BOOL PendingIrp = m_PendingFifoWorkItemList.Count();

	m_PendingFifoWorkItemList.Unlock();/**/

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioDataPipe::CancelTransfer] - Pending Irps: %d", m_PendingIrps));
	
	if (PendingIrp)
	{
		// Cancel the FIFO work item. Safe to touch these Irps because the completion 
		// routine always returns STATUS_MORE_PRCESSING_REQUIRED.	
		for (ULONG i=0; i<MAX_AUDIO_IRP; i++)
		{
			if (m_FifoWorkItem[i])
			{
				if (m_FifoWorkItem[i]->Irp)
				{
					IoCancelIrp(m_FifoWorkItem[i]->Irp);
				}
			}
		}

		// Wait for the queued FIFO work item Irps to complete.
		KeWaitForSingleObject(&m_NoPendingIrpEvent, Executive, KernelMode, FALSE, NULL);
	}

	if (m_Direction == AUDIO_INPUT)
	{
		// Wait a little bit for the USB device to catch up. Otherwise the next input transfer
		// will fail intermittently.
		KEVENT Event; KeInitializeEvent(&Event, NotificationEvent, FALSE);

		LARGE_INTEGER TimeOut; TimeOut.QuadPart = -5*10000; // 5ms
		
		KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, &TimeOut);
	}

	_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioDataPipe::CancelTransfer] - Done"));
}

/*****************************************************************************
 * CAudioDataPipe::GetPosition()
 *****************************************************************************
 *//*!
 * @brief
 */
AUDIOSTATUS 
CAudioDataPipe::
GetPosition
(
	OUT		ULONGLONG *	OutTransferPosition
)
{
	if (OutTransferPosition)
	{
		*OutTransferPosition = m_TotalBytesTransfered;
	}

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioDataPipe::SetPosition()
 *****************************************************************************
 *//*!
 * @brief
 */
AUDIOSTATUS 
CAudioDataPipe::
SetPosition
(
	IN		ULONGLONG 	TransferPosition
)
{
	m_TotalBytesTransfered = TransferPosition;

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioDataPipe::SetRequest()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS
CAudioDataPipe::
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
 * CAudioDataPipe::GetRequest()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS
CAudioDataPipe::
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
 * CAudioDataPipe::QueryControlSupport()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CAudioDataPipe::
QueryControlSupport
(
	IN		UCHAR	ControlSelector
)
{
	NTSTATUS ntStatus = STATUS_NOT_SUPPORTED;
	
	switch (ControlSelector)
	{
		case USB_AUDIO_EP_CONTROL_SAMPLING_FREQUENCY:
		{
			if (m_ParameterBlock.SamplingFrequency.Support)
			{
				ntStatus = STATUS_SUCCESS;
			}
			else
			{
				ntStatus = STATUS_NOT_SUPPORTED;
			}
		}
		break;

		case USB_AUDIO_EP_CONTROL_PITCH:
		{
			if (m_ParameterBlock.PitchControl.Support)
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
 * CAudioDataPipe::WriteParameterBlock()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CAudioDataPipe::
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
		case USB_AUDIO_EP_CONTROL_SAMPLING_FREQUENCY:
		{
			if (m_ParameterBlock.SamplingFrequency.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						ULONG SamplingFrequency = *(PULONG(ParameterBlock));

						ULONG Current = SamplingFrequency & 0xFFFFFF;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, 3);
						}
						else					
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock.SamplingFrequency.Current = SamplingFrequency;
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

		case USB_AUDIO_EP_CONTROL_PITCH:
		{
			if (m_ParameterBlock.PitchControl.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						BOOL PitchControl = *(PBOOL(ParameterBlock));

						UCHAR Current = PitchControl ? 1 : 0;					

						if (Flags & PARAMETER_BLOCK_FLAGS_IO_HARDWARE)
						{
							ntStatus = SetRequest(RequestCode, Control, &Current, sizeof(UCHAR));
						}
						else					
						{
							ntStatus = STATUS_SUCCESS;
						}

						if (NT_SUCCESS(ntStatus))
						{
							if (Flags & PARAMETER_BLOCK_FLAGS_IO_SOFTWARE)
							{
								m_ParameterBlock.PitchControl.Current = PitchControl;
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
 * CAudioDataPipe::ReadParameterBlock()
 *****************************************************************************
 *//*!
 * @brief
 */
NTSTATUS 
CAudioDataPipe::
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
		case USB_AUDIO_EP_CONTROL_SAMPLING_FREQUENCY:
		{
			if (m_ParameterBlock.SamplingFrequency.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(ULONG))
					{
						PULONG SamplingFrequency = PULONG(ParameterBlock);
						
						*SamplingFrequency = m_ParameterBlock.SamplingFrequency.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(ULONG);
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

		case USB_AUDIO_EP_CONTROL_PITCH:
		{
			if (m_ParameterBlock.PitchControl.Support)
			{
				if (RequestCode == REQUEST_CUR)
				{
					if (ParameterBlockSize >= sizeof(BOOL))
					{
						PBOOL PitchControl = PBOOL(ParameterBlock);
						
						*PitchControl = m_ParameterBlock.PitchControl.Current;

						if (OutParameterBlockSize)
						{
							*OutParameterBlockSize = sizeof(BOOL);
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
 * CAudioDataPipe::RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CAudioDataPipe::
RestoreParameterBlock
(
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	if (ParameterBlock && (ParameterBlockSize == sizeof(AUDIO_DATA_PIPE_PARAMETER_BLOCK)))
	{
		for (UCHAR i=0; i<2; i++)
		{
			UCHAR ControlSelector = USB_AUDIO_EP_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, PAUDIO_DATA_PIPE_PARAMETER_BLOCK(ParameterBlock), FALSE);
		}

		RtlCopyMemory(&m_ParameterBlock, ParameterBlock, sizeof(AUDIO_DATA_PIPE_PARAMETER_BLOCK));
	}
	else
	{
		for (UCHAR i=0; i<2; i++)
		{
			UCHAR ControlSelector = USB_AUDIO_EP_CONTROL_UNDEFINED;

			BOOL Support = _FindControl(i, &ControlSelector);

			_RestoreParameterBlock(ControlSelector, Support, &m_ParameterBlock, TRUE);
		}
	}

	return STATUS_SUCCESS;
}

/*****************************************************************************
 * CAudioDataPipe::_FindControl()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
BOOL
CAudioDataPipe::
_FindControl
(
	IN		UCHAR	Index,
	OUT		UCHAR *	OutControlSelector
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	UCHAR BitMask = 0x01 << Index;

	if (m_Attributes & BitMask)
	{
		Found = TRUE;
	}

	*OutControlSelector = Index+1;

	return Found;
}

/*****************************************************************************
 * CAudioDataPipe::_RestoreParameterBlock()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS 
CAudioDataPipe::
_RestoreParameterBlock
(
	IN		UCHAR								ControlSelector,
	IN		BOOL								Support,
	IN		PAUDIO_DATA_PIPE_PARAMETER_BLOCK	ParameterBlock,
	IN		BOOL								Read
)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;

	USHORT Control = USHORT(ControlSelector)<<8;

	switch (ControlSelector)
	{
		case USB_AUDIO_EP_CONTROL_SAMPLING_FREQUENCY:
		{
			ParameterBlock->SamplingFrequency.Support = Support;

			if (Support)
			{
				if (Read)
				{
					ULONG Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, 3, NULL);
					ParameterBlock->SamplingFrequency.Current = Current;
				}
				else
				{
					ULONG Current = ParameterBlock->SamplingFrequency.Current & 0xFFFFFF;
					SetRequest(REQUEST_CUR, Control, &Current, 3);
				}
			}
		}
		break;

		case USB_AUDIO_EP_CONTROL_PITCH:
		{
			ParameterBlock->PitchControl.Support = Support;

			if (Support)
			{
				if (Read)
				{
					UCHAR Current = 0;					
					GetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR), NULL);
					ParameterBlock->PitchControl.Current = Current ? TRUE : FALSE;
				}
				else
				{
					UCHAR Current = ParameterBlock->PitchControl.Current ? 1 : 0;					
					SetRequest(REQUEST_CUR, Control, &Current, sizeof(UCHAR));
				}
			}
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

/*****************************************************************************
 * CAudioDataPipe::ProcessFifoWorkItem()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Process the FIFO work item.
 * @param
 * FifoWorkItem FIFO work item to service.
 * @return
 * Returns the status of the FIFO work item.
 */
NTSTATUS
CAudioDataPipe::
ProcessFifoWorkItem
(
	IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
)
{
	NTSTATUS ntStatus = FifoWorkItem->Irp->IoStatus.Status;

    if (!NT_SUCCESS(ntStatus)) 
	{
        _DbgPrintF(DEBUGLVL_BLAB, ("Isochronous read/write Irp failed with status = 0x%x", ntStatus));
    }

    USBD_STATUS usbdStatus = FifoWorkItem->Urb->UrbHeader.Status;

    if (!USBD_SUCCESS(usbdStatus)) 
	{
        _DbgPrintF(DEBUGLVL_BLAB, ("Urb failed with status = 0x%x", usbdStatus));
    }

	if (NT_SUCCESS(ntStatus))
	{
		if (FifoWorkItem->Read) 
		{
			// Check each of the urb packets.
			for (ULONG i = 0; i < FifoWorkItem->Urb->UrbIsochronousTransfer.NumberOfPackets; i++) 
			{
				// Skip the number of indicated packets.
				if (i < FifoWorkItem->SkipPackets) continue;

				//TEST: Uncomment for testing... DO NOT CHECK IT IN UNCOMMENTED.
				//FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length = m_FfPerPacketInterval.Whole * m_SampleFrameSize;
				
				if (USBD_SUCCESS(FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Status))
				{
					#ifdef MICROSOFT_USB_EHCI_BUG_WORKAROUND
					if (m_UseEmbeddedPacketLength)
					{
						// The packet length is embedded in the packet itself. The first 4 bytes denotes the length
						// of the packet.
						ULONG EmbeddedPacketLength = *(PULONG(FifoWorkItem->FifoBuffer + FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Offset));

						if (EmbeddedPacketLength > FifoWorkItem->PacketSize)
						{
							EmbeddedPacketLength = FifoWorkItem->PacketSize;
						}

						if (EmbeddedPacketLength > 4)
						{
							FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length = EmbeddedPacketLength  - 4;
						}
						else
						{
							FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length = 0;
						}

						FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Offset += sizeof(ULONG);
					}
					else/**/
					{
						// This work for most EHCI implementation, except for Nvidia NForce chipset.
						if ((i > 0) && (FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length == 0))
						{
							if (RtlCompareMemory(UnusedPattern, FifoWorkItem->FifoBuffer + FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Offset, m_SampleFrameSize) != m_SampleFrameSize)
							{
								FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length = FifoWorkItem->PacketSize;

								for (ULONG k=FifoWorkItem->PacketSize/m_SampleFrameSize; k > 0; k--)
								{
									if (RtlCompareMemory(UnusedPattern, FifoWorkItem->FifoBuffer + FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Offset + ((k - 1) * m_SampleFrameSize), m_SampleFrameSize) != m_SampleFrameSize)
									{
										break;
									}

									FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length -= m_SampleFrameSize;
								}
							}
						}
					}
					#endif // MICROSOFT_USB_EHCI_BUG_WORKAROUND			

					FifoWorkItem->BytesInFifoBuffer += FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length;
				}

				/*if (!USBD_SUCCESS(FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Status))
				//if (FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length)
					_DbgPrintF(DEBUGLVL_BLAB, ("[%d] IsoPacket[%d].Length = %d IsoPacket[%d].Status = %X",
											((_URB_ISOCH_TRANSFER*)(FifoWorkItem->Urb))->StartFrame,
											i,
											FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length,
											i,
											FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Status));/**/
			}

			m_TotalBytesTransfered += FifoWorkItem->BytesInFifoBuffer;
		}
		else
		{
			// Finished with this buffer.
			m_TotalBytesTransfered += FifoWorkItem->BytesInFifoBuffer;
		}
	}
	else
	{
		if (!FifoWorkItem->Read) 
		{
			// The buffer returned with error !!! Too bad... can't really do anything 
			// with it as it is too late, so increment the byte transferred. This
			// shouldn't happened often, and only in situation where the device is
			// starved.
			m_TotalBytesTransfered += FifoWorkItem->BytesInFifoBuffer;
		}
	}

	return ntStatus;
}

/*****************************************************************************
 * CAudioDataPipe::Service()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Service the audio interrupts.
 * @param
 * FifoWorkItem FIFO work item to service.
 * @return
 * <None>
 */
VOID
CAudioDataPipe::
Service
(
	IN		PAUDIO_FIFO_WORK_ITEM	FifoWorkItem
)
{
	NTSTATUS ntStatus = ProcessFifoWorkItem(FifoWorkItem);

	if ((ntStatus == STATUS_CANCELLED) || (ntStatus == STATUS_DEVICE_NOT_CONNECTED) || (ntStatus == STATUS_NO_SUCH_DEVICE)) 
	{
		//_DbgPrintF(DEBUGLVL_BLAB, ("Isoch Irp cancelled/device removed: %d", m_PendingIrps));
		m_PendingFifoWorkItemList.Lock();
		m_PendingFifoWorkItemList.Remove(FifoWorkItem);
		m_PendingFifoWorkItemList.Unlock();

		if (m_PipeState == AUDIO_DATA_PIPE_STATE_PAUSE)
		{
			if (FifoWorkItem->Read)
			{
				m_FreeFifoWorkItemList.Lock();
				m_FreeFifoWorkItemList.Put(FifoWorkItem);
				m_FreeFifoWorkItemList.Unlock();
			}
			else
			{
				m_QueuedFifoWorkItemList.Lock();
				m_QueuedFifoWorkItemList.Put(FifoWorkItem);
				m_QueuedFifoWorkItemList.Unlock();
			}
		}
		else
		{
			m_FreeFifoWorkItemList.Lock();
			m_FreeFifoWorkItemList.Put(FifoWorkItem);
			m_FreeFifoWorkItemList.Unlock();
		}

		// This is the last irp to complete with this erroneous value. 
		// Signal an event.
        if (InterlockedDecrement(&m_PendingIrps) == 0) 
		{
			KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
        }
	}
	else if (m_PipeState == AUDIO_DATA_PIPE_STATE_PAUSE)
	{
		//_DbgPrintF(DEBUGLVL_BLAB, ("Pipe paused: %d", m_PendingIrps));
		m_PendingFifoWorkItemList.Lock();
		m_PendingFifoWorkItemList.Remove(FifoWorkItem);
		m_PendingFifoWorkItemList.Unlock();

		if (FifoWorkItem->Read)
		{
			m_FreeFifoWorkItemList.Lock();
			m_FreeFifoWorkItemList.Put(FifoWorkItem);
			m_FreeFifoWorkItemList.Unlock();
		}
		else
		{
			m_QueuedFifoWorkItemList.Lock();
			m_QueuedFifoWorkItemList.Put(FifoWorkItem);
			m_QueuedFifoWorkItemList.Unlock();
		}

		// This is the last irp to complete with this erroneous value. 
		// Signal an event.
        if (InterlockedDecrement(&m_PendingIrps) == 0) 
		{
			KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
        }
	}
	else if (m_PipeState == AUDIO_DATA_PIPE_STATE_STOP)
	{
		//_DbgPrintF(DEBUGLVL_BLAB, ("Pipe stopped: %d", m_PendingIrps));
		m_PendingFifoWorkItemList.Lock();
		m_PendingFifoWorkItemList.Remove(FifoWorkItem);
		m_PendingFifoWorkItemList.Unlock();

		m_FreeFifoWorkItemList.Lock();
		m_FreeFifoWorkItemList.Put(FifoWorkItem);
		m_FreeFifoWorkItemList.Unlock();

		// This is the last irp to complete with this erroneous value. 
		// Signal an event.
        if (InterlockedDecrement(&m_PendingIrps) == 0) 
		{
			KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
        }
	}
	else
	{
		if (FifoWorkItem->Read)
		{
			if (NT_SUCCESS(ntStatus))
			{
				if (m_IsDeviceHighSpeed)
				{
					m_Client->Lock();

					for (ULONG i = 0; i < FifoWorkItem->Urb->UrbIsochronousTransfer.NumberOfPackets; i++) 
					{
						// Skip the number of indicated packets.
						if (i < FifoWorkItem->SkipPackets) 
						{
							//DbgPrint("Skipped packet at frame %d\n", ((_URB_ISOCH_TRANSFER*)(FifoWorkItem->Urb))->StartFrame);
							continue;
						}

						if (USBD_SUCCESS(FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Status))
						{
							// Add the audio frames to the client FIFO.
							m_Client->AddFramesToFifo(FifoWorkItem->FifoBuffer + FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Offset, FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[i].Length / m_SampleFrameSize);
						}
					}

					m_Client->Unlock();

					if (FifoWorkItem->Flags & 0x1)
					{
						// Callback to client...
						if (m_Client)
						{
							m_Client->Service(FifoWorkItem);
						}
					}
				}
				else
				{
					#ifdef MICROSOFT_USB_OHCI_BUG_WORKAROUND
					ULONG Ops = 0;

					ULONG RecordFrameNumber = ((_URB_ISOCH_TRANSFER*)(FifoWorkItem->Urb))->StartFrame;

					if (RecordFrameNumber >= m_LastRecordFrameNumber)
					{
						ULONG Delta = RecordFrameNumber - m_LastRecordFrameNumber;

						if (Delta > 1)
						{
							//DbgPrint("--[A] Buzz... %d, %d", m_LastRecordFrameNumber, ((_URB_ISOCH_TRANSFER*)(FifoWorkItem->Urb))->StartFrame);
							Ops = 1;
						}

						m_LastRecordFrameNumber = RecordFrameNumber;
					}
					else
					{
						//DbgPrint("--[B] Buzz... %d, %d", m_LastRecordFrameNumber, ((_URB_ISOCH_TRANSFER*)(FifoWorkItem->Urb))->StartFrame);
						Ops = 2;
					}

					if (Ops == 0)
					#endif // MICROSOFT_USB_OHCI_BUG_WORKAROUND
					{
						m_Client->Lock();

						// Skip the indicated packet.
						if (!FifoWorkItem->SkipPackets) 
						{
							// Add the audio frames to the client FIFO.
							m_Client->AddFramesToFifo(FifoWorkItem->FifoBuffer + FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[0].Offset, FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[0].Length / m_SampleFrameSize);
						}

						m_Client->Unlock();

						if (FifoWorkItem->Flags & 0x1)
						{
							// Callback to client...
							if (m_Client)
							{
								m_Client->Service(FifoWorkItem);
							}
						}
					}
					#ifdef MICROSOFT_USB_OHCI_BUG_WORKAROUND
					else if (Ops == 1)
					{
						if (m_LastRecordBuffer)
						{
							RtlCopyMemory(m_LastRecordBuffer, FifoWorkItem->FifoBuffer, FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[0].Length);

							m_LastRecordBufferSize = FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[0].Length;
						}
					}
					else if (Ops == 2)
					{
						m_Client->Lock();

						// Skip the indicated packet.
						if (!FifoWorkItem->SkipPackets) 
						{
							// Add the audio frames to the client FIFO.
							m_Client->AddFramesToFifo(FifoWorkItem->FifoBuffer + FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[0].Offset, FifoWorkItem->Urb->UrbIsochronousTransfer.IsoPacket[0].Length / m_SampleFrameSize);
						}

						if (m_LastRecordBufferSize)
						{
							// Add the audio frames to the client FIFO.
							m_Client->AddFramesToFifo((PUCHAR)m_LastRecordBuffer, m_LastRecordBufferSize / m_SampleFrameSize);

							m_LastRecordBufferSize = 0;
						}

						m_Client->Unlock();

						if (FifoWorkItem->Flags & 0x1)
						{
							// Callback to client...
							if (m_Client)
							{
								m_Client->Service(FifoWorkItem);
							}
						}
					}
					#endif // MICROSOFT_USB_OHCI_BUG_WORKAROUND
				}
			}

			InitializeFifoWorkItemUrb(FifoWorkItem);

			NTSTATUS ntStatus = m_UsbDevice->RecycleIrp(FifoWorkItem->Urb, FifoWorkItem->Irp, IoCompletionRoutine, (PVOID)FifoWorkItem);
		}
		else
		{
			m_PendingFifoWorkItemList.Lock();
			m_PendingFifoWorkItemList.Remove(FifoWorkItem);
			m_PendingFifoWorkItemList.Unlock();

			if (FifoWorkItem->Flags & 0x1)
			{
				// Callback to client...
				if (m_Client)
				{
					m_Client->Service(FifoWorkItem);
				}
			}

			m_FreeFifoWorkItemList.Lock();
			m_FreeFifoWorkItemList.Put(FifoWorkItem);
			m_FreeFifoWorkItemList.Unlock();

			if (InterlockedDecrement(&m_PendingIrps) == 0) 
			{
				KeSetEvent(&m_NoPendingIrpEvent, IO_SOUND_INCREMENT, FALSE);
			}

			FlushBuffer();
		}
	}
}

/*****************************************************************************
 * CAudioDataPipe::IoCompletionRoutine()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
NTSTATUS
CAudioDataPipe::
IoCompletionRoutine
(
    IN		PDEVICE_OBJECT	DeviceObject,
    IN		PIRP			Irp,
    IN		PVOID			Context
)
{
    //_DbgPrintF(DEBUGLVL_BLAB,("[CAudioDataPipe::IoCompletionRoutine]"));

	PAUDIO_FIFO_WORK_ITEM FifoWorkItem = (PAUDIO_FIFO_WORK_ITEM)Context;

	CAudioDataPipe * that = (CAudioDataPipe*)FifoWorkItem->Context;

	that->Service(FifoWorkItem);

    return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioInterface::~CAudioInterface()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Destructor.
 */
CAudioInterface::
~CAudioInterface
(	void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioInterface::~CAudioInterface]"));

	m_DataPipeList.DeleteAllItems();

	m_SynchPipeList.DeleteAllItems();

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CAudioInterface::Init()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioInterface::
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

	KeInitializeMutex(&m_InterfaceAcquireLock, 0);

	m_InterfaceTag = NULL;

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioInterface::AcquireInterface()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioInterface::
AcquireInterface
(
	IN		PVOID	Tag,
	IN		ULONG	TagPriority
)
{
	PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioInterface::AcquireInterface] - %p, %d", Tag, TagPriority));

	AUDIOSTATUS audioStatus = AUDIOERR_INSUFFICIENT_RESOURCES;

	KeWaitForSingleObject(&m_InterfaceAcquireLock, Executive, KernelMode, TRUE, NULL);

	if (m_InterfaceTag)
	{
		if (m_InterfaceTag == Tag)
		{
			m_InterfaceTagPriority = TagPriority;

			audioStatus = AUDIOERR_SUCCESS;
		}
		else
		{
			if (TagPriority > m_InterfaceTagPriority)
			{
				// Rip off the current interface owner.
				CAudioClient * AudioClient = (CAudioClient *)m_InterfaceTag;

				AudioClient->OnResourcesRipOff();
							
				m_InterfaceTag = Tag;
                
				m_InterfaceTagPriority = TagPriority;

				audioStatus = AUDIOERR_SUCCESS;
			}
		}
	}
	else
	{
		m_InterfaceTag = Tag;

		m_InterfaceTagPriority = TagPriority;

		audioStatus = AUDIOERR_SUCCESS;
	}

	KeReleaseMutex(&m_InterfaceAcquireLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioInterface::ReleaseInterface()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioInterface::
ReleaseInterface
(
	IN		PVOID	Tag
)
{
	PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioInterface::ReleaseInterface] - %p", Tag));

	AUDIOSTATUS audioStatus = AUDIOERR_BAD_REQUEST;

	KeWaitForSingleObject(&m_InterfaceAcquireLock, Executive, KernelMode, TRUE, NULL);

	if (m_InterfaceTag)
	{
		if (m_InterfaceTag == Tag)
		{
			m_InterfaceTag = NULL;

			m_InterfaceTagPriority = 0;

			if (m_InterfaceAvailabilityCallbackTag)
			{
				// Inform the previous owner that resources are available.
				CAudioClient * AudioClient = (CAudioClient *)m_InterfaceAvailabilityCallbackTag;

				AudioClient->OnResourcesAvailability();						
			}

			audioStatus = AUDIOERR_SUCCESS;
		}
	}

	KeReleaseMutex(&m_InterfaceAcquireLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioInterface::SetInterfaceAvailabilityCallback()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioInterface::
SetInterfaceAvailabilityCallback
(
	IN		PVOID	Tag,
	IN		BOOL	Enable
)
{
	PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioInterface::SetInterfaceAvailabilityCallback] - %p, %d", Tag, Enable));

	AUDIOSTATUS audioStatus = AUDIOERR_BAD_REQUEST;

	KeWaitForSingleObject(&m_InterfaceAcquireLock, Executive, KernelMode, TRUE, NULL);

	if (m_InterfaceAvailabilityCallbackTag)
	{
		if (m_InterfaceAvailabilityCallbackTag == Tag)
		{
			m_InterfaceAvailabilityCallbackTag = Enable ? Tag : NULL;

			audioStatus = AUDIOERR_SUCCESS;
		}
	}
	else
	{
		m_InterfaceAvailabilityCallbackTag = Enable ? Tag : NULL;
	}

	KeReleaseMutex(&m_InterfaceAcquireLock, FALSE);

	return audioStatus;
}

/*****************************************************************************
 * CAudioInterface::SelectAlternateSetting()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioInterface::
SelectAlternateSetting
(
	IN		UCHAR	AlternateSetting
)
{
	PAGED_CODE();

	m_DataPipeList.DeleteAllItems();

	m_SynchPipeList.DeleteAllItems();

	AUDIOSTATUS audioStatus = m_UsbDevice->SelectAlternateInterface(m_InterfaceNumber, AlternateSetting);

	_DbgPrintF(DEBUGLVL_BLAB,("[CAudioInterface::SelectAlternateSetting] - SelectAlternate: 0x%x", audioStatus));

	if (AUDIO_SUCCESS(audioStatus))
	{
		m_AlternateSetting = AlternateSetting;

		PUSBD_INTERFACE_INFORMATION InterfaceInfo = NULL;
		
		audioStatus = m_UsbDevice->GetInterfaceInformation(m_InterfaceNumber, AlternateSetting, &InterfaceInfo);

		if (AUDIO_SUCCESS(audioStatus))
		{
			for (UCHAR i=0; i<InterfaceInfo->NumberOfPipes; i++)
			{
				PUSB_AUDIO_ENDPOINT_DESCRIPTOR EndpointDescriptor = NULL;

				if (AUDIO_SUCCESS(m_UsbDevice->GetEndpointDescriptorByIndex(m_InterfaceNumber, AlternateSetting, i, (PUSB_ENDPOINT_DESCRIPTOR*)&EndpointDescriptor)))
				{
					if (((EndpointDescriptor->bmAttributes & 0x0C) != 0) ||
						// Backup conditionals to protect against malformed endpoint descriptor as found in 
						// Audigy 2 NX device (POS).
						((EndpointDescriptor->bLength >= sizeof(USB_AUDIO_ENDPOINT_DESCRIPTOR)) &&
						((EndpointDescriptor->bRefresh == 0) || (EndpointDescriptor->bSynchAddress != 0)))) 
					{
						// Data endpoint.
						CAudioDataPipe * Pipe = new(NonPagedPool) CAudioDataPipe();

						if (Pipe)
						{
							audioStatus = Pipe->Init
											(
												m_UsbDevice,
												m_InterfaceNumber,
												AlternateSetting,
												InterfaceInfo->Pipes[i]
											);

							if (AUDIO_SUCCESS(audioStatus))
							{
								m_DataPipeList.Put(Pipe);
							}
							else
							{
								delete Pipe;
								break;
							}
						}
						else
						{
							audioStatus = AUDIOERR_NO_MEMORY;
							break;
						}
					}
					else
					{
						// Synch endpoint.
						CAudioSynchPipe * Pipe = new(NonPagedPool) CAudioSynchPipe();

						if (Pipe)
						{
							audioStatus = Pipe->Init
											(
												m_UsbDevice,
												m_InterfaceNumber,
												AlternateSetting,
												InterfaceInfo->Pipes[i]
											);

							if (AUDIO_SUCCESS(audioStatus))
							{
								m_SynchPipeList.Put(Pipe);
							}
							else
							{
								delete Pipe;
								break;
							}
						}
						else
						{
							audioStatus = AUDIOERR_NO_MEMORY;
							break;
						}
					}
				}
			}
		}
		else
		{
			audioStatus = AUDIOERR_DEVICE_CONFIGURATION_ERROR;
		}
	}
	else
	{
		audioStatus = AUDIOERR_DEVICE_CONFIGURATION_ERROR;
	}

	if (!AUDIO_SUCCESS(audioStatus))
	{
		// Cleanup mess...
		m_DataPipeList.DeleteAllItems();

		m_SynchPipeList.DeleteAllItems();
	}

	return audioStatus;
}

/*****************************************************************************
 * CAudioInterface::InterfaceNumber()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
UCHAR
CAudioInterface::
InterfaceNumber
(	void
)
{
	return m_InterfaceNumber;
}

/*****************************************************************************
 * CAudioInterface::TerminalLink()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
UCHAR
CAudioInterface::
TerminalLink
(
	IN		UCHAR	AlternateSetting
)
{
	UCHAR TerminalLink = 0;

	PUSB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR CsAsInterfaceDescriptor = NULL;

	if (AUDIO_SUCCESS(m_UsbDevice->GetClassInterfaceDescriptor(m_InterfaceNumber, AlternateSetting, USB_AUDIO_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsAsInterfaceDescriptor)))
	{
		TerminalLink = CsAsInterfaceDescriptor->bTerminalLink;
	}

	return TerminalLink;
}

// BEGIN_HACK
/*****************************************************************************
 * CAudioInterface::HasBadDescriptors()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
BOOL
CAudioInterface::
HasBadDescriptors
(	void
)
{
	BOOL BadDescriptorsFound = FALSE;

	PUSB_DEVICE_DESCRIPTOR DeviceDescriptor = NULL;

	if (m_UsbDevice->IsDeviceHighSpeed())
	{
		if (AUDIO_SUCCESS(m_UsbDevice->GetDeviceDescriptor(&DeviceDescriptor)))
		{
			if ((DeviceDescriptor->idVendor == 0x041E/*Creative*/) &&
				((DeviceDescriptor->idProduct == 0x3F02/*MicroPod*/) ||
				(DeviceDescriptor->idProduct == 0x3F0A/*MicroPre*/) ||
				(DeviceDescriptor->idProduct == 0x3F0B/*Itey*/) ||
				(DeviceDescriptor->idProduct == 0x3F04/*HulaPod*/)))
			{
				BadDescriptorsFound = TRUE;
			}
		}
	}

	return BadDescriptorsFound;
}
// END_HACK

/*****************************************************************************
 * CAudioInterface::ParseSupportedFormat()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
BOOL
CAudioInterface::
ParseSupportedFormat
(
	IN		UCHAR		AlternateSetting,
	OUT		USHORT *	FormatTag
)
{
	BOOL Found = FALSE;

	PUSB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR CsAsInterfaceDescriptor = NULL;

	if (AUDIO_SUCCESS(m_UsbDevice->GetClassInterfaceDescriptor(m_InterfaceNumber, AlternateSetting, USB_AUDIO_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsAsInterfaceDescriptor)))
	{
		*FormatTag = CsAsInterfaceDescriptor->wFormatTag;

		Found = TRUE;
	}

	return Found;
}

/*****************************************************************************
 * CAudioInterface::FormatTypeDescriptor()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR 
CAudioInterface::
GetFormatTypeDescriptor
(
	IN		UCHAR		AlternateSetting
)
{
	PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR FormatTypeDescriptor = NULL;

	PUSB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR CsAsInterfaceDescriptor = NULL;

	if (AUDIO_SUCCESS(m_UsbDevice->GetClassInterfaceDescriptor(m_InterfaceNumber, AlternateSetting, USB_AUDIO_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsAsInterfaceDescriptor)))
	{
		FormatTypeDescriptor = PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR(PUCHAR(CsAsInterfaceDescriptor)+CsAsInterfaceDescriptor->bLength);
	}

	return FormatTypeDescriptor;
}

/*****************************************************************************
 * CAudioInterface::FormatSpecificDescriptor()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
PUSB_AUDIO_COMMON_FORMAT_SPECIFIC_DESCRIPTOR 
CAudioInterface::
GetFormatSpecificDescriptor
(
	IN		UCHAR		AlternateSetting
)
{
	PUSB_AUDIO_COMMON_FORMAT_SPECIFIC_DESCRIPTOR FormatSpecificDescriptor = NULL;

	PUSB_AUDIO_CS_AS_INTERFACE_DESCRIPTOR CsAsInterfaceDescriptor = NULL;

	if (AUDIO_SUCCESS(m_UsbDevice->GetClassInterfaceDescriptor(m_InterfaceNumber, AlternateSetting, USB_AUDIO_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsAsInterfaceDescriptor)))
	{
		PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR FormatTypeDescriptor = PUSB_AUDIO_COMMON_FORMAT_TYPE_DESCRIPTOR(PUCHAR(CsAsInterfaceDescriptor)+CsAsInterfaceDescriptor->bLength);

		FormatSpecificDescriptor = PUSB_AUDIO_COMMON_FORMAT_SPECIFIC_DESCRIPTOR(PUCHAR(FormatTypeDescriptor)+FormatTypeDescriptor->bLength);
	}

	return FormatSpecificDescriptor;
}

/*****************************************************************************
 * CAudioInterface::GetDataEndpointDescriptor()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
PUSB_ENDPOINT_DESCRIPTOR 
CAudioInterface::
GetDataEndpointDescriptor
(
	IN		UCHAR		AlternateSetting
)
{
	PUSB_ENDPOINT_DESCRIPTOR EndpointDescriptor = NULL;

	PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

	if (AUDIO_SUCCESS(m_UsbDevice->GetInterfaceDescriptor(m_InterfaceNumber, AlternateSetting, -1, -1, -1, &InterfaceDescriptor)))
	{
		for (UCHAR i=0; i<InterfaceDescriptor->bNumEndpoints; i++)
		{
			PUSB_AUDIO_ENDPOINT_DESCRIPTOR AudioEndpointDescriptor = NULL;

			if (AUDIO_SUCCESS(m_UsbDevice->GetEndpointDescriptorByIndex(m_InterfaceNumber, AlternateSetting, i, (PUSB_ENDPOINT_DESCRIPTOR*)&AudioEndpointDescriptor)))
			{
				if (((AudioEndpointDescriptor->bmAttributes & 0x0C) != 0) ||
					// Backup conditionals to protect against malformed endpoint descriptor as found in 
					// Audigy 2 NX device (POS).
					((AudioEndpointDescriptor->bLength >= sizeof(USB_AUDIO_ENDPOINT_DESCRIPTOR)) &&
					((AudioEndpointDescriptor->bRefresh == 0) || (AudioEndpointDescriptor->bSynchAddress != 0)))) 
				{
					// Data endpoint.
					EndpointDescriptor = PUSB_ENDPOINT_DESCRIPTOR(AudioEndpointDescriptor);
					break;
				}
				//else
				//{
				//	// Synch endpoint.
				//}
			}
		}
	}

	return EndpointDescriptor;
}

/*****************************************************************************
 * CAudioInterface::PowerStateChange()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns AUDIOERR_SUCCESS if the power state changed.
 */
AUDIOSTATUS
CAudioInterface::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

    if (NewState != m_PowerState)
	{
		if (NewState == PowerDeviceD0)
		{
			// Power up.
			m_UsbDevice->SelectAlternateInterface(m_InterfaceNumber, m_AlternateSetting);
		}

		for (CAudioDataPipe * DataPipe = m_DataPipeList.First(); DataPipe; DataPipe = m_DataPipeList.Next(DataPipe))
		{
			DataPipe->PowerStateChange(NewState);
		}

		for (CAudioSynchPipe * SynchPipe = m_SynchPipeList.First(); SynchPipe; SynchPipe = m_SynchPipeList.Next(SynchPipe))
		{
			SynchPipe->PowerStateChange(NewState);
		}

		if (NewState != PowerDeviceD0)
		{
			// Power down.
			m_UsbDevice->SelectAlternateInterface(m_InterfaceNumber, 0);
		}

		m_PowerState = NewState;
	}

    return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioInterface::FindDataPipe()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Find the audio data pipe.
 * @return
 * Returns the audio data pipe.
 */
CAudioDataPipe *
CAudioInterface::
FindDataPipe
(	void
)
{
    PAGED_CODE();

	// There should only be 1 data endpoint.
	CAudioDataPipe * Pipe = m_DataPipeList.First(); 

	return Pipe;
}

/*****************************************************************************
 * CAudioInterface::FindSynchPipe()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Find the audio synch pipe.
 * @return
 * Returns the audio synch pipe.
 */
CAudioSynchPipe *
CAudioInterface::
FindSynchPipe
(	void
)
{
    PAGED_CODE();

	// There should only be 1 data endpoint.
	CAudioSynchPipe * Pipe = m_SynchPipeList.First(); 

	return Pipe;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioTopology::~CAudioTopology()
 *****************************************************************************
 *//*!
 * @ingroup AUDIO_GROUP
 * @brief
 * Destructor.
 */
CAudioTopology::
~CAudioTopology
(	void
)
{
    PAGED_CODE();

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioTopology::~CAudioTopology]"));

	m_EntityList.DeleteAllItems();

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CAudioTopology::Init()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Initialize the audio control & audio device.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful, AUDIOERR_NO_MEMORY if the audio 
 * topology couldn't be created.
 */
AUDIOSTATUS
CAudioTopology::
Init
(
	IN		PUSB_DEVICE	UsbDevice
)
{
	PAGED_CODE();

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

	AUDIOSTATUS audioStatus = m_UsbDevice->GetInterfaceDescriptor(-1, -1, ClassCode, USB_AUDIO_SUBCLASS_AUDIOCONTROL, -1, &AcInterfaceDescriptor);

	if (AUDIO_SUCCESS(audioStatus))
	{
		m_InterfaceNumber = AcInterfaceDescriptor->bInterfaceNumber;

		audioStatus = ParseCsAcInterfaceDescriptor(AcInterfaceDescriptor->bInterfaceNumber);
	}

	if (!AUDIO_SUCCESS(audioStatus))
	{
		// Cleanup mess...
		if (m_UsbDevice)
		{
			m_UsbDevice->Release();
			m_UsbDevice = NULL;
		}
	}

	return audioStatus;
}

/*****************************************************************************
 * CAudioTopology::PowerStateChange()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns AUDIOERR_SUCCESS if the power state changed.
 */
AUDIOSTATUS
CAudioTopology::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

	if (NewState != m_PowerState)
	{
		for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
		{
			UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

			switch (DescriptorSubtype)
			{
				case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
				case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
				{
					PTERMINAL Terminal = PTERMINAL(Entity);

					Terminal->PowerStateChange(NewState);
				}
				break;
			
				case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
				case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
				{
					PUNIT Unit = PUNIT(Entity);

					Unit->PowerStateChange(NewState);
				}
				break;
			}
		}

		m_PowerState = NewState;
	}

    return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * DriverResyncExtensionUnitDescriptor[]
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 */
static
UCHAR DriverResyncExtensionUnitDescriptor[] =
{
	13+0+1, // bLength
	USB_AUDIO_CS_INTERFACE, // bDescriptorType
	USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT, // bDescriptorSubType
	0xFE, // bUnitID
	XU_CODE_DRIVER_RESYNC&0xFF, (XU_CODE_DRIVER_RESYNC&0xFF00)>>8, // wExtensionCode
	0, // bNrInPins
	0, // bNrChannels
	0, 0, // wChannelConfig
	0, // iChannelNames
	1, // bControlSize
	0x1, // bmControls
	0 // iExtension
};

/*****************************************************************************
 * CAudioTopology::ParseCsAcInterfaceDescriptor()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 */
AUDIOSTATUS
CAudioTopology::
ParseCsAcInterfaceDescriptor
(
	IN		UCHAR	InterfaceNumber
)
{
    PAGED_CODE();

	CList<CEntity> dependentUnits;

	PUSB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR CsAcInterfaceDescriptor = NULL;

	AUDIOSTATUS audioStatus = m_UsbDevice->GetClassInterfaceDescriptor(InterfaceNumber, 0, USB_AUDIO_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsAcInterfaceDescriptor);

	if (AUDIO_SUCCESS(audioStatus))
	{
		// Parse the input/output terminals and units...
		PUCHAR DescriptorEnd = PUCHAR(CsAcInterfaceDescriptor) + CsAcInterfaceDescriptor->wTotalLength;

		PUSB_AUDIO_COMMON_DESCRIPTOR CommonDescriptor = (PUSB_AUDIO_COMMON_DESCRIPTOR)(PUCHAR(CsAcInterfaceDescriptor)+CsAcInterfaceDescriptor->bLength);

		while (((PUCHAR(CommonDescriptor) + sizeof(USB_AUDIO_COMMON_DESCRIPTOR)) < DescriptorEnd) &&
  				((PUCHAR(CommonDescriptor) + CommonDescriptor->bLength) <= DescriptorEnd))
		{
			if (CommonDescriptor->bDescriptorType == USB_AUDIO_CS_INTERFACE)
			{
				switch (CommonDescriptor->bDescriptorSubtype)
				{
					case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
					{
						CInputTerminal * Terminal = new(NonPagedPool) CInputTerminal();

						if (Terminal)
						{
							if (AUDIO_SUCCESS(Terminal->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_TERMINAL_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Terminal);
							}
							else
							{
								delete Terminal;
							}
						}
					}
					break;

					case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
					{
						COutputTerminal * Terminal = new(NonPagedPool) COutputTerminal();

						if (Terminal)
						{
							if (AUDIO_SUCCESS(Terminal->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_TERMINAL_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Terminal);
							}
							else
							{
								delete Terminal;
							}
						}
					}
					break;

					case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
					{
						CMixerUnit * Unit = new(NonPagedPool) CMixerUnit();

						if (Unit)
						{
							if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Unit);
							}
							else
							{
								delete Unit;
							}
						}
					}
					break;

					case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
					{
						CSelectorUnit * Unit = new(NonPagedPool) CSelectorUnit();

						if (Unit)
						{
							if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Unit);
							}
							else
							{
								delete Unit;
							}
						}
					}
					break;

					case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
					{
						CFeatureUnit * Unit = new(NonPagedPool) CFeatureUnit();

						if (Unit)
						{
							if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Unit);
							}
							else
							{
								delete Unit;
							}
						}
					}
					break;

					case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
					{
						PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR ProcessingUnitDescriptor = PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR(CommonDescriptor);

						switch (ProcessingUnitDescriptor->wProcessType)
						{
							case USB_AUDIO_PROCESS_UPMIX_DOWNMIX:
							{
								CUpDownMixUnit * Unit = new(NonPagedPool) CUpDownMixUnit();

								if (Unit)
								{
									if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							case USB_AUDIO_PROCESS_DOLBY_PROLOGIC:
							{
								CDolbyPrologicUnit * Unit = new(NonPagedPool) CDolbyPrologicUnit();

								if (Unit)
								{
									if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							case USB_AUDIO_PROCESS_3D_STEREO_EXTENDER:
							{
								C3dStereoExtenderUnit * Unit = new(NonPagedPool) C3dStereoExtenderUnit();

								if (Unit)
								{
									if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							case USB_AUDIO_PROCESS_REVERBERATION:
							{
								CReverberationUnit * Unit = new(NonPagedPool) CReverberationUnit();

								if (Unit)
								{
									if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							case USB_AUDIO_PROCESS_CHORUS:
							{
								CChorusUnit * Unit = new(NonPagedPool) CChorusUnit();

								if (Unit)
								{
									if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							case USB_AUDIO_PROCESS_DYNAMIC_RANGE_COMPRESSION:
							{
								CDynamicRangeCompressionUnit * Unit = new(NonPagedPool) CDynamicRangeCompressionUnit();

								if (Unit)
								{
									if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
									{
										m_EntityList.Put(Unit);
									}
									else
									{
										delete Unit;
									}
								}
							}
							break;

							default:
								break;
						}
					}
					break;

					case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
					{
						CExtensionUnit * Unit = new(NonPagedPool) CExtensionUnit();

						if (Unit)
						{
							if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(CommonDescriptor))))
							{
								m_EntityList.Put(Unit);
							}
							else
							{
								delete Unit;
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

		if (m_EntityList.Count())
		{
			// Add an extension unit for handling driver resync request.
			CExtensionUnit * Unit = new(NonPagedPool) CExtensionUnit();

			if (Unit)
			{
				if (AUDIO_SUCCESS(Unit->Init(this, m_UsbDevice, InterfaceNumber, PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR(DriverResyncExtensionUnitDescriptor))))
				{
					m_EntityList.Put(Unit);
				}
				else
				{
					delete Unit;
				}
			}
		}

		// Go thru all entities and Configure();
		for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
		{
			Entity->Configure();
		}
	}

	return audioStatus;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioTopology::FindEntity()
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
CAudioTopology::
FindEntity
(
	IN		UCHAR		EntityID,
	OUT		PENTITY *	OutEntity
)
{
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

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioTopology::ParseTerminals()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Enumerate the terminals that are available on this device.
 * @param
 * Index Enumeration index.
 * @param
 * OutTerminal A pointer to the PTERMINAL which will receive the terminal.
 * @return
 * TRUE if the specified index matches one of the terminals, otherwise FALSE.
 */
BOOL
CAudioTopology::
ParseTerminals
(
	IN		ULONG		Index,
	OUT		PTERMINAL *	OutTerminal
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	ULONG idx = 0;

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

		if ((DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL) ||
			(DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL))
		{
			if (idx == Index)
			{
				*OutTerminal = PTERMINAL(Entity);

				Found = TRUE;
				break;
			}

			idx++;
		}
	}

	return Found;
}

/*****************************************************************************
 * CAudioTopology::ParseUnits()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Enumerate the units that are available on this device.
 * @param
 * Index Enumeration index.
 * @param
 * OutTerminal A pointer to the PUNIT which will receive the unit.
 * @return
 * TRUE if the specified index matches one of the units, otherwise FALSE.
 */
BOOL
CAudioTopology::
ParseUnits
(
	IN		ULONG	Index,
	OUT		PUNIT *	OutUnit
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	ULONG idx = 0;

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

		if ((DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT) ||
			(DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT) ||
			(DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT) ||
			(DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT) ||
			(DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT))
		{
			if (idx == Index)
			{
				*OutUnit = PUNIT(Entity);

				Found = TRUE;
				break;
			}

			idx++;
		}
	}

	return Found;
}

/*****************************************************************************
 * PARAMETER_BLOCK_HEADER
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
typedef struct
{
	ULONG	Tag;
	ULONG	Size;
	UCHAR	EntityID;
	UCHAR	DescriptorSubtype;
	USHORT	Reserved;
	ULONG	ParameterBlockSize;
} PARAMETER_BLOCK_HEADER, *PPARAMETER_BLOCK_HEADER;

/*****************************************************************************
 * CAudioTopology::SaveParameterBlocks()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioTopology::
SaveParameterBlocks
(
	IN		PVOID	ParameterBlocks_,
	IN		ULONG	SizeOfParameterBlocks,
	OUT		ULONG *	OutSizeOfParameterBlocks
)
{
    PAGED_CODE();

	if (OutSizeOfParameterBlocks)
	{
		*OutSizeOfParameterBlocks = 0;
	}

	PUCHAR ParameterBlocks = PUCHAR(ParameterBlocks_);

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

		switch (DescriptorSubtype)
		{
			case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
			case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
			{
				if (SizeOfParameterBlocks > sizeof(PARAMETER_BLOCK_HEADER))
				{
					PPARAMETER_BLOCK_HEADER Header = PPARAMETER_BLOCK_HEADER(ParameterBlocks); 

					PUCHAR ParameterBlock = ParameterBlocks + sizeof(PARAMETER_BLOCK_HEADER);

					PTERMINAL Terminal = PTERMINAL(Entity);

					ULONG ParameterBlockSize = 0;

					if (NT_SUCCESS(Terminal->SaveParameterBlock(ParameterBlock, SizeOfParameterBlocks - sizeof(PARAMETER_BLOCK_HEADER), &ParameterBlockSize)))
					{
						Header->Tag = ' Y2H';
						Header->Size = sizeof(PARAMETER_BLOCK_HEADER) + ((ParameterBlockSize + 3) / 4 * 4);
						Header->EntityID = Entity->EntityID();
						Header->DescriptorSubtype = DescriptorSubtype;
						Header->Reserved = 0;
						Header->ParameterBlockSize = ParameterBlockSize;

						SizeOfParameterBlocks -= Header->Size;

						ParameterBlocks += Header->Size;

						*OutSizeOfParameterBlocks += Header->Size;
					}
				}
			}
			break;
		
			case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
			{
				if (SizeOfParameterBlocks > sizeof(PARAMETER_BLOCK_HEADER))
				{
					PPARAMETER_BLOCK_HEADER Header = PPARAMETER_BLOCK_HEADER(ParameterBlocks); 

					PUCHAR ParameterBlock = ParameterBlocks + sizeof(PARAMETER_BLOCK_HEADER);

					PUNIT Unit = PUNIT(Entity);

					ULONG ParameterBlockSize = 0;

					if (NT_SUCCESS(Unit->SaveParameterBlock(ParameterBlock, SizeOfParameterBlocks - sizeof(PARAMETER_BLOCK_HEADER), &ParameterBlockSize)))
					{
						Header->Tag = ' Y2H';
						Header->Size = sizeof(PARAMETER_BLOCK_HEADER) + ((ParameterBlockSize + 3) / 4 * 4);
						Header->EntityID = Entity->EntityID();
						Header->DescriptorSubtype = DescriptorSubtype;
						Header->Reserved = 0;
						Header->ParameterBlockSize = ParameterBlockSize;

						SizeOfParameterBlocks -= Header->Size;

						ParameterBlocks += Header->Size;

						*OutSizeOfParameterBlocks += Header->Size;
					}
				}
			}
			break;
		}
	}

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioTopology::RestoreParameterBlocks()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioTopology::
RestoreParameterBlocks
(
	IN		PVOID	ParameterBlocks_,
	IN		ULONG	SizeOfParameterBlocks
)
{
    PAGED_CODE();

	PUCHAR ParameterBlocks = PUCHAR(ParameterBlocks_);

	while (SizeOfParameterBlocks > sizeof(PARAMETER_BLOCK_HEADER))
	{
		PPARAMETER_BLOCK_HEADER Header = PPARAMETER_BLOCK_HEADER(ParameterBlocks); 

		PUCHAR ParameterBlock = ParameterBlocks + sizeof(PARAMETER_BLOCK_HEADER);

		if (Header->Tag == ' Y2H')
		{
			PENTITY Entity = NULL;
			
			if (FindEntity(Header->EntityID, &Entity))
			{
				if (Entity->DescriptorSubtype() == Header->DescriptorSubtype)
				{
					switch (Header->DescriptorSubtype)
					{
						case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
						case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
						{
							PTERMINAL Terminal = PTERMINAL(Entity);

							Terminal->RestoreParameterBlock(ParameterBlock, Header->ParameterBlockSize);
						}
						break;
					
						case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
						case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
						case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
						case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
						case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
						{
							PUNIT Unit = PUNIT(Entity);

							Unit->RestoreParameterBlock(ParameterBlock, Header->ParameterBlockSize);
						}
						break;
					}
				}
			}
		}

		SizeOfParameterBlocks -= Header->Size;

		ParameterBlocks += Header->Size;
	}

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioTopology::GetSizeOfParameterBlocks()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
ULONG
CAudioTopology::
GetSizeOfParameterBlocks
(	void
)
{
    PAGED_CODE();

	ULONG SizeOfParameterBlocks = 0;

	for (PENTITY Entity = m_EntityList.First(); Entity; Entity = m_EntityList.Next(Entity))
	{
		UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

		switch (DescriptorSubtype)
		{
			case USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL:
			case USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL:
			{
				PTERMINAL Terminal = PTERMINAL(Entity);

				SizeOfParameterBlocks += sizeof(PARAMETER_BLOCK_HEADER) + ((Terminal->GetParameterBlockSize() + 3) / 4 * 4);
			}
			break;
		
			case USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT:
			case USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT:
			{
				PUNIT Unit = PUNIT(Entity);

				SizeOfParameterBlocks += sizeof(PARAMETER_BLOCK_HEADER) + ((Unit->GetParameterBlockSize() + 3) / 4 * 4);
			}
			break;
		}
	}

	return SizeOfParameterBlocks;
}

/*****************************************************************************
 * CAudioDevice::NonDelegatingQueryInterface()
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
CAudioDevice::
NonDelegatingQueryInterface
(
    IN		REFIID  Interface,
    OUT		PVOID * Object
)
{
    PAGED_CODE();

    ASSERT(Object);

    _DbgPrintF(DEBUGLVL_BLAB,("[CAudioDevice::NonDelegatingQueryInterface]"));

    if (IsEqualGUIDAligned(Interface,IID_IUnknown))
    {
        *Object = PVOID(PUNKNOWN(PAUDIO_DEVICE(this)));
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
        return AUDIOERR_SUCCESS;
    }

    return AUDIOERR_BAD_PARAM;
}

/*****************************************************************************
 * CAudioDevice::~CAudioDevice()
 *****************************************************************************
 *//*!
 * @ingroup AUDIO_GROUP
 * @brief
 * Destructor.
 */
CAudioDevice::
~CAudioDevice
(	void
)
{
    PAGED_CODE();

	ASSERT(m_MagicNumber == AUDIO_MAGIC);

    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioDevice::~CAudioDevice]"));

	m_InterfaceList.DeleteAllItems();

	if (m_Topology)
	{
		m_Topology->Destruct();
	}

	if (m_InterruptPipe)
	{
		m_InterruptPipe->Stop();
		
		m_InterruptPipe->Destruct();
	}

	if (m_UsbDevice)
	{
		m_UsbDevice->Release();
	}
}

/*****************************************************************************
 * CAudioDevice::Init()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Initialize the audio device.
 * @param
 * <None>
 * @return
 * Returns AUDIOERR_SUCCESS if successful, AUDIOERR_NO_MEMORY if the audio
 * device couldn't be created.
 */
AUDIOSTATUS
CAudioDevice::
Init
(
	IN		PUSB_DEVICE	UsbDevice
)
{
	PAGED_CODE();

    m_MagicNumber = AUDIO_MAGIC;

	m_UsbDevice = UsbDevice;
	m_UsbDevice->AddRef();

	m_PowerState = PowerDeviceD0;

    // Init variables for software master vol/mute
    for(ULONG count = 0; count < AUDIO_CLIENT_MAX_CHANNEL; count++)
    {
        m_MasterVolumeStep[count]   = MASTERVOL_STEP_SIZE_DB;
        m_MasterVolumeMin[count]    = MASTERVOL_MIN_DB;
        m_MasterVolumeMax[count]    = MASTERVOL_MAX_DB;
        m_MasterVolume[count]       = MASTERVOL_0_DB;
    }

    m_MasterMute        = FALSE;
    m_requireSoftMaster = FALSE;    // default assume software master vol/mute is not required
    m_NoOfSoftNode      = 0;        // default no software node

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

	if((UsbDeviceDescriptor->idVendor == 0x41E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F02/*MicroPod*/))
	{
		m_NumOfClientChannel = AUDIO_EMU0202_CHANNEL;		
	}else
	if((UsbDeviceDescriptor->idVendor == 0x41E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F04/*HulaPod*/))
	{
		m_NumOfClientChannel = AUDIO_EMU0404_CHANNEL;
	}else
	if((UsbDeviceDescriptor->idVendor == 0x41E/*Creative*/) && (UsbDeviceDescriptor->idProduct == 0x3F0A/*MicroPre*/))
	{
		m_NumOfClientChannel = AUDIO_EMU1616_CHANNEL;
	}else
    {
		m_NumOfClientChannel = AUDIO_CLIENT_MAX_CHANNEL;
    }
    // Checking to enable software master volume/mute
//    if((UsbDeviceDescriptor->idVendor == 0x41E/*Creative*/) && 
//		((UsbDeviceDescriptor->idProduct == 0x3F02/*MicroPod*/) || 
//		(UsbDeviceDescriptor->idProduct == 0x3F0A/*MicroPre*/)))
//    {
        // This is an 0202 Micropod. 
        m_requireSoftMaster = TRUE; //Requires software master volume/mute
        m_NoOfSoftNode += 2; // volume and mute node
//    } 

	PUSB_INTERFACE_DESCRIPTOR AcInterfaceDescriptor = NULL;

	AUDIOSTATUS audioStatus = m_UsbDevice->GetInterfaceDescriptor(-1, -1, ClassCode, USB_AUDIO_SUBCLASS_AUDIOCONTROL, -1, &AcInterfaceDescriptor);

	if (AUDIO_SUCCESS(audioStatus))
	{
		PUSB_AUDIO_CS_AC_INTERFACE_DESCRIPTOR CsAcInterfaceDescriptor = NULL;

		audioStatus = m_UsbDevice->GetClassInterfaceDescriptor(AcInterfaceDescriptor->bInterfaceNumber, 0, USB_AUDIO_CS_INTERFACE, (PUSB_INTERFACE_DESCRIPTOR *)&CsAcInterfaceDescriptor);

		if (AUDIO_SUCCESS(audioStatus))
		{
			// Determine if this is an Audio Device Class Specification Release 1.0 compliant device.
			// This driver will only work with USB audio devices which is 1.0 compliant.
			if (CsAcInterfaceDescriptor->bcdADC != 0x100)
			{
				audioStatus = AUDIOERR_NOT_SUPPORTED;
			}
		}

		if (AUDIO_SUCCESS(audioStatus))
		{
			ASSERT(CsAcInterfaceDescriptor->bDescriptorType == USB_AUDIO_CS_INTERFACE);
			ASSERT(CsAcInterfaceDescriptor->bDescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_HEADER);

			for (UCHAR i=0; i<CsAcInterfaceDescriptor->bInCollection; i++)
			{
				PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;

				audioStatus = m_UsbDevice->GetInterfaceDescriptor(CsAcInterfaceDescriptor->baInterfaceNr[i], 0, -1, -1, -1, &InterfaceDescriptor);

				if (AUDIO_SUCCESS(audioStatus))
				{
					if (((ClassCode == -1) || (InterfaceDescriptor->bInterfaceClass == ClassCode)) &&
						(InterfaceDescriptor->bInterfaceSubClass == USB_AUDIO_SUBCLASS_AUDIOSTREAMING))
					{
						UCHAR InterfaceNumber = InterfaceDescriptor->bInterfaceNumber;

						_DbgPrintF(DEBUGLVL_VERBOSE,("[CAudioDevice::Init]- InterfaceNumber = %d", InterfaceNumber));

						CAudioInterface * Interface = new(NonPagedPool) CAudioInterface();

						if (Interface)
						{
							audioStatus = Interface->Init(UsbDevice, InterfaceNumber);

							if (AUDIO_SUCCESS(audioStatus))
							{
								m_InterfaceList.Put(Interface);
							}
							else
							{
								delete Interface;
								break;
							}
						}
						else
						{
							audioStatus = AUDIOERR_NO_MEMORY;
							break;
						}
					}
				}
			}
		}
	}

	if (AUDIO_SUCCESS(audioStatus))
	{
		m_Topology = new(NonPagedPool) CAudioTopology();
		
		if (m_Topology)
		{
			audioStatus = m_Topology->Init(m_UsbDevice);
		}
		else
		{
			audioStatus = AUDIOERR_NO_MEMORY;
		}
	}

	if (AUDIO_SUCCESS(audioStatus))
	{
		// No need select the interface again as it is the default selection which never change.
		PUSBD_INTERFACE_INFORMATION InterfaceInfo = NULL;

		audioStatus = m_UsbDevice->GetInterfaceInformation(AcInterfaceDescriptor->bInterfaceNumber, 0, &InterfaceInfo);

		if (AUDIO_SUCCESS(audioStatus))
		{
			ASSERT(InterfaceInfo->NumberOfPipes <= 1);

			if (InterfaceInfo->NumberOfPipes)
			{
				m_InterruptPipe = new(NonPagedPool) CAudioInterruptPipe();
				
				if (m_InterruptPipe)
				{
					audioStatus = m_InterruptPipe->Init(m_UsbDevice, AcInterfaceDescriptor->bInterfaceNumber, InterfaceInfo->Pipes[0], this);

					if (AUDIO_SUCCESS(audioStatus))
					{
						audioStatus = m_InterruptPipe->Start();
					}
				}
				else
				{
					audioStatus = AUDIOERR_NO_MEMORY;
				}
			}
		}
	}

	if (!AUDIO_SUCCESS(audioStatus))
	{
		// Cleanup mess...
		if (m_Topology)
		{
			m_Topology->Destruct();
			m_Topology = NULL;
		}

		m_InterfaceList.DeleteAllItems();

		if (m_InterruptPipe)
		{
			m_InterruptPipe->Stop();
			m_InterruptPipe->Destruct();
			m_InterruptPipe = NULL;
		}

		if (m_UsbDevice)
		{
			m_UsbDevice->Release();
			m_UsbDevice = NULL;
		}
	}

	return audioStatus;
}

/*****************************************************************************
 * CAudioDevice::PowerStateChange()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Change the current power status.
 * @param
 * NewState The new power state.
 * @return
 * Returns AUDIOERR_SUCCESS if the power state changed.
 */
AUDIOSTATUS
CAudioDevice::
PowerStateChange
(
	IN		DEVICE_POWER_STATE	NewState
)
{
    PAGED_CODE();

	ASSERT(m_MagicNumber == AUDIO_MAGIC);

	if (m_Topology)
	{
		m_Topology->PowerStateChange(NewState);
	}

	for (CAudioInterface * Interface = m_InterfaceList.First(); Interface; Interface = m_InterfaceList.Next(Interface))
	{
		Interface->PowerStateChange(NewState);
	}

	if (m_InterruptPipe)
	{
		m_InterruptPipe->PowerStateChange(NewState);
	}

	m_PowerState = NewState;

    return AUDIOERR_SUCCESS;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioDevice::SetInterruptHandler()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 */
AUDIOSTATUS
CAudioDevice::
SetInterruptHandler
(
	IN		AUDIO_INTERRUPT_HANDLER_ROUTINE	InterruptHandlerRoutine,
	IN		PVOID							InterruptHandlerContext
)
{
	m_InterruptHandlerRoutine = InterruptHandlerRoutine;

	m_InterruptHandlerContext = InterruptHandlerContext;

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioDevice::OnStatusInterrupt()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 */
VOID
CAudioDevice::
OnStatusInterrupt
(
	IN		ULONG					NumberOfStatusWord,
	IN		PUSB_AUDIO_STATUS_WORD	StatusWord,
	IN		PVOID					InterruptFilter
)
{
	for (ULONG i=0; i<NumberOfStatusWord; i++)
	{
		//DbgPrint("StatusWord[%d].bmStatusType: 0x%x, bOriginator: 0x%x\n", i, StatusWord[i].bmStatusType, StatusWord[i].bOriginator);

		if (StatusWord[i].bmStatusType & USB_AUDIO_STATUS_TYPE_INTERRUPT_PENDING)
		{
			switch (StatusWord[i].bmStatusType & 0x0F) // Originator
			{
				case USB_AUDIO_STATUS_TYPE_ORIGINATOR_AC_INTERFACE:
				{
					UCHAR EntityID = StatusWord[i].bOriginator;

					AUDIO_INTERRUPT_ORIGINATOR Originator = AUDIO_INTERRUPT_ORIGINATOR_UNKNOWN;

					if (EntityID)
					{
						PENTITY Entity = NULL;

						if (m_Topology->FindEntity(EntityID, &Entity))
						{
							UCHAR DescriptorSubtype = Entity->DescriptorSubtype();

							if ((DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_INPUT_TERMINAL) ||
								(DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_OUTPUT_TERMINAL))
							{
								PTERMINAL Terminal = PTERMINAL(Entity);

								Terminal->InvalidateParameterBlock(StatusWord[i].bmStatusType);

								Originator = AUDIO_INTERRUPT_ORIGINATOR_AC_TERMINAL;
							}
							else
							if ((DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_MIXER_UNIT) ||
								(DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_SELECTOR_UNIT) ||
								(DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_FEATURE_UNIT) ||
								(DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_PROCESSING_UNIT) ||
								(DescriptorSubtype == USB_AUDIO_AC_DESCRIPTOR_EXTENSION_UNIT))
							{
								PUNIT Unit = PUNIT(Entity);

								Unit->InvalidateParameterBlock(StatusWord[i].bmStatusType);

								Originator = AUDIO_INTERRUPT_ORIGINATOR_AC_UNIT;
							}						
						}
					}
					else
					{
						// 'Virtual' Entity Interface
						//TODO: Implement this

						Originator = AUDIO_INTERRUPT_ORIGINATOR_AC_FUNCTION;
					}

					// Callback to the interrupt handler...
					if (m_InterruptHandlerRoutine)
					{
						m_InterruptHandlerRoutine(m_InterruptHandlerContext, Originator, EntityID, InterruptFilter);
					}
				}
				break;

				case USB_AUDIO_STATUS_TYPE_ORIGINATOR_AS_INTERFACE:
				{
					UCHAR InterfaceNumber = StatusWord[i].bOriginator;
					//TODO: Implement this

					AUDIO_INTERRUPT_ORIGINATOR Originator = AUDIO_INTERRUPT_ORIGINATOR_AS_INTERFACE;

					// Callback to the interrupt handler...
					if (m_InterruptHandlerRoutine)
					{
						m_InterruptHandlerRoutine(m_InterruptHandlerContext, Originator, InterfaceNumber, InterruptFilter);
					}
				}
				break;

				case USB_AUDIO_STATUS_TYPE_ORIGINATOR_AS_ENDPOINT:
				{
					UCHAR EndpointAddress = StatusWord[i].bOriginator;
					//TODO: Implement this

					AUDIO_INTERRUPT_ORIGINATOR Originator = AUDIO_INTERRUPT_ORIGINATOR_AS_ENDPOINT;

					// Callback to the interrupt handler...
					if (m_InterruptHandlerRoutine)
					{
						m_InterruptHandlerRoutine(m_InterruptHandlerContext, Originator, EndpointAddress, InterruptFilter);
					}
				}
				break;
			}
		}
	}
}

/*****************************************************************************
 * CAudioDevice::FindInterface()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Find the audio interface that matches the specified criteria.
 * @return
 * Returns the audio interface that matches the specified criteria.
 */
CAudioInterface *
CAudioDevice::
FindInterface
(
	IN		UCHAR			InterfaceNumber
)
{

	CAudioInterface * Interface;

	for (Interface = m_InterfaceList.First(); Interface; Interface = m_InterfaceList.Next(Interface))
	{
		if (Interface->InterfaceNumber() == InterfaceNumber)
		{
			break;
		}
	}

	return Interface;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioDevice::ParseInterfaces()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Enumerate the interfaces that are available on this device.
 * @param
 * Index Enumeration index.
 * @param
 * OutInterface Pointer to the PAUDIO_INTERFACE which will receive the
 * enumerated audio interface.
 * @return
 * TRUE if the specified index matches one of the interfaces, otherwise FALSE.
 */
BOOL
CAudioDevice::
ParseInterfaces
(
	IN		ULONG				Index,
	OUT		PAUDIO_INTERFACE *	OutInterface
)
{
	PAGED_CODE();

	BOOL Found = FALSE;

	ULONG idx = 0;

	for (CAudioInterface * Interface = m_InterfaceList.First(); Interface; Interface = m_InterfaceList.Next(Interface))
	{
		if (idx == Index)
		{
			*OutInterface = Interface;

			Found = TRUE;
			break;
		}

		idx++;
	}

	return Found;
}

/*****************************************************************************
 * CAudioDevice::ParseTopology()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Enumerate the audio topology that are available on this device.
 * @param
 * Index Enumeration index.
 * @return
 * Returns the audio topology that matches the specified criteria.
 */
BOOL
CAudioDevice::
ParseTopology
(
	IN		ULONG				Index,
	OUT		PAUDIO_TOPOLOGY	*	OutTopology
)
{
    PAGED_CODE();

	BOOL Found = FALSE;

	if (Index == 0)
	{
		*OutTopology = m_Topology;

		Found = TRUE;
	}
	else
	{
		*OutTopology = NULL;
	}

	return Found;
}

/*****************************************************************************
 * CAudioDevice::GetDeviceDescriptor()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioDevice::
GetDeviceDescriptor
(
	OUT		PUSB_DEVICE_DESCRIPTOR *	OutDeviceDescriptor
)
{
    PAGED_CODE();

	return m_UsbDevice->GetDeviceDescriptor(OutDeviceDescriptor);
}

/*****************************************************************************
 * CAudioDevice::SaveParameterBlocks()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioDevice::
SaveParameterBlocks
(
	IN		PVOID	ParameterBlocks,
	IN		ULONG	SizeOfParameterBlocks,
	OUT		ULONG *	OutSizeOfParameterBlocks
)
{
    PAGED_CODE();

	AUDIOSTATUS audioStatus = m_Topology->SaveParameterBlocks(ParameterBlocks, SizeOfParameterBlocks, OutSizeOfParameterBlocks);

	return audioStatus;
}

/*****************************************************************************
 * CAudioDevice::RestoreParameterBlocks()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioDevice::
RestoreParameterBlocks
(
	IN		PVOID	ParameterBlocks,
	IN		ULONG	SizeOfParameterBlocks
)
{
    PAGED_CODE();

	AUDIOSTATUS audioStatus = m_Topology->RestoreParameterBlocks(ParameterBlocks, SizeOfParameterBlocks);

	return audioStatus;
}

/*****************************************************************************
 * CAudioDevice::GetSizeOfParameterBlocks()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
ULONG
CAudioDevice::
GetSizeOfParameterBlocks
(	void
)
{
    PAGED_CODE();

	ULONG SizeOfParameterBlocks = m_Topology->GetSizeOfParameterBlocks();

	return SizeOfParameterBlocks;
}

#pragma code_seg()

/*****************************************************************************
 * CAudioDevice::AttachClient()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioDevice::
AttachClient
(
	IN		CAudioClient *	Client
)
{
	m_ClientList.Lock();

    /* Add it to the device list.  Since we're just inserting onto the
     * head, we don't have to worry about any readers. */
	m_ClientList.Put(Client);

	m_ClientList.Unlock();

	return AUDIOERR_SUCCESS;
}

/*****************************************************************************
 * CAudioDevice::DetachClient()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 */
AUDIOSTATUS
CAudioDevice::
DetachClient
(
	IN		CAudioClient *	Client
)
{
	AUDIOSTATUS audioStatus = AUDIOERR_BAD_PARAM;

	m_ClientList.Lock();

	if (m_ClientList.IsItemInList(Client))
	{
        /* Dequeue the info structure from the open list */
		m_ClientList.Remove(Client);

		audioStatus = AUDIOERR_SUCCESS;
    }

	m_ClientList.Unlock();

	return audioStatus;
}

#pragma code_seg("PAGE")

/*****************************************************************************
 * CAudioDevice::Open()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Open a particular audio channel for input or output.
 * @param
 * CallbackRoutine The function to call when the audio transitions to an empty
 * state (on output) or data becomes available (on input).
 * @param
 * CallbackData A user-specified handle which is passed as a parameter when
 * the callback is called.
 * @param
 * OutAudioClient A pointer to the CAudioClient* which will receive the newly
 * opened audio client.
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioDevice::
Open
(
    IN		AUDIO_CALLBACK_ROUTINE	CallbackRoutine,
	IN		PVOID					CallbackData,
	OUT		PAUDIO_CLIENT *			OutClient
)
{
    PAGED_CODE();

	ASSERT(m_MagicNumber == AUDIO_MAGIC);

	AUDIOSTATUS audioStatus = AUDIOERR_SUCCESS;

    CAudioClient * Client = new(NonPagedPool) CAudioClient();

	if (Client)
	{
		audioStatus = Client->Init(this, CallbackRoutine, CallbackData);

		if (AUDIO_SUCCESS(audioStatus))
		{
			AttachClient(Client);
		}

		if (!AUDIO_SUCCESS(audioStatus))
		{
			Client->Destruct();
			Client = NULL;
		}
	}
	else
	{
		audioStatus = AUDIOERR_NO_MEMORY;
	}

    *OutClient = Client;

	return audioStatus;
}

/*****************************************************************************
 * CAudioDevice::Close()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Close a previously opened audio client instance.
 * @param
 * Client The client to close.
 * @return
 * Returns AUDIOERR_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
AUDIOSTATUS
CAudioDevice::
Close
(
	IN		PAUDIO_CLIENT	Client
)
{
    PAGED_CODE();

	ASSERT(Client);
	ASSERT(m_MagicNumber == AUDIO_MAGIC);

	AUDIOSTATUS audioStatus = DetachClient(Client);

	if (AUDIO_SUCCESS(audioStatus))
	{
		Client->Destruct();
	}

    return audioStatus;
}


/*****************************************************************************
 * CAudioDevice::GetMasterVolumeRange()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Get the master volume range and step size.
 * @param
 * channel The channel to get. 
 * pVolumeMinDB The minimum volume in dB.
 * pVolumeMaxDB The maximum volume in dB.
 * pVolumeStepDB The step size in dB.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioDevice::
GetMasterVolumeRange
(
    IN  LONG    channel,
    OUT LONG*   pVolumeMinDB,
    OUT LONG*   pVolumeMaxDB,
    OUT LONG*   pVolumeStepDB
)
{
    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if(channel >= 0 && channel < m_NumOfClientChannel)
    {
        *pVolumeMinDB = m_MasterVolumeMin[channel];
        *pVolumeMaxDB = m_MasterVolumeMax[channel];
        *pVolumeStepDB = m_MasterVolumeStep[channel];
        ntStatus = STATUS_SUCCESS;
    }
//    DbgPrint("GetMasterVolumeRange return ntStatus=%x channel=%x\n",ntStatus,channel);
    return ntStatus;
}

//edit yuanfen
#pragma code_seg()
/*****************************************************************************
 * CAudioDevice::GetMasterVolume()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Get the current master volume.
 * @param
 * channel The channel to get. 
 * pVolumeDB The current volume in dB.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioDevice::
GetMasterVolume
(
    IN  LONG    channel,
    OUT LONG*   pVolumeDB
)
{
    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if(channel >= KSAUDIO_VOLUME_CHAN_MASTER && channel < m_NumOfClientChannel)
    {
        if(channel == KSAUDIO_VOLUME_CHAN_MASTER)
        {
            *pVolumeDB = max( m_MasterVolume[0],m_MasterVolume[1]);
        }
        else
            *pVolumeDB = m_MasterVolume[channel];

        ntStatus = STATUS_SUCCESS;
    }
//    DbgPrint("GetMasterVolume return ntStatus=%x channel=%x\n",ntStatus,channel);
    return ntStatus;
}

/*****************************************************************************
 * CAudioDevice::SetMasterVolume()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Set the master volume.
 * @param
 * channel The channel to set. 
 * volumeDB The volume to set in dB.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioDevice::
SetMasterVolume
(
    IN  LONG    channel,
    IN  LONG    volumeDB
)
{
    NTSTATUS ntStatus = STATUS_INVALID_PARAMETER;

	if(channel >= KSAUDIO_VOLUME_CHAN_MASTER && channel < m_NumOfClientChannel)
    {
        if(channel == KSAUDIO_VOLUME_CHAN_MASTER)
        {
            m_MasterVolume[0] = volumeDB;
            m_MasterVolume[1] = volumeDB;
        }
        else
        {
            if(volumeDB % m_MasterVolumeStep[channel] == 0)
            {
                // Volume is in the step size
                m_MasterVolume[channel] = volumeDB;
            }
            else
            {
                m_MasterVolume[channel] = volumeDB;
#if 0  //not needed at the moment
                if(volumeDB >= 0)
                {
                    // +ve volume - gain
                    if(volumeDB % m_MasterVolumeStep[channel-1] > (m_MasterVolumeStep[channel-1]/2))
                    {
                         //Round up to nearest step value
                        m_MasterVolume[channel-1] = ((volumeDB / m_MasterVolumeStep[channel-1]) + 1) * m_MasterVolumeStep[channel-1];
                    }
                    else
                    {
                         //Round down to nearest step value
                        m_MasterVolume[channel-1] = (volumeDB / m_MasterVolumeStep[channel-1]) * m_MasterVolumeStep[channel-1];
                    }
                }
                else
                {
                    // -ve volume - attenuation
                    if(volumeDB % m_MasterVolumeStep[channel-1] < (m_MasterVolumeStep[channel-1]/2))
                    {
                         //Round up to nearest step value
                        m_MasterVolume[channel-1] = ((volumeDB / m_MasterVolumeStep[channel-1]) + 1) * m_MasterVolumeStep[channel-1];
                    }
                    else
                    {
                         //Round down to nearest step value
                        m_MasterVolume[channel-1] = (volumeDB / m_MasterVolumeStep[channel-1]) * m_MasterVolumeStep[channel-1];
                    }
                }
#endif
            }
        }
        ntStatus = STATUS_SUCCESS;
    }
    DbgPrint("SetMasterVolume return ntStatus=%x channel=%x\n",ntStatus,channel);
    return ntStatus;
}

/*****************************************************************************
 * CAudioDevice::GetMasterMute()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Get the master mute state.
 * @param
 * pMute The master mute state.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioDevice::
GetMasterMute
(
    OUT BOOL*   pMute
)
{
    *pMute = m_MasterMute;
    return STATUS_SUCCESS;
}


/*****************************************************************************
 * CAudioDevice::SetMasterMute()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Set the master mute state.
 * @param
 * mute The master mute state to set.
 * @return
 * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
 * error code.
 */
NTSTATUS
CAudioDevice::
SetMasterMute(
    IN  BOOL    mute
)
{
    m_MasterMute = mute;
    return STATUS_SUCCESS;
}

//edit yuanfen
#pragma code_seg("PAGE")

BOOL
CAudioDevice::
IsSoftwareMasterVolMute(void)
{
    return m_requireSoftMaster;
}

/*****************************************************************************
 * CAudioDevice::GetNoOfSupportedChannel()
 *****************************************************************************
 * @ingroup AUDIO_GROUP
 * @brief
 * Get the number of support channel.
 * @param
 * number of support channel must be less than or equal to AUDIO_CLIENT_MAX_CHANNEL
 * @return
 * 
 */
void
CAudioDevice::
GetNoOfSupportedChannel(OUT ULONG* pChNumber)
{
	*pChNumber = m_NumOfClientChannel;
}

#pragma code_seg()
