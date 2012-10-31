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
 * @file	   device.h
 * @brief	   USB device definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __USB_DEVICE_H__
#define __USB_DEVICE_H__

#include "common.h"
#include "utils.h"

#include "config.h"

/*****************************************************************************
 * Defines
 */
typedef struct
{
	PVOID	Context;
	BOOL	ProcessEvent;
	KEVENT	Event;
} USBD_IO_COMPLETION_CONTEXT, *PUSBD_IO_COMPLETION_CONTEXT;

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CUsbDevice
 *****************************************************************************
 */
class CUsbDevice
{
private:
    CUsbDevice *				m_Next;			/*!< @brief The next enity in the linked list. */
    CUsbDevice *				m_Prev;			/*!< @brief The previous enity in the linked list. */
    PVOID						m_Owner;		/*!< @brief The link list owner. */

protected:
	PDEVICE_OBJECT				m_FunctionalDeviceObject;
	PDEVICE_OBJECT				m_NextLowerDeviceObject;

	USB_DEVICE_DESCRIPTOR		m_UsbDeviceDescriptor;

	UCHAR						m_CurrentConfigurationIndex;

	CList<CUsbConfiguration>	m_ConfigurationList;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CUsbDevice() { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
	~CUsbDevice();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CUsbDevice public methods
     *
     * These are public member functions.  See CONFIG.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
	    IN      PDEVICE_OBJECT      NextLowerDeviceObject
	);
		
	NTSTATUS GetUsbConfiguration
	(
		IN		UCHAR					ConfigurationIndex,
		OUT		CUsbConfiguration **	OutUsbConfiguration
	);

	NTSTATUS SelectUsbConfiguration
	(
		IN		UCHAR	ConfigurationIndex
	);

	NTSTATUS DeselectUsbConfiguration
	(	void
	);

	NTSTATUS GetCurrentUsbConfiguration
	(
		OUT		CUsbConfiguration **	OutUsbConfiguration
	);

    /*************************************************************************
	 * The other USB-Audio specification descriptions.
     */
	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);


	/*************************************************************************
     * Usb Device functions
     */
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
	NTSTATUS GetDeviceDescriptor
	(
		OUT		PUSB_DEVICE_DESCRIPTOR	DeviceDescriptor
	);
	PUSB_CONFIGURATION_DESCRIPTOR GetConfigurationDescriptor
	(
		IN		UCHAR	ConfigurationIndex
	);
	NTSTATUS GetStringDescriptor
	(
		IN		UCHAR					Index,
		IN		USHORT					LanguageId,
		IN		PUSB_STRING_DESCRIPTOR	StringDescriptor
	);

	/*************************************************************************
     * Control Class functions
     */
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

typedef CUsbDevice * PUSB_DEVICE;

#endif // __USB_DEVICE_H__
