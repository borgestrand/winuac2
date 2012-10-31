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
 * @file       asiocpl.cpp
 * @brief      ASIO control panel implementation.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */

#include "asiocpl.h"
#include "windowsx.h"
#include "resource.h"

#define STR_MODULENAME "ASIOCPL: "

extern HINSTANCE hDllInstance;

/*****************************************************************************
 * CAsioControlPanel::Instance()
 *****************************************************************************
 *//*!
 * @brief
 */
CAsioControlPanel *
CAsioControlPanel::m_ActiveCpl = NULL;

/*****************************************************************************
 * CAsioControlPanel::CAsioControlPanel()
 *****************************************************************************
 *//*!
 * @brief
 * Constructor.
 */
CAsioControlPanel::
CAsioControlPanel
(   void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioControlPanel::CAsioControlPanel]"));

	m_ActiveCpl = this;
}

/*****************************************************************************
 * CAsioControlPanel::~CAsioControlPanel()
 *****************************************************************************
 *//*!
 * @brief
 * Destructor.
 */
CAsioControlPanel::
~CAsioControlPanel
(   void
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioControlPanel::~CAsioControlPanel]"));

	m_ActiveCpl = NULL;
}

/*****************************************************************************
 * CAsioControlPanel::Init()
 *****************************************************************************
 *//*!
 * @brief
 */
ASIOError 
CAsioControlPanel::
Init
(
	IN		HWND		WndMain, 
	IN		DOUBLE *	BufferSizes, 
	IN		ULONG		NumberOfBufferSizes, 
	IN		ULONG		PreferredBufferSizeIndex, 
	IN		ULONG *		BitDepths, 
	IN		ULONG		NumberOfBitDepths, 
	IN		ULONG		PreferredBitDepthIndex, 
	IN		BOOL		PerApplicationPreferences,
	IN		CHAR *		DeviceName
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioControlPanel::Init]"));

	m_WndMain = WndMain;

	// Buffer sizes
	ASSERT(NumberOfBufferSizes <= 64);

	if (PreferredBufferSizeIndex >= NumberOfBufferSizes)
	{
		PreferredBufferSizeIndex = NumberOfBufferSizes - 1;
	}

	m_NumberOfBufferSizes = NumberOfBufferSizes;

	m_PreferredBufferSizeIndex = PreferredBufferSizeIndex;

	CopyMemory(m_BufferSizes, BufferSizes, NumberOfBufferSizes * sizeof(DOUBLE));

	// Bit depths
	ASSERT(NumberOfBitDepths <= 16);

	if (PreferredBitDepthIndex >= NumberOfBitDepths)
	{
		PreferredBitDepthIndex = NumberOfBitDepths - 1;
	}

	m_NumberOfBitDepths = NumberOfBitDepths;

	m_PreferredBitDepthIndex = PreferredBitDepthIndex;

	CopyMemory(m_BitDepths, BitDepths, NumberOfBitDepths * sizeof(ULONG));

	// Per application preferences
	m_PerApplicationPreferences = PerApplicationPreferences;

	// Device name
	strcpy(m_DeviceName, DeviceName);

	return ASE_OK;
}

/*****************************************************************************
 * CAsioControlPanel::Run()
 *****************************************************************************
 *//*!
 * @brief
 */
ASIOError 
CAsioControlPanel::
Run
(
	OUT		DOUBLE *	OutPreferredBufferSize,
	OUT		ULONG *		OutPreferredBitDepth,
	OUT		BOOL *		OutPerApplicationPreferences
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioControlPanel::Run]"));

    DialogBox(hDllInstance, "IDD_ASIOCP_TITANIUM", m_WndMain, CAsioControlPanel::DialogProc);

	if (OutPreferredBufferSize)
	{
		*OutPreferredBufferSize = m_BufferSizes[m_PreferredBufferSizeIndex];
	}

	if (OutPreferredBitDepth)
	{
		*OutPreferredBitDepth = m_BitDepths[m_PreferredBitDepthIndex];
	}

	if (OutPerApplicationPreferences)
	{
		*OutPerApplicationPreferences = m_PerApplicationPreferences;
	}

	return ASE_OK;
}

/*****************************************************************************
 * CAsioControlPanel::DialogProc()
 *****************************************************************************
 *//*!
 * @brief
 */
INT_PTR CALLBACK 
CAsioControlPanel::
DialogProc
(
	IN		HWND	Dlg, 
	IN		UINT	Message, 
	IN		WPARAM	wParam, 
	IN		LPARAM	lParam
)
{
    _DbgPrintF(DEBUGLVL_VERBOSE,("[CAsioControlPanel::DialogProc]"));

	if (m_ActiveCpl)
	{
		switch (Message)
		{
			HANDLE_MSG(Dlg, WM_INITDIALOG,   CAsioControlPanel::m_ActiveCpl->_OnInitDialog);
			HANDLE_MSG(Dlg, WM_COMMAND,      CAsioControlPanel::m_ActiveCpl->_OnCommand);
		}

		return FALSE;
	}
	else
	{
		return FALSE;
	}
}

/*****************************************************************************
 * CAsioControlPanel::_OnInitDialog()
 *****************************************************************************
 * Process the WM_INITDIALOG message which is sent immediately before the
 * dialog box is displayed. Initialize the controls and carry out any other
 * initialization tasks that affect the appearance of th dialog box.
 */
