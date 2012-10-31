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
 * @brief      MIDI topology tables.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _MIDI_TOPO_TABLES_H_
#define _MIDI_TOPO_TABLES_H_

/*****************************************************************************
 * MidiElementPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * MIDI element property items.
 */
DEFINE_KSPROPERTY_TABLE(MidiElementPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		0,											// Id
		CMidiFilter::GetMidiElementCapability,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),						// MinProperty
		sizeof(BOOL),								// MinData
		NULL,										// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		CMidiFilter::SupportMidiElementCapability,	// SupportHandler
		0											// SerializedSize
	)
};

/*****************************************************************************
 * MidiElementPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * MIDI element property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(MidiElementPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&GUID_DMUS_PROP_GM_Hardware,			// Set
		SIZEOF_ARRAY(MidiElementPropertyTable),	// PropertiesCount
		MidiElementPropertyTable,				// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&GUID_DMUS_PROP_GS_Hardware,			// Set
		SIZEOF_ARRAY(MidiElementPropertyTable),	// PropertiesCount
		MidiElementPropertyTable,				// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&GUID_DMUS_PROP_XG_Hardware,			// Set
		SIZEOF_ARRAY(MidiElementPropertyTable),	// PropertiesCount
		MidiElementPropertyTable,				// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&GUID_DMUS_PROP_XG_Capable,				// Set
		SIZEOF_ARRAY(MidiElementPropertyTable),	// PropertiesCount
		MidiElementPropertyTable,				// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&GUID_DMUS_PROP_GS_Capable,				// Set
		SIZEOF_ARRAY(MidiElementPropertyTable),	// PropertiesCount
		MidiElementPropertyTable,				// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&GUID_DMUS_PROP_DLS1,					// Set
		SIZEOF_ARRAY(MidiElementPropertyTable),	// PropertiesCount
		MidiElementPropertyTable,				// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&GUID_DMUS_PROP_DLS2,					// Set
		SIZEOF_ARRAY(MidiElementPropertyTable),	// PropertiesCount
		MidiElementPropertyTable,				// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * MidiElementAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * MIDI element automation table.
 */
DEFINE_KSAUTOMATION_TABLE(MidiElementAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(MidiElementPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS_NULL
};

#ifdef ENABLE_DIRECTMUSIC_SUPPORT
/*****************************************************************************
 * SynthPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Synth property items.
 */
DEFINE_KSPROPERTY_TABLE(SynthPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_SYNTH_CAPS,					// Id
		CMidiFilter::GetSynthCaps,				// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(SYNTHCAPS),						// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		NULL,									// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_SYNTH_PORTPARAMETERS,					// Id
		CMidiFilter::GetSynthPortParameters,				// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY)+ sizeof(SYNTH_PORTPARAMS),	// MinProperty
		sizeof(SYNTH_PORTPARAMS),							// MinData
		NULL,												// SetPropertyHandler or SetSupported
		NULL,												// Values
		0,													// RelationsCount
		NULL,												// Relations
		NULL,												// SupportHandler
		0													// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_SYNTH_CHANNELGROUPS,			// Id
		CMidiFilter::GetSynthChannelGroups,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		CMidiFilter::SetSynthChannelGroups,		// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		NULL,									// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_SYNTH_LATENCYCLOCK,			// Id
		CMidiFilter::GetSynthLatencyClock,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONGLONG),						// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		NULL,									// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * SynthClockPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Synth clock property items.
 */
DEFINE_KSPROPERTY_TABLE(SynthClockPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_SYNTH_MASTERCLOCK,			// Id
		CMidiFilter::GetSynthMasterClock,		// GetPropertyHandler or GetSupported
		sizeof(KSPROPERTY),						// MinProperty
		sizeof(ULONGLONG),						// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CMidiFilter::SupportSynthMasterClock,	// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * MidiSynthPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Midi synth property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(MidiSynthPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Synth,						// Set
		SIZEOF_ARRAY(SynthPropertyTable),		// PropertiesCount
		SynthPropertyTable,						// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_SynthClock,				// Set
		SIZEOF_ARRAY(SynthClockPropertyTable),	// PropertiesCount
		SynthClockPropertyTable,				// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	),
};

/*****************************************************************************
 * MidiSynthAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * MIDI synth automation table.
 */
DEFINE_KSAUTOMATION_TABLE(MidiSynthAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(MidiSynthPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS_NULL
};
#endif // ENABLE_DIRECTMUSIC_SUPPORT

#endif // _MIDI_TOPO_TABLES_H_
