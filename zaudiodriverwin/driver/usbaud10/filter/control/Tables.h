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
 * @file       Tables.h
 * @brief      Device control filter tables.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  04-11-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _DEVICECONTROL_TABLES_H_
#define _DEVICECONTROL_TABLES_H_

/*****************************************************************************
 * Defines.
 */
/*****************************************************************************
 * KsFilterPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Filter properties.
 */
DEFINE_KSPROPERTY_TABLE(KsFilterPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_DEVICE_DESCRIPTOR,			// Id
		CControlFilter::GetPropertyHandler,					// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_DEVICE_DESCRIPTOR),			// MinProperty
		sizeof(USB_DEVICE_DESCRIPTOR),						// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CONFIGURATION_DESCRIPTOR,	// Id
		CControlFilter::GetPropertyHandler,					// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CONFIGURATION_DESCRIPTOR),		// MinProperty
		sizeof(USB_CONFIGURATION_DESCRIPTOR),				// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_INTERFACE_DESCRIPTOR,		// Id
		CControlFilter::GetPropertyHandler,					// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_INTERFACE_DESCRIPTOR),			// MinProperty
		sizeof(USB_INTERFACE_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_ENDPOINT_DESCRIPTOR,		// Id
		CControlFilter::GetPropertyHandler,					// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_ENDPOINT_DESCRIPTOR),			// MinProperty
		sizeof(USB_ENDPOINT_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_STRING_DESCRIPTOR,			// Id
		CControlFilter::GetPropertyHandler,					// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_STRING_DESCRIPTOR),			// MinProperty
		sizeof(USB_STRING_DESCRIPTOR),						// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR,// Id
		CControlFilter::GetPropertyHandler,					// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CLASS_INTERFACE_DESCRIPTOR),	// MinProperty
		sizeof(USB_INTERFACE_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR,	// Id
		CControlFilter::GetPropertyHandler,					// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CLASS_ENDPOINT_DESCRIPTOR),	// MinProperty
		sizeof(USB_ENDPOINT_DESCRIPTOR),					// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND,			// Id
		CControlFilter::GetPropertyHandler,					// GetPropertyHandler or GetSupported
		sizeof(DEVICECONTROL_CUSTOM_COMMAND),				// MinProperty
		0,													// MinData
		CControlFilter::SetPropertyHandler,					// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_LOCK,		// Id
		NULL,												// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),									// MinProperty
		0,													// MinData
		CControlFilter::SetPropertyHandler,					// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_UNLOCK,	// Id
		NULL,												// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),									// MinProperty
		0,													// MinData
		CControlFilter::SetPropertyHandler,					// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	)
};	

/*****************************************************************************
 * KsFilterPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Filter property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(KsFilterPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_DeviceControl,				// Set
		SIZEOF_ARRAY(KsFilterPropertyTable),	// PropertiesCount
		KsFilterPropertyTable,					// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * KsFilterAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Filter automation table.
 */
DEFINE_KSAUTOMATION_TABLE(KsFilterAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(KsFilterPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS_NULL
};

/*****************************************************************************
 * KSNAME_FILTER
 *****************************************************************************
 * {4C9207A9-F9E0-4a74-8B35-4BC6CB0BF986}
 */
static const 
GUID KSNAME_FILTER = 
{ 0x4c9207a9, 0xf9e0, 0x4a74, { 0x8b, 0x35, 0x4b, 0xc6, 0xcb, 0xb, 0xf9, 0x86 } };

/*****************************************************************************
 * KsFilterCategories
 *****************************************************************************
 */
/*!
 * @brief
 * List of categories.
 */
const
GUID KsFilterCategories[] =
{
    STATICGUIDOF(KSCATEGORY_DEVICECONTROL)
};

/*****************************************************************************
 * KsFilterDispatch
 *****************************************************************************
 *//*!
 * @brief
 * This is the dispatch table for the filter.  It provides notification
 * of creation, closure, processing for the filter.
 */
const 
KSFILTER_DISPATCH KsFilterDispatch = 
{
    CControlFilter::DispatchCreate,		// Filter Create
    NULL,								// Filter Close
    NULL,                               // Filter Process
    NULL                                // Filter Reset
};

/*****************************************************************************
 * KsFilterDescriptor
 *****************************************************************************
 *//*!
 * @brief
 * The KS filter description. 
 */
const 
KSFILTER_DESCRIPTOR KsFilterDescriptor = 
{
    &KsFilterDispatch,					// Dispatch
    &KsFilterAutomationTable,			// AutomationTable
    KSFILTER_DESCRIPTOR_VERSION,		// Version
    0,									// Flags
    &KSNAME_FILTER,						// ReferenceGuid
	0,									// PinDescriptorsCount
	sizeof(KSPIN_DESCRIPTOR_EX),		// PinDescriptorSize
    NULL,								// PinDescriptors
    SIZEOF_ARRAY(KsFilterCategories),	// CategoriesCount
	KsFilterCategories,					// Categories
	0,									// NodeDescriptorsCount
    sizeof(KSNODE_DESCRIPTOR),			// NodeDescriptorSize
    NULL,								// NodeDescriptors
    0,									// ConnectionsCount
    NULL,								// Connections
    NULL								// ComponentId
};

#endif // _DEVICECONTROL_TABLES_H_
