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
 * @file       Tables.h
 * @brief      Audio topology tables.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-04-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _AUDIO_TOPO_TABLES_H_
#define _AUDIO_TOPO_TABLES_H_

#include "Formats.h"

/*****************************************************************************
 * ControlEventTable
 *****************************************************************************
 *//*!
 * @brief
 * Control event items.
 */
DEFINE_KSEVENT_TABLE(ControlEventTable)
{
	DEFINE_KSEVENT_ITEM
	(
		KSEVENT_CONTROL_CHANGE,				// EventId
		sizeof(KSEVENTDATA),				// DataInput
		sizeof(ULONG) * 2,					// ExtraEntryData
		CAudioFilter::AddControlEvent,		// AddHandler
		CAudioFilter::RemoveControlEvent,	// RemoveHandler
		NULL								// SupportHandler
	)
};

/*****************************************************************************
 * ControlEventSetTable
 *****************************************************************************
 *//*!
 * @brief
 * Control event set table.
 */
DEFINE_KSEVENT_SET_TABLE(ControlEventSetTable)
{
	DEFINE_KSEVENT_SET
	(
		&KSEVENTSETID_AudioControlChange,	// Set
		SIZEOF_ARRAY(ControlEventTable),	// EventsCount
		ControlEventTable					// EventItem
	)
};

/*****************************************************************************
 * VolumeLevelPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Volume level property items.
 */
