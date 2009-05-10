/*
This file is part of Plugin Template for Notepad++
Copyright (C)2006 Jens Lorenz <jens.plugin.npp@gmx.de>

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
#ifndef SWITCH_DIALOG_H
#define SWITCH_DIALOG_H
#include <windows.h>
#include <tchar.h>
#include <map>
#include "StaticDialog.h"
#include "EditFile.h"
#include "resource.h" 
#include "FileSwitcher.h"
#include "FileListView.h"
#include "ConfigDialog.h"

class SwitchDialog : public StaticDialog
{

public:
	
	SwitchDialog() : StaticDialog() {};
    
	

	void init(HINSTANCE hInst, NppData nppData, struct options_t *options, std::map<int, TCHAR *> *typedForFile, ConfigDialog *configDlg)
	{
		_nppData = nppData;
		_configDlg = configDlg;
		Window::init(hInst, nppData._nppHandle);

		// Flag that the dimensions and position have not been set
		_dialogX = -1;
		
		_typedForFile = typedForFile;

		_options = options;
		_displayingOptionsDialog = FALSE;

	};

   	void doDialog(EditFileContainer &editFiles, BOOL ignoreCtrlTab);
	void setWindowPosition(int x, int y, int width, int height);
	void getWindowPosition(RECT &rc);
	int  getCurrentSortOrder(void);
	void getColumnOrderString(TCHAR *buffer, int bufferLength);
	void setColumnOrder(TCHAR *columnOrder);
	TCHAR *getColumnWidths(TCHAR *buffer, int bufferLength);
	//void setColumnWidths(TCHAR *buffer);

    virtual void destroy() {};
	
protected :
	virtual BOOL CALLBACK run_dlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	virtual void showAndPositionWindow(void);

private:
	/* Handles */
	NppData			_nppData;
    HWND			_hSource;
	HWND			_hEditbox;
	HWND			_hListView;

	/* ConfigDialog for opening config dialog from switch dialog */
	ConfigDialog	*_configDlg;

	FileListView    _listView;

	/* File/buffer info */
	EditFileContainer _editFiles;
	int _nbFiles;

	/* Rectangles for current size / position */
	RECT _okButtonRect;
	RECT _cancelButtonRect;
	RECT _optionsButtonRect;

	RECT _listboxRect;
	RECT _editboxRect;

	/* Current size and position */
	int _dialogWidth, _dialogHeight;
	int _dialogX, _dialogY;

	/* Flag to indicate that Ctrl-Tab functionality has stopped, as a key has been pressed */
	BOOL _overrideCtrlTab;


	/* Search flags */
	BOOL _startOnly;
	BOOL _caseSensitive;

	/* Options structure */
	struct options_t *_options;
	BOOL _columnForView;

	/* If Ctrl-Tab has been activated, and the sort order has been overridden to Index*/
	BOOL _haveOverriddenSortOrder;

	/* If we're displaying the options dialog from the switch dialog, hence don't hide the switch dialog */
	BOOL _displayingOptionsDialog;

	/* Record of what was typed in for each file */
	std::map<int, TCHAR *> *_typedForFile;

	/* Link between BufferID and file index */
	std::map<int, int> _bufferToIndex;



	/* Private methods */
    void cleanup(void);
	void searchFiles(TCHAR* search, int selectedEditFileView, int selectedEditFileIndex);
	void addListEntry(EditFile *editFile, bool selected);
	void clearList(void);
	void moveSelectionUp(BOOL wrap);
	void moveSelectionDown(BOOL wrap);
	void moveSelectionTop(void);
	void moveSelectionBottom(void);
	void moveSelectionPageDown(void);
	void moveSelectionPageUp(void);
	void updateWindowPosition(void);
	void switchToSelectedBuffer(void);
	void setupColumnWidths(EditFileContainer &editFiles);
	
	int getCurrentIndex(int currentView);
	int getCurrentView(void);

	/* Constants */
	static const int SEARCH_STRING_BUFFER_MAX = 255;
	static const int COLUMN_PADDING = 30;
	
	
	

	
    
		


	};



#endif // SWITCH_DIALOG_H
