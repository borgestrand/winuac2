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
 * @file       IKsAdapter.h
 * @brief      Interfaces exported by the KS adapter object.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  12-16-2004 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _IKS_ADAPTER_H_
#define _IKS_ADAPTER_H_

#include "Common.h"
#include "UsbDev.h"
#include "Audio.h"
#include "Midi.h"

/*!
 * @defgroup DRIVER_ADAPTER_GROUP Driver Adapter Module
 */

/*****************************************************************************
 * Defines
 *****************************************************************************
 */

/*****************************************************************************
 * Constants
 *****************************************************************************
 */
/*! @brief IKsAdapter interface GUID. */
DEFINE_GUID(IID_IKsAdapter,
0x2701f54f, 0xca70, 0x4256, 0x9e, 0xb8, 0x57, 0x91, 0x32, 0x72, 0x59, 0xa8);

/*****************************************************************************
 *//*! @interface IKsAdapter
 *****************************************************************************
 * @ingroup DRIVER_ADAPTER_GROUP
 * @brief
 * Interface for KS adapter object.
 */
DECLARE_INTERFACE_(IKsAdapter,IUnknown)
{
    /*!
     * @brief
     * Initialize an adapter common object.
     * @param
     * KsDevice Pointer to the KSDEVICE structure.
     * @return
     * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
     * error code.
     */
	STDMETHOD_(NTSTATUS,Init)
    (   THIS_
        IN      PKSDEVICE	KsDevice
	)   PURE;

    /*!
     * @brief
     * Gets a pointer to the KS device.
     * @param
     * None
     * @return
     * Returns the pointer to the KS device.
     */
    STDMETHOD_(PKSDEVICE,GetKsDevice)
    (   THIS
    )   PURE;

    /*!
     * @brief
     * Gets a pointer to the USB device.
     * @param
     * None
     * @return
     * Returns the pointer to the USB device.
     */
    STDMETHOD_(PUSB_DEVICE,GetUsbDevice)
    (   THIS
    )   PURE;

    /*!
     * @brief
     * Gets a pointer to the audio device.
     * @param
     * None
     * @return
     * Returns the pointer to the audio device.
     */
    STDMETHOD_(PAUDIO_DEVICE,GetAudioDevice)
    (   THIS
    )   PURE;

    /*!
     * @brief
     * Gets a pointer to the MIDI device.
     * @param
     * None
     * @return
     * Returns the pointer to the MIDI device.
     */
    STDMETHOD_(PMIDI_DEVICE,GetMidiDevice)
    (   THIS
    )   PURE;

	/*!
     * @brief
     * Gets the current language ID.
     * @param
     * None
     * @return
     * Returns the current language ID.
     */
    STDMETHOD_(USHORT,GetLanguageId)
    (   THIS
    )   PURE;

    /*!
     * @brief
     * Determine if the adapter is ready for I/O operations.
     * @param
     * None
     * @return
     * Returns TRUE if the adapter is ready for I/O, otherwise FALSE.
     */
    STDMETHOD_(BOOL,IsReadyForIO)
    (   THIS
    )   PURE;

    /*!
     * @brief
     * Add a reference to the device.
     * @param
     * None
     * @return
     * Returns the previous reference count.
     */
    STDMETHOD_(LONG,ReferenceDevice)
    (   THIS
    )   PURE;

    /*!
     * @brief
     * Remove a reference from the device.
     * @param
     * None
     * @return
     * Returns the previous reference count.
     */
    STDMETHOD_(LONG,DereferenceDevice)
    (   THIS
    )   PURE;

    /*!
     * @brief
     * Setup the device for firmware upgrade process.
     * @param
     * LockOperation TRUE to obtain exclusive use of the device while upgrading
     * the device firmware. FALSE to relinquish the exclusive use of the device.
     * @return
     * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
     * error code.
     */
    STDMETHOD_(NTSTATUS,SetFirmwareUpgradeLock)
    (   THIS_
		IN		BOOL	LockOperation
    )   PURE;

	/*!
	* @brief
	* Store information about a device such as device description.
	* @param
	* DeviceProperty Type of device property.
	* @param
	* BufferLength Length in bytes of the @em PrpertyBuffer.
	* @param
	* PropertyBuffer Pointer to the property buffer.
	* @return
	* Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
	* error code.
	*/
    STDMETHOD_(NTSTATUS,SetDeviceProperty)
    (	THIS_
        IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,
        IN      ULONG                       BufferLength,
        IN      PVOID                       PropertyBuffer
    )	PURE;

    /*!
     * @brief
     * Install the sub devices.
     * @param
     * DeviceObject Pointer to the DEVICE_OBJECT structure.
     * @param
     * Irp Pointer to IRP structure.
     * @param
     * ResourceList The resource list from StartDevice().
     * @return
     * Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
     * error code.
     */
	STDMETHOD_(NTSTATUS, InstallFilterFactories)
	(	THIS
	)	PURE;

   /*!
	* @brief
	* Add the subdevice interface.
	* @param
	* SubDeviceName Name of the sub device.
	* @param
	* InterfaceClassGuid Interface class guid.
	* @return
	* Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
	* error code.
	*/
	STDMETHOD_(NTSTATUS,AddSubDeviceInterface)
	(	THIS_
		IN		PWCHAR	SubDeviceName,
		IN      REFGUID InterfaceClassGuid
	)	PURE;

   /*!
	* @brief
	* Set the subdevice parameters.
	* @param
	* SubDeviceName Name of the sub device.
	* @param
	* InterfaceClassGuid Interface class guid.
	* @param
	* ValueName Name of the value to set.
	* @param
	* Type Type fo the value to set.
	* @param
	* Data Pointer to the value.
	* @param
	* DataSize Size of the value.
	* @return
	* Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
	* error code.
	*/
	STDMETHOD_(NTSTATUS,SetSubDeviceParameter)
	(	THIS_
		IN		PWCHAR	SubDeviceName,
		IN      REFGUID InterfaceClassGuid,
		IN      PWCHAR  ValueName,
		IN      ULONG   Type,
		IN      PVOID   Data,
		IN      ULONG   DataSize
	)	PURE;

   /*!
	* @brief
	* Get the subdevice parameter.
	* @param
	* SubDeviceName Name of the sub device.
	* @param
	* InterfaceClassGuid Interface class guid.
	* @param
	* ValueName Name of the value to get.
	* @param
	* Type Type fo the value to get.
	* @param
	* Data Pointer to the value.
	* @param
	* DataSize Pointer to the location that this method put the size of the
	* value.
	* @return
	* Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
	* error code.
	*/
	STDMETHOD_(NTSTATUS, GetSubDeviceParameter)
	(	THIS_
		IN		PWCHAR	SubDeviceName,
		IN      REFGUID InterfaceClassGuid,
		IN      PWCHAR  ValueName,
		IN      PULONG  Type,
		IN      PVOID   Data,
		IN      PULONG  DataSize
	) PURE;

   /*!
	* @brief
	* Read specified data from Driver/<SubKeyName> key.
	* @param
	* SubKeyName Sub key name.
	* @param
	* ValueName Value name.
	* @param
	* Data Pointer to the buffer that store the information read.
	* @param
	* DataSize Size of the data buffer.
	* @param
	* OutDataSize Size of returned data.
	* @param
	* OutDataType Type of data.
	* @return
	* Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
	* error code.
	*/
	STDMETHOD_(NTSTATUS, RegistryReadFromDriverSubKey)
	(	THIS_
		IN      PWCHAR	SubKeyName,
		IN      PWCHAR  ValueName,
		OUT     PVOID   Data,
		IN      ULONG   DataSize,
		OUT     ULONG * OutDataSize		OPTIONAL,
		OUT     ULONG * OutDataType		OPTIONAL
	)	PURE;

   /*!
	* @brief
	* Write specified data to Driver/<SubKeyName> key.
	* @param
	* SubKeyName Sub key name.
	* @param
	* ValueName Value name.
	* @param
	* Data Pointer to the buffer that store the information read.
	* @param
	* DataSize Size of the data buffer.
    * @param
    * DataType Type of data.
	* @return
	* Returns STATUS_SUCCESS if successful. Otherwise, returns an appropriate
	* error code.
	*/
	STDMETHOD_(NTSTATUS, RegistryWriteToDriverSubKey)
	(	THIS_
		IN      PWCHAR	SubKeyName,
		IN      PWCHAR  ValueName,
		IN		PVOID   Data,
		IN      ULONG   DataSize,
		IN		ULONG	DataType
	)	PURE;
};

