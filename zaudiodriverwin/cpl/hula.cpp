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

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <dbt.h>
#include <cpl.h>
#include <winioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include <setupapi.h>
#include "resource.h"
#include <initguid.h>
#include "prvprop.h"
#include <usb.h>

typedef struct
{
	HDEVNOTIFY	DeviceNotificationHandle;
	TCHAR		DevicePath[1];
} DEVICE_CONTEXT_INFORMATION, *PDEVICE_CONTEXT_INFORMATION;
//
//  Global Variables.
//
HINSTANCE  ghInstance = NULL;      // module handle.
const TCHAR AppletName[] = TEXT("Kahana control panel");

BOOL KahanaPropPageProvider
(
    PSP_PROPSHEETPAGE_REQUEST   pPropPageRequest,
    HPROPSHEETPAGE *            phPropSheetPage
);

BOOL GetUsbDeviceDescriptor (TCHAR * DevicePath,
                      USB_DEVICE_DESCRIPTOR * pUsbDeviceDescriptor);

BOOL GetUsbStringDescriptor (TCHAR * DevicePath,
							 UCHAR Index, USHORT LanguageId,
                      USB_STRING_DESCRIPTOR * pUsbStringDescriptor);

BOOL GetDeviceInterfaceDetail (PSP_PROPSHEETPAGE_REQUEST pPropPageRequest,
                               PSP_DEVICE_INTERFACE_DETAIL_DATA *ppDeviceInterfaceDetailData);

BOOL UpgradeFirmware(HWND hDlg, TCHAR * pDevicePath, TCHAR * szFileName);

BOOL SwitchToUsbVersion(HWND hDlg, TCHAR * pDevicePath, USHORT Version);

#if (DBG)
/////////////////////////////////////////////////////////////////////////////////
// dbgError
/////////////////////////////////////////////////////////////////////////////////
// This function prints an error message.
// It prints first the string passed and then the error that it gets with
// GetLastError as a string.
//
// Arguments:
//    szMsg - message to print.
//
// Return Value:
//    None.
//
void dbgError (LPCTSTR szMsg)
{
    LPTSTR errorMessage;
    DWORD  count;

    // Get the error message from the system.
    count = FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL,
                GetLastError (),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPTSTR)&errorMessage,
                0,
                NULL);

    // Print the msg + error + \n\r.
    if (count)
    {
        OutputDebugString (szMsg);
        OutputDebugString (errorMessage);
        OutputDebugString (TEXT("\n\r"));

        // This is for those without a debugger.
        MessageBox (NULL, errorMessage, szMsg, MB_OK | MB_ICONSTOP);

        LocalFree (errorMessage);
    }
    else
    {
        OutputDebugString (AppletName);
        OutputDebugString (TEXT(": Low memory condition. Cannot ")
                TEXT("print error message.\n\r"));
    }
}
#else
#define dbgError(a) __noop
#endif

/////////////////////////////////////////////////////////////////////////////////
// DllMain
/////////////////////////////////////////////////////////////////////////////////
// Main enty point of the DLL.
// Save the instance handle; it is needed for property sheet creation.
//
// Arguments:
//    hModule            - instance data, is equal to module handle
//    ul_reason_for_call - the reason for the call
//    lpReserved         - some additional parameter.
//
// Return Value:
//    BOOL: FALSE if DLL should fail, TRUE on success
//
BOOL APIENTRY DllMain (HANDLE hModule, ULONG ul_reason_for_call, LPVOID lpReserved)
{
    ghInstance = (HINSTANCE) hModule;
    return TRUE;
}

