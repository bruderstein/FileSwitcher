
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

#define GENERAL_SETTINGS    _T("General")
#define KEY_EMULATECTRLTAB  _T("emulateCtrlTab")

#define SORT_SETTINGS       _T("Sort")
#define KEY_DEFAULTSORT     _T("default")
#define KEY_ACTIVESORT      _T("active")
#define KEY_RESETSORT       _T("reset")

#define DIALOGY_DEFAULT			100
#define DIALOGWIDTH_DEFAULT		400
#define DIALOGHEIGHT_DEFAULT	300

#define FSM_START			(WM_APP + 1600)
#define FSM_ITEMDBLCLK		(FSM_START + 1)

typedef std::map<int, EditFile*>	EditFileContainer;


typedef  std::basic_string<TCHAR>	tstring;


struct options_t
{
	int searchFlags;
	BOOL emulateCtrlTab;
	int defaultSortOrder;
	int activeSortOrder;
	BOOL resetSortOrder;
};



#endif
