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
 * @file	   entity.h
 * @brief	   Entity definitions.
 * @copyright  E-MU Systems, 2005.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  10-31-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "common.h"
#include "utils.h"

/*****************************************************************************
 * Defines
 */
#include <pshpack1.h>

typedef struct
{
	union
	{
		struct
		{
			CHAR	bMIN;
			CHAR	bMAX;
			CHAR	bRES;
		} Signed;

		struct
		{
			UCHAR	bMIN;
			UCHAR	bMAX;
			UCHAR	bRES;
		} Unsigned;
	};
} RANGE1, *PRANGE1;

typedef struct
{
	union
	{
		struct
		{
			SHORT	wMIN;
			SHORT	wMAX;
			SHORT	wRES;
		} Signed;

		struct
		{
			USHORT	wMIN;
			USHORT	wMAX;
			USHORT	wRES;
		} Unsigned;
	};
} RANGE2, *PRANGE2;

typedef struct
{
	union
	{
		struct
		{
			LONG	dMIN;
			LONG	dMAX;
			LONG	dRES;
		} Signed;

		struct
		{
			ULONG	dMIN;
			ULONG	dMAX;
			ULONG	dRES;
		} Unsigned;
	};
} RANGE4, *PRANGE4;

#include <poppack.h>

/*****************************************************************************
 * Classes
 */
class CUsbDevice;

/*****************************************************************************
 *//*! @class CEntity
 *****************************************************************************
 */
class CEntity
{
private:
    CEntity *			m_Next;			/*!< @brief The next enity in the linked list. */
    CEntity *			m_Prev;			/*!< @brief The previous enity in the linked list. */
    PVOID				m_Owner;		/*!< @brief The link list owner. */

protected:
	CUsbDevice *		m_UsbDevice;

	UCHAR				m_InterfaceNumber;

	UCHAR				m_DescriptorSubtype;
	UCHAR				m_EntityID;

public:
    /*************************************************************************
     * Constructor/destructor.
     */
    /*! @brief Constructor. */
	CEntity() { m_Next = m_Prev = NULL; m_Owner = NULL; }
    /*! @brief Destructor. */
	~CEntity() {}
    /*! @brief Self-destructor. */
	virtual void Destruct() = 0;

    /*************************************************************************
     * CEntity public methods
     *
     * These are public member functions.  See TOPOLOGY.CPP for specific
	 * descriptions.
     */
	UCHAR DescriptorSubtype
	(	void
	);

	UCHAR EntityID
	(	void
	);

	NTSTATUS SetRequest
	(
		IN		UCHAR	RequestCode,
		IN		USHORT	Value,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	);

	NTSTATUS GetRequest
	(
		IN		UCHAR	RequestCode,
		IN		USHORT	Value,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	);

	virtual ULONG GetOtherUsbAudioDescriptorSize
	(	void
	) = 0;

	virtual ULONG GetOtherUsbAudioDescriptor
	(
		IN		PUCHAR	Buffer
	) = 0;

	virtual NTSTATUS WriteParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN		ULONG	ParameterBlockSize
	) = 0;

	virtual NTSTATUS ReadParameterBlock
	(
		IN		UCHAR	RequestCode,
		IN		UCHAR	RequestValueH,
		IN		UCHAR	RequestValueL,
		IN		PVOID	ParameterBlock,
		IN 		ULONG 	ParameterBlockSize,
		OUT		ULONG *	OutParameterBlockSize
	) = 0;

	/*************************************************************************
     * Static
     */

    /*************************************************************************
     * Friends
     */
	friend class CList<CEntity>;
};

typedef CEntity * PENTITY;

#endif // __ENTITY_H__
