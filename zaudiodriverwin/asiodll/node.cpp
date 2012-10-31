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
 * @file       node.cpp
 * @brief      This is the implementation file for C++ classes that expose 
 *             functionality of KS nodes.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#include "node.h"

#define STR_MODULENAME "KSNODE: "


/*****************************************************************************
 * CKsNode::CKsNode()
 *****************************************************************************
 *//*!
 * @brief
 * Copy constructor for CKsNode.
 */
CKsNode::
CKsNode
(
	IN		CKsNode *	KsNodeCopy,
    OUT		HRESULT *   OutHResult
)
{
	m_NodeId = 0;
	
    m_CpuResources = KSAUDIO_CPU_RESOURCES_UNINITIALIZED;
    
	m_NodeType = GUID_NULL;

    HRESULT hr = S_OK;

    if (KsNodeCopy)
    {
        m_NodeId = KsNodeCopy->m_NodeId;
        
		m_CpuResources = KsNodeCopy->m_CpuResources;
        
		m_NodeType = KsNodeCopy->m_NodeType;
    }
    
	*OutHResult = hr;
}

/*****************************************************************************
 * CKsNode::CKsNode()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CKsNode::
CKsNode
(
    IN		ULONG		NodeId,
    IN		REFGUID		NodeType,
    OUT		HRESULT *   OutHResult
)
{
	m_NodeId = NodeId;
	
    m_CpuResources = KSAUDIO_CPU_RESOURCES_UNINITIALIZED;
    
	m_NodeType = NodeType;

    *OutHResult = S_OK;
}

/*****************************************************************************
 * CKsNode::()
 *****************************************************************************
 *//*!
 * @brief
 * Returns the node type.
 */
GUID 
CKsNode::
GetType
(	void
)
{
    return m_NodeType;
}

/*****************************************************************************
 * CKsNode::GetNodeId()
 *****************************************************************************
 *//*!
 * @brief
 * Returns the node ID.
 */
ULONG 
CKsNode::
GetNodeId
(	void
)
{
    return m_NodeId;
}

/*****************************************************************************
 * CKsNode::GetCpuResources()
 *****************************************************************************
 *//*!
 * @brief
 * Returns the CPU resources.
 */
ULONG 
CKsNode::
GetCpuResources
(	void
)
{
    return m_CpuResources;
}
