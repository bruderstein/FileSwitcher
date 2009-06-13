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
#include "ConfigDialog.h"
#include "PluginInterface.h"
#include "FileSwitcher.h"
#include "Compare.h"

#include <windows.h>
#include <tchar.h>
#include <commctrl.h>
#include <map>

WNDPROC g_oldEditProc = 0;
WNDPROC g_oldListProc = 0;
WNDPROC g_oldListViewProc = 0;

LRESULT CALLBACK editProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	bool extended;

 	switch (message)
	{
		case WM_KEYDOWN:
		{
			SHORT ctrlState = ::GetKeyState(VK_CONTROL) & 0x80;
			SHORT shiftState = ::GetKeyState(VK_SHIFT) & 0x80;

			extended = ((lParam & 0x01000000) == 0x01000000);
			if (wParam == VK_DOWN || wParam == VK_UP || (wParam == VK_NEXT && extended) || (wParam == VK_PRIOR && extended)
				|| ((wParam == VK_HOME || wParam == VK_END) && extended && ((g_options.useHomeForEdit && ctrlState) || (!g_options.useHomeForEdit && !ctrlState && !shiftState)))
               )
			{
				::SendMessage(GetParent(hwnd), WM_KEYDOWN, wParam, 0);
				return TRUE;
			}
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
		
		
	
		case WM_KILLFOCUS:
			// Ignore the kill focus message, so the highlight bar stays blue
			return TRUE;

		case WM_GETDLGCODE :
		{
			LRESULT dlgCode = CallWindowProc(g_oldListProc, hwnd, message, wParam, lParam);
			dlgCode |= DLGC_WANTTAB;
			return dlgCode;
		}
		
		case WM_KEYUP:
			if (wParam == VK_CONTROL)
			{
				::SendMessage(GetParent(hwnd), WM_KEYUP, wParam, 0);
				return TRUE;
			}
			break;
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


TCHAR *fswtcsstr(const TCHAR *str, const TCHAR *substr)
{
	return const_cast<TCHAR *>(_tcsstr(str, substr));
	
}

TCHAR *fswtcsistr(const TCHAR *str, const TCHAR *substr)
{
	return _tcsistr(str, substr);
	
}



void SwitchDialog::doDialog(EditFileContainer &editFiles, BOOL ignoreCtrlTab, BOOL previousFile)
{
	_editFiles = editFiles;
	_nbFiles = _editFiles.size();

    if (!isCreated())
	{
        create(IDD_SWITCHDIALOG);

		// Get the handles of the controls for later 
		_hEditbox = GetDlgItem(_hSelf, IDC_FILEEDIT);
		_hListView = GetDlgItem(_hSelf, IDC_LISTVIEW);
		
		_listView.init(_options, _hInst, _hSelf, _hListView);
		_columnForView = _options->columnForView;

		g_oldEditProc = (WNDPROC)::GetWindowLong(_hEditbox, GWLP_WNDPROC);
		::SetWindowLong(_hEditbox, GWLP_WNDPROC, (LONG)editProc);

		g_oldListProc = (WNDPROC)::GetWindowLong(_hListView, GWLP_WNDPROC);
		::SetWindowLong(_hListView, GWLP_WNDPROC, (LONG)listProc);

		::GetClientRect(GetDlgItem(_hSelf, IDOK), &_okButtonRect);
		::GetClientRect(GetDlgItem(_hSelf, IDCANCEL), &_cancelButtonRect);
		::GetClientRect(_hEditbox, &_editboxRect);
		::GetClientRect(_hListView, &_listboxRect);
		
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

		::GetWindowInfo(GetDlgItem(_hSelf, IDC_OPTIONS), &controlInfo);
		_optionsButtonRect.top = dialogRect.bottom - (controlInfo.rcClient.top - dialogInfo.rcClient.top);
		_optionsButtonRect.bottom = controlInfo.rcClient.bottom - controlInfo.rcClient.top; 
		_optionsButtonRect.left = _okButtonRect.left;
		_optionsButtonRect.right = controlInfo.rcClient.right - controlInfo.rcClient.left;


		::GetWindowInfo(_hEditbox, &controlInfo);
		_editboxRect.top = controlInfo.rcClient.top - dialogInfo.rcClient.top;
		_editboxRect.left = controlInfo.rcClient.left - dialogInfo.rcClient.left;
		_editboxRect.right = dialogRect.right - _editboxRect.right;

		::GetWindowInfo(_hListView, &controlInfo);
		_listboxRect.top = controlInfo.rcClient.top - dialogInfo.rcClient.top;
		_listboxRect.left = controlInfo.rcClient.left - dialogInfo.rcClient.left;
		_listboxRect.right = dialogRect.right - _listboxRect.right;
		_listboxRect.bottom = dialogRect.bottom - _listboxRect.bottom;
		
		_haveOverriddenSortOrder = FALSE;


		// If the sort order won't be done later anyway, set the default
		if (!_options->resetSortOrder)
		{
			if (_options->defaultSortOrder == ALWAYSREMEMBER)
                _listView.sortItems(_options->activeSortOrder);
			else
				_listView.sortItems(_options->defaultSortOrder);
		}

		// If the position and dimensions have not been set (from disk-loaded settings)
		if (_dialogX == -1)
		{
			goToCenter();
			updateWindowPosition();
		}


	}
	else
	{
		if (_options->columnForView != _columnForView)
		{
			_listView.updateColumns();
			_columnForView = _options->columnForView;
		}
	}

	int currentView = getCurrentView();
	int currentIndex = getCurrentIndex(currentView);

	_listView.setCurrentView(currentView);
	setupColumnWidths(editFiles);
    
	
	if (_options->emulateCtrlTab && !ignoreCtrlTab)
	{
			_overrideCtrlTab = FALSE;
			SHORT ctrlState = ::GetKeyState(VK_CONTROL) & 0x80;
			SHORT shiftState = ::GetKeyState(VK_SHIFT) & 0x80;
			
			


			searchFiles(_T(""), currentView, currentIndex);
			
			::SetDlgItemText(_hSelf, IDC_FILEEDIT, _T(""));
			
			if (_options->overrideSortWhenTabbing)
			{
				_haveOverriddenSortOrder = TRUE;
				_listView.sortItems(INDEX);
			} else
				_listView.sortItems();

			::SetFocus(_hListView);
			
			if (shiftState || previousFile)
				moveSelectionUp(TRUE);
			else
				moveSelectionDown(TRUE);

			EnableWindow(_hEditbox, FALSE);
			
	}
	else 
	{
		if (ignoreCtrlTab)
			_overrideCtrlTab = TRUE;

		EnableWindow(_hEditbox, TRUE);

		int currentBufferID = ::SendMessage(_nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
		if (_typedForFile->find(currentBufferID) != _typedForFile->end())
		{
			::SetDlgItemText(_hSelf, IDC_FILEEDIT, (*_typedForFile)[currentBufferID]);
		}
		

		TCHAR searchString[SEARCH_STRING_BUFFER_MAX];
		

		::GetDlgItemText(_hSelf, IDC_FILEEDIT, (LPTSTR)searchString, SEARCH_STRING_BUFFER_MAX);
		searchFiles(searchString, currentView, currentIndex);

		::SendDlgItemMessage(_hSelf, IDC_FILEEDIT, EM_SETSEL, 0, _tcslen(searchString));
		
		::SetFocus(_hListView); // draw selected row dark blue (WM_KILLFOCUS will be ignored)
		::SetFocus(_hEditbox);
	
		if (_options->resetSortOrder)
		{
			if (_options->defaultSortOrder == ALWAYSREMEMBER)
				_listView.sortItems(_options->activeSortOrder);
			else
				_listView.sortItems(_options->defaultSortOrder);
		}
		else if (_options->emulateCtrlTab && _haveOverriddenSortOrder)
		{
			if (_options->defaultSortOrder == ALWAYSREMEMBER)
				_listView.sortItems(_options->activeSortOrder);
			else
				_listView.sortItems(_options->defaultSortOrder);
		}
			
	
	}

	
		
	showAndPositionWindow();

}

int SwitchDialog::getCurrentView(void)
{
	int currentView;
	::SendMessage(_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&currentView));
	return currentView;
}

int SwitchDialog::getCurrentIndex(int currentView)
{
	int currentBufferID = ::SendMessage(_nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
	EditFileContainer::iterator lb = _editFiles.lower_bound(currentBufferID);
	EditFileContainer::iterator ub = _editFiles.upper_bound(currentBufferID);

	while(lb != ub)
	{
		if (lb->second->getView() == currentView)
			return lb->second->getIndex();

		++lb;
	}

	return 0;
}

	

void SwitchDialog::showAndPositionWindow()
{
	::SetWindowPos(_hSelf, HWND_TOP, _dialogX, _dialogY, _dialogWidth, _dialogHeight, SWP_SHOWWINDOW);
}


void SwitchDialog::setupColumnWidths(EditFileContainer &editFiles)
{

	if (_options->autoSizeColumns)
	{
		int filenameColumnMaxWidth = 25;
		int pathColumnMaxWidth = 25;
		int tempWidth;

		for(EditFileContainer::iterator iter = editFiles.begin(); 
				iter != editFiles.end(); iter++)
		{
			tempWidth = ListView_GetStringWidth(_hListView, iter->second->getFilename());
			if (tempWidth > filenameColumnMaxWidth)
				filenameColumnMaxWidth = tempWidth;

			tempWidth = ListView_GetStringWidth(_hListView, iter->second->getPath());
			if (tempWidth > pathColumnMaxWidth)
				pathColumnMaxWidth = tempWidth;

		}
		
		ListView_SetColumnWidth(_hListView, 0, filenameColumnMaxWidth + COLUMN_PADDING);
		ListView_SetColumnWidth(_hListView, 1, pathColumnMaxWidth + COLUMN_PADDING);
		if (_options->autoSizeWindow)
		{
			int orderColumnWidth = ListView_GetColumnWidth(_hListView, 2);
			_dialogWidth  = filenameColumnMaxWidth + COLUMN_PADDING + pathColumnMaxWidth + COLUMN_PADDING + orderColumnWidth;
			_dialogWidth  += _okButtonRect.right + _okButtonRect.left + _listboxRect.left;
		}
			
	}
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
	int optionsButtonTop;

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
					moveSelectionDown(TRUE);
					return TRUE;
				}
				case VK_UP:
				{
					moveSelectionUp(TRUE);
					return TRUE;
				}
				case VK_NEXT:
				{
					moveSelectionPageDown();
					return TRUE;
				}
				
				case VK_PRIOR:
				{
					moveSelectionPageUp();
					return TRUE;
				}
				case VK_HOME:
				{
					moveSelectionTop();
					return TRUE;
				}
				case VK_END:
				{
					moveSelectionBottom();
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


							if (_options->revertSortWhenTabbing)
							{
								
								if (_options->resetSortOrder)
								{
									if (_options->defaultSortOrder == ALWAYSREMEMBER)
										_listView.sortItems(_options->activeSortOrder);
									else
										_listView.sortItems(_options->defaultSortOrder);
								}
								else
									_listView.sortItems(_options->activeSortOrder);
		
							}
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
					updateWindowPosition();
					display(FALSE);
					cleanup();
				}
			}
			break;

		case WM_SIZE:
			width = LOWORD(lParam);
			height = HIWORD(lParam);
			::MoveWindow(GetDlgItem(_hSelf, IDOK), width - _okButtonRect.left, _okButtonRect.top, _okButtonRect.right, _okButtonRect.bottom, TRUE);
			::MoveWindow(GetDlgItem(_hSelf, IDCANCEL), width - _cancelButtonRect.left, _cancelButtonRect.top, _cancelButtonRect.right, _cancelButtonRect.bottom, TRUE);			
			optionsButtonTop = height - _optionsButtonRect.top;
			if (optionsButtonTop < (_cancelButtonRect.top + _cancelButtonRect.bottom + 5)) 
				optionsButtonTop = (_cancelButtonRect.top + _cancelButtonRect.bottom + 5);

			::MoveWindow(GetDlgItem(_hSelf, IDC_OPTIONS), width - _optionsButtonRect.left, optionsButtonTop, _optionsButtonRect.right, _optionsButtonRect.bottom, TRUE);
			::MoveWindow(_hEditbox, _editboxRect.left, _editboxRect.top, width - _editboxRect.right, _editboxRect.bottom, TRUE);			
			::MoveWindow(_hListView, _listboxRect.left, _listboxRect.top, width - _listboxRect.right, height - _listboxRect.bottom, TRUE);			
			
			return TRUE;

#ifndef _DEBUG
		case WM_ACTIVATE:
			switch(LOWORD(wParam))
			{
				case WA_INACTIVE:
					if (!_displayingOptionsDialog)
					{
						updateWindowPosition();
						cleanup();
						display(FALSE);
						return TRUE;
					}
				
			}
			break;
#endif

		case WM_NOTIFY:
			if (((LPNMHDR)lParam)->hwndFrom == _hListView)
			{
				_listView.notify(wParam, lParam);
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
						searchFiles(searchString, -1, -1);
					}
					break;

				case IDC_OPTIONS:
					_displayingOptionsDialog = TRUE;
					_configDlg->doModal(_hSelf);
					_displayingOptionsDialog = FALSE;
					_listView.updateColumns();
					TCHAR searchString[SEARCH_STRING_BUFFER_MAX];
					::GetDlgItemText(_hSelf, IDC_FILEEDIT, (LPTSTR)searchString, SEARCH_STRING_BUFFER_MAX);
					searchFiles(searchString, -1, -1);
					SetFocus(_hEditbox);

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
		break;

		case FSM_ITEMDBLCLK:
			switchToSelectedBuffer();
			updateWindowPosition();
			display(FALSE);
			cleanup();
			break;

		case FSN_LISTVIEWSETFOCUS:
			if (!_options->emulateCtrlTab 
				|| (_options->emulateCtrlTab && _overrideCtrlTab))
			{
				SetFocus(_hEditbox);
			}
			break;

	}
	return FALSE;
}

