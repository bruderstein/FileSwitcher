// BufferSwitcher.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "FileSwitcher.h"
#include "PluginInterface.h"
#include "SwitchDialog.h"
#include "AboutDialog.h"
#include "ConfigDialog.h"
#include <windows.h>
#include <TCHAR.H>
#include <shlwapi.h>
#include <map>

#ifdef _MANAGED
#pragma managed(push, off)
#endif



/* Info for Notepad++ */
CONST TCHAR PLUGIN_NAME[]	= _T("File Switcher");
const int	nbFunc			= 3;
FuncItem	funcItem[nbFunc];

/* Global data */
NppData				nppData;
HANDLE				g_hModule			= NULL;
HIMAGELIST			ghImgList			= NULL;
TCHAR				iniFilePath[MAX_PATH];
winVer				gWinVersion			= WV_UNKNOWN;
struct options_t	options;


EditFileContainer   editFiles;

std::map<int, TCHAR *> typedForFile;


/* Dialogs */
SwitchDialog	switchDlg;
AboutDialog		aboutDlg;
ConfigDialog	configDlg;







void showSwitchDialog();
void doAbout();
void doConfig();
void loadSettings();
void saveSettings();
void addEditFile(int bufferID);
void removeEditFile(int bufferID);
void updateCurrentStatus(FileStatus status);
void clearEditFiles();


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	g_hModule = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		  funcItem[0]._pFunc = showSwitchDialog;
		  funcItem[1]._pFunc = doConfig;
		  funcItem[2]._pFunc = doAbout;
		  _tcscpy(funcItem[0]._itemName, _T("Show File Switcher"));
		  _tcscpy(funcItem[1]._itemName, _T("Options"));
		  _tcscpy(funcItem[2]._itemName, _T("About"));

		  funcItem[0]._init2Check = false;
		  funcItem[1]._init2Check = false;
		  funcItem[2]._init2Check = false;
		  funcItem[0]._pShKey = new ShortcutKey;
		  funcItem[0]._pShKey->_isAlt = false;
		  funcItem[0]._pShKey->_isCtrl = true;
		  funcItem[0]._pShKey->_isShift = true;
		  funcItem[0]._pShKey->_key = 0x4F; //VK_O
		  funcItem[1]._pShKey = NULL;
		  funcItem[2]._pShKey = NULL;
		  
		  /* create image list with icons */
		  ghImgList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 6, 30);
		  ImageList_AddIcon(ghImgList, ::LoadIcon((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDI_SAVED)));
		  ImageList_AddIcon(ghImgList, ::LoadIcon((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDI_UNSAVED)));
		  ImageList_AddIcon(ghImgList, ::LoadIcon((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDI_READONLY)));


	      break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		delete funcItem[0]._pShKey;
		break;
	}
    return TRUE;
}

extern "C" __declspec(dllexport) void setInfo(NppData notepadPlusData)
{
	nppData = notepadPlusData;
	switchDlg.init((HINSTANCE)g_hModule, nppData, &options, &typedForFile);
	aboutDlg.init((HINSTANCE)g_hModule, nppData);
	configDlg.init((HINSTANCE)g_hModule, nppData, &options);

	gWinVersion  = (winVer)::SendMessage(nppData._nppHandle, NPPM_GETWINDOWSVERSION, 0, 0);
	loadSettings();

}

extern "C" __declspec(dllexport) CONST TCHAR * getName()
{
	return PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}

HWND getCurrentHScintilla(int which)
{
	return (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;
};

extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	switch(notifyCode->nmhdr.code)
	{
		case NPPN_FILECLOSED:
			if (typedForFile.find(notifyCode->nmhdr.idFrom) != typedForFile.end())
			{
				delete[] typedForFile[notifyCode->nmhdr.idFrom];
				typedForFile.erase(notifyCode->nmhdr.idFrom);
			}
			removeEditFile(notifyCode->nmhdr.idFrom);

			break;

		case NPPN_SHORTCUTREMAPPED:
			if (notifyCode->nmhdr.idFrom == funcItem[0]._cmdID)
			{
				ShortcutKey *sKey = (ShortcutKey*)(notifyCode->nmhdr.hwndFrom);
				if (sKey->_key == VK_TAB 
					&& sKey->_isCtrl 
					&& !sKey->_isAlt 
					&& !sKey->_isShift)
				{
					options.emulateCtrlTab = TRUE;
				}
			}
			break;

		case NPPN_SHUTDOWN:
			saveSettings();
			clearEditFiles();
			break;


		case NPPN_FILEOPENED:
			addEditFile(notifyCode->nmhdr.idFrom);
			break;
	}

	if (notifyCode->nmhdr.hwndFrom == nppData._scintillaMainHandle 
		|| notifyCode->nmhdr.hwndFrom == nppData._scintillaSecondHandle)
	{
		switch (notifyCode->nmhdr.code)
		{

			case SCN_SAVEPOINTLEFT:
				updateCurrentStatus(UNSAVED);
			break;

			case SCN_SAVEPOINTREACHED:
				updateCurrentStatus(SAVED);
			break;
		}
	}

}

extern "C" __declspec(dllexport) LRESULT messageProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}


#ifdef _UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
	return TRUE;
}
#endif


