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
 * @file       UsbDev.h
 * @brief      USB device class definitions.
 * @details	   This is a derivative work from the KS1000's SBUSB.* codebase.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-26-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _USB_DEVICE_H_
#define _USB_DEVICE_H_

#include "Common.h"

#pragma warning(push)
#pragma warning(disable: 4200)
#include <usbdi.h>
#pragma warning(pop)

extern "C"
{
#include <usbdlib.h>
}

#include "usbbusif.h"

typedef struct
{
	UCHAR				EndpointAddress;
    USBD_PIPE_HANDLE	PipeHandle;
    ULONG               MaximumTransferSize;
    ULONG               MaximumPacketSize;
} INTERFACE_INITIALIZATION_PARAMETERS, *PINTERFACE_INITIALIZATION_PARAMETERS;

typedef VOID (*USB_DEVICE_CALLBACK)(ULONG Reason, PVOID Parameter, PVOID Context);

#define USB_DEVICE_CALLBACK_REASON_DEVICE_DISCONNECTED	0x00000001

typedef struct
{
	PVOID	Context;
	BOOL	ProcessEvent;
	KEVENT	Event;
} USBD_IO_COMPLETION_CONTEXT, *PUSBD_IO_COMPLETION_CONTEXT;

typedef struct
{
	USHORT	Location;
	USHORT	wLANGID;
} LANGUAGE_SUPPORT, *PLANGUAGE_SUPPORT;

#define LANGUAGE_SUPPORT_LOCATION_UNKNOWN	0
#define LANGUAGE_SUPPORT_LOCATION_FILE		1
#define LANGUAGE_SUPPORT_LOCATION_DEVICE	2

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CUsbDevice
 *****************************************************************************
 * @brief
 * MIDI miniport.
 * @details
 * This object is associated with the device and is
 * created when the device is started.  The class inherits CUnknown so it
 * automatically gets reference counting and aggregation support.
 */
