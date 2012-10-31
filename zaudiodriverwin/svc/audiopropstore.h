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


/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 7.00.0499 */
/* Compiler settings for audiopropstore.idl:
    Oicf, W1, Zp8, env=Win64 (32b run)
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __audiopropstore_h__
#define __audiopropstore_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IAudioPropStore_FWD_DEFINED__
#define __IAudioPropStore_FWD_DEFINED__
typedef interface IAudioPropStore IAudioPropStore;
#endif 	/* __IAudioPropStore_FWD_DEFINED__ */


/* header files for imported files */
//below header file will cause error LNK2005
//#include "mmdeviceapi.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_audiopropstore_0000_0000 */
/* [local] */ 

#include <mmreg.h>
#if 0

#pragma pack(push, 1)
typedef struct tWAVEFORMATEX
    {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
    BYTE pExtraBytes[ 1 ];
    } 	WAVEFORMATEX;

typedef struct tWAVEFORMATEX *PWAVEFORMATEX;

typedef struct tWAVEFORMATEX *NPWAVEFORMATEX;

typedef struct tWAVEFORMATEX *LPWAVEFORMATEX;

typedef /* [public] */ struct __MIDL___MIDL_itf_audiopropstore_0000_0000_0001
    {
    WORD wFormatTag;
    WORD nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD nBlockAlign;
    WORD wBitsPerSample;
    WORD cbSize;
    WORD wValidBitsPerSample;
    DWORD dwChannelMask;
    GUID SubFormat;
    } 	WAVEFORMATEXTENSIBLE;

typedef struct __MIDL___MIDL_itf_audiopropstore_0000_0000_0001 *PWAVEFORMATEXTENSIBLE;


#pragma pack(pop)
#endif /* 0 */
#if 0
typedef PROPERTYKEY *REFPROPERTYKEY;

#endif // 0
#include <propkeydef.h>


extern RPC_IF_HANDLE __MIDL_itf_audiopropstore_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_audiopropstore_0000_0000_v0_0_s_ifspec;

#ifndef __IAudioPropStore_INTERFACE_DEFINED__
#define __IAudioPropStore_INTERFACE_DEFINED__

/* interface IAudioPropStore */
/* [unique][object][helpstring][uuid] */ 


EXTERN_C const IID IID_IAudioPropStore;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("D81A7E53-D02A-4922-975E-07B48FFB21FC")
    IAudioPropStore : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE GetValue( 
            /* [in] */ __RPC__in REFPROPERTYKEY key,
            /* [out] */ __RPC__out PROPVARIANT *pv) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE SetValue( 
            /* [in] */ __RPC__in REFPROPERTYKEY key,
            /* [in] */ __RPC__in REFPROPVARIANT propvar) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAudioPropStoreVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IAudioPropStore * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IAudioPropStore * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IAudioPropStore * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetValue )( 
            IAudioPropStore * This,
            /* [in] */ __RPC__in REFPROPERTYKEY key,
            /* [out] */ __RPC__out PROPVARIANT *pv);
        
        HRESULT ( STDMETHODCALLTYPE *SetValue )( 
            IAudioPropStore * This,
            /* [in] */ __RPC__in REFPROPERTYKEY key,
            /* [in] */ __RPC__in REFPROPVARIANT propvar);
        
        END_INTERFACE
    } IAudioPropStoreVtbl;

    interface IAudioPropStore
    {
        CONST_VTBL struct IAudioPropStoreVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAudioPropStore_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IAudioPropStore_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IAudioPropStore_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IAudioPropStore_GetValue(This,key,pv)	\
    ( (This)->lpVtbl -> GetValue(This,key,pv) ) 

#define IAudioPropStore_SetValue(This,key,propvar)	\
    ( (This)->lpVtbl -> SetValue(This,key,propvar) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IAudioPropStore_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_audiopropstore_0000_0001 */
/* [local] */ 

typedef /* [unique] */  __RPC_unique_pointer IAudioPropStore *LPAUDIOPROPSTORE;

//
// GetEndpointPropertyStore
//
STDAPI GetEndpointPropertyStore(
    IN  LPCWSTR pwstrEndptId,
    OUT IAudioPropStore** ppIAudioPropStore
    );
//
// GetFXPropertyStore
//
STDAPI GetFXPropertyStore(
    IN  LPCWSTR pwstrEndptId,
    OUT IAudioPropStore** ppIAudioPropStore
    );
//
// GetSpeakerConfig
//
STDAPI GetSpeakerConfig(
    IN  LPCWSTR pwstrEndptId,
    OUT UINT32* pSpeakerConfigMask
    );
//
// SetSpeakerConfig
//
STDAPI SetSpeakerConfig(
    IN  LPCWSTR pwstrEndptId,
    IN  UINT32  speakerConfig,
    IN  BOOL    isFullRangeSpeakers = FALSE
    );
//
// SetSpeakerConfigAndSpeakerMasks
//
STDAPI SetSpeakerConfigAndSpeakerMasks(
    IN  LPCWSTR pwstrEndptId,
    IN  UINT32  speakerConfig,
    IN  UINT32  physicalSpeakerMask,
    IN  UINT32  fullRangSpeakerMask
    );
//
// GetEndpointFormat
//
STDAPI GetEndpointFormat(
    IN  LPCWSTR      pwstrEndptId,
    OUT UINT32*      pSampleRate,
    OUT UINT32*      pNumBits
    );
//
// SetEndpointFormat
//
STDAPI SetEndpointFormat(
    IN  LPCWSTR      pwstrEndptId,
    IN  UINT32       sampleRate,
    IN  UINT32       numBits
    );


extern RPC_IF_HANDLE __MIDL_itf_audiopropstore_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_audiopropstore_0000_0001_v0_0_s_ifspec;

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree(     unsigned long *, LPSAFEARRAY * ); 

unsigned long             __RPC_USER  BSTR_UserSize64(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal64(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal64(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree64(     unsigned long *, BSTR * ); 

unsigned long             __RPC_USER  LPSAFEARRAY_UserSize64(     unsigned long *, unsigned long            , LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserMarshal64(  unsigned long *, unsigned char *, LPSAFEARRAY * ); 
unsigned char * __RPC_USER  LPSAFEARRAY_UserUnmarshal64(unsigned long *, unsigned char *, LPSAFEARRAY * ); 
void                      __RPC_USER  LPSAFEARRAY_UserFree64(     unsigned long *, LPSAFEARRAY * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