BOOL QueryPropertySetSupport (TCHAR * DevicePath, GUID PropertySet)
{
    HANDLE          hFilter;
    KSPROPERTY		Property;
    ULONG           ulBytesReturned;
    BOOL            fSuccess;

    // Open the filter.
    hFilter = CreateFile (DevicePath,
                            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, 0, NULL);
    // Check for error.
    if (hFilter == INVALID_HANDLE_VALUE)
    {
        dbgError (TEXT("QueryPropertySetSupport: CreateFile: "));
        return FALSE;
    }

    // Fill the KSPROPERTY structure.
    Property.Set = PropertySet;
    Property.Flags = KSPROPERTY_TYPE_SETSUPPORT;
    Property.Id = 0;

    fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
                                &Property, sizeof (Property),
                                NULL, 0,
                                &ulBytesReturned, NULL);

    // We don't need the handle anymore.
    CloseHandle (hFilter);
    
    // Check for error.
    if (!fSuccess)
    {
        dbgError (TEXT("QueryPropertySetSupport: DeviceIoControl: "));
        return FALSE;
    }

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// FindKahanaDevice
/////////////////////////////////////////////////////////////////////////////////
// This function stores the device info and device data of the "kahana" driver.
// It first creates a list of all devices that have a audio filter exposed
// and then searches through the list to find the device with the service
// "kahana". The information is stored in pspRequest.
//
// For simplicity we search for the service name "kahana".
// Alternately, one could get more information about the device or driver, 
// then decide if it is suitable (regardless of its name).
//
// Arguments:
//    pspRequest        pointer to Property sheet page request structure.
//    DeviceInfoData    pointer to Device info data structure.
//
// Return Value:
//    TRUE on success, otherwise FALSE.
//    Note: on success that we have the device list still open - we have to destroy
//    the list later. The handle is stored at pspRequest->DeviceInfoSet.
//
BOOL FindKahanaDevice (PSP_PROPSHEETPAGE_REQUEST  pspRequest,
                       PSP_DEVINFO_DATA DeviceInfoData,
					   ULONG Index)
{
    //
    // Prepare the pspRequest structure...
    //
    pspRequest->cbSize = sizeof (SP_PROPSHEETPAGE_REQUEST);
    pspRequest->DeviceInfoData = DeviceInfoData;
    pspRequest->PageRequested = SPPSR_ENUM_ADV_DEVICE_PROPERTIES;

    // ...and the DeviceInfoData structure.
    DeviceInfoData->cbSize = sizeof (SP_DEVINFO_DATA);

    // Create a list of devices with Audio interface.
    pspRequest->DeviceInfoSet = SetupDiGetClassDevs (&KSCATEGORY_DEVICECONTROL,
                                     NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    // None found?
    if (pspRequest->DeviceInfoSet == INVALID_HANDLE_VALUE)
    {
        dbgError (TEXT("Test: SetupDiGetClassDevs: "));
        return FALSE;
    }

	int DeviceCount = 0;

	BOOL Found = FALSE;
    //
    // Go through the list of all devices found.
    //
    int nIndex = 0;
    
    while (SetupDiEnumDeviceInfo (pspRequest->DeviceInfoSet, nIndex, DeviceInfoData))
    {
		PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData = NULL;

		// Find the interface that support the KSPROPSETID_DeviceControl
		if (GetDeviceInterfaceDetail (pspRequest, &pDeviceInterfaceDetailData))
		{
			if (QueryPropertySetSupport(pDeviceInterfaceDetailData->DevicePath, KSPROPSETID_DeviceControl))
			{
				if (DeviceCount == Index)
				{
					Found = TRUE;
				}

				DeviceCount++;
			}

	        LocalFree (pDeviceInterfaceDetailData);

			if (Found) break;
		}

        // Take the next in the list.
        nIndex++;
    }
    
	if (!Found)
	{
		SetupDiDestroyDeviceInfoList (pspRequest->DeviceInfoSet);
	}

    return Found;
}


/////////////////////////////////////////////////////////////////////////////////
// AddPropSheet
/////////////////////////////////////////////////////////////////////////////////
// This function is a callback that is passed to the KahanaPropPageProvider
// function. It is used to add the property page sheet to the property dialog.
// What we do here is store the property sheet handle and return.
//
// Arguments:
//    hPSP       property sheet handle
//    lParam     parameter that we passed to "KahanaPropPageProvider".
//
// Return Value:
//    TRUE on success, otherwise FALSE.
//
BOOL APIENTRY AddPropSheet (HPROPSHEETPAGE hPSP, HPROPSHEETPAGE * phPropSheetPage)
{
    *phPropSheetPage = hPSP;
    
    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// DisplayPropertySheet
/////////////////////////////////////////////////////////////////////////////////
// This function displays the property dialog.
// It is called by CPlApplet when the Kahana.cpl icon gets a double click.
//
// Arguments:
//    hWnd      parent window handle
//
// Return Value:
//    None.
//
void DisplayPropertySheet (HWND hWnd)
{
    SP_PROPSHEETPAGE_REQUEST    pspRequest;         // structure passed to Kahanaprop
    SP_DEVINFO_DATA             DeviceInfoData;     // pspRequest points to it.
    PROPSHEETHEADER             psh;

    //
    // Prepare the header for the property sheet.
    //
    psh.nStartPage = 0;
    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE | PSH_NOAPPLYNOW;
    psh.hwndParent = hWnd;
    psh.hInstance = ghInstance;
    psh.pszIcon = NULL;
    psh.pszCaption = MAKEINTRESOURCE(IDS_KAHANACPL);
    psh.nPages = 0;
    // Allocate the space for 32 handle.	
	psh.phpage = (HPROPSHEETPAGE *)LocalAlloc (LPTR, 32*sizeof(HPROPSHEETPAGE));

    // You could move this to CPL_INIT, then it would be called
    // before the control panel window appears.
    // In case of an failure the icon would not be displayed. In our sample
    // however, the icon will be displayed and when the user clicks on it he
    // gets the error message.
	ULONG DeviceIndex = 0;
    
	while (FindKahanaDevice(&pspRequest, &DeviceInfoData, DeviceIndex))
	{
		psh.nPages++;

		// Call the function to request the property sheet page.
		KahanaPropPageProvider(&pspRequest, &psh.phpage[DeviceIndex]);

	    SetupDiDestroyDeviceInfoList (pspRequest.DeviceInfoSet);

		DeviceIndex++;
	}

	if (DeviceIndex == 0)
    {
        MessageBox (hWnd, TEXT("Make sure that your E-MU USB-MIDI device is installed."),
                    AppletName, MB_ICONSTOP | MB_OK);
        return;
    }

    // Create the dialog. The function returns when the dialog is closed.
    if (PropertySheet (&psh) < 0)
    {
        //
        // Dialog closed abnormally. This might be a system failure.
        //
        MessageBox (hWnd, TEXT("Please reinstall the Kahana sample driver."),
                    AppletName, MB_ICONSTOP | MB_OK);
    }

    // Clean up.
    LocalFree (psh.phpage);
}


/////////////////////////////////////////////////////////////////////////////////
// CPlApplet
/////////////////////////////////////////////////////////////////////////////////
// This function is called by the control panel manager. It is very similar to
// a Windows message handler (search for CplApplet in MSDN for description).
//
// Arguments:
//    HWND hWnd         Parent window handle
//    UINT uMsg         The message
//    LPARAM lParam1    depends on message
//    LPARAM lParam2    depends on message
//
// Return Value:
//    LONG (depends on message; in general 0 means failure).
//
LONG APIENTRY CPlApplet (HWND hWnd, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
    switch (uMsg)
    {
        // Initialize. You can allocate memory here.
        case CPL_INIT:
            return TRUE;
        
        //  How many applets are in this DLL?
        case CPL_GETCOUNT:
            return 1;
        
        // Requesting information about the dialog box.
        // lParam1 is the dialog box number (we have only one) and 
        // lParam2 is the pointer to a CPLINFO structure.
        // There is no return value.
        case CPL_INQUIRE:
        {
            UINT      uAppNum = (UINT)lParam1;
            LPCPLINFO pCplInfo = (LPCPLINFO)lParam2;

            if (!pCplInfo)
                return TRUE;    // unsuccessful
                
            if (uAppNum == 0)   // first Applet?
            {
                pCplInfo->idIcon = IDI_KAHANACPL;
                pCplInfo->idName = IDS_KAHANACPL;
                pCplInfo->idInfo = IDS_KAHANACPLINFO;
            }
            break;
        }
        
        // This is basically the same as CPL_INQUIRE, but passes a pointer
        // to a different structure.
        // This function is called before CPL_INQUIRE and if we return zero
        // here, then CPL_INQUIRE is called.
        case CPL_NEWINQUIRE:
            break;
        
        // One of these messages are sent when we should display the dialog box.
        // There is no return value.
        // lParam1 is the dialog box number (we have only one)
        case CPL_DBLCLK:
        case CPL_STARTWPARMS:
        {
            UINT    uAppNum = (UINT)lParam1;

            if (uAppNum == 0)   // first Applet?
                DisplayPropertySheet (hWnd);
            break;
        }
        
        // We get unloaded in a second.
        // There is no return value.
        case CPL_EXIT:
            break;
        
        default:    // Who knows?
            break;
    }

    return 0;
}

typedef struct
{
	ULONG Status;

	struct
	{
		USHORT nDay:5;
		USHORT nMonth:4;
		USHORT nYear:7;
	} BuildNumber;

	USHORT bcdFirmware;
	UCHAR bBoardID;
	UCHAR Reserved1[3];
	UCHAR bSerialNumber[16];
	UCHAR Reserved2[4];
} CREATIVE_FIRMWARE_INFORMATION, *PCREATIVE_FIRMWARE_INFORMATION;

BOOL GetCreativeFirmwareInfo(TCHAR *pDevicePath, PCREATIVE_FIRMWARE_INFORMATION FirmwareInformation)
{
    // Open the filter.
    HANDLE hFilter = CreateFile (pDevicePath,
                            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, 0, NULL);
    // Check for error.
    if (hFilter == INVALID_HANDLE_VALUE)
    {
        dbgError (TEXT("GetCreativeFirmwareInfo: CreateFile: "));
        return FALSE;
    }

	DEVICECONTROL_CUSTOM_COMMAND CustomProperty;

	CustomProperty.Property.Set = KSPROPSETID_DeviceControl;
	CustomProperty.Property.Flags = KSPROPERTY_TYPE_GET;
	CustomProperty.Property.Id = KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND;

	CustomProperty.Parameters.RequestType = URB_FUNCTION_VENDOR_OTHER;
	CustomProperty.Parameters.Request = 0x1; // Get device information
	CustomProperty.Parameters.Value = 0;
	CustomProperty.Parameters.Index = 0;
	CustomProperty.Parameters.BufferLength = 0;

	ULONG ulBytesReturned;
	
	BOOL fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
								&CustomProperty, sizeof (CustomProperty),
								FirmwareInformation, sizeof(CREATIVE_FIRMWARE_INFORMATION),
								&ulBytesReturned, NULL);

	CloseHandle(hFilter);

	return fSuccess;
}

/////////////////////////////////////////////////////////////////////////////////
// UpdateDlgControls 
/////////////////////////////////////////////////////////////////////////////////
// This function gets called when the property sheet page gets created by
// "KahanaPropPage_OnInitDialog". It initializes the different dialog controls that
// get displayed.
// By default all dlg items are set to "Yes", so we only change to "No" if it
// applies.
//
//
// Arguments:
//    hWnd           - handle to the dialog window
//    pUsbDeviceDescriptor  - pointer to USB_DEVICE_DESCRIPTOR structure
//
// Return Value:
//    None.
void UpdateDlgControls (HWND hWnd, TCHAR * pDevicePath)
{
    USB_DEVICE_DESCRIPTOR * pUsbDeviceDescriptor = (USB_DEVICE_DESCRIPTOR *) LocalAlloc (LPTR, sizeof (USB_DEVICE_DESCRIPTOR) + 3*(sizeof(USB_STRING_DESCRIPTOR)+127*sizeof(WCHAR))/*3 USB_STRING_DESCRIPTOR*/);

    if (!pUsbDeviceDescriptor)
    {
        return;
    }

	ZeroMemory(pUsbDeviceDescriptor, sizeof (USB_DEVICE_DESCRIPTOR) + 3*(sizeof(USB_STRING_DESCRIPTOR)+127*sizeof(WCHAR))/*3 USB_STRING_DESCRIPTOR*/);

    // Get the Kahana features through the private property call.
    if (!GetUsbDeviceDescriptor (pDevicePath, pUsbDeviceDescriptor))
    {
        LocalFree (pUsbDeviceDescriptor);
        return;
    }

	PUSB_STRING_DESCRIPTOR pUsbStringDescriptor = PUSB_STRING_DESCRIPTOR(pUsbDeviceDescriptor+1);
	pUsbStringDescriptor->bLength = 2 + 126*sizeof(WCHAR);
	if (pUsbDeviceDescriptor->iManufacturer)
	GetUsbStringDescriptor(pDevicePath, pUsbDeviceDescriptor->iManufacturer, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), pUsbStringDescriptor);

	pUsbStringDescriptor = PUSB_STRING_DESCRIPTOR(PUCHAR(pUsbStringDescriptor)+pUsbStringDescriptor->bLength+2);
	pUsbStringDescriptor->bLength = 2 + 126*sizeof(WCHAR);
	if (pUsbDeviceDescriptor->iProduct)
	GetUsbStringDescriptor(pDevicePath, pUsbDeviceDescriptor->iProduct, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), pUsbStringDescriptor);

	pUsbStringDescriptor = PUSB_STRING_DESCRIPTOR(PUCHAR(pUsbStringDescriptor)+pUsbStringDescriptor->bLength+2);
	pUsbStringDescriptor->bLength = 2 + 126*sizeof(WCHAR);
	if (pUsbDeviceDescriptor->iSerialNumber)
	GetUsbStringDescriptor(pDevicePath, pUsbDeviceDescriptor->iSerialNumber, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), pUsbStringDescriptor);

	WCHAR Text[32];
	wsprintf(Text, L"%x.%02x", pUsbDeviceDescriptor->bcdUSB>>8, pUsbDeviceDescriptor->bcdUSB&0xFF);
	SetWindowText (GetDlgItem (hWnd, IDC_BCDUSB_TEXT), Text);

	wsprintf(Text, L"%04X", pUsbDeviceDescriptor->idVendor);
	SetWindowText (GetDlgItem (hWnd, IDC_VENDORID_TEXT), Text);

	wsprintf(Text, L"%04X", pUsbDeviceDescriptor->idProduct);
	SetWindowText (GetDlgItem (hWnd, IDC_PRODUCTID_TEXT), Text);

	wsprintf(Text, L"%x.%02x", pUsbDeviceDescriptor->bcdDevice>>8, pUsbDeviceDescriptor->bcdDevice&0xFF);
	SetWindowText (GetDlgItem (hWnd, IDC_BCDDEVICE_TEXT), Text);

	pUsbStringDescriptor = PUSB_STRING_DESCRIPTOR(pUsbDeviceDescriptor+1);
	SetWindowText (GetDlgItem (hWnd, IDC_IMANUFACTURER_TEXT), pUsbStringDescriptor->bString);

	pUsbStringDescriptor = PUSB_STRING_DESCRIPTOR(PUCHAR(pUsbStringDescriptor)+pUsbStringDescriptor->bLength+2);
	SetWindowText (GetDlgItem (hWnd, IDC_IPRODUCT_TEXT), pUsbStringDescriptor->bString);

	pUsbStringDescriptor = PUSB_STRING_DESCRIPTOR(PUCHAR(pUsbStringDescriptor)+pUsbStringDescriptor->bLength+2);
	SetWindowText (GetDlgItem (hWnd, IDC_ISERIALNUMBER_TEXT), pUsbStringDescriptor->bString);

	#define TEXT_NOT_AVAILABLE TEXT("<not available>")

	if (pUsbDeviceDescriptor->idVendor == 0x041E)
	{
		// Creative.
		// Send custom command to get firmware information.
		CREATIVE_FIRMWARE_INFORMATION FirmwareInformation;

		if (GetCreativeFirmwareInfo(pDevicePath, &FirmwareInformation))
		{	
			wsprintf(Text, TEXT("%02d/%02d/%02d"), FirmwareInformation.BuildNumber.nDay, FirmwareInformation.BuildNumber.nMonth, FirmwareInformation.BuildNumber.nYear);
			SetWindowText (GetDlgItem (hWnd, IDC_FIRMWARE_BUILD_DATE_TEXT), Text);
			
			wsprintf(Text, TEXT("%x.%02x"), FirmwareInformation.bcdFirmware>>8, FirmwareInformation.bcdFirmware&0xFF);
			SetWindowText (GetDlgItem (hWnd, IDC_BCDFIRMWARE_VERSION_TEXT), Text);
			
			wsprintf(Text, TEXT("%02X"), FirmwareInformation.bBoardID);
			SetWindowText (GetDlgItem (hWnd, IDC_BOARDID_TEXT), Text);

			wsprintf(Text, TEXT("%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X"), 
						FirmwareInformation.bSerialNumber[0], FirmwareInformation.bSerialNumber[1], FirmwareInformation.bSerialNumber[2], FirmwareInformation.bSerialNumber[3],
						FirmwareInformation.bSerialNumber[4], FirmwareInformation.bSerialNumber[5], FirmwareInformation.bSerialNumber[6], FirmwareInformation.bSerialNumber[7],
						FirmwareInformation.bSerialNumber[8], FirmwareInformation.bSerialNumber[9], FirmwareInformation.bSerialNumber[10], FirmwareInformation.bSerialNumber[11],
						FirmwareInformation.bSerialNumber[12], FirmwareInformation.bSerialNumber[13], FirmwareInformation.bSerialNumber[14], FirmwareInformation.bSerialNumber[15]
						);
			SetWindowText (GetDlgItem (hWnd, IDC_SZSERIALNUMBER_TEXT), Text);
		}
		else
		{
			SetWindowText (GetDlgItem (hWnd, IDC_FIRMWARE_BUILD_DATE_TEXT), TEXT_NOT_AVAILABLE);
			SetWindowText (GetDlgItem (hWnd, IDC_BCDFIRMWARE_VERSION_TEXT), TEXT_NOT_AVAILABLE);
			SetWindowText (GetDlgItem (hWnd, IDC_BOARDID_TEXT), TEXT_NOT_AVAILABLE);
			SetWindowText (GetDlgItem (hWnd, IDC_SZSERIALNUMBER_TEXT), TEXT_NOT_AVAILABLE);
		}
	}
	else
	{
		// Unsupported.
		SetWindowText (GetDlgItem (hWnd, IDC_FIRMWARE_BUILD_DATE_TEXT), TEXT_NOT_AVAILABLE);
		SetWindowText (GetDlgItem (hWnd, IDC_BCDFIRMWARE_VERSION_TEXT), TEXT_NOT_AVAILABLE);
		SetWindowText (GetDlgItem (hWnd, IDC_BOARDID_TEXT), TEXT_NOT_AVAILABLE);
		SetWindowText (GetDlgItem (hWnd, IDC_SZSERIALNUMBER_TEXT), TEXT_NOT_AVAILABLE);
	}

	SendMessage(GetDlgItem (hWnd, IDC_FIRMWARE_UPGRADE_PROGRESS), PBM_SETPOS, 0, 0);
}

