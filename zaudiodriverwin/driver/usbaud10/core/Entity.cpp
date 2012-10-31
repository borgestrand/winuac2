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
 * @file	   Entity.cpp
 * @brief	   Entity implementation.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "Entity.h"

#define STR_MODULENAME "Entity: "


#pragma code_seg()

/*****************************************************************************
 * CEntity::DescriptorSubtype()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CEntity::
DescriptorSubtype
(	void
)
{
	return m_DescriptorSubtype;
}

/*****************************************************************************
 * CEntity::EntityID()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
UCHAR
CEntity::
EntityID
(	void
)
{
	return m_EntityID;
}

/*****************************************************************************
 * CEntity::SetRequest()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS
CEntity::
SetRequest
(
	IN		UCHAR	RequestCode,
	IN		USHORT	Value,
	IN		PVOID	ParameterBlock,
	IN		ULONG	ParameterBlockSize
)
{
	NTSTATUS ntStatus = m_UsbDevice->ControlClassInterfaceCommand
						(
							RequestCode,
							Value,
							(USHORT(m_EntityID)<<8) | m_InterfaceNumber,
							ParameterBlock,
							ParameterBlockSize,
							NULL,
							FALSE
						);

	return ntStatus;
}

/*****************************************************************************
 * CEntity::GetRequest()
 *****************************************************************************
 * @ingroup TOPOLOGY_GROUP
 * @brief
 */
NTSTATUS
CEntity::
GetRequest
(
	IN		UCHAR	RequestCode,
	IN		USHORT	Value,
	IN		PVOID	ParameterBlock,
	IN 		ULONG 	ParameterBlockSize,
	OUT		ULONG *	OutParameterBlockSize
)
{
	NTSTATUS ntStatus = m_UsbDevice->ControlClassInterfaceCommand
						(
							RequestCode | 0x80,
							Value,
							(USHORT(m_EntityID)<<8) | m_InterfaceNumber,
							ParameterBlock,
							ParameterBlockSize,
							OutParameterBlockSize,
							TRUE
						);

	return ntStatus;
}

#pragma code_seg()
