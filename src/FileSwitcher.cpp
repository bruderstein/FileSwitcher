// BufferSwitcher.cpp : Defines the entry point for the DLL application.
//

#include <precompiledHeaders.h>
#include "FileSwitcher.h"
#include "PluginInterface.h"
#include "SwitchDialog.h"
#include "AboutDialog.h"
#include "ConfigDialog.h"
#include <windows.h>

#ifdef _MANAGED
#pragma managed(push, off)
#endif

using namespace std;

/* Info for Notepad++ */
CONST TCHAR PLUGIN_NAME[]	= _T("File Switcher");
const int	nbFunc			= 8;
FuncItem	funcItem[nbFunc];

/* Global data */
NppData				nppData;
HANDLE				g_hModule			= NULL;
HIMAGELIST			ghImgList			= NULL;
TCHAR				iniFilePath[MAX_PATH];
winVer				gWinVersion			= WV_UNKNOWN;
struct options_t	g_options;
BOOL				_nppReady;
int					_previousBufferID;
int					_previousView;
int					_currentBufferID;
int					_currentView;

/* Maps for lookups */
EditFileContainer		editFiles;
FilenameContainer		filenameMap;
map<int, TCHAR *>		typedForFile;

/* Dialogs */
SwitchDialog	switchDlg;
AboutDialog		aboutDlg;
ConfigDialog	configDlg;

void showSwitchDialog(BOOL ignoreCtrlTab, BOOL previousFile);
void showSwitchDialogNormal();
void showSwitchDialogNext();
void showSwitchDialogPrevious();

