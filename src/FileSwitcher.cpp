// BufferSwitcher.cpp : Defines the entry point for the DLL application.
//

#include "precompiledHeaders.h"
#include "FileSwitcher.h"
#include "SwitchDialog.h"
#include "AboutDialog.h"
#include "ConfigDialog.h"
#include <TCHAR.H>
#include <shlwapi.h>
#include <commctrl.h>
#include <atlbase.h>
#include <map>
#include <string>
#include "../thirdPartyLibs/dirent.h"

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
winVer				gWinVersion			= WV_UNKNOWN;
struct options_t	g_options;
TCHAR iniFilePath[MAX_PATH];
BOOL				_nppReady;
int					_previousBufferID;
int					_previousView;
uptr_t				_currentBufferID;
int					_currentView;

/* Maps for lookups */
EditFileContainer		editFiles;
FilenameContainer		filenameMap;
SimpleFileContainer		simpleFiles;
map<LRESULT, TCHAR *>	typedForFile;

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
EditFile* addEditFile(uptr_t bufferID);
EditFile* addEditFile(TCHAR *filename, int view, int index, uptr_t bufferID);
void removeEditFile(uptr_t bufferID);
void updateCurrentStatus(FileStatus status);
void updateBufferStatus(LRESULT bufferID, FileStatus status);
void updateCurrentBuffer();
void clearEditFiles();

bool _filesNeedToBeReloaded;
bool g_filesNeedToBeReloaded()
{
	return _filesNeedToBeReloaded;
}
void g_filesNeedToBeReloaded(bool val)
{
	if(val)
	{
		simpleFiles.clear();
	}
	_filesNeedToBeReloaded = val;
}

BOOL APIENTRY DllMain( HMODULE hModule,
					  DWORD  ul_reason_for_call,
					  LPVOID lpReserved
					  )
{
	g_filesNeedToBeReloaded(true);
	g_hModule = hModule;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
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
		}
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

HMODULE GetCurrentModule()
{ // NB: XP+ solution!
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurrentModule,
		&hModule);

	return hModule;
}

