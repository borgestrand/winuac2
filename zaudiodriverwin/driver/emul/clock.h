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
 * @file	   clock.h
 * @brief	   Clock definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __CLOCK_H__
#define __CLOCK_H__

#include "entity.h"


/*****************************************************************************
 * Defines
 */

/*****************************************************************************
 * Classes
 */
class CUsbDevice;

/*****************************************************************************
 *//*! @class CClockEntity
 *****************************************************************************
 */
class CClockEntity
:	public CEntity
{
private:

protected:

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CClockEntity() {}
    /*! @brief Destructor. */
	~CClockEntity() {}

    /*************************************************************************
     * CClockEntity public methods
     *
     * These are public member functions.  See CLOCK.CPP for specific
	 * descriptions.
     */
	UCHAR ClockID
	(	void
	);

	virtual UCHAR iClock
	(	void
	) = 0;

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
	friend class CList<CClockEntity>;
};

typedef CClockEntity * PCLOCK_ENTITY;

/*****************************************************************************
 *//*! @class CClockSource
 *****************************************************************************
 */
class CClockSource
:	public CClockEntity
{
private:
	UCHAR		m_NumberOfFrequencyRanges;
	RANGE4		m_ClockFrequencyRanges[64];

	ULONG		m_CurrentClockFrequency;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CClockSource() : CClockEntity() {}
    /*! @brief Destructor. */
	~CClockSource();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CClockSource public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *	UsbDevice,
		IN		UCHAR			InterfaceNumber,
		IN		UCHAR			ClockID
	);

	UCHAR iClock
	(	void
	);

	VOID AddClockFrequency
	(
		IN		ULONG	MinFrequency,
		IN		ULONG	MaxFrequency,
		IN		ULONG	Resolution
	);

	BOOL IsClockFrequencySupported
	(
		IN		ULONG	ClockFrequency
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

typedef CClockSource * PCLOCK_SOURCE;

/*****************************************************************************
 *//*! @class CClockSelector
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Topology clock selector object.
 */
class CClockSelector
:	public CClockEntity
{
private:
	UCHAR		m_NrInPins;
	UCHAR		m_ClockSourceIDs[64];
	UCHAR		m_PinId;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CClockSelector() : CClockEntity() {}
    /*! @brief Destructor. */
	~CClockSelector();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CClockSelector public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *	UsbDevice,
		IN		UCHAR			InterfaceNumber,
		IN		UCHAR			ClockID
	);

	UCHAR iClock
	(	void
	);

	VOID AddClockSource
	(
		UCHAR	ClockSourceID
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

typedef CClockSelector * PCLOCK_SELECTOR;

/*****************************************************************************
 *//*! @class CClockMultiplier
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Topology clock multiplier object.
 */
class CClockMultiplier
:	public CClockEntity
{
private:
	UCHAR	m_ClockSourceID;
	USHORT	m_Numerator;
	USHORT	m_Denominator;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CClockMultiplier() : CClockEntity() {}
    /*! @brief Destructor. */
	~CClockMultiplier();
    /*! @brief Self-destructor. */
	void Destruct() { delete this; }

    /*************************************************************************
     * CClockMultiplier public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	NTSTATUS Init
	(
		IN		CUsbDevice *	UsbDevice,
		IN		UCHAR			InterfaceNumber,
		IN		UCHAR			ClockID,
		IN		UCHAR			ClockSourceID,
		IN		USHORT			Numerator,
		IN		USHORT			Denominator
	);

	UCHAR iClock
	(	void
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

typedef CClockMultiplier * PCLOCK_MULTIPLIER;

#endif // __CLOCK_H__ 