void doAbout();
void doConfig();
void loadSettings();
void saveSettings();
EditFile* addEditFile(int bufferID);
EditFile* addEditFile(TCHAR *filename, int view, int index, int bufferID, void* scintillaDoc);
void removeEditFile(int bufferID);
void updateCurrentStatus(FileStatus status);
void updateBufferStatus(int bufferID, FileStatus status);
void updateCurrentBuffer();
void updateCurrentScintillaDoc(int bufferID);
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
		  funcItem[0]._pFunc = showSwitchDialogNormal;
		  funcItem[1]._pFunc = NULL;
		  funcItem[2]._pFunc = showSwitchDialogPrevious;
		  funcItem[3]._pFunc = showSwitchDialogNext;
		  funcItem[4]._pFunc = NULL;
		  funcItem[5]._pFunc = doConfig;
		  funcItem[6]._pFunc = NULL;
		  funcItem[7]._pFunc = doAbout;

		  _tcscpy_s(funcItem[0]._itemName, 64, _T("Show File Switcher"));
		  _tcscpy_s(funcItem[1]._itemName, 64, _T("-----------"));
		  _tcscpy_s(funcItem[2]._itemName, 64, _T("Switch to previous document"));
		  _tcscpy_s(funcItem[3]._itemName, 64, _T("Switch to next document"));
		  _tcscpy_s(funcItem[4]._itemName, 64, _T("-----------"));
		  _tcscpy_s(funcItem[5]._itemName, 64, _T("Options"));
		  _tcscpy_s(funcItem[6]._itemName, 64, _T("-----------"));
		  _tcscpy_s(funcItem[7]._itemName, 64, _T("About"));
		  funcItem[0]._init2Check = false;
		  funcItem[1]._init2Check = false;
		  funcItem[2]._init2Check = false;
		  funcItem[3]._init2Check = false;
		  funcItem[4]._init2Check = false;
		  funcItem[5]._init2Check = false;
		  funcItem[6]._init2Check = false;
		  funcItem[7]._init2Check = false;
		  funcItem[0]._pShKey = new ShortcutKey;
		  funcItem[0]._pShKey->_isAlt = false;
		  funcItem[0]._pShKey->_isCtrl = true;
		  funcItem[0]._pShKey->_isShift = true;
		  funcItem[0]._pShKey->_key = 0x4F; //VK_O
		  funcItem[1]._pShKey = NULL;
		  funcItem[2]._pShKey = NULL;
		  funcItem[3]._pShKey = NULL;
		  funcItem[4]._pShKey = NULL;
		  funcItem[5]._pShKey = NULL;
		  funcItem[6]._pShKey = NULL;
		  funcItem[7]._pShKey = NULL;

		  /* create image list with icons */
		  ghImgList = ImageList_Create(16, 16, ILC_COLOR32 | ILC_MASK, 6, 30);
		  ImageList_AddIcon(ghImgList, ::LoadIcon((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDI_SAVED)));
		  ImageList_AddIcon(ghImgList, ::LoadIcon((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDI_UNSAVED)));
		  ImageList_AddIcon(ghImgList, ::LoadIcon((HINSTANCE)g_hModule, MAKEINTRESOURCE(IDI_READONLY)));

		  _nppReady = FALSE;
		  _previousBufferID = 0;
		  _previousView = 0;
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
	switchDlg.init((HINSTANCE)g_hModule, nppData, &g_options, &typedForFile, &configDlg);
	aboutDlg.init((HINSTANCE)g_hModule, nppData);
	configDlg.init((HINSTANCE)g_hModule, nppData, &g_options);

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
		case NPPN_READY:
			ShortcutKey shKey;

			if (::SendMessage(nppData._nppHandle, NPPM_GETSHORTCUTBYCMDID, funcItem[3]._cmdID, reinterpret_cast<LPARAM>(&shKey)))
			{
				if (shKey._key == 0x09  // TAB
					&& shKey._isCtrl
					&& !(shKey._isAlt)
					&& !(shKey._isShift))
				{
					g_options.emulateCtrlTab = true;
				}
			}

			::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&_previousView));
			_nppReady = TRUE;
			break;

		case NPPN_FILECLOSED:
			if (typedForFile.find(notifyCode->nmhdr.idFrom) != typedForFile.end())
			{
				delete[] typedForFile[notifyCode->nmhdr.idFrom];
				typedForFile.erase(notifyCode->nmhdr.idFrom);
			}
			removeEditFile(notifyCode->nmhdr.idFrom);

			break;

		case NPPN_SHORTCUTREMAPPED:
			if (notifyCode->nmhdr.idFrom == funcItem[3]._cmdID)
			{
				ShortcutKey *sKey = (ShortcutKey*)(notifyCode->nmhdr.hwndFrom);
				if (sKey->_key == VK_TAB
					&& sKey->_isCtrl
					&& !sKey->_isAlt
					&& !sKey->_isShift)
				{
					g_options.emulateCtrlTab = TRUE;
				}
				else
				{
					g_options.emulateCtrlTab = FALSE;
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

		case NPPN_BUFFERACTIVATED:
			_currentBufferID = notifyCode->nmhdr.idFrom;
			::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&_currentView));
			updateCurrentBuffer();
			break;

		case NPPN_READONLYCHANGED:
		{
			FileStatus newStatus;
			if (notifyCode->nmhdr.idFrom & 1)
			{
				newStatus = READONLY;
			}
			else
			{
				if (notifyCode->nmhdr.idFrom & 2)
					newStatus = UNSAVED;
				else
					newStatus = SAVED;
			}
			updateBufferStatus(reinterpret_cast<int>(notifyCode->nmhdr.hwndFrom), newStatus);

			break;
		}

		case SCN_SAVEPOINTLEFT:
			if (notifyCode->nmhdr.hwndFrom == nppData._scintillaMainHandle
				|| notifyCode->nmhdr.hwndFrom == nppData._scintillaSecondHandle)
			{
				updateCurrentStatus(UNSAVED);
			}
			break;

		case SCN_SAVEPOINTREACHED:
			if (notifyCode->nmhdr.hwndFrom == nppData._scintillaMainHandle
				|| notifyCode->nmhdr.hwndFrom == nppData._scintillaSecondHandle)
			{
				updateCurrentStatus(SAVED);
			}
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

void showSwitchDialog(BOOL ignoreCtrlTab, BOOL previousFile)
{
	int nbFile[2];
	nbFile[0] = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, 0, PRIMARY_VIEW);
	nbFile[1] = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, 0, SECOND_VIEW);

	int selectedTab[2];
	selectedTab[0] = (INT)::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDOCINDEX, 0, MAIN_VIEW);
	selectedTab[1] = (INT)::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDOCINDEX, 0, SUB_VIEW);

	// Clear the view/index arrays in each EditFile
	for(EditFileContainer::iterator iter = editFiles.begin(); iter != editFiles.end(); iter++)
	{
		iter->second->clearIndexes();
	}

	int currentView;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&currentView));
	updateCurrentBuffer();

	for(int view = 0; view < 2; view++)
	{
		if (nbFile[view] && selectedTab[view] >= 0)
		{
			TCHAR **fileNames = (TCHAR **)new TCHAR*[nbFile[view]];
			for (int i = 0 ; i < nbFile[view] ; i++)
			{
				fileNames[i] = new TCHAR[MAX_PATH];
			}

			if (::SendMessage(nppData._nppHandle, view ? NPPM_GETOPENFILENAMESSECOND : NPPM_GETOPENFILENAMESPRIMARY,
							reinterpret_cast<WPARAM>(fileNames), static_cast<LPARAM>(nbFile[view])))
			{
				int bufferID;
				tstring filenameString;
				for(int position = 0; position < nbFile[view]; position++)
				{
					filenameString = fileNames[position];
					FilenameContainer::iterator filenameIterator = filenameMap.lower_bound(filenameString);
					FilenameContainer::iterator upperBoundIterator = filenameMap.upper_bound(filenameString);

					void* scintillaDoc = NULL;
					while(filenameIterator != upperBoundIterator
						  && filenameIterator->second->getView() >= 0)
					{
						scintillaDoc = filenameIterator->second->getScintillaDoc();
						++filenameIterator;
					}

					if (filenameIterator == filenameMap.end()
						|| filenameIterator->first != filenameString)
					{
						bufferID = ::SendMessage(nppData._nppHandle, NPPM_GETBUFFERIDFROMPOS, position, view);

						addEditFile(fileNames[position], view, position, bufferID, scintillaDoc);
					}
					else
					{
						filenameIterator->second->setIndex(view, position);
					}
				}
			}
		}
	}

	switchDlg.doDialog(editFiles, ignoreCtrlTab, previousFile);
}