extern "C" __declspec(dllexport) void setInfo(NppData notepadPlusData)
{
	nppData = notepadPlusData;
	switchDlg.init(GetCurrentModule(), nppData, &g_options, &typedForFile, &configDlg);
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

void RecursivelyAddFilesFromConfiguredContextPath(tstring root, size_t maxFiles)
{
	TDIR *dir;
	struct tdirent *ent;
	if((dir = topendir(root.c_str())) != NULL)
	{
		while(simpleFiles.size() < maxFiles && (ent = treaddir(dir)) != NULL)
		{
			if(ent->d_type == DT_DIR
				&& (_tcscmp(ent->d_name, _T(".")) != 0)
				&& (_tcscmp(ent->d_name, _T("..")) != 0)
				){
					tstring newRoot = tstring().append(root).append(ent->d_name).append(_T("\\"));
					RecursivelyAddFilesFromConfiguredContextPath(newRoot, maxFiles);
			}
			else if(ent->d_type == DT_REG)
			{
				tstring tmpName = ent->d_name;
				simpleFiles.insert(SimpleFileContainer::value_type(tmpName, new SimpleFileInfo(tmpName, root)));
			}
		}

		tclosedir(dir);
	}
	else
	{
		//can't think of why this else statement would be reached, but notify the user in case.
		int res = MessageBox(NULL, _T("Error occurred when opening the configured context folder (or a subfolder).  Please make sure the path is correct and that permissions are set appropriately.  Press OK to modify the configurations."), _T("Error"), MB_OKCANCEL);

		if(res == IDOK)
			doConfig();
	}
}

void loadFiles(tstring root, int maxFiles)
{
	RecursivelyAddFilesFromConfiguredContextPath(g_options.configuredContextPath, g_options.maxTraverseFiles);
	if(simpleFiles.size() == maxFiles)
	{
		TCHAR buffer[20];
		_itot_s(maxFiles, buffer, 20, 10);
		tstring tmp = buffer;
		tstring msg = tstring(_T("Your configured maximum number of ("))
			.append(buffer)
			.append(_T(") files have been traversed.  This feature was not optimized for large projects, so increasing the "))
			.append(_T("number _may_ slow down search and initial traversal load speeds depending on your hardware."))
		;
		MessageBox(NULL, msg.c_str(), _T("Notification"), MB_OK);
	}
}

void showSwitchDialog(BOOL ignoreCtrlTab, BOOL previousFile)
{
	bool error = false;

	//showSwitchDialog only shows the normal way (not via ctrl+tab) when ignoreCtrlTab is true
	//  and previousFile is false.  When we have a hasConfiguredContext(),
	//  then we don't cycle through the currently open files
	if(ignoreCtrlTab && !previousFile && g_options.hasConfiguredContext)
	{
		if(!g_options.ConfiguredContextPathIsValid())
		{
			error = true;
			int res = MessageBox(NULL, _T("Configured context path is not valid.  Press OK to modify the configurations."), _T("Error"), MB_OKCANCEL);

			if(res == IDOK)
				doConfig();
		}
		else if(g_filesNeedToBeReloaded())
		{
			loadFiles(g_options.configuredContextPath, g_options.maxTraverseFiles);
			g_filesNeedToBeReloaded(false);
		}

		if(!error)
			switchDlg.doDialog(simpleFiles);
	}
	else //cycle through currently opened files
	{
		editFiles.clear();
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
		//updateCurrentBuffer();

		for(int view = 0; view < 2; view++)
		{
			std::string tmp = "Step " + std::to_string(view + 3);
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
					LRESULT bufferID;
					tstring filenameString;
					for(int position = 0; position < nbFile[view]; position++)
					{
						filenameString = fileNames[position];
						FilenameContainer::iterator filenameIterator = filenameMap.lower_bound(filenameString);
						FilenameContainer::iterator upperBoundIterator = filenameMap.upper_bound(filenameString);

						//Not exactly sure why this loop is necessary other than to fulfill the else statement below.
						//  There's probably a better way to do this.
						while(filenameIterator != upperBoundIterator
							&& filenameIterator->second->getView() >= 0)
						{
							++filenameIterator;
						}

						if (filenameIterator == filenameMap.end()
							|| filenameIterator->first != filenameString)
						{
							bufferID = ::SendMessage(nppData._nppHandle, NPPM_GETBUFFERIDFROMPOS, position, view);

							addEditFile(fileNames[position], view, position, bufferID);
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
}

void showSwitchDialogNormal()
{
	showSwitchDialog(TRUE, FALSE);
}

void showSwitchDialogNext()
{
	if (g_options.emulateCtrlTab && g_options.showDialogForCtrlTab)
	{
		showSwitchDialog(FALSE, FALSE);
	}
	else
	{
		int currentView;
		::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&currentView));
		int nbFile = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, 0, currentView + 1);
		auto position = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDOCINDEX, 0, currentView);
		if (position < nbFile - 1)
			++position;
		else
			position = 0;
		::SendMessage(nppData._nppHandle, NPPM_ACTIVATEDOC, currentView, position);
	}
}

void showSwitchDialogPrevious()
{
	if (g_options.emulateCtrlTab && g_options.showDialogForCtrlTab)
	{
		showSwitchDialog(FALSE, TRUE);
	}
	else
	{
		int currentView;
		::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, reinterpret_cast<LPARAM>(&currentView));

		auto position = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTDOCINDEX, 0, currentView);
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

EditFile* addEditFile(TCHAR *filename, int view, int index, uptr_t bufferID)
{
	EditFile *editFile = new EditFile(view, index, filename, 0, bufferID);

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

EditFile* addEditFile(uptr_t bufferID)
{
	TCHAR filePath[MAX_PATH];
	::SendMessage(nppData._nppHandle, NPPM_GETFULLPATHFROMBUFFERID, bufferID, reinterpret_cast<LPARAM>(filePath));
	auto index = ::SendMessage(nppData._nppHandle, NPPM_GETPOSFROMBUFFERID, bufferID, reinterpret_cast<LPARAM>(filePath));

	auto readonly = ::SendMessage(getCurrentHScintilla(VIEW(index)), SCI_GETREADONLY, 0, 0);

	EditFile* editFile = addEditFile(filePath, VIEW(index), INDEX(index), bufferID);

	if (readonly)
		editFile->setFileStatus(READONLY);

	return editFile;
}

void removeEditFile(uptr_t bufferID)
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

void updateBufferStatus(LRESULT bufferID, FileStatus status)
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
	auto bufferID = ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTBUFFERID, 0, 0);
	if (editFiles.find(bufferID) == editFiles.end())
	{
		addEditFile(bufferID);
	}

	updateBufferStatus(bufferID, status);
}