/////////////////////////////////////////////////////////////////////////////////
// KahanaPropPage_OnInitDialog
/////////////////////////////////////////////////////////////////////////////////
// This function gets called when the property sheet page gets created.  This
// is the perfect opportunity to initialize the dialog items that get displayed.
//
// Arguments:
//    ParentHwnd - handle to the dialog window
//    FocusHwnd  - handle to the control that would get the focus.
//    lParam     - initialization parameter (pUsbDeviceDescriptor).
//
// Return Value:
//    TRUE if focus should be set to FocusHwnd, FALSE if you set the focus yourself.
BOOL KahanaPropPage_OnInitDialog (HWND ParentHwnd, HWND FocusHwnd, LPARAM lParam)
{
    // Check the parameters (lParam is USB_DEVICE_DESCRIPTOR pointer)
    if (!lParam)
        return FALSE;
    
    // put up the wait cursor
    HCURSOR hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

	PDEVICE_CONTEXT_INFORMATION pDeviceContext = (PDEVICE_CONTEXT_INFORMATION)((LPPROPSHEETPAGE)lParam)->lParam;

    UpdateDlgControls (ParentHwnd, pDeviceContext->DevicePath);

	// Register for device notification.
    DEV_BROADCAST_DEVICEINTERFACE BroadcastInterface;
    BroadcastInterface.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    BroadcastInterface.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    BroadcastInterface.dbcc_classguid = KSCATEGORY_DEVICECONTROL; 

    pDeviceContext->DeviceNotificationHandle = RegisterDeviceNotification(ParentHwnd, &BroadcastInterface, DEVICE_NOTIFY_WINDOW_HANDLE);

	// Save this for later use...
	SetWindowLongPtr (ParentHwnd, DWLP_USER,  LONG_PTR(pDeviceContext));

    // remove the wait cursor
    SetCursor(hCursor);

    return TRUE;
}

