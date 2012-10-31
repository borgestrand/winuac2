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
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __UNIT_H__
#define __UNIT_H__

#include "Common.h"
#include "UsbDev.h"
#include "usbaudio.h"
#include "Entity.h"


/*****************************************************************************
 * Defines
 */
#define dB			65536
#define INFINITY	(-32768)	

/*****************************************************************************
 * Classes
 */
class CAudioTopology;

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
	PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	m_UnitDescriptor;

	UCHAR								m_ParameterBlockStatusType;

	DEVICE_POWER_STATE					m_PowerState;

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

	virtual BOOL FindAudioChannelCluster
	(
		OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
	) = 0;

	virtual ULONG NumberOfChannels
	(
		IN		BOOL	Direction
	) = 0;

	virtual NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	) = 0;

	virtual NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags = PARAMETER_BLOCK_FLAGS_IO_BOTH
	) = 0;

	virtual NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags = PARAMETER_BLOCK_FLAGS_IO_SOFTWARE
	) = 0;

	virtual NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	) = 0;

	virtual NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	) = 0;

	virtual ULONG GetParameterBlockSize
	(	void
	) = 0;

	NTSTATUS UpdateParameterBlock
	(	void
	);

	NTSTATUS InvalidateParameterBlock
	(
		IN		UCHAR	StatusType
	);

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
 * MIXER_UNIT_PARAMETER_BLOCK
 */