void showSwitchDialogNormal()
{
		showSwitchDialog(TRUE, FALSE);
}

void showSwitchDialogNext()
{
	if (g_options.emulateCtrlTab && !g_options.noDialogForCtrlTab)
	{
		showSwitchDialog(FALSE, FALSE);
	}
	else
	{
		int currentView;
		::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&currentView));
		int nbFile = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, 0, currentView + 1);
		int position = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDOCINDEX, 0, currentView);
		if (position < nbFile - 1)
			++position;
		else
			position = 0;
		::SendMessage(nppData._nppHandle, NPPM_ACTIVATEDOC, currentView, position);
	}
}

void showSwitchDialogPrevious()
{
	if (g_options.emulateCtrlTab && !g_options.noDialogForCtrlTab)
	{
		showSwitchDialog(FALSE, TRUE);
	}
	else
	{
		int currentView;
		::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&currentView));

		int position = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDOCINDEX, 0, currentView);
		if (position > 0)
		{
			--position;
		}
		else
		{
			int nbFile = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, 0, currentView + 1);
			position = nbFile - 1;
		}

		::SendMessage(nppData._nppHandle, NPPM_ACTIVATEDOC, currentView, position);
	}
}

EditFile* addEditFile(TCHAR *filename, int view, int index, int bufferID, void* scintillaDoc)
{
	EditFile *editFile = new EditFile(view, index, filename, 0, bufferID, scintillaDoc);

	multimap<tstring, EditFile*>::iterator iter = filenameMap.find(filename);

	FileStatus status = SAVED;

	if (iter != filenameMap.end())
		status = iter->second->getFileStatus();

	editFile->setFileStatus(status);

	editFiles.insert(EditFileContainer::value_type(bufferID, editFile));

	tstring filenameString = filename;
	filenameMap.insert(multimap<tstring, EditFile*>::value_type(filenameString, editFile));

	return editFile;
}

EditFile* addEditFile(int bufferID)
{
	TCHAR filePath[MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, bufferID, reinterpret_cast<LPARAM>(filePath));
	int index = ::SendMessage(nppData._nppHandle, NPPM_GETPOSFROMBUFFERID, bufferID, reinterpret_cast<LPARAM>(filePath));

	BOOL readonly = ::SendMessage(getCurrentHScintilla(VIEW(index)), SCI_GETREADONLY, 0, 0);
	//void* pDoc = reinterpret_cast<void*>(::SendMessage(getCurrentHScintilla(VIEW(index)), SCI_GETDOCPOINTER, 0, 0));

	EditFile* editFile = addEditFile(filePath, VIEW(index), INDEX(index), bufferID, 0);

	if (readonly)
		editFile->setFileStatus(READONLY);

	return editFile;
}

void removeEditFile(int bufferID)
{
	tstring filename;
	// Free the EditFile
	EditFileContainer::iterator iter = editFiles.find(bufferID);
	if (iter != editFiles.end())
	{
		filename = iter->second->getFullFilename();
		do {
			delete iter->second;
			++iter;
		} while (iter != editFiles.end()
			     && iter->second->getBufferID() == bufferID);

		// Erase the EditFile from the two lookup maps
		filenameMap.erase(filename);
		editFiles.erase(bufferID);
	}
}

void updateBufferStatus(int bufferID, FileStatus status)
{
	EditFileContainer::iterator iter = editFiles.lower_bound(bufferID);
	EditFileContainer::iterator ub = editFiles.upper_bound(bufferID);

	while (iter != ub)
	{
		iter->second->setFileStatus(status);
		++iter;
	}
}