void showSwitchDialog()
{
	
	int nbFile = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, 0, 0);

	TCHAR **fileNames = (TCHAR **)new TCHAR*[nbFile];
	for (int i = 0 ; i < nbFile ; i++)
	{
		fileNames[i] = new TCHAR[MAX_PATH];
	}

	if (::SendMessage(nppData._nppHandle, NPPM_GETOPENFILENAMES, reinterpret_cast<WPARAM>(fileNames), static_cast<LPARAM>(nbFile)))
	{
		
		//switchDlg.setFilenames(fileNames, nbFile - 1);
		
		int bufferID;
		for(int position = 0; position < nbFile; position++)
		{
			bufferID = ::SendMessage(nppData._nppHandle, NPPM_GETBUFFERIDFROMPOS, position, 0);
			if (editFiles.find(bufferID) != editFiles.end())
				editFiles[bufferID]->setIndex(position);
		}

		switchDlg.doDialog(editFiles);
		
	}
	
	
	
	/*

	for (int i = 0 ; i < nbFile ; i++)
	{
		delete [] fileNames[i];
	}
	delete [] fileNames;
	*/
	

}


void addEditFile(int bufferID)
{
	TCHAR filePath[MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, bufferID, reinterpret_cast<LPARAM>(filePath));
	int index = ::SendMessage(nppData._nppHandle, NPPM_GETPOSFROMBUFFERID, bufferID, reinterpret_cast<LPARAM>(filePath));
	editFiles[bufferID] = new EditFile(index, filePath, 0, bufferID);

}

void removeEditFile(int bufferID)
{
	editFiles.erase(bufferID);
}

void updateCurrentStatus(FileStatus status)
{
	int bufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
	editFiles[bufferID]->setFileStatus(status);
}

void clearEditFiles()
{
}

void doAbout()
{
	aboutDlg.doDialog();
}

void doConfig()
{
	configDlg.doDialog();
	
}


void loadSettings(void)
{
	TCHAR configPath[MAX_PATH];
	/* initialize the config directory */
	::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, MAX_PATH, (LPARAM)configPath);

	/* Test if config path exist */
	if (PathFileExists(configPath) == FALSE) {
		::CreateDirectory(configPath, NULL);
	}

	
	
	_tcscpy(iniFilePath, configPath);
	_tcscat(iniFilePath, FILESWITCHER_INI);
	if (PathFileExists(iniFilePath) == FALSE)
	{
		::CloseHandle(::CreateFile(iniFilePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL));
	}
	
	int dialogX, dialogY, dialogWidth, dialogHeight;

	dialogX = ::GetPrivateProfileInt(WINDOW_SETTINGS, KEY_WINDOWX, -1, iniFilePath);
	dialogY = ::GetPrivateProfileInt(WINDOW_SETTINGS, KEY_WINDOWY, DIALOGY_DEFAULT, iniFilePath);
	dialogWidth = ::GetPrivateProfileInt(WINDOW_SETTINGS, KEY_WINDOWWIDTH, DIALOGWIDTH_DEFAULT, iniFilePath);
	dialogHeight = ::GetPrivateProfileInt(WINDOW_SETTINGS, KEY_WINDOWHEIGHT, DIALOGHEIGHT_DEFAULT, iniFilePath);
	
	if (dialogX != -1)
	{
		switchDlg.setWindowPosition(dialogX, dialogY, dialogWidth, dialogHeight);
	}

	options.searchFlags = ::GetPrivateProfileInt(SEARCH_SETTINGS, KEY_SEARCHFLAGS, SEARCHFLAG_INCLUDEFILENAME, iniFilePath);
	if (options.searchFlags == 0)
		options.searchFlags = SEARCHFLAG_INCLUDEFILENAME;

	options.emulateCtrlTab = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_EMULATECTRLTAB, 0, iniFilePath);

	options.resetSortOrder = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_RESETSORT, 0, iniFilePath);
	options.defaultSortOrder = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_DEFAULTSORT, 0, iniFilePath);
	options.activeSortOrder = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_ACTIVESORT, 0, iniFilePath);
	
}


void saveSettings(void)
{
	TCHAR temp[16];
	
	if (switchDlg.isCreated())
	{
		RECT rc;
		TCHAR temp[16];
		switchDlg.getWindowPosition(rc);

		_itot(rc.left, temp, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWX, temp, iniFilePath);

		_itot(rc.top, temp, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWY, temp, iniFilePath);

		_itot(rc.right - rc.left, temp, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWWIDTH, temp, iniFilePath);

		_itot(rc.bottom - rc.top, temp, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWHEIGHT, temp, iniFilePath);


		_itot(switchDlg.getCurrentSortOrder(), temp, 10);
		::WritePrivateProfileString(SORT_SETTINGS, KEY_ACTIVESORT, temp, iniFilePath);

	}

	_itot(options.searchFlags, temp, 10);
	::WritePrivateProfileString(SEARCH_SETTINGS, KEY_SEARCHFLAGS, temp, iniFilePath);

	_itot(options.emulateCtrlTab, temp, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_EMULATECTRLTAB, temp, iniFilePath);

	_itot(options.defaultSortOrder, temp, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_DEFAULTSORT, temp, iniFilePath);

	_itot(options.resetSortOrder, temp, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_RESETSORT, temp, iniFilePath);

	
}


#ifdef _MANAGED
#pragma managed(pop)
#endif