VOID
KahanaPropPage_OnCommand
(
    IN      HWND    hDlg,
    IN      INT     Id,
    IN      HWND    CtlWnd,
    IN      UINT    NotificationCode
)
{
	switch (Id)
	{
		case IDC_UPGRADE_FIRMWARE_BUTTON:
		{
			MessageBox(hDlg, TEXT("Before proceeding with firmware upgrade, make sure\r\nthat you close all the audio & MIDI applications."), TEXT("Firmware Upgrade"), MB_OK);

			OPENFILENAME ofn;
			TCHAR szFileName[MAX_PATH] = TEXT("");

			ZeroMemory(&ofn, sizeof(ofn));

			ofn.lStructSize = sizeof(ofn); // SEE NOTE BELOW
			ofn.hwndOwner = hDlg;
			ofn.lpstrFilter = TEXT("Firmware package files (*.pkg)\0*.pkg\0All Files (*.*)\0*.*\0");
			ofn.lpstrFile = szFileName;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
			ofn.lpstrDefExt = TEXT("pkg");

			if (GetOpenFileName(&ofn))
			{
				PDEVICE_CONTEXT_INFORMATION pDeviceContext = (PDEVICE_CONTEXT_INFORMATION)GetWindowLongPtr (hDlg, DWLP_USER);

				//MessageBox(hDlg, szFileName, pDeviceContext->DevicePath, MB_OK);

				if (UpgradeFirmware(hDlg, pDeviceContext->DevicePath, szFileName))
				{
				}
			}
		}
		break;

		case IDC_SWITCH_TO_USB20_BUTTON:
		{
			PDEVICE_CONTEXT_INFORMATION pDeviceContext = (PDEVICE_CONTEXT_INFORMATION)GetWindowLongPtr (hDlg, DWLP_USER);

			USB_DEVICE_DESCRIPTOR UsbDeviceDescriptor;

			GetUsbDeviceDescriptor(pDeviceContext->DevicePath, &UsbDeviceDescriptor);

			if (UsbDeviceDescriptor.bcdUSB == 0x0110)
			{
				SwitchToUsbVersion(hDlg, pDeviceContext->DevicePath, 0x0200);
			}
			else
			{
				SwitchToUsbVersion(hDlg, pDeviceContext->DevicePath, 0x0110);
			}
		}
		break;
	}
}

BOOL
KahanaPropPage_OnDeviceChange
(
    IN      HWND        hDlg,
    IN      INT         Event,
    IN      DWORD_PTR   EventData
)
{
    switch (Event)
    {
        case DBT_DEVICEARRIVAL:
        {
			PDEVICE_CONTEXT_INFORMATION pDeviceContext = (PDEVICE_CONTEXT_INFORMATION)GetWindowLongPtr (hDlg, DWLP_USER);

		    UpdateDlgControls (hDlg, pDeviceContext->DevicePath);
        }
        break;
        case DBT_DEVICEREMOVECOMPLETE:
	    case DBT_DEVICEREMOVEPENDING:
        case DBT_DEVICEQUERYREMOVE:
        case DBT_DEVNODES_CHANGED:
        default:
        break;
    }

    return TRUE;
}

VOID
KahanaPropPage_OnDestroy
(
    IN      HWND    hDlg
)
{
	PDEVICE_CONTEXT_INFORMATION pDeviceContext = (PDEVICE_CONTEXT_INFORMATION)GetWindowLongPtr (hDlg, DWLP_USER);

	if (pDeviceContext)
	{
		UnregisterDeviceNotification(pDeviceContext->DeviceNotificationHandle);

		LocalFree(pDeviceContext);
	}
}
	
