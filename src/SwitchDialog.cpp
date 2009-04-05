/*
This file is part of BufferSwitcher for Notepad++
Copyright (C)2009 Dave Brotherstone <davegb@pobox.com>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#include "stdafx.h"

#include "SwitchDialog.h"
#include "PluginInterface.h"
#include <windows.h>
#include <tchar.h>
#include <map>

WNDPROC g_oldEditProc = 0;
WNDPROC g_oldListProc = 0;

LRESULT CALLBACK editProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool extended;

 	switch (message)
	{
		case WM_KEYDOWN:
			extended = ((lParam & 0x01000000) == 0x01000000);
			if (wParam == VK_DOWN || wParam == VK_UP || (wParam == VK_NEXT && extended) || (wParam == VK_PRIOR && extended))
			{
				::SendMessage(GetParent(hwnd), WM_KEYDOWN, wParam, 0);
				return TRUE;
			}
				
		default:
			if (g_oldEditProc) 
			{
				return ::CallWindowProc(g_oldEditProc, hwnd, message, wParam, lParam);
			}
			else
			{
				return ::DefWindowProc(hwnd, message, wParam, lParam);
			}



	}
	
}


LRESULT CALLBACK listProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool extended;

 	switch (message)
	{
		case WM_KEYDOWN:
			// extended = ((lParam & 0x01000000) == 0x01000000);
			::SendMessage(GetParent(hwnd), WM_KEYDOWN, wParam, 0);
			return TRUE;
		
		case WM_KEYUP:
			if (wParam == VK_CONTROL)
			{
				::SendMessage(GetParent(hwnd), WM_KEYUP, wParam, 0);
				return TRUE;
			}
			break;

		case WM_GETDLGCODE :
		{
			LRESULT dlgCode = CallWindowProc(g_oldListProc, hwnd, message, wParam, lParam);
			dlgCode |= DLGC_WANTTAB;
			return dlgCode;
		}

		default:
			if (g_oldListProc) 
			{
				return ::CallWindowProc(g_oldListProc, hwnd, message, wParam, lParam);
			}
			else
			{
				return ::DefWindowProc(hwnd, message, wParam, lParam);
			}



	}
	
}



void SwitchDialog::setFilenames(TCHAR **filenames, int nbFiles)
{
	_editFiles = new EditFile*[nbFiles];
	_bufferToIndex.clear();
	for(int index = 0; index < nbFiles; ++index)
	{
		int bufferID = ::SendMessage(_nppData._nppHandle, NPPM_GETBUFFERIDFROMPOS, index, 0);

		_editFiles[index] = new EditFile(index, filenames[index], _options->searchFlags, bufferID);
		_bufferToIndex[bufferID] = index;
	}
	
	_nbFiles = nbFiles;
}


void SwitchDialog::doDialog()
{
	
    if (!isCreated())
	{
        create(IDD_SWITCHDIALOG);
		g_oldEditProc = (WNDPROC)::GetWindowLong(GetDlgItem(_hSelf, IDC_FILEEDIT), GWLP_WNDPROC);
		::SetWindowLong(GetDlgItem(_hSelf, IDC_FILEEDIT), GWLP_WNDPROC, (LONG)editProc);

		g_oldListProc = (WNDPROC)::GetWindowLong(GetDlgItem(_hSelf, IDC_FILELIST), GWLP_WNDPROC);
		::SetWindowLong(GetDlgItem(_hSelf, IDC_FILELIST), GWLP_WNDPROC, (LONG)listProc);

		::GetClientRect(GetDlgItem(_hSelf, IDOK), &_okButtonRect);
		::GetClientRect(GetDlgItem(_hSelf, IDCANCEL), &_cancelButtonRect);
		::GetClientRect(GetDlgItem(_hSelf, IDC_FILEEDIT), &_editboxRect);
		::GetClientRect(GetDlgItem(_hSelf, IDC_FILELIST), &_listboxRect);
		
		WINDOWINFO dialogInfo;
		RECT dialogRect;
		::GetWindowInfo(_hSelf, &dialogInfo);
		::GetClientRect(_hSelf, &dialogRect);

		WINDOWINFO controlInfo;
		::GetWindowInfo(GetDlgItem(_hSelf, IDOK), &controlInfo);
		_okButtonRect.top = controlInfo.rcClient.top - dialogInfo.rcClient.top;
		_okButtonRect.left = dialogRect.right - (controlInfo.rcClient.left - dialogInfo.rcClient.left);

		::GetWindowInfo(GetDlgItem(_hSelf, IDCANCEL), &controlInfo);
		_cancelButtonRect.top = controlInfo.rcClient.top - dialogInfo.rcClient.top;
		_cancelButtonRect.left = _okButtonRect.left;

		::GetWindowInfo(GetDlgItem(_hSelf, IDC_FILEEDIT), &controlInfo);
		_editboxRect.top = controlInfo.rcClient.top - dialogInfo.rcClient.top;
		_editboxRect.left = controlInfo.rcClient.left - dialogInfo.rcClient.left;
		_editboxRect.right = dialogRect.right - _editboxRect.right;

		::GetWindowInfo(GetDlgItem(_hSelf, IDC_FILELIST), &controlInfo);
		_listboxRect.top = controlInfo.rcClient.top - dialogInfo.rcClient.top;
		_listboxRect.left = controlInfo.rcClient.left - dialogInfo.rcClient.left;
		_listboxRect.right = dialogRect.right - _listboxRect.right;
		_listboxRect.bottom = dialogRect.bottom - _listboxRect.bottom;
		
		// If the position and dimensions have not been set (from disk-loaded settings)
		if (_dialogX == -1)
		{
			goToCenter();
			updateWindowPosition();
		}

		

	}
	

	refreshSearchFlags();
    showAndPositionWindow();
	
	if (_options->emulateCtrlTab)
	{
			_overrideCtrlTab = FALSE;
			SHORT ctrlState = ::GetKeyState(VK_CONTROL) & 0x80;
			SHORT shiftState = ::GetKeyState(VK_SHIFT) & 0x80;
			
			searchFiles(_T(""));
			int currentBufferID = ::SendMessage(_nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
			
			int currentIndex = _bufferToIndex[currentBufferID];


			::SetDlgItemText(_hSelf, IDC_FILEEDIT, _T(""));
			::SendDlgItemMessage(_hSelf, IDC_FILELIST, LB_SETCURSEL, currentIndex, 0);
			if (shiftState) 
				moveSelectionUp(TRUE);
			else 
				moveSelectionDown(TRUE);

			::SetFocus(GetDlgItem(_hSelf, IDC_FILELIST));
			EnableWindow(GetDlgItem(_hSelf, IDC_FILEEDIT), FALSE);
			
	}
	else 
	{

		int currentBufferID = ::SendMessage(_nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
		if (_typedForFile->find(currentBufferID) != _typedForFile->end())
		{
			::SetDlgItemText(_hSelf, IDC_FILEEDIT, (*_typedForFile)[currentBufferID]);
		}
		

		TCHAR searchString[SEARCH_STRING_BUFFER_MAX];
		

		::GetDlgItemText(_hSelf, IDC_FILEEDIT, (LPTSTR)searchString, SEARCH_STRING_BUFFER_MAX);
		searchFiles(searchString);

		::SendDlgItemMessage(_hSelf, IDC_FILEEDIT, EM_SETSEL, 0, _tcslen(searchString));
		::SetFocus(GetDlgItem(_hSelf, IDC_FILEEDIT));
	}
}

void SwitchDialog::showAndPositionWindow()
{
	::SetWindowPos(_hSelf, HWND_TOP, _dialogX, _dialogY, _dialogWidth, _dialogHeight, SWP_SHOWWINDOW);
}

void SwitchDialog::updateWindowPosition(void)
{
	WINDOWINFO wi;
	::GetWindowInfo(_hSelf, &wi);
	_dialogX = wi.rcWindow.left;
	_dialogY = wi.rcWindow.top;
	_dialogWidth = wi.rcWindow.right - wi.rcWindow.left;
	_dialogHeight = wi.rcWindow.bottom - wi.rcWindow.top;

}


BOOL CALLBACK SwitchDialog::run_dlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	int width, height;

	switch (Message) 
	{
        case WM_INITDIALOG :
		{
			return TRUE;
		}


			

		case WM_KEYDOWN :
		{
			switch (wParam)
			{
				case VK_DOWN:
				{
					moveSelectionDown(FALSE);
					return TRUE;
				}
				case VK_UP:
				{
					moveSelectionUp(FALSE);
					return TRUE;
				}
				case VK_NEXT:
				{
					moveSelectionBottom();
					return TRUE;
				}
				case VK_PRIOR:
				{
					moveSelectionTop();
					return TRUE;
				}
				case VK_TAB:
				{
					if (_options->emulateCtrlTab && !_overrideCtrlTab)
					{
						if (::GetKeyState(VK_SHIFT) & 0x80)
							moveSelectionUp(TRUE);
						else
							moveSelectionDown(TRUE);
					}
					else {
						::SetFocus(GetDlgItem(_hSelf, IDC_FILEEDIT));
					}
					return TRUE;
					break;
				}

				case VK_SHIFT:
					return TRUE;
					break;

				default:
					int result;
					if (_options->emulateCtrlTab && !_overrideCtrlTab)
					{
						HWND hFileEdit = ::GetDlgItem(_hSelf, IDC_FILEEDIT);
						::EnableWindow(hFileEdit, TRUE);
						::SetFocus(hFileEdit);
						BYTE keyState[256];
						
						::GetKeyboardState((PBYTE)&keyState);
						keyState[VK_CONTROL] = 0;
						WORD buffer;
						
						result = ::ToAscii(wParam, (lParam & 0x00FF0000) >> 16, (const BYTE *)&keyState, &buffer, 0);
						if (1 == result)
						{
							char temp[2];
							temp[0] = (char)buffer;
							temp[1] = '\0';
							::SetDlgItemTextA(_hSelf, IDC_FILEEDIT, temp);
							::SendDlgItemMessage(_hSelf, IDC_FILEEDIT, EM_SETSEL, 1, 1);
							::SetFocus(GetDlgItem(_hSelf, IDC_FILEEDIT));
							_overrideCtrlTab = TRUE;
						}

						


						/*
						INPUT input;
						input.type = INPUT_KEYBOARD;
						input.ki.wVk = wParam;
						input.ki.wScan = (lParam & 0x00FF0000) >> 16;
						input.ki.dwFlags = 0;
						UINT inputResult = ::SendInput(1, &input, sizeof(INPUT));
						input.type = INPUT_KEYBOARD;
						*/
						//::SendMessage(hFileEdit, Message, wParam, lParam);
						//::SendMessage(hFileEdit, WM_KEYUP, wParam, lParam);
					}
					break;
			}
			break;
		}

		case WM_KEYUP:
			if (_options->emulateCtrlTab && !_overrideCtrlTab)
			{
				if (VK_CONTROL == wParam)
				{
					switchToSelectedBuffer();
				}
			}
			break;

		case WM_SIZE:
			width = LOWORD(lParam);
			height = HIWORD(lParam);
			::MoveWindow(GetDlgItem(_hSelf, IDOK), width - _okButtonRect.left, _okButtonRect.top, _okButtonRect.right, _okButtonRect.bottom, TRUE);
			::MoveWindow(GetDlgItem(_hSelf, IDCANCEL), width - _cancelButtonRect.left, _cancelButtonRect.top, _cancelButtonRect.right, _cancelButtonRect.bottom, TRUE);			
			::MoveWindow(GetDlgItem(_hSelf, IDC_FILEEDIT), _editboxRect.left, _editboxRect.top, width - _editboxRect.right, _editboxRect.bottom, TRUE);			
			::MoveWindow(GetDlgItem(_hSelf, IDC_FILELIST), _listboxRect.left, _listboxRect.top, width - _listboxRect.right, height - _listboxRect.bottom, TRUE);			
			
			return TRUE;


		case WM_ACTIVATE:
			switch(LOWORD(wParam))
			{
				case WA_INACTIVE:
					updateWindowPosition();
					cleanup();
					display(FALSE);
					return TRUE;
				
			}
			break;


		case WM_COMMAND : 
		{
			switch(LOWORD(wParam))
			{
				case IDC_FILEEDIT:
					if (HIWORD(wParam) == EN_CHANGE)
					{
						TCHAR searchString[SEARCH_STRING_BUFFER_MAX];
						::GetDlgItemText(_hSelf, IDC_FILEEDIT, (LPTSTR)searchString, SEARCH_STRING_BUFFER_MAX);
						searchFiles(searchString);
					}
					break;


				case IDC_FILELIST:
					if (HIWORD(wParam) == LBN_DBLCLK)
						switchToSelectedBuffer();
					break;
					
				default:

			
					
					switch (wParam)
					{
						case IDOK :
							switchToSelectedBuffer();
						case IDCANCEL :
							updateWindowPosition();
							display(FALSE);
							cleanup();
							return TRUE;

						default :
							break;
					}
					break;
			}
		}
	}
	return FALSE;
}

