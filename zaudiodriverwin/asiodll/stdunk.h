
/*****************************************************************************
 * stdunk.h - standard IUnknown implementaton definitions
 *****************************************************************************
 * Copyright (c) Microsoft Corporation. All rights reserved.
 */

#ifndef _STDUNK_H_
#define _STDUNK_H_

#include <windows.h>

#include <unknwn.h>
#ifdef PUT_GUIDS_HERE
#include <initguid.h>
#endif

typedef IUnknown *PUNKNOWN;

/*****************************************************************************
 * PFNCREATEINSTANCE
 *****************************************************************************
 * Type for object create function.
 */
typedef
HRESULT
(*PFNCREATEINSTANCE)
(
    OUT PUNKNOWN *  Unknown,
    IN  REFCLSID    ClassId,
    IN  PUNKNOWN    OuterUnknown
);

/*****************************************************************************
 * Interfaces
 */

/*****************************************************************************
 * INonDelegatingUnknown
 *****************************************************************************
 * Non-delegating unknown interface.
 */
DECLARE_INTERFACE(INonDelegatingUnknown)
{
    STDMETHOD_(HRESULT,NonDelegatingQueryInterface)
    (   THIS_
        IN      REFIID,
        OUT     PVOID *
    )   PURE;

    STDMETHOD_(ULONG,NonDelegatingAddRef)
    (   THIS
    )   PURE;

    STDMETHOD_(ULONG,NonDelegatingRelease)
    (   THIS
    )   PURE;
};

typedef INonDelegatingUnknown *PNONDELEGATINGUNKNOWN;





/*****************************************************************************
 * Classes
 */

/*****************************************************************************
 * CUnknown
 *****************************************************************************
 * Base INonDelegatingUnknown implementation.
 */
class CUnknown : public INonDelegatingUnknown
{
private:

    LONG            m_lRefCount;        // Reference count.
    PUNKNOWN        m_pUnknownOuter;    // Outer IUnknown.

	static 
	LONG			m_lActiveObjects;	// Total number of active objects.

public:

    /*************************************************************************
	 * CUnknown methods.
     */
    CUnknown(PUNKNOWN pUnknownOuter);
	virtual ~CUnknown(void);
    PUNKNOWN GetOuterUnknown(void)
    {
        return m_pUnknownOuter;
    }

    /*************************************************************************
	 * INonDelegatingUnknown methods.
     */
	STDMETHODIMP_(ULONG) NonDelegatingAddRef
    (   void
    );
	STDMETHODIMP_(ULONG) NonDelegatingRelease
    (   void
    );
    STDMETHODIMP_(HRESULT) NonDelegatingQueryInterface
	(
		REFIID		rIID,
		PVOID *	    ppVoid
	);

    /*************************************************************************
	 * Static methods.
     */
	static
	LONG ObjectsActive
	(	void
	)
	{
		return m_lActiveObjects;
	}
};





/*****************************************************************************
 * Macros
 */

/*****************************************************************************
 * DECLARE_STD_UNKNOWN
 *****************************************************************************
 * Various declarations for standard objects based on CUnknown.
 */
#define DECLARE_STD_UNKNOWN()                                   \
    STDMETHODIMP_(HRESULT) NonDelegatingQueryInterface          \
	(                                                           \
		REFIID		iid,                                        \
		PVOID *	    ppvObject                                   \
	);                                                          \
    STDMETHODIMP_(HRESULT) QueryInterface(REFIID riid, void **ppv)        \
    {                                                           \
        return GetOuterUnknown()->QueryInterface(riid,ppv);     \
    };                                                          \
    STDMETHODIMP_(ULONG) AddRef()                               \
    {                                                           \
        return GetOuterUnknown()->AddRef();                     \
    };                                                          \
    STDMETHODIMP_(ULONG) Release()                              \
    {                                                           \
        return GetOuterUnknown()->Release();                    \
    }

#define DEFINE_STD_CONSTRUCTOR(Class)                           \
    Class(PUNKNOWN pUnknownOuter)                               \
    :   CUnknown(pUnknownOuter)                                 \
    {                                                           \
    }

#define DEFINE_STD_CONSTRUCTOR_EX(Class, BaseClass)             \
    Class(PUNKNOWN pUnknownOuter)                               \
    :   BaseClass(pUnknownOuter)                                \
    {                                                           \
    }

#define QICAST(Type)                                            \
    PVOID((Type)(this))

#define QICASTUNKNOWN(Type)                                     \
    PVOID(PUNKNOWN((Type)(this)))

#define STD_CREATE_BODY_(Class,ppUnknown,pUnknownOuter,base)            \
    HRESULT hr;                                                         \
    Class *p = new Class(pUnknownOuter);                                \
    if (p)                                                              \
    {                                                                   \
        *ppUnknown = PUNKNOWN((base)(p));                               \
        (*ppUnknown)->AddRef();                                         \
        hr = S_OK;                                                      \
    }                                                                   \
    else                                                                \
    {                                                                   \
        hr = E_OUTOFMEMORY;                                             \
    }                                                                   \
    return hr

#define STD_CREATE_BODY(Class,ppUnknown,pUnknownOuter) \
    STD_CREATE_BODY_(Class,ppUnknown,pUnknownOuter,PUNKNOWN)






/*****************************************************************************
 * Functions
 */
#ifndef PC_KDEXT    // this is not needed for the KD extensions.
#ifndef _NEW_DELETE_OPERATORS_
#define _NEW_DELETE_OPERATORS_

extern HANDLE hDllHeap;

/*****************************************************************************
 * ::new()
 *****************************************************************************
 * New function for creating objects with a specified allocation tag.
 */
inline PVOID __cdecl operator new
(
    size_t          iSize
)
{
    PVOID result = HeapAlloc(hDllHeap, 0, iSize);

    if (result)
    {
        ZeroMemory(result,iSize);
    }

    return result;
}

/*****************************************************************************
 * ::delete()
 *****************************************************************************
 * Delete function.
 */
inline void __cdecl operator delete
(
    PVOID pVoid
)
{
    HeapFree(hDllHeap, 0, pVoid);
}


#endif //!_NEW_DELETE_OPERATORS_

#endif  // PC_KDEXT



#endif