void SwitchDialog::switchToSelectedBuffer()
{
	
	EditFile* editFile = _listView.getCurrentEditFile();

	if (NULL != editFile)
	{
		if (!_options->emulateCtrlTab || _overrideCtrlTab)
		{
			int currentBufferID = ::SendMessage(_nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
			TCHAR *searchCopy = new TCHAR[SEARCH_STRING_BUFFER_MAX];
			::GetDlgItemText(_hSelf, IDC_FILEEDIT, (LPTSTR)searchCopy, SEARCH_STRING_BUFFER_MAX);
			(*_typedForFile)[currentBufferID] = searchCopy;
		}

		::SendMessage(_nppData._nppHandle, NPPM_ACTIVATEDOC, editFile->getView(), editFile->getIndex());
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
    


void SwitchDialog::searchFiles(TCHAR* search, int selectedEditFileView, int selectedEditFileIndex)
{
	
	int lbIndex = ::SendMessage(_hListView, LVM_GETNEXTITEM, -1, LVIS_SELECTED);
	
	int currentView = getCurrentView();

	if (selectedEditFileView == -1)
	{
		EditFile *selectedEditFile = _listView.getCurrentEditFile();
		if (selectedEditFile != NULL)
		{
			selectedEditFileIndex = selectedEditFile->getIndex();
			selectedEditFileView = selectedEditFile->getView();
		}
	}

	clearList();
	bool items = false;
	bool selected = false;
	bool madeSelection = false;

	

	EditFileContainer::iterator iter = _editFiles.begin();
	bool include;


	typedef TCHAR* (*strstrFunction_t)(const TCHAR *str1, const TCHAR *str2);

	strstrFunction_t strstrFunc;

	if (_options->searchFlags & SEARCHFLAG_CASESENSITIVE)
	{
		strstrFunc = fswtcsstr;
	}
	else
	{
		strstrFunc = fswtcsistr;
	}


	
	
	
	while(iter != _editFiles.end())
	{
		include = false;

		if (_options->onlyUseCurrentView && iter->second->getView() != currentView)
		{
			++iter;
			continue;
		}

		if (iter->second->getIndex() == -1)
		{
			++iter;
			continue;
		}

		if (_options->searchFlags & SEARCHFLAG_STARTONLY)
		{

			if (_options->searchFlags & SEARCHFLAG_INCLUDEPATH)
			{
				TCHAR *searchResult = (*strstrFunc)(iter->second->getPath(), search);
				if (searchResult == iter->second->getPath())
					include = true;
			}
			if (!include && (_options->searchFlags & SEARCHFLAG_INCLUDEFILENAME))
			{
				TCHAR *searchResult = (*strstrFunc)(iter->second->getFilename(), search);
				if (searchResult == iter->second->getFilename())
					include = true;
			}
			if (!include && (_options->searchFlags & SEARCHFLAG_INCLUDEINDEX))
			{
				int typedNumber = _ttoi(search);
				if (typedNumber - 1 == iter->second->getIndex())
					include = true;

			}

			if (!include && (_options->searchFlags & SEARCHFLAG_INCLUDEVIEW))
			{
				int typedNumber = _ttoi(search);
				if (typedNumber - 1 == iter->second->getView())
					include = true;
			}

		}
		else
		{
			if ((_options->searchFlags & SEARCHFLAG_INCLUDEPATH)
				&& (_options->searchFlags & SEARCHFLAG_INCLUDEFILENAME))
			{
				if ((*strstrFunc)(iter->second->getFullFilename(), search))
					include = true;
			}
			else if (_options->searchFlags & SEARCHFLAG_INCLUDEPATH)
			{
				if ((*strstrFunc)(iter->second->getPath(), search))
					include = true;
			}
			else if (_options->searchFlags & SEARCHFLAG_INCLUDEFILENAME)
			{
				if ((*strstrFunc)(iter->second->getFilename(), search))
					include = true;
			}

			if (!include && _options->searchFlags & SEARCHFLAG_INCLUDEINDEX)
			{
				int typedNumber = _ttoi(search);
				if (typedNumber - 1 == iter->second->getIndex())
					include = true;
			}

			if (!include && (_options->searchFlags & SEARCHFLAG_INCLUDEVIEW))
			{
				int typedNumber = _ttoi(search);
				if (typedNumber - 1 == iter->second->getView())
					include = true;
			}
		}


		if (include)
		{
			if (iter->second->getIndex() == selectedEditFileIndex
				&& iter->second->getView() == selectedEditFileView)
			{
				selected = true;
				madeSelection = true;
			}
			else
			{
				selected = false;
			}
			
			addListEntry(iter->second, selected);
		
			items = true;
		}

		++iter;
	}

	_listView.sortItems();

	if (madeSelection)
	{
		int currentItem = SendMessage(_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, currentItem, 0);
	}
	else if (items && !madeSelection)
	{
		LVITEM item;
		item.stateMask = LVIS_SELECTED;
		item.state = LVIS_SELECTED;
		SendMessage(_hListView, LVM_SETITEMSTATE, 0, reinterpret_cast<LPARAM>(&item));
	}
    
	
}


void SwitchDialog::moveSelectionUp(BOOL wrap)
{
	
	int currentItem = SendMessage(_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	if (currentItem == -1)
		currentItem = 0;

	if (currentItem > 0)
	{
	
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state     = LVIS_SELECTED;
		::SendMessage(_hListView, LVM_SETITEMSTATE, currentItem - 1, reinterpret_cast<LPARAM>(&lvi));
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, currentItem - 1, 0);
	}
	else if (wrap)
	{
		int itemCount = ::SendMessage(_hListView, LVM_GETITEMCOUNT, 0, 0);
	
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state     = LVIS_SELECTED;
		::SendMessage(_hListView, LVM_SETITEMSTATE, itemCount - 1, reinterpret_cast<LPARAM>(&lvi));
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, itemCount - 1, 0);
	}

}

void SwitchDialog::moveSelectionPageUp()
{
	
	int currentItem = SendMessage(_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	if (currentItem == -1)
		currentItem = 0;

	int pageItems   = ::SendMessage(_hListView, LVM_GETCOUNTPERPAGE, 0, 0); 

	if (currentItem >= pageItems)
	{
	
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state     = LVIS_SELECTED;
		::SendMessage(_hListView, LVM_SETITEMSTATE, currentItem - pageItems, reinterpret_cast<LPARAM>(&lvi));
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, currentItem - pageItems, 0);
	}
	else
	{
		
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state     = LVIS_SELECTED;
		::SendMessage(_hListView, LVM_SETITEMSTATE, 0, reinterpret_cast<LPARAM>(&lvi));
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, 0, 0);
	}

}



void SwitchDialog::moveSelectionDown(BOOL wrap)
{
	
	int currentItem = ::SendMessage(_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	int itemCount   = ::SendMessage(_hListView, LVM_GETITEMCOUNT, 0, 0);

	if (currentItem < itemCount - 1)
	{
	
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state     = LVIS_SELECTED;
		::SendMessage(_hListView, LVM_SETITEMSTATE, currentItem + 1, reinterpret_cast<LPARAM>(&lvi));
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, currentItem + 1, 0);
	}
	else if (wrap)
	{
		
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state     = LVIS_SELECTED;
		::SendMessage(_hListView, LVM_SETITEMSTATE, 0, reinterpret_cast<LPARAM>(&lvi));
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, 0, 0);
	}


}