BOOL
CAsioControlPanel::
_OnInitDialog
(
    IN      HWND    Dlg,
    IN      HWND    DlgFocus,
    IN      LPARAM  lParam
)
{
	TCHAR Text[256];

	// Setup dialog text.
	LoadString(hDllInstance, IDS_PREFERENCES, Text, sizeof(Text)/sizeof(TCHAR));
	SetDlgItemText(Dlg, IDC_PREFERENCES, Text);

	LoadString(hDllInstance, IDS_BUFFERSIZE, Text, sizeof(Text)/sizeof(TCHAR));
	SetDlgItemText(Dlg, IDC_BUFFERSIZE, Text);

	LoadString(hDllInstance, IDS_BITDEPTH, Text, sizeof(Text)/sizeof(TCHAR));
	SetDlgItemText(Dlg, IDC_BITDEPTH, Text);

	LoadString(hDllInstance, IDS_OK, Text, sizeof(Text)/sizeof(TCHAR));
	SendMessage(GetDlgItem(Dlg, IDC_OK), WM_SETTEXT, 0, (LPARAM)Text);

	LoadString(hDllInstance, IDS_CANCEL, Text, sizeof(Text)/sizeof(TCHAR));
	SendMessage(GetDlgItem(Dlg, IDC_CANCEL), WM_SETTEXT, 0, (LPARAM)Text);

	// Set the dialog title.
	SendMessage(Dlg, WM_SETTEXT, 0, (LPARAM)m_DeviceName);

	HBITMAP Bitmap = LoadBitmap(hDllInstance, MAKEINTRESOURCE(IDB_EMU));	

	if (Bitmap)
	{
		// Got the bitmap, so slap the bitmap on the logo dialog item.
		SendMessage(GetDlgItem(Dlg, IDC_CTLOGO), STM_SETIMAGE, (WPARAM)IMAGE_BITMAP, (LPARAM)Bitmap);
	}

	// Latencies
	HWND ComboControl = GetDlgItem(Dlg, IDC_COMBOLATENCIES);
	
	for (ULONG i=0; i<m_NumberOfBufferSizes; i++)
	{
		CHAR BufferSizeStr[64];

		if (ULONG(m_BufferSizes[i]) < 1)
		{
			sprintf(BufferSizeStr, "%1.2f ms", m_BufferSizes[i]);
		}
		else
		{
			sprintf(BufferSizeStr, "%d ms", ULONG(m_BufferSizes[i]));
		}

        SendMessage(ComboControl, CB_INSERTSTRING, i, (LPARAM)BufferSizeStr);
	}

	SendMessage(ComboControl, CB_SETCURSEL, m_PreferredBufferSizeIndex, 0);

	// Bit depths
	ComboControl = GetDlgItem(Dlg, IDC_COMBOBITDEPTHS);
	
	for (ULONG i=0; i<m_NumberOfBitDepths; i++)
	{
		CHAR BitDepthStr[64];

		sprintf(BitDepthStr, "%d-bit", m_BitDepths[i]);

        SendMessage(ComboControl, CB_INSERTSTRING, i, (LPARAM)BitDepthStr);
	}

	SendMessage(ComboControl, CB_SETCURSEL, m_PreferredBitDepthIndex, 0);

	EnableWindow(ComboControl, (m_NumberOfBitDepths <= 1) ? FALSE : TRUE);

	// Per application preferences
	LoadString(hDllInstance, IDS_PERAPPLICATIONPREFERENCES, Text, sizeof(Text)/sizeof(TCHAR));
	SetDlgItemText(Dlg, IDC_PERAPPLICATIONPREFERENCES, Text);

	HWND CheckBox = GetDlgItem(Dlg, IDC_PERAPPLICATIONPREFERENCES);

	SendMessage(CheckBox, BM_SETCHECK, m_PerApplicationPreferences ? BST_CHECKED : BST_UNCHECKED, 0);

	return TRUE;
}

/*****************************************************************************
 * CAsioControlPanel::_OnCommand()
 *****************************************************************************
 * Process the WM_COMMAND message which is sent when the user selects a
 * command item from a menu, when a control sends a notification message to
 * its parent window, or when an accelerator keystroke is translated.
 */
VOID
CAsioControlPanel::
_OnCommand
(
    IN      HWND    Dlg,
    IN      INT     Id,
    IN      HWND    Control,
    IN      UINT    NotificationCode
)
{
    switch (Id)
    {
		case IDC_OK:
		{
			m_PreferredBufferSizeIndex = (ULONG)SendMessage(GetDlgItem(Dlg, IDC_COMBOLATENCIES), CB_GETCURSEL, 0, 0);

			m_PreferredBitDepthIndex = (ULONG)SendMessage(GetDlgItem(Dlg, IDC_COMBOBITDEPTHS), CB_GETCURSEL, 0, 0);

			m_PerApplicationPreferences = (SendMessage(GetDlgItem(Dlg, IDC_PERAPPLICATIONPREFERENCES), BM_GETCHECK, 0, 0) == BST_CHECKED) ? TRUE : FALSE;

			// Destory the window.
            EndDialog(Dlg, 0);
		}
		break;

        case IDC_CANCEL:
        {
            // Destory the window.
            EndDialog(Dlg, 0);
        }
        break;

		case IDC_CTLOGO:
        {
            if (NotificationCode == STN_DBLCLK)
            {
				// Show the author of this program.
				ShowWindow(GetDlgItem(Dlg, IDC_AUTHOR), TRUE);
            }
        }
        break;
	}
}