void updateCurrentBuffer()
{
	auto readonly = ::SendMessage(getCurrentHScintilla(_currentView), SCI_GETREADONLY, 0, 0);

	if (readonly)
		updateCurrentStatus(READONLY);
	else
	{
		auto modified = ::SendMessage(getCurrentHScintilla(_currentView), SCI_GETMODIFY, 0, 0);
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

	g_options.reversedSortOrder = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_REVERSEDSORT, 0, iniFilePath);
	g_options.activeReversedSortOrder = g_options.reversedSortOrder;
	g_options.resetSortOrder = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_RESETSORT, 0, iniFilePath);
	g_options.defaultSortOrder = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_DEFAULTSORT, 0, iniFilePath);

	if(g_options.defaultSortOrder == ALWAYSREMEMBER)
		g_options.activeSortOrder = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_ACTIVESORT, 0, iniFilePath);
	else
		g_options.activeSortOrder = g_options.defaultSortOrder;

	g_options.disabledSelectedSortOrder = (SortOrder)::GetPrivateProfileInt(SORT_SETTINGS, KEY_DISABLEDSELECTEDSORTORDER, 0, iniFilePath);
	g_options.overrideSortWhenTabbing = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_OVERRIDESORT, 0, iniFilePath);
	g_options.revertSortWhenTabbing = ::GetPrivateProfileInt(SORT_SETTINGS, KEY_REVERTSORT, 0, iniFilePath);
	g_options.onlyUseCurrentView = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_ONLYUSECURRENTVIEW, 0, iniFilePath);
	g_options.autoSizeColumns = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_AUTOSIZECOLUMNS, 0, iniFilePath);
	g_options.autoSizeWindow = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_AUTOSIZEWINDOW, 0, iniFilePath);
	g_options.columnForView = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_COLUMNFORVIEW, 0, iniFilePath);
	g_options.showDialogForCtrlTab = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_SHOWDIALOGFORCTRLTAB, 1, iniFilePath);
	g_options.useHomeForEdit = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_USEHOMEFOREDIT, 0, iniFilePath);
	g_options.hasConfiguredContext = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_CONFIGURESEARCHCONTEXT, 0, iniFilePath) != false;
	int tmp = ::GetPrivateProfileInt(GENERAL_SETTINGS, KEY_DISABLEDSELECTEDSORTORDER, -2, iniFilePath);
	g_options.disabledSelectedSortOrder = static_cast<SortOrder>(tmp);

	::GetPrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNORDER, _T("012"), g_options.columnOrder, COLUMNORDER_LENGTH, iniFilePath);
	if(g_options.hasConfiguredContext)
	{
		::GetPrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNWIDTHS, _T("160,100,60"), g_options.columnWidths, COLUMNWIDTH_LENGTH, iniFilePath);
	}
	else
	{
		::GetPrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNWIDTHSSIMPLE, _T("200,300"), g_options.columnWidths, COLUMNWIDTH_LENGTH, iniFilePath);
	}

	TCHAR tmpPath[MAX_PATH];
	::GetPrivateProfileString(GENERAL_SETTINGS, KEY_SEARCHCONTEXTPATH, _T(""), tmpPath, MAX_PATH, iniFilePath);
	g_options.configuredContextPath = tmpPath;

	TCHAR tmpMax[20];
	::GetPrivateProfileString(GENERAL_SETTINGS, KEY_MAXTRAVERSEFILE, _T("1000"), tmpMax, 20, iniFilePath);
	g_options.maxTraverseFiles = _tstoi(tmpMax);

	::GetPrivateProfileString(GENERAL_SETTINGS, KEY_MAXDISPLAYFILE, _T("20"), tmpMax, 20, iniFilePath);
	g_options.maxDisplayFiles = _tstoi(tmpMax);
}

void redisplaySwitchDialog()
{
	//only want to redisplay the switch dialog if the options was accessed via the switch dialog [options] button
	if(switchDlg._displayingOptionsDialog)
	{
		switchDlg.run_dlgProc(WM_COMMAND, IDCLOSE, NULL);
		showSwitchDialogNormal();
	}
}

void saveColumnWidths()
{
	TCHAR columnInfo[50];
	if(switchDlg.getNumColumns() == 2)
	{
		switchDlg.getColumnWidths(columnInfo, 50);
		::WritePrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNWIDTHSSIMPLE, columnInfo, iniFilePath);
	}
	else
	{
		switchDlg.getColumnWidths(columnInfo, 50);
		::WritePrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNWIDTHS, columnInfo, iniFilePath);
	}
}