/////////////////////////////////////////////////////////////////////////////////
// KahanaDlgProc
/////////////////////////////////////////////////////////////////////////////////
// This callback function gets called by the system whenever something happens
// with the dialog sheet. Please take a look at the SDK for further information
// on dialog messages.
//
// Arguments:
//    hDlg     - handle to the dialog window
//    uMessage - the message
//    wParam   - depending on message sent
//    lParam   - depending on message sent
//
// Return Value:
//    int (depends on message).
INT_PTR APIENTRY KahanaDlgProc (HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uMessage)
    {
		HANDLE_MSG(hDlg, WM_INITDIALOG, KahanaPropPage_OnInitDialog);
		HANDLE_MSG(hDlg, WM_COMMAND, KahanaPropPage_OnCommand);
		HANDLE_MSG(hDlg, WM_DEVICECHANGE, KahanaPropPage_OnDeviceChange);
		HANDLE_MSG(hDlg, WM_DESTROY, KahanaPropPage_OnDestroy);
    }

    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////
// GetUsbDeviceDescriptor
/////////////////////////////////////////////////////////////////////////////////
// This function gets called by the property page provider (in this module) to
// get all the Kahana features that are normally not displayed by the drivers.
// To get the UsbDeviceDescriptor structure we pass down the private property. As you
// can see this is fairly easy.
//
// Arguments:
//    pDeviceInterfaceDetailData - device interface details (path to device driver)
//    pUsbDeviceDescriptor              - pointer to Kahana features structure.
//
// Return Value:
//    BOOL: FALSE if we couldn't get the information, TRUE on success.
BOOL GetUsbDeviceDescriptor (TCHAR * DevicePath,
                      USB_DEVICE_DESCRIPTOR * pUsbDeviceDescriptor)
{
    HANDLE          hFilter;
    DEVICECONTROL_DEVICE_DESCRIPTOR  KahanaProperty;
    ULONG           ulBytesReturned;
    BOOL            fSuccess;

    // Open the filter.
    hFilter = CreateFile (DevicePath,
                            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, 0, NULL);
    // Check for error.
    if (hFilter == INVALID_HANDLE_VALUE)
    {
        dbgError (TEXT("GetUsbDeviceDescriptor: CreateFile: "));
        return FALSE;
    }

    // Fill the KSPROPERTY structure.
    KahanaProperty.Property.Set = KSPROPSETID_DeviceControl;
    KahanaProperty.Property.Flags = KSPROPERTY_TYPE_GET;
    KahanaProperty.Property.Id = KSPROPERTY_DEVICECONTROL_DEVICE_DESCRIPTOR;

    fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
                                &KahanaProperty, sizeof (KahanaProperty),
                                pUsbDeviceDescriptor, sizeof (USB_DEVICE_DESCRIPTOR),
                                &ulBytesReturned, NULL);
    // We don't need the handle anymore.
    CloseHandle (hFilter);
    
    // Check for error.
    if (!fSuccess)
    {
        dbgError (TEXT("GetUsbDeviceDescriptor: DeviceIoControl: "));
        return FALSE;
    }

    return TRUE;
}

BOOL GetUsbStringDescriptor (TCHAR * DevicePath,
							 UCHAR Index, USHORT LanguageId,
                      USB_STRING_DESCRIPTOR * pUsbStringDescriptor)
{
    HANDLE          hFilter;
    DEVICECONTROL_STRING_DESCRIPTOR  KahanaProperty;
    ULONG           ulBytesReturned;
    BOOL            fSuccess;

    // Open the filter.
    hFilter = CreateFile (DevicePath,
                            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL, OPEN_EXISTING, 0, NULL);
    // Check for error.
    if (hFilter == INVALID_HANDLE_VALUE)
    {
        dbgError (TEXT("GetUsbStringDescriptor: CreateFile: "));
        return FALSE;
    }

    // Fill the KSPROPERTY structure.
    KahanaProperty.Property.Set = KSPROPSETID_DeviceControl;
    KahanaProperty.Property.Flags = KSPROPERTY_TYPE_GET;
    KahanaProperty.Property.Id = KSPROPERTY_DEVICECONTROL_STRING_DESCRIPTOR;
	KahanaProperty.Parameters.Index = Index;
	KahanaProperty.Parameters.LanguageId = LanguageId;

    fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
                                &KahanaProperty, sizeof (KahanaProperty),
                                pUsbStringDescriptor, pUsbStringDescriptor->bLength,
                                &ulBytesReturned, NULL);

    // We don't need the handle anymore.
    CloseHandle (hFilter);
    
    // Check for error.
    if (!fSuccess)
    {
        dbgError (TEXT("GetUsbStringDescriptor: DeviceIoControl: "));
        return FALSE;
    }

    return TRUE;
}

BOOL LockDevice(HANDLE hFilter)
{
	KSPROPERTY Property;

	// Fill the KSPROPERTY structure.
	Property.Set =KSPROPSETID_DeviceControl;
	Property.Flags = KSPROPERTY_TYPE_SET;
	Property.Id = KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_LOCK;

	ULONG ulBytesReturned;
	
	BOOL fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
								&Property, sizeof (Property),
								NULL, 0,
								&ulBytesReturned, NULL);

	return fSuccess;
}

BOOL UnlockDevice(HANDLE hFilter)
{
	KSPROPERTY Property;

	// Fill the KSPROPERTY structure.
	Property.Set = KSPROPSETID_DeviceControl;
	Property.Flags = KSPROPERTY_TYPE_SET;
	Property.Id = KSPROPERTY_DEVICECONTROL_FIRMWARE_UPGRADE_UNLOCK;

	ULONG ulBytesReturned;
	
	BOOL fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
								&Property, sizeof (Property),
								NULL, 0,
								&ulBytesReturned, NULL);

	return fSuccess;
}

ULONG GetStatus(HANDLE hFilter)
{
	DEVICECONTROL_CUSTOM_COMMAND CustomProperty;

	CustomProperty.Property.Set = KSPROPSETID_DeviceControl;
	CustomProperty.Property.Flags = KSPROPERTY_TYPE_GET;
	CustomProperty.Property.Id = KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND;

	CustomProperty.Parameters.RequestType = URB_FUNCTION_VENDOR_OTHER;
	CustomProperty.Parameters.Request = 0x0; // Get status
	CustomProperty.Parameters.Value = 0;
	CustomProperty.Parameters.Index = 0;
	CustomProperty.Parameters.BufferLength = 0;

	ULONG Status = 1; // general error

	ULONG ulBytesReturned;
	
	BOOL fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
								&CustomProperty, sizeof (CustomProperty),
								&Status, sizeof(Status),
								&ulBytesReturned, NULL);

	return Status;
}

