
/*****************************************************************************
 * stdunk.cpp - standard unknown implementation
 *****************************************************************************
 * Copyright (c) 1997-2000 Microsoft Corporation.  All Rights Reserved.
 *
 */

#include "stdunk.h"
#include "dbg.h"




/*****************************************************************************
 * CUnknown implementation
 */

/*****************************************************************************
 * CUnknown::m_lActiveObjects
 *****************************************************************************
 */
LONG CUnknown::m_lActiveObjects = 0;

/*****************************************************************************
 * CUnknown::CUnknown()
 *****************************************************************************
 * Constructor.
 */
CUnknown::CUnknown(PUNKNOWN pUnknownOuter)
:   m_lRefCount(0)
{
	InterlockedIncrement(&m_lActiveObjects);

    if (pUnknownOuter)
    {
        m_pUnknownOuter = pUnknownOuter;
    }
    else
    {
        m_pUnknownOuter = PUNKNOWN(dynamic_cast<PNONDELEGATINGUNKNOWN>(this));
    }
}

/*****************************************************************************
 * CUnknown::~CUnknown()
 *****************************************************************************
 * Destructor.
 */
CUnknown::~CUnknown(void)
{
	InterlockedDecrement(&m_lActiveObjects);
}




/*****************************************************************************
 * INonDelegatingUnknown implementation
 */

/*****************************************************************************
 * CUnknown::NonDelegatingAddRef()
 *****************************************************************************
 * Register a new reference to the object without delegating to the outer
 * unknown.
 */
STDMETHODIMP_(ULONG) CUnknown::NonDelegatingAddRef(void)
{
    ASSERT(m_lRefCount >= 0);

    InterlockedIncrement(&m_lRefCount);

    return ULONG(m_lRefCount);
}

/*****************************************************************************
 * CUnknown::NonDelegatingRelease()
 *****************************************************************************
 * Release a reference to the object without delegating to the outer unknown.
 */
STDMETHODIMP_(ULONG) CUnknown::NonDelegatingRelease(void)
{
    ASSERT(m_lRefCount > 0);

    if (InterlockedDecrement(&m_lRefCount) == 0)
	{
        m_lRefCount++;
        delete this;
        return 0;
	}

    return ULONG(m_lRefCount);
}

/*****************************************************************************
 * CUnknown::NonDelegatingQueryInterface()
 *****************************************************************************
 * Obtains an interface.
 */
STDMETHODIMP_(HRESULT) CUnknown::NonDelegatingQueryInterface
(
    REFIID  rIID,
    PVOID * ppVoid
)
{
    ASSERT(ppVoid);

    if (IsEqualGUID(rIID,IID_IUnknown))
    {
        *ppVoid = PVOID(PUNKNOWN(this));
    }
    else
    {
        *ppVoid = NULL;
    }

    if (*ppVoid)
    {
        PUNKNOWN(*ppVoid)->AddRef();
        return S_OK;
    }

    return E_INVALIDARG;
}