typedef struct
{
	BOOL	Programmable;
	LONG	Minimum;
	LONG	Maximum;
	LONG	Resolution;
	LONG	Current;
} MIXER_UNIT_PARAMETER_BLOCK, *PMIXER_UNIT_PARAMETER_BLOCK;

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
	CAudioTopology *	m_AudioTopology;

	PUSB_AUDIO_MIXER_UNIT_DESCRIPTOR	m_MixerUnitDescriptor;

	ULONG								m_NumInputChannels;
	ULONG								m_NumOutputChannels;

	PMIXER_UNIT_PARAMETER_BLOCK			m_ParameterBlock;
	ULONG								m_ParameterBlockSize;

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR						InputChannelNumber,
		IN		UCHAR						OutputChannelNumber,
		IN		PMIXER_UNIT_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL						Read
	);

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
		IN		CAudioTopology *					AudioTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	NTSTATUS Configure
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

	BOOL FindAudioChannelCluster
	(
		OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
	);

	ULONG NumberOfChannels
	(
		IN		BOOL	Direction
	);

	ULONG NumberOfInputChannels
	(
		IN		UCHAR	Index,
		OUT		ULONG *	OutChannelOffset
	);

	NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	InputChannelNumber,
		IN		UCHAR	OutputChannelNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	InputChannelNumber,
		IN		UCHAR	OutputChannelNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	ULONG GetParameterBlockSize
	(	void
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
 * SELECTOR_UNIT_PARAMETER_BLOCK
 */
typedef struct
{
	ULONG	PinId; // Zero-based
} SELECTOR_UNIT_PARAMETER_BLOCK, *PSELECTOR_UNIT_PARAMETER_BLOCK;

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
	CAudioTopology *	m_AudioTopology;

	PUSB_AUDIO_SELECTOR_UNIT_DESCRIPTOR	m_SelectorUnitDescriptor;

	SELECTOR_UNIT_PARAMETER_BLOCK		m_ParameterBlock;
	UCHAR								m_ParameterBlockStatusType;

	NTSTATUS _RestoreParameterBlock
	(
		IN		PSELECTOR_UNIT_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL							Read
	);

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
		IN		CAudioTopology *					AudioTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	UCHAR iUnit
	(	void
	);

	BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID
	);

	BOOL FindAudioChannelCluster
	(
		OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
	);

	ULONG NumberOfChannels
	(
		IN		BOOL	Direction
	);

	NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	ULONG GetParameterBlockSize
	(	void
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
 * FEATURE_UNIT_PARAMETER_BLOCK
 */
typedef struct
{
	struct 	{
		BOOL	Support;
		BOOL	Current;
	}							Mute;
	struct 	{
		BOOL	Support;
		LONG	Minimum;
		LONG	Maximum;
		LONG	Resolution;
		LONG	Current;
	}							Volume;
	struct 	{
		BOOL	Support;
		LONG	Minimum;
		LONG	Maximum;
		LONG	Resolution;
		LONG	Current;
	}							Bass;
	struct 	{
		BOOL	Support;
		LONG	Minimum;
		LONG	Maximum;
		LONG	Resolution;
		LONG	Current;
	}							Mid;
	struct 	{
		BOOL	Support;
		LONG	Minimum;
		LONG	Maximum;
		LONG	Resolution;
		LONG	Current;
	}							Treble;
	struct	{
		BOOL	Support;
		ULONG	NumberOfBands;
		ULONG	BandsPresent;
		struct 	{
			BOOL	Support;
			LONG	Minimum;
			LONG	Maximum;
			LONG	Resolution;
			LONG	Current;
		}		Levels[32];
	}							GraphicEQ;
	struct 	{
		BOOL	Support;
		BOOL	Current;
	}							AGC;
	struct
	{
		BOOL	Support;
		KSTIME	Minimum;
		KSTIME	Maximum;
		KSTIME	Resolution;
		KSTIME	Current;
	}							Delay;
	struct 	{
		BOOL	Support;
		BOOL	Current;
	}							BassBoost;
	struct 	{
		BOOL	Support;
		BOOL	Current;
	}							Loudness;
} FEATURE_UNIT_PARAMETER_BLOCK, *PFEATURE_UNIT_PARAMETER_BLOCK;

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
	CAudioTopology *	m_AudioTopology;

	PUSB_AUDIO_FEATURE_UNIT_DESCRIPTOR	m_FeatureUnitDescriptor;

	PFEATURE_UNIT_PARAMETER_BLOCK		m_ParameterBlock;
	ULONG								m_ParameterBlockSize;
	UCHAR								m_ParameterBlockStatusType;

	BOOL _FindControl
	(
		IN		UCHAR	Channel,
		IN		UCHAR	Index,
		OUT		UCHAR *	OutControlSelector
	);

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR							ControlSelector,
		IN		UCHAR							ChannelNumber,
		IN		BOOL							Support,
		IN		PFEATURE_UNIT_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL							Read
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
		IN		CAudioTopology *					AudioTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	NTSTATUS Configure
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

	BOOL ParseControls
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutControlSelector
	);
	
	BOOL FindAudioChannelCluster
	(
		OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
	);

	ULONG NumberOfChannels
	(
		IN		BOOL	Direction
	);

	NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR	ChannelNumber,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	ULONG GetParameterBlockSize
	(	void
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
	CAudioTopology *	m_AudioTopology;

	PUSB_AUDIO_COMMON_PROCESSING_UNIT_DESCRIPTOR m_ProcessorUnitDescriptor;

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

	BOOL FindAudioChannelCluster
	(
		OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
	);

	ULONG NumberOfChannels
	(
		IN		BOOL	Direction
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
 * UP_DOWNMIX_UNIT_PARAMETER_BLOCK
 */
typedef struct
{
	struct 
	{
		BOOL	Support;
		BOOL	Current;
	}						Enable;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						Mode;
} UP_DOWNMIX_UNIT_PARAMETER_BLOCK, *PUP_DOWNMIX_UNIT_PARAMETER_BLOCK;

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
	PUSB_AUDIO_UP_DOWNMIX_UNIT_DESCRIPTOR	m_UpDownMixUnitDescriptor;

	UP_DOWNMIX_UNIT_PARAMETER_BLOCK		m_ParameterBlock;
	UCHAR								m_ParameterBlockStatusType;

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR								ControlSelector,
		IN		BOOL								Support,
		IN		PUP_DOWNMIX_UNIT_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL								Read
	);

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
		IN		CAudioTopology *					AudioTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	ULONG NumberOfModes
	(
		IN OUT	PLONG	ChannelConfigs
	);

	NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	ULONG GetParameterBlockSize
	(	void
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
 * DOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK
 */
typedef struct
{
	struct 
	{
		BOOL	Support;
		BOOL	Current;
	}						Enable;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						Mode;
} DOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK, *PDOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK;

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
	PUSB_AUDIO_PROLOGIC_UNIT_DESCRIPTOR	m_PrologicUnitDescriptor;

	DOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK	m_ParameterBlock;
	UCHAR								m_ParameterBlockStatusType;

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR									ControlSelector,
		IN		BOOL									Support,
		IN		PDOLBY_PROLOGIC_UNIT_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL									Read
	);

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
		IN		CAudioTopology *					AudioTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	ULONG NumberOfModes
	(
		IN OUT	PLONG	ChannelConfigs
	);

	NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	ULONG GetParameterBlockSize
	(	void
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
 * STEREO_EXTENDER_UNIT_PARAMETER_BLOCK
 */
typedef struct
{
	struct 
	{
		BOOL	Support;
		BOOL	Current;
	}						Enable;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						Spaciousness;
} STEREO_EXTENDER_UNIT_PARAMETER_BLOCK, *PSTEREO_EXTENDER_UNIT_PARAMETER_BLOCK;

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
	PUSB_AUDIO_3D_EXTENDER_UNIT_DESCRIPTOR	m_3dExtenderUnitDescriptor;

	STEREO_EXTENDER_UNIT_PARAMETER_BLOCK	m_ParameterBlock;
	UCHAR									m_ParameterBlockStatusType;

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR									ControlSelector,
		IN		BOOL									Support,
		IN		PSTEREO_EXTENDER_UNIT_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL									Read
	);

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
		IN		CAudioTopology *					AudioTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	ULONG GetParameterBlockSize
	(	void
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
 * REVERBERATION_UNIT_PARAMETER_BLOCK
 */
typedef struct
{
	struct 
	{
		BOOL	Support;
		BOOL	Current;
	}						Enable;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						Type;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						Level;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						Time;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						Feedback;
} REVERBERATION_UNIT_PARAMETER_BLOCK, *PREVERBERATION_UNIT_PARAMETER_BLOCK;

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
	PUSB_AUDIO_REVERBERATION_UNIT_DESCRIPTOR	m_ReveberationUnitDescriptor;

	REVERBERATION_UNIT_PARAMETER_BLOCK			m_ParameterBlock;
	UCHAR										m_ParameterBlockStatusType;

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR								ControlSelector,
		IN		BOOL								Support,
		IN		PREVERBERATION_UNIT_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL								Read
	);

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
		IN		CAudioTopology *					AudioTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	ULONG GetParameterBlockSize
	(	void
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
 * CHORUS_UNIT_PARAMETER_BLOCK
 */
typedef struct
{
	struct 
	{
		BOOL	Support;
		BOOL	Current;
	}						Enable;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						Level;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						Rate;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						Depth;
} CHORUS_UNIT_PARAMETER_BLOCK, *PCHORUS_UNIT_PARAMETER_BLOCK;

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
	PUSB_AUDIO_CHORUS_UNIT_DESCRIPTOR	m_ChorusUnitDescriptor;

	CHORUS_UNIT_PARAMETER_BLOCK			m_ParameterBlock;
	UCHAR								m_ParameterBlockStatusType;

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR							ControlSelector,
		IN		BOOL							Support,
		IN		PCHORUS_UNIT_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL							Read
	);

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
		IN		CAudioTopology *					AudioTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	ULONG GetParameterBlockSize
	(	void
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
 * DRC_UNIT_PARAMETER_BLOCK
 */
typedef struct
{
	struct 
	{
		BOOL	Support;
		BOOL	Current;
	}						Enable;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						CompressionRatio;
	struct 
	{
		BOOL	Support;
		LONG	Minimum;
		LONG	Maximum;
		LONG	Resolution;
		LONG	Current;
	}						MaxAmplitude;
	struct 
	{
		BOOL	Support;
		LONG	Minimum;
		LONG	Maximum;
		LONG	Resolution;
		LONG	Current;
	}						Threshold;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						AttackTime;
	struct 
	{
		BOOL	Support;
		ULONG	Minimum;
		ULONG	Maximum;
		ULONG	Resolution;
		ULONG	Current;
	}						ReleaseTime;
} DRC_UNIT_PARAMETER_BLOCK, *PDRC_UNIT_PARAMETER_BLOCK;

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
	PUSB_AUDIO_DYNAMIC_RANGE_COMPRESSION_UNIT_DESCRIPTOR	m_DrcUnitDescriptor;

	DRC_UNIT_PARAMETER_BLOCK		m_ParameterBlock;
	UCHAR							m_ParameterBlockStatusType;

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR						ControlSelector,
		IN		BOOL						Support,
		IN		PDRC_UNIT_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL						Read
	);

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
		IN		CAudioTopology *					AudioTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	ULONG GetParameterBlockSize
	(	void
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
 * EXTENSION_UNIT_DETAILS
 */
typedef struct
{
	USHORT	VendorID;
	USHORT	ProductID;
	USHORT	ExtensionCode;
	USHORT	ControlSize;
} EXTENSION_UNIT_DETAILS, *PEXTENSION_UNIT_DETAILS;

/*****************************************************************************
 * EXTENSION_UNIT_PARAMETER_BLOCK
 */
typedef struct
{
	struct 
	{
		BOOL	Support;
		BOOL	Current;
	}						Enable;
	struct 
	{
		BOOL	Support;
	}						Parameters[1];
} EXTENSION_UNIT_PARAMETER_BLOCK, *PEXTENSION_UNIT_PARAMETER_BLOCK;

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
	CAudioTopology *	m_AudioTopology;

	PUSB_AUDIO_EXTENSION_UNIT_DESCRIPTOR	m_ExtensionUnitDescriptor;

	ULONG									m_ControlSize;

	PEXTENSION_UNIT_PARAMETER_BLOCK			m_ParameterBlock;
	ULONG									m_ParameterBlockSize;
	UCHAR									m_ParameterBlockStatusType;

	BOOL _FindControl
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutControlSelector
	);

	NTSTATUS _RestoreParameterBlock
	(
		IN		UCHAR							ControlSelector,
		IN		BOOL							Support,
		IN		PEXTENSION_UNIT_PARAMETER_BLOCK	ParameterBlock,
		IN		BOOL							Read
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
		IN		CAudioTopology *					AudioTopology,
		IN		PUSB_DEVICE							UsbDevice,
		IN		UCHAR								InterfaceNumber,
		IN		PUSB_AUDIO_COMMON_UNIT_DESCRIPTOR	UnitDescriptor
	);

	UCHAR iUnit
	(	void
	);

	BOOL ParseSources
	(
		IN		UCHAR	Index,
		OUT		UCHAR *	OutSourceID
	);

	BOOL FindAudioChannelCluster
	(
		OUT		USB_AUDIO_CHANNEL_CLUSTER_DESCRIPTOR *	OutClusterDescriptor
	);

	ULONG NumberOfChannels
	(
		IN		BOOL	Direction
	);

	NTSTATUS ExtensionDetails
	(
		OUT		EXTENSION_UNIT_DETAILS *	OutDetails
	);

	NTSTATUS PowerStateChange
	(
		IN		DEVICE_POWER_STATE	NewState
	);

	NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	ControlSelector,
		IN		UCHAR,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize,
		IN 		ULONG	Flags
	);

	NTSTATUS RestoreParameterBlock
	(
		IN		PVOID	ParameterBlock = NULL,
		IN		ULONG	ParameterBlockSize = 0
	);

	NTSTATUS SaveParameterBlock
	(
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	ULONG GetParameterBlockSize
	(	void
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
