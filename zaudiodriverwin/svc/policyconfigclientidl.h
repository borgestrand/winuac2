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
/* Compiler settings for policyconfigclientidl.idl:
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

#ifndef __policyconfigclientidl_h__
#define __policyconfigclientidl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IPolicyConfigClient_FWD_DEFINED__
#define __IPolicyConfigClient_FWD_DEFINED__
typedef interface IPolicyConfigClient IPolicyConfigClient;
#endif 	/* __IPolicyConfigClient_FWD_DEFINED__ */


/* header files for imported files */
#include "mmdeviceapi.h"

#ifdef __cplusplus
extern "C"{
#endif 


/* interface __MIDL_itf_policyconfigclientidl_0000_0000 */
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

typedef /* [public][public][public] */ struct __MIDL___MIDL_itf_policyconfigclientidl_0000_0000_0001
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

typedef struct __MIDL___MIDL_itf_policyconfigclientidl_0000_0000_0001 *PWAVEFORMATEXTENSIBLE;


#pragma pack(pop)
#endif /* 0 */
#if 0
typedef PROPERTYKEY *REFPROPERTYKEY;

#endif // 0
#include <propkeydef.h>
#define PROPSTORE_EP 0
#define PROPSTORE_FX 1


extern RPC_IF_HANDLE __MIDL_itf_policyconfigclientidl_0000_0000_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_policyconfigclientidl_0000_0000_v0_0_s_ifspec;

#ifndef __IPolicyConfigClient_INTERFACE_DEFINED__
#define __IPolicyConfigClient_INTERFACE_DEFINED__

/* interface IPolicyConfigClient */
/* [unique][helpstring][uuid][object] */ 


EXTERN_C const IID IID_IPolicyConfigClient;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("568b9108-44bf-40b4-9006-86afe5b5a620")
    IPolicyConfigClient : public IUnknown
    {
    public:
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetMixFormat_ParamsUnknown( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetDeviceFormat( 
            /* [in] */ __RPC__in LPCWSTR pwstrId,
            /* [in] */ DWORD someZero,
            /* [out] */ __RPC__deref_out_opt WAVEFORMATEXTENSIBLE **ppWaveFormat) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetDeviceFormat( 
            /* [in] */ __RPC__in LPCWSTR pwstrId,
            /* [in] */ __RPC__in WAVEFORMATEXTENSIBLE *pWaveFormat,
            /* [in] */ DWORD someZero) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetProcessingPeriod_ParamsUnknown( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetProcessingPeriod_ParamsUnknown( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetSharedMode_ParamsUnknown( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetSharedMode_ParamsUnknown( void) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE GetPropertyValue( 
            /* [in] */ __RPC__in LPCWSTR pwstrId,
            /* [in] */ DWORD storeType,
            /* [in] */ __RPC__in REFPROPERTYKEY key,
            /* [out] */ __RPC__out PROPVARIANT *pv) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetPropertyValue( 
            /* [in] */ __RPC__in LPCWSTR pwstrId,
            /* [in] */ DWORD storeType,
            /* [in] */ __RPC__in REFPROPERTYKEY key,
            /* [in] */ __RPC__in REFPROPVARIANT propvar) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetDefaultEndpoint( 
            /* [in] */ __RPC__in LPCWSTR pwstrId,
            /* [in] */ ERole role) = 0;
        
        virtual /* [helpstring][id] */ HRESULT STDMETHODCALLTYPE SetEndpointVisibility_ParamsUnknown( void) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IPolicyConfigClientVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IPolicyConfigClient * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [iid_is][out] */ 
            __RPC__deref_out  void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IPolicyConfigClient * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IPolicyConfigClient * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetMixFormat_ParamsUnknown )( 
            IPolicyConfigClient * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetDeviceFormat )( 
            IPolicyConfigClient * This,
            /* [in] */ __RPC__in LPCWSTR pwstrId,
            /* [in] */ DWORD someZero,
            /* [out] */ __RPC__deref_out_opt WAVEFORMATEXTENSIBLE **ppWaveFormat);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetDeviceFormat )( 
            IPolicyConfigClient * This,
            /* [in] */ __RPC__in LPCWSTR pwstrId,
            /* [in] */ __RPC__in WAVEFORMATEXTENSIBLE *pWaveFormat,
            /* [in] */ DWORD someZero);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetProcessingPeriod_ParamsUnknown )( 
            IPolicyConfigClient * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetProcessingPeriod_ParamsUnknown )( 
            IPolicyConfigClient * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetSharedMode_ParamsUnknown )( 
            IPolicyConfigClient * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetSharedMode_ParamsUnknown )( 
            IPolicyConfigClient * This);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *GetPropertyValue )( 
            IPolicyConfigClient * This,
            /* [in] */ __RPC__in LPCWSTR pwstrId,
            /* [in] */ DWORD storeType,
            /* [in] */ __RPC__in REFPROPERTYKEY key,
            /* [out] */ __RPC__out PROPVARIANT *pv);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetPropertyValue )( 
            IPolicyConfigClient * This,
            /* [in] */ __RPC__in LPCWSTR pwstrId,
            /* [in] */ DWORD storeType,
            /* [in] */ __RPC__in REFPROPERTYKEY key,
            /* [in] */ __RPC__in REFPROPVARIANT propvar);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetDefaultEndpoint )( 
            IPolicyConfigClient * This,
            /* [in] */ __RPC__in LPCWSTR pwstrId,
            /* [in] */ ERole role);
        
        /* [helpstring][id] */ HRESULT ( STDMETHODCALLTYPE *SetEndpointVisibility_ParamsUnknown )( 
            IPolicyConfigClient * This);
        
        END_INTERFACE
    } IPolicyConfigClientVtbl;

    interface IPolicyConfigClient
    {
        CONST_VTBL struct IPolicyConfigClientVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IPolicyConfigClient_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IPolicyConfigClient_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IPolicyConfigClient_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IPolicyConfigClient_GetMixFormat_ParamsUnknown(This)	\
    ( (This)->lpVtbl -> GetMixFormat_ParamsUnknown(This) ) 

#define IPolicyConfigClient_GetDeviceFormat(This,pwstrId,someZero,ppWaveFormat)	\
    ( (This)->lpVtbl -> GetDeviceFormat(This,pwstrId,someZero,ppWaveFormat) ) 

#define IPolicyConfigClient_SetDeviceFormat(This,pwstrId,pWaveFormat,someZero)	\
    ( (This)->lpVtbl -> SetDeviceFormat(This,pwstrId,pWaveFormat,someZero) ) 

#define IPolicyConfigClient_GetProcessingPeriod_ParamsUnknown(This)	\
    ( (This)->lpVtbl -> GetProcessingPeriod_ParamsUnknown(This) ) 

#define IPolicyConfigClient_SetProcessingPeriod_ParamsUnknown(This)	\
    ( (This)->lpVtbl -> SetProcessingPeriod_ParamsUnknown(This) ) 

#define IPolicyConfigClient_GetSharedMode_ParamsUnknown(This)	\
    ( (This)->lpVtbl -> GetSharedMode_ParamsUnknown(This) ) 

#define IPolicyConfigClient_SetSharedMode_ParamsUnknown(This)	\
    ( (This)->lpVtbl -> SetSharedMode_ParamsUnknown(This) ) 

#define IPolicyConfigClient_GetPropertyValue(This,pwstrId,storeType,key,pv)	\
    ( (This)->lpVtbl -> GetPropertyValue(This,pwstrId,storeType,key,pv) ) 

#define IPolicyConfigClient_SetPropertyValue(This,pwstrId,storeType,key,propvar)	\
    ( (This)->lpVtbl -> SetPropertyValue(This,pwstrId,storeType,key,propvar) ) 

#define IPolicyConfigClient_SetDefaultEndpoint(This,pwstrId,role)	\
    ( (This)->lpVtbl -> SetDefaultEndpoint(This,pwstrId,role) ) 

#define IPolicyConfigClient_SetEndpointVisibility_ParamsUnknown(This)	\
    ( (This)->lpVtbl -> SetEndpointVisibility_ParamsUnknown(This) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IPolicyConfigClient_INTERFACE_DEFINED__ */


/* interface __MIDL_itf_policyconfigclientidl_0000_0001 */
/* [local] */ 

typedef IPolicyConfigClient *PPOLICYCONFIGCLIENT;



extern RPC_IF_HANDLE __MIDL_itf_policyconfigclientidl_0000_0001_v0_0_c_ifspec;
extern RPC_IF_HANDLE __MIDL_itf_policyconfigclientidl_0000_0001_v0_0_s_ifspec;

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


