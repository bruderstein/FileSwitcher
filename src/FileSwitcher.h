
#ifndef _FILESWITCHER_H
#define _FILESWITCHER_H


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

#define DIALOGY_DEFAULT			100
#define DIALOGWIDTH_DEFAULT		400
#define DIALOGHEIGHT_DEFAULT	300


struct options_t
{
	int searchFlags;
	BOOL emulateCtrlTab;
};

#endif
