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
 * @file	   Unit.h
 * @brief	   Unit definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __UNIT_H__
#define __UNIT_H__

#include "entity.h"

/*****************************************************************************
 * Defines
 */
#define dB			65536
#define INFINITY	(-32768)	

/*****************************************************************************
 * Classes
 */
class CUsbDevice;

/*****************************************************************************
 *//*! @class CUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Topology unit object.
 */
class CUnit
:	public CEntity
{
private:

protected:
	PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	m_UnitDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CUnit() {}
    /*! @brief Destructor. */
	~CUnit() {}

    /*************************************************************************
     * CUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	UCHAR UnitID
	(	void
	);

	virtual UCHAR iUnit
	(	void
	) = 0;

	virtual BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID
	) = 0;

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
	friend class CList<CUnit>;
};

typedef CUnit * PUNIT;

/*****************************************************************************
 *//*! @class CMixerUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Topology mixer unit object.
 */
class CMixerUnit
:	public CUnit
{
private:
	PUSB_AUDIO_10_MIXER_UNIT_DESCRIPTOR	m_MixerUnitDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CMixerUnit() : CUnit() {}
    /*! @brief Destructor. */
	~CMixerUnit();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CMixerUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	UCHAR iUnit
	(	void
	);

	BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	MixerControlNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	MixerControlNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CMixerUnit * PMIXER_UNIT;

/*****************************************************************************
 *//*! @class CSelectorUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Topology selector unit object.
 */
class CSelectorUnit
:	public CUnit
{
private:
	PUSB_AUDIO_10_SELECTOR_UNIT_DESCRIPTOR	m_SelectorUnitDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CSelectorUnit() : CUnit() {}
    /*! @brief Destructor. */
	~CSelectorUnit();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CSelectorUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	UCHAR iUnit
	(	void
	);

	BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CSelectorUnit * PSELECTOR_UNIT;

/*****************************************************************************
 *//*! @class CFeatureUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Topology feature unit object.
 */
class CFeatureUnit
:	public CUnit
{
private:
	PUSB_AUDIO_10_FEATURE_UNIT_DESCRIPTOR	m_FeatureUnitDescriptor;

	BOOL _FindControl
	(
		IN		UCHAR	Channel,
		IN		UCHAR	Index,
		OUT		UCHAR *	OutControlSelector
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CFeatureUnit() : CUnit() {}
    /*! @brief Destructor. */
	~CFeatureUnit();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CFeatureUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	UCHAR iUnit
	(	void
	);

	BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CFeatureUnit * PFEATURE_UNIT;

/*****************************************************************************
 *//*! @class CProcessorUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Topology processor unit object.
 */
class CProcessorUnit
:	public CUnit
{
private:

protected:
	PUSB_AUDIO_10_COMMON_PROCESSING_UNIT_DESCRIPTOR m_ProcessorUnitDescriptor;

	BOOL _FindControl
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutControlSelector
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CProcessorUnit() : CUnit() {}
    /*! @brief Destructor. */
	~CProcessorUnit() {};

    /*************************************************************************
     * CProcessorUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	USHORT ProcessType
	(	void
	);

	UCHAR iUnit
	(	void
	);

	BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CProcessorUnit * PPROCESSOR_UNIT;

/*****************************************************************************
 *//*! @class CUpDownMixUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Up/Down-mix processor unit object.
 */
class CUpDownMixUnit
:	public CProcessorUnit
{
private:
	PUSB_AUDIO_10_UP_DOWNMIX_UNIT_DESCRIPTOR	m_UpDownMixUnitDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CUpDownMixUnit() : CProcessorUnit() {}
    /*! @brief Destructor. */
	~CUpDownMixUnit();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CUpDownMixUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CUpDownMixUnit * PUPDOWN_MIX_UNIT;

/*****************************************************************************
 *//*! @class CDolbyPrologicUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Dolby Prologic processor unit object.
 */
class CDolbyPrologicUnit
:	public CProcessorUnit
{
private:
	PUSB_AUDIO_10_PROLOGIC_UNIT_DESCRIPTOR	m_PrologicUnitDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CDolbyPrologicUnit() : CProcessorUnit() {}
    /*! @brief Destructor. */
	~CDolbyPrologicUnit();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CDolbyPrologicUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CDolbyPrologicUnit * PDOLBY_PROLOGIC_UNIT;

/*****************************************************************************
 *//*! @class C3dStereoExtenderUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * 3D stereo extender processor unit object.
 */
class C3dStereoExtenderUnit
:	public CProcessorUnit
{
private:
	PUSB_AUDIO_10_3D_EXTENDER_UNIT_DESCRIPTOR	m_3dExtenderUnitDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	C3dStereoExtenderUnit() : CProcessorUnit() {}
    /*! @brief Destructor. */
	~C3dStereoExtenderUnit();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * C3dStereoExtenderUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

    /*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef C3dStereoExtenderUnit * P3D_STEREO_EXTENDER_UNIT;

/*****************************************************************************
 *//*! @class CReverberationUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Reverberation processor unit object.
 */
class CReverberationUnit
:	public CProcessorUnit
{
private:
	PUSB_AUDIO_10_REVERBERATION_UNIT_DESCRIPTOR	m_ReverberationUnitDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CReverberationUnit() : CProcessorUnit() {}
    /*! @brief Destructor. */
	~CReverberationUnit();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CReverberationUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

    /*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CReverberationUnit * PREVERBERATION_UNIT;

/*****************************************************************************
 *//*! @class CChorusUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Chorus processor unit object.
 */
class CChorusUnit
:	public CProcessorUnit
{
private:
	PUSB_AUDIO_10_CHORUS_UNIT_DESCRIPTOR	m_ChorusUnitDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CChorusUnit() : CProcessorUnit() {}
    /*! @brief Destructor. */
	~CChorusUnit();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CChorusUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

    /*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CChorusUnit * PCHORUS_UNIT;

/*****************************************************************************
 *//*! @class CDynamicRangeCompressionUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Dynamic range compression processor unit object.
 */
class CDynamicRangeCompressionUnit
:	public CProcessorUnit
{
private:
	PUSB_AUDIO_10_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR	m_DrcUnitDescriptor;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CDynamicRangeCompressionUnit() : CProcessorUnit() {}
    /*! @brief Destructor. */
	~CDynamicRangeCompressionUnit();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CDynamicRangeCompressionUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

    /*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CDynamicRangeCompressionUnit * PDYNAMIC_RANGE_COMPRESSION_UNIT;

/*****************************************************************************
 *//*! @class CExtensionUnit
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Topology extension unit object.
 */
class CExtensionUnit
:	public CUnit
{
private:
	PUSB_AUDIO_10_EXTENSION_UNIT_DESCRIPTOR	m_ExtensionUnitDescriptor;

	BOOL _FindControl
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutControlSelector
	);

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CExtensionUnit() : CUnit() {}
    /*! @brief Destructor. */
	~CExtensionUnit();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CExtensionUnit public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *							UsbDevice,
		IN		UCHAR									InterfaceNumber,
		IN		PUSB_AUDIO_10_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	UCHAR iUnit
	(	void
	);

	BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID
	);

	ULONG GetOtherUsbAudioDescriptorSize
	(	void
	);

	ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
};

typedef CExtensionUnit * PEXTENSION_UNIT;

#endif // __UNIT_H__ 
