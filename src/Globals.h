#ifndef _GLOBALS_H
#define _GLOBALS_H

#include "precompiledHeaders.h"
#include "../common/Notepad_plus_msgs.h"
#include "../common/PluginInterface.h"
#include "SearchOptions.h"
#include <map>

/***********/
/* Defines */
/***********/
#pragma region Defines
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
#define KEY_SHOWDIALOGFORCTRLTAB  _T("showDialogForCtrlTab")
#define KEY_USEHOMEFOREDIT      _T("useHomeForEdit")
#define SORT_SETTINGS			_T("Sort")
#define KEY_DEFAULTSORT			_T("default")
#define KEY_ACTIVESORT			_T("active")
#define KEY_REVERSEDSORT		_T("reversedSort")
#define KEY_RESETSORT			_T("reset")
#define KEY_OVERRIDESORT		_T("overrideSort")
#define KEY_REVERTSORT			_T("revertSort")
#define LISTVIEW_SETTINGS		_T("ListView")
#define KEY_COLUMNORDER		    _T("columnOrder")
#define KEY_COLUMNWIDTHS		_T("columnWidths")
#define KEY_COLUMNWIDTHSSIMPLE	_T("columnWidthsSimple")
#define KEY_CONFIGURESEARCHCONTEXT _T("configureSearchContext")
#define KEY_SEARCHCONTEXTPATH	_T("searchContextPath")
#define KEY_DISABLEDSELECTEDSORTORDER _T("disabledSelectedSortOrder")
#define KEY_MAXTRAVERSEFILE _T("maxTraverseFile")
#define KEY_MAXDISPLAYFILE _T("maxDisplayFile")

#define DIALOGY_DEFAULT			100
#define DIALOGWIDTH_DEFAULT		400
#define DIALOGHEIGHT_DEFAULT	300

#define FSM_START			(WM_APP + 1600)
#define FSM_ITEMDBLCLK		(FSM_START + 1)

#define FSN_START           (FSM_START + 100)
#define FSN_LISTVIEWSETFOCUS (FSN_START + 1)

#define INDEX(x)           (x & 0x3FFFFFFF)
#define VIEW(x)			   ((x & 0xC0000000) >> 30)

#define COLUMNORDER_LENGTH     5
#define COLUMNWIDTH_LENGTH     50
#pragma endregion Global definitions

/********/
/* Vars */
/********/
const int MAX_TOOLTIP_LENGTH = 3000;

/* Flag to indicate that Ctrl-Tab functionality has stopped, as a key has been pressed */
extern BOOL _overrideCtrlTab;
extern BOOL _ctrlTabSearch;

/***********************/
/* Structs && typedefs */
/***********************/
#pragma region Structs && Typedefs

typedef std::basic_string<TCHAR>	tstring;

struct SimpleFileInfo
{
	tstring fileName;
	tstring filePath;

	SimpleFileInfo(tstring fileName_, tstring filePath_)
	{
		fileName = fileName_;
		filePath = filePath_;
	}
};

struct options_t
{
	int searchFlags;
	BOOL emulateCtrlTab;
	int defaultSortOrder;
	BOOL activeReversedSortOrder;
	BOOL reversedSortOrder;
	BOOL resetSortOrder;
	SortOrder disabledSelectedSortOrder;
	int activeSortOrder;
	BOOL overrideSortWhenTabbing;
	BOOL revertSortWhenTabbing;
	BOOL onlyUseCurrentView;
	BOOL autoSizeColumns;
	BOOL autoSizeWindow;
	BOOL columnForView;
	TCHAR columnOrder[COLUMNORDER_LENGTH];
	TCHAR columnWidths[COLUMNWIDTH_LENGTH];
	BOOL showDialogForCtrlTab;
	BOOL useHomeForEdit;
	BOOL hasConfiguredContext;
	tstring configuredContextPath;
	int maxTraverseFiles;
	int maxDisplayFiles;

	BOOL ConfiguredContextPathIsEnabledAndValid(){
		return hasConfiguredContext && PathFileExists(configuredContextPath.c_str());
	}
	BOOL ConfiguredContextPathIsValid(){
		return PathFileExists(configuredContextPath.c_str());
	}
};

class EditFile;

typedef std::multimap<int, EditFile*>			EditFileContainer;
typedef std::multimap<tstring, EditFile*>		FilenameContainer;
typedef std::multimap<tstring, SimpleFileInfo*> SimpleFileContainer;

#pragma endregion Global Structs

/*****************/
/* External Fxns */
/*****************/
extern void g_filesNeedToBeReloaded(bool val);
extern bool g_filesNeedToBeReloaded();
extern HWND CreateToolTip(int toolID, HWND hDlg, PTSTR pszText);
extern bool is_number(const tstring &s);

/*****************/
/* External Vars */
/*****************/
extern winVer gWinVersion;
extern HIMAGELIST ghImgList;
extern TCHAR iniFilePath[MAX_PATH];
extern options_t g_options;
extern SimpleFileContainer simpleFiles;
extern NppData nppData;

#endif