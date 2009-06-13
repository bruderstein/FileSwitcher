
#ifndef _FILESWITCHER_H
#define _FILESWITCHER_H


#include "EditFile.h"
#include <Notepad_plus_msgs.h>
#include <commctrl.h>
#include <map>
#include <tchar.h>
#include <string>

extern winVer gWinVersion;
extern HIMAGELIST ghImgList;

#define FILESWITCHER_INI	_T("\\Fileswitcher.ini")

#define WINDOW_SETTINGS		_T("WindowSettings")
#define KEY_WINDOWX			_T("windowX")
#define KEY_WINDOWY			_T("windowY")
#define KEY_WINDOWHEIGHT	_T("windowHeight")
#define KEY_WINDOWWIDTH		_T("windowWidth")

#define SEARCH_SETTINGS		_T("SearchSettings")
#define KEY_SEARCHFLAGS		_T("searchFlags")

#define GENERAL_SETTINGS		_T("General")
#define KEY_EMULATECTRLTAB		_T("emulateCtrlTab")
#define KEY_ONLYUSECURRENTVIEW  _T("onlyUseCurrentView")
#define KEY_AUTOSIZECOLUMNS		_T("autoSizeColumns")
#define KEY_AUTOSIZEWINDOW		_T("autoSizeWindow")
#define KEY_COLUMNFORVIEW       _T("columnForView")
#define KEY_NODIALOGFORCTRLTAB  _T("noDialogForCtrlTab")
#define KEY_USEHOMEFOREDIT      _T("useHomeForEdit")
#define SORT_SETTINGS			_T("Sort")
#define KEY_DEFAULTSORT			_T("default")
#define KEY_ACTIVESORT			_T("active")
#define KEY_RESETSORT			_T("reset")
#define KEY_OVERRIDESORT		_T("overrideSort")
#define KEY_REVERTSORT			_T("revertSort")
#define LISTVIEW_SETTINGS		_T("ListView")
#define KEY_COLUMNORDER		    _T("columnOrder")
#define KEY_COLUMNWIDTHS		_T("columnWidths")

#define DIALOGY_DEFAULT			100
#define DIALOGWIDTH_DEFAULT		400
#define DIALOGHEIGHT_DEFAULT	300

#define FSM_START			(WM_APP + 1600)
#define FSM_ITEMDBLCLK		(FSM_START + 1)

#define FSN_START           (FSM_START + 100)
#define FSN_LISTVIEWSETFOCUS (FSN_START + 1)


#define INDEX(x)           (x & 0x3FFFFFFF)
#define VIEW(x)			   ((x & 0xC0000000) >> 30)


typedef  std::basic_string<TCHAR>	tstring;

typedef std::multimap<int, EditFile*>			EditFileContainer;
typedef std::multimap<tstring, EditFile*>		FilenameContainer;


#define COLUMNORDER_LENGTH     5
#define COLUMNWIDTH_LENGTH     50


struct options_t
{
	int searchFlags;
	BOOL emulateCtrlTab;
	int defaultSortOrder;
	int activeSortOrder;
	BOOL resetSortOrder;
	BOOL overrideSortWhenTabbing;
	BOOL revertSortWhenTabbing;
	BOOL onlyUseCurrentView;
	BOOL autoSizeColumns;
	BOOL autoSizeWindow;
	BOOL columnForView;
	TCHAR columnOrder[COLUMNORDER_LENGTH];
	TCHAR columnWidths[COLUMNWIDTH_LENGTH];
	BOOL noDialogForCtrlTab;
	BOOL useHomeForEdit;
};

extern options_t g_options;

#endif
