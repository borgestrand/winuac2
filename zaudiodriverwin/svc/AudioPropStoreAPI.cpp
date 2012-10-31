/**
*******************************************************************************
Confidential & Proprietary
Private & Confidential
Creative Confidential
*******************************************************************************
*/
/**
*******************************************************************************
 
@file       AudioPropStoreAPI.cpp
 
@brief      Implementation of the STDAPI calls for getting IAudioPropStore
 
@author     Tan Kee Seng
 
@remarks    

 
Created:    10/10/2006
 
$Date: 2007/07/20 08:20:52 $
 
$Revision: 1.1.2.1 $
 
 
*******************************************************************************

*******************************************************************************
*/
#include "AudioPropStoreWrapper.h"

// 294935ce-f637-4e7c-a41b-ab255460b862
static const GUID CLSID_AUDIOSES = 
{ 0x294935ce, 0xf637, 0x4e7c, { 0xa4, 0x1b, 0xab, 0x25, 0x54, 0x60, 0xb8, 0x62 } };

#include <initguid.h>   // DEFINE_GUID
#include <propkeydef.h>
DEFINE_PROPERTYKEY(MYPKEY_AudioEndpoint_FullRangeSpeakers, 0x1da5d803, 0xd492, 0x4edd, 0x8c, 0x23, 0xe0, 0xc0, 0xff, 0xee, 0x7f, 0x0e, 6);
DEFINE_PROPERTYKEY(MYPKEY_AudioEndpoint_PhysicalSpeakers, 0x1da5d803, 0xd492, 0x4edd, 0x8c, 0x23, 0xe0, 0xc0, 0xff, 0xee, 0x7f, 0x0e, 3);

///////////////////////////////////////////////////////////////////////////////
STDAPI GetPolicyPropertyStore(
    IN  UINT32  storeType,
    IN  LPCWSTR pwstrEndptId,
    OUT IAudioPropStore** ppIAudioPropStore
)
{
    HRESULT hRes = E_FAIL;

    IPolicyConfigClient* pIPolicyCfgClient = NULL;

    hRes = CoCreateInstance(
                CLSID_AUDIOSES, 
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IPolicyConfigClient),
                reinterpret_cast<LPVOID*>(&pIPolicyCfgClient));

    if(SUCCEEDED(hRes))
    {
        CAudioPropStoreWrapper* pPropStoreWrapper = NULL;
        pPropStoreWrapper = new CAudioPropStoreWrapper();
        if(pPropStoreWrapper)
        {
            pPropStoreWrapper->AddRef();
            hRes = pPropStoreWrapper->Init(pwstrEndptId, pIPolicyCfgClient, storeType);
            if(SUCCEEDED(hRes))
            {
                hRes = pPropStoreWrapper->QueryInterface(__uuidof(IAudioPropStore), reinterpret_cast<PVOID*>(ppIAudioPropStore));
            }
            pPropStoreWrapper->Release();
        }
        pIPolicyCfgClient->Release();
    }
    return hRes;
}


///////////////////////////////////////////////////////////////////////////////
STDAPI GetEndpointPropertyStore(
    IN  LPCWSTR pwstrEndptId,
    OUT IAudioPropStore** ppIAudioPropStore
)
{
    return GetPolicyPropertyStore(PROPSTORE_EP, pwstrEndptId, ppIAudioPropStore);
}

///////////////////////////////////////////////////////////////////////////////
STDAPI GetFXPropertyStore(
    IN  LPCWSTR pwstrEndptId,
    OUT IAudioPropStore** ppIAudioPropStore
)
{
    return GetPolicyPropertyStore(PROPSTORE_FX, pwstrEndptId, ppIAudioPropStore);
}

///////////////////////////////////////////////////////////////////////////////
STDAPI GetSpeakerConfig(
    IN  LPCWSTR pwstrEndptId,
    OUT UINT32* pSpeakerConfigMask
)
{
    HRESULT hRes = E_FAIL;

    IPolicyConfigClient* pIPolicyCfgClient = NULL;

    hRes = CoCreateInstance(
                CLSID_AUDIOSES, 
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IPolicyConfigClient),
                reinterpret_cast<LPVOID*>(&pIPolicyCfgClient));

    if(SUCCEEDED(hRes))
    {
        WAVEFORMATEXTENSIBLE* pWaveFormat = NULL;
        hRes = pIPolicyCfgClient->GetDeviceFormat(pwstrEndptId, eRender, &pWaveFormat);

        if(SUCCEEDED(hRes))
        {
            *pSpeakerConfigMask = pWaveFormat->dwChannelMask;
            CoTaskMemFree(pWaveFormat);
            pWaveFormat = NULL;
        }
        pIPolicyCfgClient->Release();
    }
    return hRes;
}