void SwitchDialog::moveSelectionPageDown()
{
	int currentItem = ::SendMessage(_hListView, LVM_GETNEXTITEM, -1, LVNI_SELECTED);
	int itemCount   = ::SendMessage(_hListView, LVM_GETITEMCOUNT, 0, 0);
	int pageItems   = ::SendMessage(_hListView, LVM_GETCOUNTPERPAGE, 0, 0);

	if (currentItem < itemCount - pageItems)
	{
	
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state     = LVIS_SELECTED;
		::SendMessage(_hListView, LVM_SETITEMSTATE, currentItem + pageItems, reinterpret_cast<LPARAM>(&lvi));
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, currentItem + pageItems, 0);
	}
	else 
	{
		
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state     = LVIS_SELECTED;
		::SendMessage(_hListView, LVM_SETITEMSTATE, itemCount - 1, reinterpret_cast<LPARAM>(&lvi));
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, itemCount - 1, 0);
	}
}

void SwitchDialog::moveSelectionBottom(void)
{
	int itemCount = ::SendMessage(_hListView, LVM_GETITEMCOUNT, 0, 0);
	
	if (itemCount > 0)
	{
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state     = LVIS_SELECTED;
		::SendMessage(_hListView, LVM_SETITEMSTATE, itemCount - 1, reinterpret_cast<LPARAM>(&lvi));
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, itemCount - 1, 0);
	}


}