void updateCurrentStatus(FileStatus status)
{
	int bufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
	if (editFiles.find(bufferID) == editFiles.end())
	{
		addEditFile(bufferID);
	}

	updateBufferStatus(bufferID, status);
}

void updateCurrentBuffer()
{
	BOOL readonly = ::SendMessage(getCurrentHScintilla(_currentView), SCI_GETREADONLY, 0, 0);

	if (readonly)
		updateCurrentStatus(READONLY);
	else
	{
		int modified = ::SendMessage(getCurrentHScintilla(_currentView), SCI_GETMODIFY, 0, 0);
		if (modified)
			updateCurrentStatus(UNSAVED);
		else
			updateCurrentStatus(SAVED);
	}
}

struct DeleteObject {
	template<typename T>
	void operator()(const T *p) const
	{
		delete p;
	}
};

void clearEditFiles()
{
	for(EditFileContainer::iterator i = editFiles.begin(); i != editFiles.end(); i++)
		delete i->second;
}

void doAbout()
{
	aboutDlg.doDialog();
}

void doConfig()
{
	configDlg.doModal(nppData._nppHandle);
}

/*
void updatePreviousDocStatus(void)
{
	if (0 == _previousBufferID)
		return;

	int currentView;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&currentView));
	int currentIndex = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDOCINDEX, currentView, 0);

	EditFileContainer::iterator previous = editFiles.find(_previousBufferID);
	if (previous != editFiles.end())
	{
		HWND previousScintilla = getCurrentHScintilla(_previousView);
		void* currentDoc = reinterpret_cast<void*>(::SendMessage(previousScintilla, SCI_GETDOCPOINTER, 0, 0));
		void* previousDoc = previous->second->getScintillaDoc();
		if (previousDoc != 0)
		{
			int anchor     = ::SendMessage(previousScintilla, SCI_GETANCHOR, 0, 0);
			int currentPos = ::SendMessage(previousScintilla, SCI_GETCURRENTPOS, 0, 0);
			int xOffset    = ::SendMessage(previousScintilla, SCI_GETXOFFSET, 0, 0);
			int firstVisible = ::SendMessage(previousScintilla, SCI_GETFIRSTVISIBLELINE, 0, 0);

			::SendMessage(previousScintilla, SCI_ADDREFDOCUMENT, 0, reinterpret_cast<LPARAM>(currentDoc));
			::SendMessage(previousScintilla, SCI_SETDOCPOINTER, 0, reinterpret_cast<LPARAM>(previousDoc));
			BOOL readonly = ::SendMessage(previousScintilla, SCI_GETREADONLY, 0, 0);
			if (readonly)
			{
				updateBufferStatus(previous->second->getBufferID(), READONLY);
			}
			else
			{
				int modified = ::SendMessage(previousScintilla, SCI_GETMODIFY, 0, 0);
				if (1 == modified)
					updateBufferStatus(previous->second->getBufferID(), UNSAVED);
				else
					updateBufferStatus(previous->second->getBufferID(), SAVED);
			}

			::SendMessage(previousScintilla, SCI_SETDOCPOINTER, 0, reinterpret_cast<LPARAM>(currentDoc));
			::SendMessage(previousScintilla, SCI_RELEASEDOCUMENT, 0, reinterpret_cast<LPARAM>(currentDoc));
			::SendMessage(previousScintilla, SCI_GOTOPOS, 0, 0);
			int lineToShow = ::SendMessage(previousScintilla, SCI_VISIBLEFROMDOCLINE, firstVisible, 0);
			::SendMessage(previousScintilla, SCI_SETANCHOR, anchor, 0);
			::SendMessage(previousScintilla, SCI_SETCURRENTPOS, currentPos, 0);
			::SendMessage(previousScintilla, SCI_SETXOFFSET, xOffset, 0);
			::SendMessage(previousScintilla, SCI_LINESCROLL, 0, lineToShow);
		}
	}
}
*/