class CUsbDevice
:	public CUnknown
{
private:
    PDEVICE_OBJECT                  m_NextLowerDeviceObject;

    // USB configuration
    USB_DEVICE_DESCRIPTOR           m_UsbDeviceDescriptor;
    PUSB_CONFIGURATION_DESCRIPTOR   m_UsbConfigurationDescriptor;
    USBD_CONFIGURATION_HANDLE       m_UsbConfigurationHandle;

	PUSBD_INTERFACE_LIST_ENTRY      m_InterfaceList;
    ULONG							m_NumInterfaces;

	USB_BUS_INTERFACE_USBDI_V0		m_BusInterfaceV0;
	USB_BUS_INTERFACE_USBDI_V1		m_BusInterfaceV1;

	// Flag
    ULONG							m_Halted;

	USB_DEVICE_CALLBACK				m_CallbackRoutine;
	PVOID							m_CallbackData;

	// Language support.
	WCHAR							m_LanguageFile[MAX_PATH];
	LANGUAGE_SUPPORT				m_LanguageSupport[126];
	ULONG							m_NumberOfLanguageSupported;

	ULONG							m_SyncFrameNumber;

public:
    /*************************************************************************
     * The following two macros are from STDUNK.H.  DECLARE_STD_UNKNOWN()
     * defines inline IUnknown implementations that use CUnknown's aggregation
     * support.  NonDelegatingQueryInterface() is declared, but it cannot be
     * implemented generically.  Its definition appears in USBDEV.CPP.
     * DEFINE_STD_CONSTRUCTOR() defines inline a constructor which accepts
     * only the outer unknown, which is used for aggregation.  The standard
     * create macro (in USBDEV.CPP) uses this constructor.
     */
    DECLARE_STD_UNKNOWN();
    DEFINE_STD_CONSTRUCTOR(CUsbDevice);

	~CUsbDevice(void);

    /*************************************************************************
     * CUsbDevice methods
     */
	NTSTATUS Init
	(
        IN      PDEVICE_OBJECT      NextLowerDeviceObject,
		IN		USB_DEVICE_CALLBACK	CallbackRoutine,
		IN		PVOID				CallbackData
	);
	NTSTATUS SetupLanguageSupport
	(
		IN		PWSTR	LanguageFile
	);
	NTSTATUS QueryBusInterface
	(
		IN		USHORT		BusInterfaceVersion,
		IN		PINTERFACE	BusInterface,
		IN		USHORT		BusInterfaceSize
	);
	NTSTATUS StopDevice
	(	void
	);
	NTSTATUS StartDevice
	(	void
	);
	NTSTATUS ConfigureDevice
	(	void
	);
	NTSTATUS UnconfigureDevice
	(	void
	);
	NTSTATUS SuspendDevice
	(	void
	);
	NTSTATUS ResumeDevice
	(	void
	);
	PUSBD_INTERFACE_LIST_ENTRY BuildInterfaceList
	(
		IN		PUSB_CONFIGURATION_DESCRIPTOR	ConfigurationDescriptor
	);
	NTSTATUS SelectDefaultInterface
	(	void
	);
	NTSTATUS SelectAlternateInterface
	(
		IN		UCHAR	InterfaceNumber,
		IN		UCHAR	AlternateSetting
	);
	NTSTATUS GetInterfaceInformation
	(
		IN		UCHAR							InterfaceNumber,
		IN		UCHAR							AlternateSetting,
		OUT		PUSBD_INTERFACE_INFORMATION *	OutInterfaceInformation
	);
	NTSTATUS CreateIrp
	(
		OUT		PIRP *	OutIrp
	);
	NTSTATUS RecycleIrp
	(
		IN		PURB					Urb,
		IN		PIRP					Irp,
		IN		PIO_COMPLETION_ROUTINE	CompletionRoutine,
		IN		PVOID					Context
	);
	NTSTATUS ResetIrp
	(
		IN		PIRP	Irp
	);
	NTSTATUS CallUSBD
	(
		IN		PURB			Urb,
		IN		PLARGE_INTEGER	TimeOut = NULL
	);
	NTSTATUS GetCurrentFrameNumber
	(
		OUT		ULONG *	OutFrameNumber
	);
	VOID SetSyncFrameNumber
	(
		IN		ULONG	SyncFrameNumber
	);
	ULONG GetSyncFrameNumber
	(	void
	);
	NTSTATUS GetDeviceDescriptor
	(
		OUT		PUSB_DEVICE_DESCRIPTOR *	OutDeviceDescriptor
	);
	NTSTATUS GetConfigurationDescriptor
	(
		OUT		PUSB_CONFIGURATION_DESCRIPTOR *	OutConfigurationDescriptor
	);
	NTSTATUS GetInterfaceDescriptor
	(
		IN		LONG						InterfaceNumber,
		IN		LONG						AlternateSetting,
		IN		LONG						InterfaceClass,
		IN		LONG						InterfaceSubClass,
		IN		LONG						InterfaceProtocol,
		OUT		PUSB_INTERFACE_DESCRIPTOR *	OutInterfaceDescriptor
	);
	NTSTATUS GetEndpointDescriptor
	(
		IN		UCHAR						InterfaceNumber,
		IN		UCHAR						AlternateSetting,
		IN		UCHAR						EndpointAddress,
		OUT		PUSB_ENDPOINT_DESCRIPTOR *	OutEndpointDescriptor
	);
	NTSTATUS GetEndpointDescriptorByIndex
	(
		IN		UCHAR						InterfaceNumber,
		IN		UCHAR						AlternateSetting,
		IN		UCHAR						PipeIndex,
		OUT		PUSB_ENDPOINT_DESCRIPTOR *	OutEndpointDescriptor
	);
	NTSTATUS GetStringDescriptor
	(
		IN		UCHAR					Index,
		IN		USHORT					LanguageId,
		IN		PUSB_STRING_DESCRIPTOR	StringDescriptor
	);
	NTSTATUS GetClassInterfaceDescriptor
	(
		IN		UCHAR						InterfaceNumber,
		IN		UCHAR						AlternateSetting,
		IN		UCHAR						ClassSpecificDescriptorType,
		OUT		PUSB_INTERFACE_DESCRIPTOR *	OutInterfaceDescriptor
	);
	NTSTATUS GetClassEndpointDescriptor
	(
		IN		UCHAR						InterfaceNumber,
		IN		UCHAR						AlternateSetting,
		IN		UCHAR						EndpointAddress,
		IN		UCHAR						ClassSpecificDescriptorType,
		OUT		PUSB_ENDPOINT_DESCRIPTOR *	OutEndpointDescriptor
	);
	BOOL IsDeviceHighSpeed
	(	void
	);
	NTSTATUS ClearFeature
	(
		IN		USHORT	Operation,
		IN		USHORT	FeatureSelector,
		IN		USHORT	Index
	);

	// USB spec chapter 9 functions

    // Control Class functions
	NTSTATUS CustomCommand
	(
		IN		UCHAR			RequestType,
		IN		UCHAR			Request,
		IN		USHORT			Value,
		IN		USHORT			Index,
		IN		PVOID			Buffer,
		IN		ULONG			BufferLength,
		OUT		PULONG			OutBufferLength,
		IN		BOOLEAN			Input,
		IN		PLARGE_INTEGER	TimeOut = NULL
	);
	NTSTATUS ControlClassEndpointCommand
	(
		IN		UCHAR	Request,
		IN		USHORT	Value,
		IN		USHORT	Index,
		IN		PVOID	Buffer,
		IN		ULONG	BufferLength,
		OUT		PULONG	OutBufferLength,
		IN		BOOLEAN Input
	);
	NTSTATUS ControlClassInterfaceCommand
	(
		IN		UCHAR	Request,
		IN		USHORT	Value,
		IN		USHORT	Index,
		IN		PVOID	Buffer,
		IN		ULONG	BufferLength,
		OUT		PULONG	OutBufferLength,
		IN		BOOLEAN Input
	);

    // pipe related functions
	NTSTATUS ResetPipe
	(
		IN		USBD_PIPE_HANDLE	PipeHandle
	);
	NTSTATUS AbortPipe
	(
		IN		USBD_PIPE_HANDLE	PipeHandle
	);

    // parent port related functions
	NTSTATUS ResetPort
	(	void
	);
	NTSTATUS GetPortStatus
	(
		OUT		PULONG	OutPortStatus
	);

	/*************************************************************************
     * Static
     */
	static
	NTSTATUS CompletionRoutine
	(
		IN		PDEVICE_OBJECT	DeviceObject,
		IN		PIRP            Irp,
		IN		PVOID           Context
	);
};

typedef class CUsbDevice *	PUSB_DEVICE;

#if 1//DBG
VOID
PrintUsbDescriptor
(
	IN		PVOID	Descriptor
);
#define PRINT_USB_DESCRIPTOR(desc)	PrintUsbDescriptor(desc)
#else
#define PRINT_USB_DESCRIPTOR(desc)
#endif

#endif  //_USB_DEVICE_H_
