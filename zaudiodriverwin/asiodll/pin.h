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
 * @file       pin.h
 * @brief      This is the header file for C++ classes that expose 
 *             functionality of KS pins.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _KS_PIN_H_
#define _KS_PIN_H_


/*****************************************************************************
 * Defines
 */
typedef struct 
{
    ULONG                       InterfaceCount;
    PKSIDENTIFIER               Interfaces;
    ULONG                       MediumCount;
    PKSIDENTIFIER               Mediums;
    ULONG                       DataRangeCount;
    PKSDATARANGE                DataRanges;
    ULONG                       ConstrainedDataRangeCount;
    PKSDATARANGE                ConstrainedDataRanges;
    KSPIN_DATAFLOW              DataFlow;
    KSPIN_COMMUNICATION         Communication;
    KSPIN_CINSTANCES            Instances;
    KSPIN_CINSTANCES            InstancesGlobal;
    KSPIN_CINSTANCES            InstancesNecessary;
    PKSPIN_PHYSICALCONNECTION   PhysicalConnection;
    GUID                        Category;
    PWCHAR                      Name;

    // these are redundant.  Free them when you are done with Interfaces, Mediums, DataRanges
    PKSMULTIPLE_ITEM            MultipleDataRanges;
    PKSMULTIPLE_ITEM            MultipleMediums;
    PKSMULTIPLE_ITEM            MultipleInterfaces;
} PIN_DESCRIPTOR, *PPIN_DESCRIPTOR;


/*****************************************************************************
 * Classes
 */
class CKsFilter;

/*****************************************************************************
 *//*! @class CKsPin
 *****************************************************************************
 * @brief
 * This is the base class for classes that proxy KS filters from user mode.
 * @details
 * The pin class is still virtual and must be overidden to be useful. 
 * Specifically, the m_KsPinConnect, m_PinType need to be properly initialized 
 * in the derived class. 
 */
class CKsPin 
:	public	CKsIrpTarget
{
private:
    KSSTATE         m_KsState;					// state of pin
    WCHAR           m_FriendlyName[MAX_PATH];	// friendly name of the pin
    BOOL            m_Looped;
    CKsPin *        m_LinkPin;					// optional pin to connect to
    BOOL            m_CompleteOnlyOnRunState;

	/*****************************************************************************
	 * CKsPin private methods
	 */
    HRESULT Init
	(	void
	);

protected:
    DWORD				m_Alignment;
    PKSPIN_CONNECT		m_KsPinConnect;     // creation parameters of pin
    DWORD				m_KsPinConnectSize;  // size of the creation parameters
    ULONG				m_PinId;
    CKsFilter *			m_KsFilter;
    GUID				m_Category;
    PIN_DESCRIPTOR		m_PinDescriptor;       // description of Pin
    KS_TECHNOLOGY_TYPE	m_PinType;

public:
	/*****************************************************************************
	 * Constructor/destructor
	 */
    CKsPin
	(
        IN		CKsFilter * KsFilter, 
        IN		ULONG		PinId,
        OUT		HRESULT *	OutHResult
	);

    virtual ~CKsPin
	(	void
	);

	/*****************************************************************************
	 * CKsPin public methods
	 */
    virtual 
	HRESULT Instantiate
	(
        IN		BOOL	Looped = FALSE
	);

    VOID ClosePin
	(	void
	);

    HRESULT SetFormat
	(
		IN		PKSDATAFORMAT	DataFormat
	);

    HRESULT GetState
	(
        OUT		PKSSTATE	OutKsState
	);

    virtual 
	HRESULT SetState
	(
        IN		KSSTATE	KsState
	);

    HRESULT Reset
	(	void
	);

    HRESULT WriteData
	(
        IN		PKSSTREAM_HEADER	KsStreamHeader,
        IN		LPOVERLAPPED		Overlapped
	);

    HRESULT ReadData
	(
        IN		PKSSTREAM_HEADER	KsStreamHeader,
        IN		LPOVERLAPPED		Overlapped
	);

    ULONG GetPinId
	(	void
	);
    
	KS_TECHNOLOGY_TYPE GetType
	(	void
	);
    
    PPIN_DESCRIPTOR GetPinDescriptor
	(	void
	);

    HRESULT SetPinConnect
	(
		IN		PKSPIN_CONNECT	KsPinConnect, 
		IN		ULONG			KsPinConnectSize
	);

    HRESULT GetDataFlow
	(
		OUT		KSPIN_DATAFLOW *	OutDataFlow
	);

    HRESULT GetCommunication
	(
		OUT		KSPIN_COMMUNICATION *	OutCommunication
	);

	HRESULT GetFriendlyName
	(
		OUT		PWCHAR *	OutFriendlyName
	);
    
	HRESULT GetLatencies
	(
		IN		KSTIME *	Latencies,
		IN OUT	ULONG *		NumberOfLatencies
	);

	BOOL HasDataRangeWithSpecifier
	(
		IN		REFGUID	FormatSpecifier
	);
};

#endif // _KS_PIN_H_