DEFINE_KSPROPERTY_TABLE(VolumeLevelPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_VOLUMELEVEL,			// Id
		CAudioFilter::GetLevelControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(LONG),							// MinData
		CAudioFilter::SetLevelControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportLevelControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * VolumeLevelPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Volume level property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(VolumeLevelPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(VolumeLevelPropertyTable),	// PropertiesCount
		VolumeLevelPropertyTable,				// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * VolumeLevelAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Volume level automation table.
 */
DEFINE_KSAUTOMATION_TABLE(VolumeLevelAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(VolumeLevelPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * AgcPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * AGC property items.
 */
DEFINE_KSPROPERTY_TABLE(AgcPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_AGC,					// Id
		CAudioFilter::GetOnOffControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(BOOL),							// MinData
		CAudioFilter::SetOnOffControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportOnOffControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * AgcPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * AGC property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(AgcPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(AgcPropertyTable),			// PropertiesCount
		AgcPropertyTable,						// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * AgcAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Agc automation table.
 */
DEFINE_KSAUTOMATION_TABLE(AgcAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(AgcPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * MutePropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Mute property items.
 */
DEFINE_KSPROPERTY_TABLE(MutePropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_MUTE,					// Id
		CAudioFilter::GetOnOffControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(BOOL),							// MinData
		CAudioFilter::SetOnOffControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportOnOffControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * MutePropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Mute property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(MutePropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(MutePropertyTable),		// PropertiesCount
		MutePropertyTable,						// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * MuteAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Mute automation table.
 */
DEFINE_KSAUTOMATION_TABLE(MuteAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(MutePropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * BassPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Bass property items.
 */
DEFINE_KSPROPERTY_TABLE(BassPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_BASS,					// Id
		CAudioFilter::GetLevelControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(LONG),							// MinData
		CAudioFilter::SetLevelControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportLevelControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * BassPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Bass property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(BassPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(BassPropertyTable),		// PropertiesCount
		BassPropertyTable,						// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * BassAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Bass automation table.
 */
DEFINE_KSAUTOMATION_TABLE(BassAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(BassPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * TreblePropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Treble property items.
 */
DEFINE_KSPROPERTY_TABLE(TreblePropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_TREBLE,				// Id
		CAudioFilter::GetLevelControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(LONG),							// MinData
		CAudioFilter::SetLevelControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportLevelControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * TreblePropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Treble property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(TreblePropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(TreblePropertyTable),		// PropertiesCount
		TreblePropertyTable,					// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * TrebleAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Treble automation table.
 */
DEFINE_KSAUTOMATION_TABLE(TrebleAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(TreblePropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * MidPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Mid property items.
 */
DEFINE_KSPROPERTY_TABLE(MidPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_MID,					// Id
		CAudioFilter::GetLevelControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(LONG),							// MinData
		CAudioFilter::SetLevelControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportLevelControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * MidPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Mid property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(MidPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(MidPropertyTable),			// PropertiesCount
		MidPropertyTable,						// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * MidAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Mid automation table.
 */
DEFINE_KSAUTOMATION_TABLE(MidAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(MidPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * EqPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * EQ property items.
 */
DEFINE_KSPROPERTY_TABLE(EqPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_NUM_EQ_BANDS,			// Id
		CAudioFilter::GetEqControl,				// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportEqControl,			// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_EQ_BANDS,				// Id
		CAudioFilter::GetEqControl,				// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportEqControl,			// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_EQ_LEVEL,				// Id
		CAudioFilter::GetEqControl,				// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(LONG),							// MinData
		CAudioFilter::SetEqControl,				// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportEqControl,			// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * EqPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * EQ property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(EqPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(EqPropertyTable),			// PropertiesCount
		EqPropertyTable,						// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * EqAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * EQ automation table.
 */
DEFINE_KSAUTOMATION_TABLE(EqAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(EqPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * MixPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Mix property items.
 */
DEFINE_KSPROPERTY_TABLE(MixPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_MIX_LEVEL_CAPS,		// Id
		CAudioFilter::GetMixControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		0,										// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportMixControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_MIX_LEVEL_TABLE,		// Id
		CAudioFilter::GetMixControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		0,										// MinData
		CAudioFilter::SetMixControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportMixControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * MixPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Mix property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(MixPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(MixPropertyTable),			// PropertiesCount
		MixPropertyTable,						// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * MixAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Mix automation table.
 */
DEFINE_KSAUTOMATION_TABLE(MixAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(MixPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * MuxPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Mux property items.
 */
DEFINE_KSPROPERTY_TABLE(MuxPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_MUX_SOURCE,			// Id
		CAudioFilter::GetMuxControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		CAudioFilter::SetMuxControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportMuxControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * MuxPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Mux property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(MuxPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(MuxPropertyTable),			// PropertiesCount
		MuxPropertyTable,						// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * MuxAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Mux automation table.
 */
DEFINE_KSAUTOMATION_TABLE(MuxAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(MuxPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * DelayPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Delay property items.
 */
DEFINE_KSPROPERTY_TABLE(DelayPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_DELAY,					// Id
		CAudioFilter::GetDelayControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(KSTIME),							// MinData
		CAudioFilter::SetDelayControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		NULL,									// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * DelayPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Delay property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(DelayPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(DelayPropertyTable),		// PropertiesCount
		DelayPropertyTable,						// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * DelayAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Delay automation table.
 */
DEFINE_KSAUTOMATION_TABLE(DelayAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(DelayPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * BassBoostPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Bass boost property items.
 */
DEFINE_KSPROPERTY_TABLE(BassBoostPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_BASS_BOOST,			// Id
		CAudioFilter::GetOnOffControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(BOOL),							// MinData
		CAudioFilter::SetOnOffControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportOnOffControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * BassBoostPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Bass boost property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(BassBoostPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(BassBoostPropertyTable),	// PropertiesCount
		BassBoostPropertyTable,					// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * BassBoostAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Bass boost automation table.
 */
DEFINE_KSAUTOMATION_TABLE(BassBoostAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(BassBoostPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * LoudnessPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Loudness property items.
 */
DEFINE_KSPROPERTY_TABLE(LoudnessPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_LOUDNESS,				// Id
		CAudioFilter::GetOnOffControl,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(BOOL),							// MinData
		CAudioFilter::SetOnOffControl,			// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportOnOffControl,		// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * LoudnessPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Loudness property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(LoudnessPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(LoudnessPropertyTable),	// PropertiesCount
		LoudnessPropertyTable,					// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * LoudnessAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Loudness automation table.
 */
DEFINE_KSAUTOMATION_TABLE(LoudnessAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(LoudnessPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * EnableProcessingPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Enable processing unit property items.
 */
DEFINE_KSPROPERTY_TABLE(EnableProcessingPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_TOPOLOGYNODE_ENABLE,					// Id
		CAudioFilter::GetEnableProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(BOOL),									// MinData
		CAudioFilter::SetEnableProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportEnableProcessingControl,	// SupportHandler
		0												// SerializedSize
	)
};

/*****************************************************************************
 * UpDownMixPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Up/down mix property items.
 */
DEFINE_KSPROPERTY_TABLE(UpDownMixPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CHANNEL_CONFIG,		// Id
		CAudioFilter::GetModeSelectControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(KSAUDIO_CHANNEL_CONFIG),			// MinData
		CAudioFilter::SetModeSelectControl,		// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportModeSelectControl,	// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * UpDownMixPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Up/down mix property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(UpDownMixPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_TopologyNode,					// Set
		SIZEOF_ARRAY(EnableProcessingPropertyTable),// PropertiesCount
		EnableProcessingPropertyTable,				// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,							// Set
		SIZEOF_ARRAY(UpDownMixPropertyTable),		// PropertiesCount
		UpDownMixPropertyTable,						// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	)
};

/*****************************************************************************
 * UpDownMixAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Up/down mix automation table.
 */
DEFINE_KSAUTOMATION_TABLE(UpDownMixAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(UpDownMixPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * DolbyPrologicPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Dolby prologic property items.
 */
DEFINE_KSPROPERTY_TABLE(DolbyPrologicPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CHANNEL_CONFIG,		// Id
		CAudioFilter::GetModeSelectControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(KSAUDIO_CHANNEL_CONFIG),			// MinData
		CAudioFilter::SetModeSelectControl,		// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportModeSelectControl,	// SupportHandler
		0										// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,			// Id
		CAudioFilter::GetCpuResources,			// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),					// MinProperty
		sizeof(ULONG),							// MinData
		NULL,									// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SupportCpuResources,		// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * DolbyPrologicPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Dolby Prologic property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(DolbyPrologicPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_TopologyNode,					// Set
		SIZEOF_ARRAY(EnableProcessingPropertyTable),// PropertiesCount
		EnableProcessingPropertyTable,				// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,							// Set
		SIZEOF_ARRAY(DolbyPrologicPropertyTable),	// PropertiesCount
		DolbyPrologicPropertyTable,					// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	)
};

/*****************************************************************************
 * DolbyPrologicAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Dolby Prologic automation table.
 */
DEFINE_KSAUTOMATION_TABLE(DolbyPrologicAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(DolbyPrologicPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * StereoExtenderPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * 3D stereo extender property items.
 */
DEFINE_KSPROPERTY_TABLE(StereoExtenderPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_WIDENESS,						// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,				// Id
		CAudioFilter::GetCpuResources,				// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),						// MinProperty
		sizeof(ULONG),								// MinData
		NULL,										// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		CAudioFilter::SupportCpuResources,			// SupportHandler
		0											// SerializedSize
	)
};

/*****************************************************************************
 * StereoExtenderPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * 3D stereo extender property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(StereoExtenderPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_TopologyNode,					// Set
		SIZEOF_ARRAY(EnableProcessingPropertyTable),// PropertiesCount
		EnableProcessingPropertyTable,				// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,							// Set
		SIZEOF_ARRAY(StereoExtenderPropertyTable),	// PropertiesCount
		StereoExtenderPropertyTable,				// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	)
};

/*****************************************************************************
 * StereoExtenderAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * 3D stereo extender automation table.
 */
DEFINE_KSAUTOMATION_TABLE(StereoExtenderAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(StereoExtenderPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * ReverbPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Reverb property items.
 */
DEFINE_KSPROPERTY_TABLE(ReverbPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_REVERB_LEVEL,					// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	// Are these available in the Longhorn DDK ???
	#if defined(KSPROPERTY_AUDIO_REVERB_TYPE)
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_REVERB_TYPE,					// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	#endif // KSPROPERTY_AUDIO_REVERB_TYPE
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_REVERB_TIME,					// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_REVERB_DELAY_FEEDBACK,			// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,				// Id
		CAudioFilter::GetCpuResources,				// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),						// MinProperty
		sizeof(ULONG),								// MinData
		NULL,										// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		CAudioFilter::SupportCpuResources,			// SupportHandler
		0											// SerializedSize
	)
};

/*****************************************************************************
 * ReverbPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Reverb property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(ReverbPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_TopologyNode,					// Set
		SIZEOF_ARRAY(EnableProcessingPropertyTable),// PropertiesCount
		EnableProcessingPropertyTable,				// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,							// Set
		SIZEOF_ARRAY(ReverbPropertyTable),			// PropertiesCount
		ReverbPropertyTable,						// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	)
};

/*****************************************************************************
 * ReverbAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Reverb automation table.
 */
DEFINE_KSAUTOMATION_TABLE(ReverbAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(ReverbPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * ChorusPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Chorus property items.
 */
DEFINE_KSPROPERTY_TABLE(ChorusPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CHORUS_LEVEL,					// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CHORUS_MODULATION_RATE,		// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CHORUS_MODULATION_DEPTH,		// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,				// Id
		CAudioFilter::GetCpuResources,				// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),						// MinProperty
		sizeof(ULONG),								// MinData
		NULL,										// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		CAudioFilter::SupportCpuResources,			// SupportHandler
		0											// SerializedSize
	)
};

/*****************************************************************************
 * ChorusPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Chorus property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(ChorusPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_TopologyNode,					// Set
		SIZEOF_ARRAY(EnableProcessingPropertyTable),// PropertiesCount
		EnableProcessingPropertyTable,				// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,							// Set
		SIZEOF_ARRAY(ChorusPropertyTable),			// PropertiesCount
		ChorusPropertyTable,						// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	)
};

/*****************************************************************************
 * ChorusAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Chorus automation table.
 */
DEFINE_KSAUTOMATION_TABLE(ChorusAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(ChorusPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * DrcPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Dynamic range compression property items.
 */
DEFINE_KSPROPERTY_TABLE(DrcPropertyTable)
{
	// Are these available in the Longhorn DDK ???
	#if defined(KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO)
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO,			// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	#endif // KSPROPERTY_AUDIO_DRC_COMPRESSION_RATIO
	#if defined(KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE)
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE,				// Id
		CAudioFilter::GetSignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetSignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportSignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	#endif // KSPROPERTY_AUDIO_DRC_MAX_AMPLITUDE
	#if defined(KSPROPERTY_AUDIO_DRC_THRESHOLD)
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_DRC_THRESHOLD,					// Id
		CAudioFilter::GetSignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetSignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportSignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	#endif // KSPROPERTY_AUDIO_DRC_THRESHOLD
	#if defined(KSPROPERTY_AUDIO_DRC_ATTACK_TIME)
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_DRC_ATTACK_TIME,				// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	#endif // KSPROPERTY_AUDIO_DRC_ATTACK_TIME
	#if defined(KSPROPERTY_AUDIO_DRC_RELEASE_TIME)
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_DRC_RELEASE_TIME,				// Id
		CAudioFilter::GetUnsignedProcessingControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),							// MinProperty
		sizeof(ULONG),									// MinData
		CAudioFilter::SetUnsignedProcessingControl,		// SetPropertyHandler or SetSupported
		NULL,											// Values
		0,												// RelationsCount
		NULL,											// Relations
		CAudioFilter::SupportUnsignedProcessingControl,	// SupportHandler
		0												// SerializedSize
	),
	#endif // KSPROPERTY_AUDIO_DRC_RELEASE_TIME
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_CPU_RESOURCES,				// Id
		CAudioFilter::GetCpuResources,				// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY),						// MinProperty
		sizeof(ULONG),								// MinData
		NULL,										// SetPropertyHandler or SetSupported
		NULL,										// Values
		0,											// RelationsCount
		NULL,										// Relations
		CAudioFilter::SupportCpuResources,			// SupportHandler
		0											// SerializedSize
	)
};

/*****************************************************************************
 * DrcPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Dynamic range compression property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(DrcPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_TopologyNode,					// Set
		SIZEOF_ARRAY(EnableProcessingPropertyTable),// PropertiesCount
		EnableProcessingPropertyTable,				// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	),
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,							// Set
		SIZEOF_ARRAY(DrcPropertyTable),				// PropertiesCount
		DrcPropertyTable,							// PropertyItem
		0,											// FastIoCount
		NULL										// FastIoTable
	)
};

/*****************************************************************************
 * DrcAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Dynamic range compression automation table.
 */
DEFINE_KSAUTOMATION_TABLE(DrcAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(DrcPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * SwVolumeLevelPropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Volume level property items.
 */
DEFINE_KSPROPERTY_TABLE(SwVolumeLevelPropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_VOLUMELEVEL,			// Id
		CAudioFilter::SwGetLevelControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(LONG),							// MinData
		CAudioFilter::SwSetLevelControl,		// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SwSupportLevelControl,	// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * SwVolumeLevelPropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Volume level property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(SwVolumeLevelPropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(SwVolumeLevelPropertyTable),	// PropertiesCount
		SwVolumeLevelPropertyTable,				// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * SwVolumeLevelAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Volume level automation table.
 */
DEFINE_KSAUTOMATION_TABLE(SwVolumeLevelAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(SwVolumeLevelPropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * SwMutePropertyTable
 *****************************************************************************
 *//*!
 * @brief
 * Mute property items.
 */
DEFINE_KSPROPERTY_TABLE(SwMutePropertyTable)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_AUDIO_MUTE,					// Id
		CAudioFilter::SwGetOnOffControl,		// GetPropertyHandler or GetSupported
		sizeof(KSNODEPROPERTY_AUDIO_CHANNEL),	// MinProperty
		sizeof(BOOL),							// MinData
		CAudioFilter::SwSetOnOffControl,		// SetPropertyHandler or SetSupported
		NULL,									// Values
		0,										// RelationsCount
		NULL,									// Relations
		CAudioFilter::SwSupportOnOffControl,	// SupportHandler
		0										// SerializedSize
	)
};

/*****************************************************************************
 * SwMutePropertySetTable
 *****************************************************************************
 *//*!
 * @brief
 * Mute property set table.
 */
DEFINE_KSPROPERTY_SET_TABLE(SwMutePropertySetTable)
{
	DEFINE_KSPROPERTY_SET
	(
		&KSPROPSETID_Audio,						// Set
		SIZEOF_ARRAY(SwMutePropertyTable),		// PropertiesCount
		SwMutePropertyTable,					// PropertyItem
		0,										// FastIoCount
		NULL									// FastIoTable
	)
};

/*****************************************************************************
 * SwMuteAutomationTable
 *****************************************************************************
 *//*!
 * @brief
 * Mute automation table.
 */
DEFINE_KSAUTOMATION_TABLE(SwMuteAutomationTable)
{
	DEFINE_KSAUTOMATION_PROPERTIES(SwMutePropertySetTable),
	DEFINE_KSAUTOMATION_METHODS_NULL,
	DEFINE_KSAUTOMATION_EVENTS(ControlEventSetTable)
};

/*****************************************************************************
 * ConnectionTable
 *****************************************************************************
 * Table of topology unit connections.
 *
 * Pin numbering is technically arbitrary, but the convention established here
 * is to number a solitary output pin 0 (looks like an 'o') and a solitary
 * input pin 1 (looks like an 'i').  Even destinations, which have no output,
 * have an input pin numbered 1 and no pin 0.
 *
 * Nodes are more likely to have multiple ins than multiple outs, so the more
 * general rule would be that inputs are numbered >=1.  If a node has multiple
 * outs, none of these conventions apply.
 *
 * Nodes have at most one control value.  Mixers are therefore simple summing
 * nodes with no per-pin levels.  Rather than assigning a unique pin to each
 * input to a mixer, all inputs are connected to pin 1.  This is acceptable
 * because there is no functional distinction between the inputs.
 *
 * A multiplexer should have a single output pin (0) and multiple input pins
 * (1..n).  Its control value is an integer in the range 1..n indicating which
 * input is connected to the output.
 *
 * In the case of connections to pins, as opposed to connections to nodes, the
 * node is identified as KSFILTER_NODE and the pin number identifies the
 * particular filter pin.
 */

#endif // _AUDIO_TOPO_TABLES_H_