ULONG StartFirmwareUpgrade(HANDLE hFilter, DWORD dwPackageSize, DWORD * pdwBlockSize)
{
	*pdwBlockSize = 0;

	DEVICECONTROL_CUSTOM_COMMAND CustomProperty;

	CustomProperty.Property.Set = KSPROPSETID_DeviceControl;
	CustomProperty.Property.Flags = KSPROPERTY_TYPE_GET;
	CustomProperty.Property.Id = KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND;

	CustomProperty.Parameters.RequestType = URB_FUNCTION_VENDOR_OTHER;
	CustomProperty.Parameters.Request = 0x4; // initiate firmware upgrade
	CustomProperty.Parameters.Value = HIWORD(dwPackageSize);
	CustomProperty.Parameters.Index = LOWORD(dwPackageSize);
	CustomProperty.Parameters.BufferLength = 0;

	struct
	{
		ULONG Status;
		ULONG dwBlockSize;
	} retValue;

	retValue.Status = 1; // GENERIC_ERROR

	ULONG ulBytesReturned;
	
	BOOL fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
								&CustomProperty, sizeof (CustomProperty),
								&retValue, sizeof(retValue),
								&ulBytesReturned, NULL);

	if (fSuccess)
	{
		if (ulBytesReturned == sizeof(retValue))
		{
			if (retValue.Status == 0)
			{
				*pdwBlockSize = retValue.dwBlockSize;
			}
		}
		else
		{
			retValue.Status = 1; // generic error
		}
	}
	else
	{
		retValue.Status = 1; // generic error
	}

	return retValue.Status;
}

ULONG EndFirmwareUpgrade(HANDLE hFilter)
{
	DEVICECONTROL_CUSTOM_COMMAND CustomProperty;

	CustomProperty.Property.Set = KSPROPSETID_DeviceControl;
	CustomProperty.Property.Flags = KSPROPERTY_TYPE_GET;
	CustomProperty.Property.Id = KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND;

	CustomProperty.Parameters.RequestType = URB_FUNCTION_VENDOR_OTHER;
	CustomProperty.Parameters.Request = 0x6; // validate firmware upgrade image
	CustomProperty.Parameters.Value = 0;
	CustomProperty.Parameters.Index = 0;
	CustomProperty.Parameters.BufferLength = 0;

	ULONG Status;

	ULONG ulBytesReturned;
	
	BOOL fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
								&CustomProperty, sizeof (CustomProperty),
								&Status, sizeof(Status),
								&ulBytesReturned, NULL);

	if (fSuccess)
	{
		if (ulBytesReturned != sizeof(Status))
		{
			Status = 1; // generic error
		}
	}

	return Status;
}

ULONG WriteFirmwareBlock(HANDLE hFilter, PVOID Buffer, DWORD dwBlockSize)
{
	ULONG PropertySize = sizeof(DEVICECONTROL_CUSTOM_COMMAND)+dwBlockSize-1;

	PDEVICECONTROL_CUSTOM_COMMAND CustomProperty = PDEVICECONTROL_CUSTOM_COMMAND(LocalAlloc (LPTR, PropertySize));

	CustomProperty->Property.Set = KSPROPSETID_DeviceControl;
	CustomProperty->Property.Flags = KSPROPERTY_TYPE_SET;
	CustomProperty->Property.Id = KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND;

	CustomProperty->Parameters.RequestType = URB_FUNCTION_VENDOR_OTHER;
	CustomProperty->Parameters.Request = 0x5; // write firmware block
	CustomProperty->Parameters.Value = 0;
	CustomProperty->Parameters.Index = 0;
	CustomProperty->Parameters.BufferLength = dwBlockSize;
	CopyMemory(CustomProperty->Parameters.Buffer, Buffer, dwBlockSize);

	ULONG ulBytesReturned;
	
	BOOL fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
								CustomProperty, PropertySize,
								NULL, 0,
								&ulBytesReturned, NULL);

	LocalFree(CustomProperty);

	return GetStatus(hFilter);
}

BOOL UpgradeFirmware(HWND hDlg, TCHAR * pDevicePath, TCHAR * szFileName)
{
	USB_DEVICE_DESCRIPTOR DeviceDescriptor;

	if (GetUsbDeviceDescriptor(pDevicePath, &DeviceDescriptor))
	{
		if (DeviceDescriptor.idVendor == 0x041E)
		{
			// Open the filter.
			HANDLE hFilter = CreateFile (pDevicePath,
									GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
									NULL, OPEN_EXISTING, 0, NULL);
			// Check for error.
			if (hFilter == INVALID_HANDLE_VALUE)
			{
				dbgError (TEXT("UpgradeFirmware: CreateFile: "));
				return FALSE;
			}

			// put up the wait cursor
			HCURSOR hCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));

			BOOL fSuccess = LockDevice(hFilter);

			if (fSuccess)
			{
				//TODO: Upgrade the firmware...
				HANDLE hFile = CreateFile(szFileName, 
										GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 
										NULL, OPEN_EXISTING, 0, NULL);

				if (hFile)
				{
					DWORD dwFileSize = GetFileSize(hFile, NULL);

					if (dwFileSize)
					{
						DWORD dwBlockSize = 0;

						DWORD Status = StartFirmwareUpgrade(hFilter, dwFileSize, &dwBlockSize);

						if (Status == 0)
						{
							if (dwBlockSize)
							{
								PVOID Buffer = (PVOID)LocalAlloc(LPTR, dwBlockSize);

								if (Buffer)
								{								
									SendMessage(GetDlgItem (hDlg, IDC_FIRMWARE_UPGRADE_PROGRESS), PBM_SETPOS, 0, 0);
									SendMessage(GetDlgItem (hDlg, IDC_FIRMWARE_UPGRADE_PROGRESS), PBM_SETRANGE32, 0, dwFileSize);
									SendMessage(GetDlgItem (hDlg, IDC_FIRMWARE_UPGRADE_PROGRESS), PBM_SETSTEP, dwBlockSize, 0);

									while (dwFileSize)
									{
										DWORD dwBytesRead = 0;

										if (ReadFile(hFile, Buffer, dwBlockSize, &dwBytesRead, NULL))
										{
											Status = WriteFirmwareBlock(hFilter, Buffer, dwBytesRead);

											if (Status != 0)
											{
												// Failure to write to firmware... abort.
												fSuccess = FALSE;
												MessageBox(hDlg, TEXT("Firmware write failed. Aborting..."), TEXT("Firmware Upgrade"), MB_OK);
												break;
											}
										}
										else
										{
											// Failure to read file... abort.
											fSuccess = FALSE;
											MessageBox(hDlg, TEXT("File read failed. Aborting..."), TEXT("Firmware Upgrade"), MB_OK);
											break;
										}

										dwFileSize -= dwBytesRead;

										SendMessage(GetDlgItem (hDlg, IDC_FIRMWARE_UPGRADE_PROGRESS), PBM_STEPIT, 0, 0);
									}

									LocalFree(Buffer);
								}
								else
								{
									MessageBox(hDlg, TEXT("Out of memory."), TEXT("Firmware Upgrade"), MB_OK);

									fSuccess = FALSE;
								}
							}

							EndFirmwareUpgrade(hFilter);
						}
					}
				}

				UnlockDevice(hFilter);
			}

			// remove the wait cursor
			SetCursor(hCursor);

			// We don't need the handle anymore.
			CloseHandle (hFilter);
			
			// Check for error.
			if (!fSuccess)
			{
				MessageBox(hDlg, TEXT("Firmware upgrade unsuccessful."), TEXT("Firmware Upgrade"), MB_OK);
				return FALSE;
			}
			else
			{
				//MessageBox(hDlg, TEXT("Firmware upgrade completed."), TEXT("Firmware Upgrade"), MB_OK);
			}

			return TRUE;
		}
		else
		{
			MessageBox(hDlg, TEXT("Firmware upgrade not supported on this device."), TEXT("Firmware Upgrade"), MB_OK);

			return FALSE;
		}
	}

	return TRUE;
}