void SwitchDialog::moveSelectionTop(void)
{
	int itemCount = ::SendMessage(_hListView, LVM_GETITEMCOUNT, 0, 0);
	
	if (itemCount > 0)
	{
		LVITEM lvi;
		lvi.stateMask = LVIS_SELECTED;
		lvi.state     = LVIS_SELECTED;
		::SendMessage(_hListView, LVM_SETITEMSTATE, 0, reinterpret_cast<LPARAM>(&lvi));
		::SendMessage(_hListView, LVM_ENSUREVISIBLE, 0, 0);
	}
}




void SwitchDialog::clearList(void)
{
	::SendDlgItemMessage(_hSelf, IDC_LISTVIEW, LVM_DELETEALLITEMS, 0, 0);
}

void SwitchDialog::addListEntry(EditFile *editFile, bool selected)
{
	HWND hListView = GetDlgItem(_hSelf, IDC_LISTVIEW);
	LVITEM item;
	item.pszText = LPSTR_TEXTCALLBACK;
	item.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE | LVIF_STATE;
	item.iSubItem = 0;
	item.iItem = 0;
	item.cchTextMax = MAX_PATH;
	item.iImage = editFile->getFileStatus();
	item.lParam = (LPARAM)editFile;
	item.stateMask = LVIS_SELECTED;
	
	if (selected)
		item.state = LVIS_SELECTED;
	else
		item.state = 0;
	
    int index = ListView_InsertItem(hListView, &item);
	
	//::SendMessage(_hListView, LVM_SETITEMSTATE, index, reinterpret_cast<LPARAM>(&item));
	
	
}


int SwitchDialog::getCurrentSortOrder()
{
	return _listView.getCurrentSortOrder();
}


void SwitchDialog::getColumnOrderString(TCHAR *buffer, int bufferLength)
{
	_listView.getColumnOrderString(buffer, bufferLength);
}

void SwitchDialog::setColumnOrder(TCHAR *columnOrder)
{
	_listView.setColumnOrder(columnOrder);
}

TCHAR* SwitchDialog::getColumnWidths(TCHAR *buffer, int bufferLength)
{
	return _listView.getColumnWidths(buffer, bufferLength);
}

/* Cleans up the editfiles array */
void SwitchDialog::cleanup(void)
{
	/*
	if (_editFiles != NULL)
	{
		for(int index = 0; index < _nbFiles; ++index)
		{
			delete _editFiles[index];
		}

		delete _editFiles;
		_editFiles = NULL;
	}
	*/
}