///////////////////////////////////////////////////////////////////////////////
STDAPI SetSpeakerConfigAndSpeakerMasks(
    IN  LPCWSTR pwstrEndptId,
    IN  UINT32  speakerConfig,
    IN  UINT32  physicalSpeakerMask,
    IN  UINT32  fullRangSpeakerMask
)
{
    HRESULT hRes = E_INVALIDARG;

    if(speakerConfig)
    {
        IPolicyConfigClient* pIPolicyCfgClient = NULL;

        hRes = CoCreateInstance(
                    CLSID_AUDIOSES, 
                    NULL,
                    CLSCTX_INPROC_SERVER,
                    __uuidof(IPolicyConfigClient),
                    reinterpret_cast<LPVOID*>(&pIPolicyCfgClient));

        if(SUCCEEDED(hRes))
        {
            WAVEFORMATEXTENSIBLE* pWaveFormat = NULL;

            hRes = pIPolicyCfgClient->GetDeviceFormat(pwstrEndptId, eRender, &pWaveFormat);

            if(SUCCEEDED(hRes))
            {
                UINT32 dwNumCh = 0;
                UINT32 dwTempMask = 0x1;
                // Evaluate the number of channels
                for (UINT32 i = 0; i < sizeof(UINT32)*8; i++)   
                {
                    if (speakerConfig & dwTempMask)
                    {
                        dwNumCh++;
                    }
                    dwTempMask <<= 1;
                }

                pWaveFormat->Format.nChannels = static_cast<WORD>(dwNumCh);
                pWaveFormat->Format.nBlockAlign = pWaveFormat->Format.nChannels * pWaveFormat->Format.wBitsPerSample/8;
                pWaveFormat->Format.nAvgBytesPerSec = pWaveFormat->Format.nBlockAlign * pWaveFormat->Format.nSamplesPerSec;
                pWaveFormat->dwChannelMask = speakerConfig;

                hRes = pIPolicyCfgClient->SetDeviceFormat(pwstrEndptId, pWaveFormat, eRender);
                if(SUCCEEDED(hRes))
                {
                    HRESULT hRes2 = E_FAIL;
                    // Set the physical speaker and full range spaker mask
                    PROPVARIANT var;
                    PropVariantInit(&var);

                    var.vt = VT_UI4;
                    var.ulVal = (speakerConfig & physicalSpeakerMask);;

                    hRes2 = pIPolicyCfgClient->SetPropertyValue(pwstrEndptId, PROPSTORE_EP, MYPKEY_AudioEndpoint_PhysicalSpeakers, var);

                    if(SUCCEEDED(hRes2))
                    {
                        var.vt = VT_UI4;
                        var.ulVal = (speakerConfig & fullRangSpeakerMask);;
                        hRes2 = pIPolicyCfgClient->SetPropertyValue(pwstrEndptId, PROPSTORE_EP, MYPKEY_AudioEndpoint_FullRangeSpeakers, var);

                        if(FAILED(hRes2))
                        {
                            hRes = S_FALSE;
                        }
                    }
                    else
                    {
                        hRes = S_FALSE;
                    }
                }

                CoTaskMemFree(pWaveFormat);
                pWaveFormat = NULL;
            }
            pIPolicyCfgClient->Release();
        }
    }
    return hRes;
}

///////////////////////////////////////////////////////////////////////////////
STDAPI SetSpeakerConfig(
    IN  LPCWSTR pwstrEndptId,
    IN  UINT32  speakerConfig,
    IN  BOOL    isFullRangeSpeakers
)
{
    UINT32 fullRangeSpeakers = 0;
    if(isFullRangeSpeakers)
    {
        fullRangeSpeakers = speakerConfig;
    }
    else
    {
        fullRangeSpeakers = (speakerConfig & SPEAKER_LOW_FREQUENCY);
    }

    return SetSpeakerConfigAndSpeakerMasks(pwstrEndptId, speakerConfig, speakerConfig, fullRangeSpeakers);
}