BOOL SwitchToUsbVersion(HWND hDlg, TCHAR * pDevicePath, USHORT Version)
{
	USB_DEVICE_DESCRIPTOR DeviceDescriptor;

	if (GetUsbDeviceDescriptor(pDevicePath, &DeviceDescriptor))
	{
		if (DeviceDescriptor.idVendor == 0x041E)
		{
			// Open the filter.
			HANDLE hFilter = CreateFile (pDevicePath,
									GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
									NULL, OPEN_EXISTING, 0, NULL);
			// Check for error.
			if (hFilter == INVALID_HANDLE_VALUE)
			{
				dbgError (TEXT("SwitchToUsbVersion: CreateFile: "));
				return FALSE;
			}

			BOOL fSuccess = LockDevice(hFilter);

			if (fSuccess)
			{
				DEVICECONTROL_CUSTOM_COMMAND CustomProperty;

				CustomProperty.Property.Set = KSPROPSETID_DeviceControl;
				CustomProperty.Property.Flags = KSPROPERTY_TYPE_SET;
				CustomProperty.Property.Id = KSPROPERTY_DEVICECONTROL_CUSTOM_COMMAND;

				CustomProperty.Parameters.RequestType = URB_FUNCTION_VENDOR_OTHER;
				CustomProperty.Parameters.Request = 0x29; // switch mode
				CustomProperty.Parameters.Value = (Version==0x0200) ? 1 : 0; // USB2.0 or USB1.1
				CustomProperty.Parameters.Index = 2000;
				CustomProperty.Parameters.BufferLength = 0;

				ULONG ulBytesReturned;
				
				fSuccess = DeviceIoControl (hFilter, IOCTL_KS_PROPERTY,
											&CustomProperty, sizeof (CustomProperty),
											NULL, 0,
											&ulBytesReturned, NULL);
				
				UnlockDevice(hFilter);
			}

			// We don't need the handle anymore.
			CloseHandle (hFilter);
			
			// Check for error.
			if (!fSuccess)
			{
				MessageBox(hDlg, TEXT("USB2.0 switch unsuccessful."), TEXT("USB2.0 Switch"), MB_OK);
				return FALSE;
			}
			else
			{
				//MessageBox(hDlg, TEXT("USB2.0 switch completed."), TEXT("USB2.0 Switch"), MB_OK);
			}

			return TRUE;
		}
		else
		{
			MessageBox(hDlg, TEXT("USB2.0 switch not supported on this device."), TEXT("USB2.0 Switch"), MB_OK);

			return FALSE;
		}
	}

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////
// GetDeviceInterfaceDetail
/////////////////////////////////////////////////////////////////////////////////
// This function gets called by the property page provider (in this module) to
// get the device interface details. The device interface detail contains a
// path to the device driver that can be used to open the device driver.
// When we parse the driver we look for the Audio interface since this
// interface exposes the private property.
//
// Arguments:
//    pPropPageRequest           - points to SP_PROPSHEETPAGE_REQUEST
//    pDeviceInterfaceDetailData - device interface details returned.
//
// Return Value:
//    BOOL: FALSE if something went wrong, TRUE on success.
BOOL GetDeviceInterfaceDetail (PSP_PROPSHEETPAGE_REQUEST pPropPageRequest,
                               PSP_DEVICE_INTERFACE_DETAIL_DATA *ppDeviceInterfaceDetailData)
{
    BOOL                        fSuccess;
    ULONG                       ulDeviceInstanceIdSize = 0;
    PTSTR                       pDeviceInstanceID = NULL;
    HDEVINFO                    hDevInfoWithInterface;
    SP_DEVICE_INTERFACE_DATA    DeviceInterfaceData;
    ULONG                       ulDeviceInterfaceDetailDataSize = 0;

    // Get the device instance id (PnP string).  The first call will retrieve
    // the buffer length in characters.  fSuccess will be FALSE.
    fSuccess = SetupDiGetDeviceInstanceId (pPropPageRequest->DeviceInfoSet,
                                           pPropPageRequest->DeviceInfoData,
                                           NULL,
                                           0,
                                           &ulDeviceInstanceIdSize);
    // Check for error.
    if ((GetLastError () != ERROR_INSUFFICIENT_BUFFER) || (!ulDeviceInstanceIdSize))
    {
        dbgError (TEXT("GetDeviceInterfaceDetail: SetupDiGetDeviceInstanceId: "));
        return FALSE;
    }

    // Allocate the buffer for the device instance ID (PnP string).
    pDeviceInstanceID = (PTSTR)LocalAlloc (LPTR, ulDeviceInstanceIdSize * sizeof (TCHAR));
    if (!pDeviceInstanceID)
    {
        dbgError (TEXT("GetDeviceInterfaceDetail: LocalAlloc: "));
        return FALSE;
    }
    
    // Now call again, this time with all parameters.
    fSuccess = SetupDiGetDeviceInstanceId (pPropPageRequest->DeviceInfoSet,
                                           pPropPageRequest->DeviceInfoData,
                                           pDeviceInstanceID,
                                           ulDeviceInstanceIdSize,
                                           NULL);
    // Check for error.
    if (!fSuccess)
    {
        dbgError (TEXT("GetDeviceInterfaceDetail: SetupDiGetDeviceInstanceId: "));
        LocalFree (pDeviceInstanceID);
        return FALSE;
    }

    // Now we can get the handle to the dev info with interface.
    // We parse the device specifically for audio interfaces.
    hDevInfoWithInterface = SetupDiGetClassDevs (&KSCATEGORY_DEVICECONTROL,
                                                 pDeviceInstanceID,
                                                 NULL,
                                                 DIGCF_DEVICEINTERFACE);
    // We don't need pDeviceInstanceID anymore.
    LocalFree (pDeviceInstanceID);

    // Check for error.
    if (hDevInfoWithInterface == INVALID_HANDLE_VALUE)
    {
        dbgError (TEXT("GetDeviceInterfaceDetail: SetupDiGetClassDevs: "));
        return FALSE;
    }

    // Go through the list of audio device interface of this device.
    // We assume that there is only one audio device interface and
    // we will store the device details in our private structure.
    DeviceInterfaceData.cbSize = sizeof (DeviceInterfaceData);
    fSuccess = SetupDiEnumDeviceInterfaces (hDevInfoWithInterface,
                                            NULL,
                                            &KSCATEGORY_DEVICECONTROL,
                                            0,
                                            &DeviceInterfaceData);
    // Check for error.
    if (!fSuccess)
    {
        dbgError (TEXT("GetDeviceInterfaceDetail: SetupDiEnumDeviceInterfaces: "));
        SetupDiDestroyDeviceInfoList (hDevInfoWithInterface);
        return FALSE;
    }

    // Get the details for this device interface.  The first call will retrieve
    // the buffer length in characters.  fSuccess will be FALSE.
    fSuccess = SetupDiGetDeviceInterfaceDetail (hDevInfoWithInterface,
                                                &DeviceInterfaceData,
                                                NULL,
                                                0,
                                                &ulDeviceInterfaceDetailDataSize,
                                                NULL);
    // Check for error.
    if ((GetLastError () != ERROR_INSUFFICIENT_BUFFER) || (!ulDeviceInterfaceDetailDataSize))
    {
        dbgError (TEXT("GetDeviceInterfaceDetail: SetupDiGetDeviceInterfaceDetail: "));
        SetupDiDestroyDeviceInfoList (hDevInfoWithInterface);
        return FALSE;
    }

    // Allocate the buffer for the device interface detail data.
    if (!(*ppDeviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)
            LocalAlloc (LPTR, ulDeviceInterfaceDetailDataSize)))
    {
        dbgError (TEXT("GetDeviceInterfaceDetail: LocalAlloc: "));
        SetupDiDestroyDeviceInfoList (hDevInfoWithInterface);
        return FALSE;
    }
    // The size contains only the structure, not the additional path.
    (*ppDeviceInterfaceDetailData)->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

    // Get the details for this device interface, this time with all paramters.
    fSuccess = SetupDiGetDeviceInterfaceDetail (hDevInfoWithInterface,
                                                &DeviceInterfaceData,
                                                *ppDeviceInterfaceDetailData,
                                                ulDeviceInterfaceDetailDataSize,
                                                NULL,
                                                NULL);
    // We don't need the handle anymore.
    SetupDiDestroyDeviceInfoList (hDevInfoWithInterface);

    if (!fSuccess)
    {
        dbgError (TEXT("GetDeviceInterfaceDetail: SetupDiGetDeviceInterfaceDetail: "));
        LocalFree (*ppDeviceInterfaceDetailData), *ppDeviceInterfaceDetailData = NULL;
        return FALSE;
    }

    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////////
// KahanaPropPageProvider
/////////////////////////////////////////////////////////////////////////////////
// This function gets called by the device manager when it asks for additional
// property sheet pages. The parameter fAddFunc is the function that we call to
// add the sheet page to the dialog.
// This routine gets called because the registry entry "EnumPropPage32" tells
// the device manager that there is a dll with a exported function that will add
// a property sheet page.
// Because we want to fail this function (not create the sheet) if the driver
// doesn't support the private property, we have to do all the work here, that
// means we open the device and get all the information, then we close the
// device and return.
//
// Arguments:
//    pPropPageRequest - points to SP_PROPSHEETPAGE_REQUEST
//    fAddFunc         - function ptr to call to add sheet.
//    lparam           - add sheet functions private data handle.
//
// Return Value:
//    BOOL: FALSE if pages could not be added, TRUE on success
BOOL KahanaPropPageProvider
(
    PSP_PROPSHEETPAGE_REQUEST   pPropPageRequest,
    HPROPSHEETPAGE *            phPropSheetPage
)
{
    PSP_DEVICE_INTERFACE_DETAIL_DATA pDeviceInterfaceDetailData;
    PROPSHEETPAGE                    PropSheetPage;
    HPROPSHEETPAGE                   hPropSheetPage;

    // Check page requested
    if (pPropPageRequest->PageRequested != SPPSR_ENUM_ADV_DEVICE_PROPERTIES)
    {
        return FALSE;
    }

    // Check device info set and data
    if ((!pPropPageRequest->DeviceInfoSet) || (!pPropPageRequest->DeviceInfoData))
    {
        return FALSE;
    }

    // Get the device interface detail which return a path to the device
    // driver that we need to open the device.
    if (!GetDeviceInterfaceDetail (pPropPageRequest, &pDeviceInterfaceDetailData))
    {
        return FALSE;
    }

	// Allocate the memory for the Kahana features.
	PDEVICE_CONTEXT_INFORMATION pDeviceContext = (PDEVICE_CONTEXT_INFORMATION)LocalAlloc(LPTR, sizeof(DEVICE_CONTEXT_INFORMATION) + sizeof(TCHAR)*(wcslen(pDeviceInterfaceDetailData->DevicePath)+1));

    if (!pDeviceContext)
    {
        dbgError (TEXT("KahanaPropPageProvider: LocalAlloc: "));
        LocalFree (pDeviceInterfaceDetailData);
        return FALSE;
    }

	wcscpy(pDeviceContext->DevicePath, pDeviceInterfaceDetailData->DevicePath);

    // We don't need the device interface details any more, get rid of it now!
    LocalFree (pDeviceInterfaceDetailData);

    USB_DEVICE_DESCRIPTOR * pUsbDeviceDescriptor = (USB_DEVICE_DESCRIPTOR *) LocalAlloc (LPTR, sizeof (USB_DEVICE_DESCRIPTOR) + 3*(sizeof(USB_STRING_DESCRIPTOR)+127*sizeof(WCHAR))/*3 USB_STRING_DESCRIPTOR*/);
	ZeroMemory(pUsbDeviceDescriptor, sizeof (USB_DEVICE_DESCRIPTOR) + 3*(sizeof(USB_STRING_DESCRIPTOR)+127*sizeof(WCHAR))/*3 USB_STRING_DESCRIPTOR*/);

    GetUsbDeviceDescriptor (pDeviceContext->DevicePath, pUsbDeviceDescriptor);

	PUSB_STRING_DESCRIPTOR pUsbStringDescriptor = PUSB_STRING_DESCRIPTOR(pUsbDeviceDescriptor+1);
	pUsbStringDescriptor->bLength = 2 + 126*sizeof(WCHAR);
	if (pUsbDeviceDescriptor->iProduct)
	GetUsbStringDescriptor(pDeviceContext->DevicePath, pUsbDeviceDescriptor->iProduct, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), pUsbStringDescriptor);

    // initialize the property page
    PropSheetPage.dwSize        = sizeof(PROPSHEETPAGE);
    PropSheetPage.dwFlags       = PSP_USETITLE;
    PropSheetPage.pszTitle      = pUsbStringDescriptor->bString;
    PropSheetPage.hInstance     = ghInstance;
    PropSheetPage.pszTemplate   = MAKEINTRESOURCE(DLG_KAHANA);
    PropSheetPage.pfnDlgProc    = KahanaDlgProc;
    PropSheetPage.lParam        = (LPARAM)pDeviceContext;
    PropSheetPage.pfnCallback   = NULL;

    // create the page and get back a handle
    hPropSheetPage = CreatePropertySheetPage (&PropSheetPage);
    if (!hPropSheetPage)
    {
        LocalFree (pDeviceContext);
        return FALSE;
    }

    // add the property page
    if (!AddPropSheet(hPropSheetPage, phPropSheetPage))
    {
        DestroyPropertySheetPage (hPropSheetPage);
		LocalFree (pUsbStringDescriptor);
        LocalFree (pDeviceContext);
        return FALSE;
    }

	LocalFree (pUsbStringDescriptor);

    return TRUE;
}
