#pragma once
#include "precompiledHeaders.h"
#include "EditFile.h"
#include "FileSwitcher.h"
#include "Window.h"

#include "resource.h"
#include <shellapi.h>

class FileListView
{
public:
	FileListView(void);
	~FileListView(void) {}

	void init(options_t *options, HINSTANCE hInst, HWND hParent, HWND hListView);

	LRESULT notify(WPARAM wParam, LPARAM lParam);

	int getCurrentSelectedIndex(void);
	SimpleFileInfo* getCurrentSimpleFile(void);
	EditFile *getCurrentEditFile(void);

	void sortSimpleFileItems(int currentSortOrder);
	void sortSimpleFileItems();
	void sortItems();
	void sortItems(int currentSortOrder);
	int getCurrentSortOrder(void);
	void setCurrentView(int currentView);

	static int CALLBACK listViewSimpleFileComparer(void* lParam1, void* lParam2, LPARAM lParamSort);
	static int CALLBACK listViewComparer(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	TCHAR *getColumnOrderString(TCHAR *buffer, int bufferSize);
	TCHAR *getColumnWidths(TCHAR *buffer, size_t bufferSize);
    void  setColumnWidths(TCHAR *widths);
	void  setColumnOrder(TCHAR *columnOrder);

	void updateColumns(void);
	int getColumnSize(void);

private:

	HWND		_hListView;
	HWND        _hParentWindow;
	HWND        _hHeader;
	HINSTANCE	_hInstance;

	int			_currentSortOrder;
	int			_currentView;

	HBITMAP     _bmpSortUp;
	HBITMAP     _bmpSortDown;

	BOOL		_columnForViewAdded;
	int			_viewColumn;

	options_t	*_options;

	void updateHeader(void);
	void setupListViewHeader(void);
};