///////////////////////////////////////////////////////////////////////////////
STDAPI GetEndpointFormat(
    IN  LPCWSTR     pwstrEndptId,
    OUT UINT32*     pSampleRate,
    OUT UINT32*     pNumBits
)
{
    HRESULT hRes = E_FAIL;

    IPolicyConfigClient* pIPolicyCfgClient = NULL;

    hRes = CoCreateInstance(
                CLSID_AUDIOSES, 
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IPolicyConfigClient),
                reinterpret_cast<LPVOID*>(&pIPolicyCfgClient));

    if(SUCCEEDED(hRes))
    {
        WAVEFORMATEXTENSIBLE* pWaveFormat = NULL;
        hRes = pIPolicyCfgClient->GetDeviceFormat(pwstrEndptId, 0, &pWaveFormat);

        if(SUCCEEDED(hRes))
        {
            *pSampleRate = pWaveFormat->Format.nSamplesPerSec;
            *pNumBits = pWaveFormat->Samples.wValidBitsPerSample;
            CoTaskMemFree(pWaveFormat);
            pWaveFormat = NULL;
        }
        pIPolicyCfgClient->Release();
    }
    return hRes;
}

///////////////////////////////////////////////////////////////////////////////
STDAPI SetEndpointFormat(
    IN  LPCWSTR     pwstrEndptId,
    IN  UINT32      sampleRate,
    IN  UINT32      numBits
)
{
    HRESULT hRes = E_INVALIDARG;

    IPolicyConfigClient* pIPolicyCfgClient = NULL;

    hRes = CoCreateInstance(
                CLSID_AUDIOSES, 
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IPolicyConfigClient),
                reinterpret_cast<LPVOID*>(&pIPolicyCfgClient));

    if(SUCCEEDED(hRes))
    {
        WAVEFORMATEXTENSIBLE* pWaveFormat = NULL;

        hRes = pIPolicyCfgClient->GetDeviceFormat(pwstrEndptId, 0, &pWaveFormat);

        if(SUCCEEDED(hRes))
        {
            pWaveFormat->Format.nSamplesPerSec = sampleRate;
            pWaveFormat->Samples.wValidBitsPerSample = static_cast<WORD>(numBits);

            switch(pWaveFormat->Samples.wValidBitsPerSample)
            {
                case 8:
                case 16:
                    pWaveFormat->Format.wBitsPerSample = 16;
                    break;
                case 24:
                case 32:
                    pWaveFormat->Format.wBitsPerSample = 32;
                    break;
            }
            pWaveFormat->Format.nBlockAlign = pWaveFormat->Format.nChannels * pWaveFormat->Format.wBitsPerSample/8;
            pWaveFormat->Format.nAvgBytesPerSec = pWaveFormat->Format.nBlockAlign * pWaveFormat->Format.nSamplesPerSec;

            hRes = pIPolicyCfgClient->SetDeviceFormat(pwstrEndptId, pWaveFormat, 0);

            CoTaskMemFree(pWaveFormat);
            pWaveFormat = NULL;
        }
        pIPolicyCfgClient->Release();
    }

    return hRes;
}

///////////////////////////////////////////////////////////////////////////////
STDAPI SetDefaultEndpoint(
    IN  LPCWSTR     pwstrEndptId,
    IN  ERole       role
)
{
    HRESULT hRes = E_INVALIDARG;

    IPolicyConfigClient* pIPolicyCfgClient = NULL;

    hRes = CoCreateInstance(
                CLSID_AUDIOSES, 
                NULL,
                CLSCTX_INPROC_SERVER,
                __uuidof(IPolicyConfigClient),
                reinterpret_cast<LPVOID*>(&pIPolicyCfgClient));

    if(SUCCEEDED(hRes))
    {
        hRes = pIPolicyCfgClient->SetDefaultEndpoint(pwstrEndptId, role);
        pIPolicyCfgClient->Release();
    }

    return hRes;
}

///////////////////////////////////////////////////////////////////////////////