void SwitchDialog::switchToSelectedBuffer()
{
	int lbIndex = SendDlgItemMessage(_hSelf, IDC_FILELIST, LB_GETCURSEL, 0, 0);
	if (LB_ERR != lbIndex)
	{
		int currentBufferID = ::SendMessage(_nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
		EditFile *editFile = (EditFile*)SendDlgItemMessage(_hSelf, IDC_FILELIST, LB_GETITEMDATA, lbIndex, 0);
		::SendMessage(_nppData._nppHandle, NPPM_SWITCHTOFILE, 0, (LPARAM)(TCHAR *)editFile->getFullFilename());
		
		TCHAR *searchCopy = new TCHAR[SEARCH_STRING_BUFFER_MAX];
		::GetDlgItemText(_hSelf, IDC_FILEEDIT, (LPTSTR)searchCopy, SEARCH_STRING_BUFFER_MAX);
		
		(*_typedForFile)[currentBufferID] = searchCopy;
	}
}

void SwitchDialog::setWindowPosition(int x, int y, int width, int height)
{
	_dialogX = x;
	_dialogY = y;
	_dialogWidth = width;
	_dialogHeight = height;

}


void SwitchDialog::getWindowPosition(RECT &rc)
{
	WINDOWINFO wi;
	::GetWindowInfo(_hSelf, &wi);
	
	rc.left = wi.rcWindow.left;
	rc.right = wi.rcWindow.right;
	rc.top = wi.rcWindow.top;
	rc.bottom = wi.rcWindow.bottom;
}
    
void SwitchDialog::refreshSearchFlags()
{

	if ((_options->searchFlags & SEARCHFLAG_CASESENSITIVE) == SEARCHFLAG_CASESENSITIVE)
		_caseSensitive = TRUE;
	else
		_caseSensitive = FALSE;

	if ((_options->searchFlags & SEARCHFLAG_STARTONLY) == SEARCHFLAG_STARTONLY)
		_startOnly = TRUE;
	else
		_startOnly = FALSE;

}


void SwitchDialog::searchFiles(TCHAR* search)
{
	HWND hFileList = GetDlgItem(_hSelf, IDC_FILELIST);
	int lbIndex = ::SendDlgItemMessage(_hSelf, IDC_FILELIST, LB_GETCURSEL, 0, 0);
	
	int selectedEditFileIndex = -1;

	if (LB_ERR != lbIndex)
	{
		EditFile *editFile = (EditFile*)::SendDlgItemMessage(_hSelf, IDC_FILELIST, LB_GETITEMDATA, lbIndex, 0);
		selectedEditFileIndex = editFile->getIndex();						
	}

	clearList();
	bool items = false;
	bool selected = false;
	bool madeSelection = false;

	if (!_caseSensitive)
		_tcslwr(search);

	for(int index = 0; index < _nbFiles; ++index)
	{
		TCHAR *candidateFilename = _editFiles[index]->getSearchFilename();
		
		// Perform the search
		TCHAR *searchResult = _tcsstr(candidateFilename, search);
		if ((searchResult && !_startOnly)
			||
			(searchResult == candidateFilename && _startOnly))
		{
			if (index == selectedEditFileIndex)
			{
				selected = true;
				madeSelection = true;
			}
			else
			{
				selected = false;
			}

			addListEntry(*_editFiles[index], selected);
			items = true;
		}
	}

	if (items && !madeSelection)
	{
		SendDlgItemMessage(_hSelf, IDC_FILELIST, LB_SETCURSEL, 0, (LPARAM)0);
	}
}


void SwitchDialog::moveSelectionUp(BOOL wrap)
{
	HWND hFileList = GetDlgItem(_hSelf, IDC_FILELIST);
	int currentlySelected = SendMessage(hFileList, LB_GETCURSEL, 0, 0);
	if (currentlySelected > 0)
	{
		::SendMessage(hFileList, LB_SETCURSEL, currentlySelected - 1, 0);
	} 
	else if (wrap)
	{
		int itemCount = ::SendMessage(hFileList, LB_GETCOUNT, 0, 0);
		if (itemCount > 0)
			::SendMessage(hFileList, LB_SETCURSEL, itemCount - 1, 0);
	}

}

void SwitchDialog::moveSelectionDown(BOOL wrap)
{
	HWND hFileList = ::GetDlgItem(_hSelf, IDC_FILELIST);
	int currentlySelected = ::SendMessage(hFileList, LB_GETCURSEL, 0, 0);
	int itemCount = ::SendMessage(hFileList, LB_GETCOUNT, 0, 0);

	if (currentlySelected < itemCount - 1)
	{
		::SendMessage(hFileList, LB_SETCURSEL, currentlySelected + 1, 0);
	}
	else if (wrap)
	{
		::SendMessage(hFileList, LB_SETCURSEL, 0, 0);
	}
}


void SwitchDialog::moveSelectionBottom(void)
{
	HWND hFileList = ::GetDlgItem(_hSelf, IDC_FILELIST);
	int itemCount = ::SendMessage(hFileList, LB_GETCOUNT, 0, 0);
	if (itemCount > 0)
		::SendMessage(hFileList, LB_SETCURSEL, itemCount - 1, 0);
}


void SwitchDialog::moveSelectionTop(void)
{
	HWND hFileList = ::GetDlgItem(_hSelf, IDC_FILELIST);
	int itemCount = ::SendMessage(hFileList, LB_GETCOUNT, 0, 0);
	if (itemCount > 0)
		::SendMessage(hFileList, LB_SETCURSEL, 0, 0);
}


void SwitchDialog::clearList(void)
{
	::SendDlgItemMessage(_hSelf, IDC_FILELIST, LB_RESETCONTENT, 0, 0);

}

void SwitchDialog::addListEntry(EditFile &editFile, bool selected)
{
	HWND hFileList = GetDlgItem(_hSelf, IDC_FILELIST);
	int lNewIdx = ::SendMessage(hFileList,
 							  LB_ADDSTRING,
							  0,
							  (LPARAM)(LPTSTR)(editFile.getDisplayString()));

	::SendMessage(hFileList, LB_SETITEMDATA, lNewIdx, (LPARAM)&editFile);
	
	if (selected)
		::SendMessage(hFileList, LB_SETCURSEL, lNewIdx, 0);
}

/* Cleans up the editfiles array */
void SwitchDialog::cleanup(void)
{
	
	if (_editFiles != NULL)
	{
		for(int index = 0; index < _nbFiles; ++index)
		{
			delete _editFiles[index];
		}

		delete _editFiles;
		_editFiles = NULL;
	}
}