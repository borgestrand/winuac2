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
 * @file       node.h
 * @brief      This is the header file for C++ classes that expose 
 *             functionality of KS nodes.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _KS_NODE_H_
#define _KS_NODE_H_

#include "ksaudio.h"

/*****************************************************************************
 * Defines
 */
// Reserved node identifiers
#define NODE_UNINITIALIZED					0xFFFFFFFF
#define NODE_WILDCARD						0xFFFFFFFE
#define KSAUDIO_CPU_RESOURCES_UNINITIALIZED 'ENON'


/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CKsNode
 *****************************************************************************
 * @brief
 * This is the base class for classes that proxy KS filters from user mode.
 * @details
 * This class wraps an IRP target and associated node id. It simplifies 
 * property calls on nodes.
 */
class CKsNode
{
private:
    ULONG           m_NodeId;
    GUID            m_NodeType;
    ULONG           m_CpuResources;

public:
	/*************************************************************************
     * Constructor/destructor.
     */  
    CKsNode
	(
		IN		ULONG		NodeId, 
		IN		REFGUID		NodeType, 
		OUT		HRESULT *	OutHResult
	);
    
	CKsNode
	(
		IN		CKsNode*	KsNodeCopy, 
		OUT		HRESULT *	OutHResult
	);

	/*************************************************************************
     * CKsNode public methods
     */  
    GUID GetType
	(	void
	);
    
	ULONG GetNodeId
	(	void
	);

    ULONG GetCpuResources
	(	void
	);
};

#endif // _KS_NODE_H_