void updateCurrentScintillaDoc(int bufferID)
{
	int currentView;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&currentView));
	HWND currentScintilla = getCurrentHScintilla(currentView);
	void* currentDoc = reinterpret_cast<void*>(::SendMessage(currentScintilla, SCI_GETDOCPOINTER, 0, 0));

	EditFileContainer::iterator iter = editFiles.lower_bound(bufferID);
	EditFileContainer::iterator ub = editFiles.upper_bound(bufferID);

	while (iter != ub)
	{
		iter->second->setScintillaDoc(currentDoc);
		++iter;
	}
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

	_tcscpy_s(iniFilePath, MAX_PATH, configPath);
	_tcscat_s(iniFilePath, MAX_PATH, FILESWITCHER_INI);
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

	g_options.searchFlags = ::GetPrivateProfileInt(SEARCH_SETTINGS, KEY_SEARCHFLAGS, SEARCHFLAG_INCLUDEFILENAME, iniFilePath);
	if (g_options.searchFlags == 0)
		g_options.searchFlags = SEARCHFLAG_INCLUDEFILENAME;

	// This will be set later, when we check the shortcut key of the "Switch to Next" command
	g_options.emulateCtrlTab = FALSE;

	g_options.resetSortOrder = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_RESETSORT, 0, iniFilePath);
	g_options.defaultSortOrder = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_DEFAULTSORT, 0, iniFilePath);
	g_options.activeSortOrder = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_ACTIVESORT, 0, iniFilePath);
	g_options.overrideSortWhenTabbing = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_OVERRIDESORT, 0, iniFilePath);
	g_options.revertSortWhenTabbing = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_REVERTSORT, 0, iniFilePath);
	g_options.onlyUseCurrentView = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_ONLYUSECURRENTVIEW, 0, iniFilePath);
	g_options.autoSizeColumns = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_AUTOSIZECOLUMNS, 0, iniFilePath);
	g_options.autoSizeWindow = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_AUTOSIZEWINDOW, 0, iniFilePath);
	g_options.columnForView = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_COLUMNFORVIEW, 0, iniFilePath);
	g_options.noDialogForCtrlTab = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_NODIALOGFORCTRLTAB, 0, iniFilePath);
	g_options.useHomeForEdit = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_USEHOMEFOREDIT, 0, iniFilePath);

	::GetPrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNORDER, _T("012"), g_options.columnOrder, COLUMNORDER_LENGTH, iniFilePath);
	::GetPrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNWIDTHS, _T("160,100,60"), g_options.columnWidths, COLUMNWIDTH_LENGTH, iniFilePath);
}

void saveSettings(void)
{
	TCHAR temp[16];

	if (switchDlg.isCreated())
	{
		RECT rc;
		switchDlg.getWindowPosition(rc);

		_itot_s(rc.left, temp, 16, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWX, temp, iniFilePath);

		_itot_s(rc.top, temp, 16, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWY, temp, iniFilePath);

		_itot_s(rc.right - rc.left, temp, 16, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWWIDTH, temp, iniFilePath);

		_itot_s(rc.bottom - rc.top, temp, 16, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWHEIGHT, temp, iniFilePath);

		TCHAR columnInfo[50];
		switchDlg.getColumnOrderString(columnInfo, 50);
		::WritePrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNORDER, columnInfo, iniFilePath);

		switchDlg.getColumnWidths(columnInfo, 50);
		::WritePrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNWIDTHS, columnInfo, iniFilePath);
	}

	_itot_s(g_options.activeSortOrder, temp, 16, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_ACTIVESORT, temp, iniFilePath);

	_itot_s(g_options.searchFlags, temp, 16, 10);
	::WritePrivateProfileString(SEARCH_SETTINGS, KEY_SEARCHFLAGS, temp, iniFilePath);

	_itot_s(g_options.defaultSortOrder, temp, 16, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_DEFAULTSORT, temp, iniFilePath);

	_itot_s(g_options.resetSortOrder, temp, 16, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_RESETSORT, temp, iniFilePath);

	_itot_s(g_options.overrideSortWhenTabbing, temp, 16, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_OVERRIDESORT, temp, iniFilePath);

	_itot_s(g_options.revertSortWhenTabbing, temp, 16, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_REVERTSORT, temp, iniFilePath);

	_itot_s(g_options.onlyUseCurrentView, temp, 16, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_ONLYUSECURRENTVIEW, temp, iniFilePath);

	_itot_s(g_options.autoSizeColumns, temp, 16, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_AUTOSIZECOLUMNS, temp, iniFilePath);

	_itot_s(g_options.autoSizeWindow, temp, 16, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_AUTOSIZEWINDOW, temp, iniFilePath);

	_itot_s(g_options.noDialogForCtrlTab, temp, 16, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_NODIALOGFORCTRLTAB, temp, iniFilePath);

	_itot_s(g_options.columnForView, temp, 16, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_COLUMNFORVIEW, temp, iniFilePath);

	_itot_s(g_options.useHomeForEdit, temp, 16, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_USEHOMEFOREDIT, temp, iniFilePath);
}

#ifdef _MANAGED
#pragma managed(pop)
#endif