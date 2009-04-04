// BufferSwitcher.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "FileSwitcher.h"
#include "PluginInterface.h"
#include "SwitchDialog.h"
#include "AboutDialog.h"
#include "ConfigDialog.h"
#include "windows.h"
#include <tchar.h>
#include <shlwapi.h>

#include <map>

#ifdef _MANAGED
#pragma managed(push, off)
#endif

CONST TCHAR PLUGIN_NAME[] = _T("File Switcher");

const int nbFunc = 3;
NppData nppData;

SwitchDialog switchDlg;
AboutDialog aboutDlg;
ConfigDialog configDlg;

std::map<int, TCHAR *> typedForFile;

FuncItem funcItem[nbFunc];
HANDLE g_hModule  = NULL;
TCHAR iniFilePath[MAX_PATH];
struct options_t options;


void showSwitchDialog();
void doAbout();
void doConfig();
void loadSettings();
void saveSettings();


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

			break;

		case NPPN_SHUTDOWN:
			saveSettings();
			break;
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

	if (::SendMessage(nppData._nppHandle, NPPM_GETOPENFILENAMES, (WPARAM)fileNames, (LPARAM)nbFile))
	{
		switchDlg.setFilenames(fileNames, nbFile - 1);
		int docIndex = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDOCINDEX, 0, 0);

		switchDlg.doDialog();
	}

	

	for (int i = 0 ; i < nbFile ; i++)
	{
		delete [] fileNames[i];
	}
	delete [] fileNames;
	
	

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

	options.searchFlags = ::GetPrivateProfileInt(SEARCH_SETTINGS, KEY_SEARCHFLAGS, 0, iniFilePath);

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
	}

	_itot(options.searchFlags, temp, 10);
	::WritePrivateProfileString(SEARCH_SETTINGS, KEY_SEARCHFLAGS, temp, iniFilePath);

}


#ifdef _MANAGED
#pragma managed(pop)
#endif