void saveSettings(void)
{
	TCHAR temp[20];

	if (switchDlg.isCreated())
	{
		RECT rc;
		switchDlg.getWindowPosition(rc);

		_itot_s(rc.left, temp, 20, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWX, temp, iniFilePath);

		_itot_s(rc.top, temp, 20, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWY, temp, iniFilePath);

		_itot_s(rc.right - rc.left, temp, 20, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWWIDTH, temp, iniFilePath);

		_itot_s(rc.bottom - rc.top, temp, 20, 10);
		::WritePrivateProfileString(WINDOW_SETTINGS, KEY_WINDOWHEIGHT, temp, iniFilePath);

		TCHAR columnInfo[50];
		switchDlg.getColumnOrderString(columnInfo, 50);
		::WritePrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNORDER, columnInfo, iniFilePath);
	}

	_itot_s(g_options.activeSortOrder, temp, 20, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_ACTIVESORT, temp, iniFilePath);

	_itot_s(g_options.disabledSelectedSortOrder, temp, 20, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_DISABLEDSELECTEDSORTORDER, temp, iniFilePath);

	_itot_s(g_options.searchFlags, temp, 20, 10);
	::WritePrivateProfileString(SEARCH_SETTINGS, KEY_SEARCHFLAGS, temp, iniFilePath);

	_itot_s(g_options.defaultSortOrder, temp, 20, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_DEFAULTSORT, temp, iniFilePath);

	_itot_s(g_options.reversedSortOrder, temp, 20, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_REVERSEDSORT, temp, iniFilePath);

	_itot_s(g_options.resetSortOrder, temp, 20, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_RESETSORT, temp, iniFilePath);

	_itot_s(g_options.overrideSortWhenTabbing, temp, 20, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_OVERRIDESORT, temp, iniFilePath);

	_itot_s(g_options.revertSortWhenTabbing, temp, 20, 10);
	::WritePrivateProfileString(SORT_SETTINGS, KEY_REVERTSORT, temp, iniFilePath);

	_itot_s(g_options.onlyUseCurrentView, temp, 20, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_ONLYUSECURRENTVIEW, temp, iniFilePath);

	_itot_s(g_options.autoSizeColumns, temp, 20, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_AUTOSIZECOLUMNS, temp, iniFilePath);

	_itot_s(g_options.autoSizeWindow, temp, 20, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_AUTOSIZEWINDOW, temp, iniFilePath);

	_itot_s(g_options.showDialogForCtrlTab, temp, 20, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_SHOWDIALOGFORCTRLTAB, temp, iniFilePath);

	_itot_s(g_options.columnForView, temp, 20, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_COLUMNFORVIEW, temp, iniFilePath);

	_itot_s(g_options.useHomeForEdit, temp, 20, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_USEHOMEFOREDIT, temp, iniFilePath);

	_itot_s(g_options.hasConfiguredContext, temp, 20, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_CONFIGURESEARCHCONTEXT, temp, iniFilePath);

	TCHAR temp2[MAX_PATH];
	_tcscpy_s(temp2, MAX_PATH, g_options.configuredContextPath.c_str());
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_SEARCHCONTEXTPATH, temp2, iniFilePath);

	_itot_s(g_options.disabledSelectedSortOrder, temp, 20, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_DISABLEDSELECTEDSORTORDER, temp, iniFilePath);

	_itot_s(g_options.maxTraverseFiles, temp, 20, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_MAXTRAVERSEFILE, temp, iniFilePath);

	_itot_s(g_options.maxDisplayFiles, temp, 20, 10);
	::WritePrivateProfileString(GENERAL_SETTINGS, KEY_MAXDISPLAYFILE, temp, iniFilePath);
}

bool is_number(const tstring &s)
{
    return !s.empty() && std::find_if(s.begin(),
        s.end(), [](TCHAR c) { return !_istdigit(c); }) == s.end();
}

// Description:
//   Creates a tooltip for an item in a dialog box.
// Parameters:
//   idTool - identifier of an dialog box item.
//   nDlg - window handle of the dialog box.
//   pszText - string to use as the tooltip text.
// Returns:
//   The handle to the tooltip.
//
HWND CreateToolTip(int toolID, HWND hDlg, PTSTR pszText)
{
    if (!toolID || !hDlg || !pszText)
    {
        return FALSE;
    }
    // Get the window of the tool.
    HWND hwndTool = GetDlgItem(hDlg, toolID);

    // Create the tooltip. g_hInst is the global instance handle.
    HWND hwndTip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
                              WS_POPUP |TTS_ALWAYSTIP | TTS_BALLOON,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              hDlg, NULL,
                              (HINSTANCE)g_hModule, NULL);

   if (!hwndTool || !hwndTip)
   {
       return (HWND)NULL;
   }

    // Associate the tooltip with the tool.
    TOOLINFO toolInfo = { 0 };
    toolInfo.cbSize = sizeof(toolInfo);
    toolInfo.hwnd = hDlg;
    toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
    toolInfo.uId = (UINT_PTR)hwndTool;
    toolInfo.lpszText = pszText;
    SendMessage(hwndTip, TTM_ADDTOOL, 0, (LPARAM)&toolInfo);
	SendMessage(hwndTip, TTM_SETMAXTIPWIDTH, 0, 200);
	SendMessage(hwndTip, TTM_SETDELAYTIME, TTDT_AUTOPOP, 10000);

    return hwndTip;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif