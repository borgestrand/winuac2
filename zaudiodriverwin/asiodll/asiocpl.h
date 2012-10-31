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
 * @file       asiocpl.h
 * @brief      ASIO control panel private definitions.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-30-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef _ASIO_CONTROL_PANEL_H_
#define _ASIO_CONTROL_PANEL_H_

#include "asiodll.h"

/*****************************************************************************
 * Defines
 */

/*****************************************************************************
 * Classes
 */
/*****************************************************************************
 *//*! @class CAsioControlPanel
 *****************************************************************************
 * @brief
 * ASIO control panel object.
 */
class CAsioControlPanel
{
private:
	HWND			m_WndMain;
	ULONG			m_NumberOfBufferSizes;
	DOUBLE			m_BufferSizes[64];
	ULONG			m_PreferredBufferSizeIndex;
	ULONG			m_NumberOfBitDepths;
	ULONG			m_BitDepths[16];
	ULONG			m_PreferredBitDepthIndex;
	BOOL			m_PerApplicationPreferences;
	CHAR			m_DeviceName[MAX_PATH];

	static CAsioControlPanel *	m_ActiveCpl;

	/*************************************************************************
     * CAsioDriver private methods
     */
	BOOL _OnInitDialog
	(
		IN      HWND    Dlg,
		IN      HWND    DlgFocus,
		IN      LPARAM  lParam
	);

	VOID _OnCommand
	(
		IN      HWND    Dlg,
		IN      INT     Id,
		IN      HWND    Control,
		IN      UINT    NotificationCode
	);

public:
	CAsioControlPanel();
	~CAsioControlPanel();

	/*************************************************************************
     * CAsioControlPanel public methods
     */
	ASIOError Init
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
	);

	ASIOError Run
	(
		OUT		DOUBLE *	OutPreferredBufferSize,
		OUT		ULONG *		OutPreferredBitDepth,
		OUT		BOOL *		OutPerApplicationPreferences
	);

	/*************************************************************************
     * Static
     */
    static 
	INT_PTR CALLBACK DialogProc
	(
		IN		HWND	Dlg, 
		IN		UINT	Message, 
		IN		WPARAM	wParam, 
		IN		LPARAM	lParam
	);

	/*************************************************************************
     * Friends
     */
};

#endif // _ASIO_CONTROL_PANEL_H_
