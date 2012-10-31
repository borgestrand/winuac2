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
 * @file	   Entity.h
 * @brief	   Entity definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __ENTITY_H__
#define __ENTITY_H__

#include "Common.h"
#include "UsbDev.h"
#include "usbaudio.h"

/*****************************************************************************
 * Defines
 */
#define REQUEST_CUR		0x1
#define REQUEST_MIN		0x2
#define REQUEST_MAX		0x3
#define REQUEST_RES		0x4
#define REQUEST_MEM		0x5
#define REQUEST_STAT	0xFF

#define PARAMETER_BLOCK_FLAGS_IO_HARDWARE	0x1
#define PARAMETER_BLOCK_FLAGS_IO_SOFTWARE	0x2
#define PARAMETER_BLOCK_FLAGS_IO_BOTH		0x3

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CEntity
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 * Topology entity object.
 */
class CEntity
{
private:
    CEntity *			m_Next;			/*!< @brief The next enity in the linked list. */
    CEntity *			m_Prev;			/*!< @brief The previous enity in the linked list. */
    PVOID				m_Owner;		/*!< @brief The link list owner. */

protected:
	PUSB_DEVICE			m_UsbDevice;	/*!< @brief Pointer to the USB device object. */

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

	virtual NTSTATUS Configure
	(	void
	) 
	{ return STATUS_SUCCESS; } // stub for those that don't need it;

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