/*! @brief Pointer to the IKsAdapter interface. */
typedef IKsAdapter *PKSADAPTER;

/*! @brief IKsAdapter interface implementation macro. */
#define IMP_IKsAdapter\
    STDMETHODIMP_(NTSTATUS) Init\
    (   IN      PKSDEVICE	KsDevice\
    );\
    STDMETHODIMP_(PKSDEVICE) GetKsDevice\
    (   void\
    );\
    STDMETHODIMP_(PUSB_DEVICE) GetUsbDevice\
    (   void\
    );\
    STDMETHODIMP_(PAUDIO_DEVICE) GetAudioDevice\
    (   void\
    );\
    STDMETHODIMP_(PMIDI_DEVICE) GetMidiDevice\
    (   void\
    );\
    STDMETHODIMP_(USHORT) GetLanguageId\
    (   void\
    );\
    STDMETHODIMP_(BOOL) IsReadyForIO\
    (   void\
    );\
    STDMETHODIMP_(LONG) ReferenceDevice\
    (   void\
    );\
    STDMETHODIMP_(LONG) DereferenceDevice\
    (   void\
    );\
    STDMETHODIMP_(NTSTATUS) SetFirmwareUpgradeLock\
    (   IN		BOOL	LockOperation\
    );\
    STDMETHODIMP_(NTSTATUS) SetDeviceProperty\
    (	IN      DEVICE_REGISTRY_PROPERTY    DeviceProperty,\
        IN      ULONG                       BufferLength,\
        IN      PVOID                       PropertyBuffer\
    );\
	STDMETHODIMP_(NTSTATUS) InstallFilterFactories\
	(	void\
	);\
	STDMETHODIMP_(NTSTATUS) AddSubDeviceInterface\
	(	IN		PWCHAR	SubDeviceName,\
		IN      REFGUID InterfaceClassGuid\
	);\
	STDMETHODIMP_(NTSTATUS) SetSubDeviceParameter\
	(	IN		PWCHAR	SubDeviceName,\
		IN      REFGUID InterfaceClassGuid,\
		IN      PWCHAR  ValueName,\
		IN      ULONG   Type,\
		IN      PVOID   Data,\
		IN      ULONG   DataSize\
	);\
	STDMETHODIMP_(NTSTATUS) GetSubDeviceParameter\
	(	IN		PWCHAR	SubDeviceName,\
		IN      REFGUID InterfaceClassGuid,\
		IN      PWCHAR  ValueName,\
		IN      PULONG  Type,\
		IN      PVOID   Data,\
		IN      PULONG  DataSize\
	);\
	STDMETHODIMP_(NTSTATUS) RegistryReadFromDriverSubKey\
	(	IN      PWCHAR	SubKeyName,\
		IN      PWCHAR  ValueName,\
		OUT     PVOID   Data,\
		IN      ULONG   DataSize,\
		OUT     ULONG * OutDataSize		OPTIONAL,\
		OUT     ULONG * OutDataType		OPTIONAL\
	);\
	STDMETHODIMP_(NTSTATUS) RegistryWriteToDriverSubKey\
	(	IN      PWCHAR	SubKeyName,\
		IN      PWCHAR  ValueName,\
		OUT     PVOID   Data,\
		IN      ULONG   DataSize,\
		IN		ULONG	DataType\
	)

#endif // _IKS_ADAPTER_H_
