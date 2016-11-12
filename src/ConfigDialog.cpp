#include "precompiledHeaders.h"
#include "ConfigDialog.h"
#include "resource.h"
#include "Globals.h"

void ConfigDialog::doDialog()
{
	if (!isCreated())
		create(IDD_CONFIGDIALOG);

	_isModal = FALSE;
	goToCenter();
}

void ConfigDialog::doModal(HWND parent)
{
	_isModal = TRUE;
	DialogBoxParam(_hInst, MAKEINTRESOURCE(IDD_CONFIGDIALOG), parent, ConfigDialog::dlgProc, reinterpret_cast<LPARAM>(this));
	_isModal = FALSE;
}

void ConfigDialog::initializeOptions()
{
	tstring error;

	if (_options->searchFlags & SEARCHFLAG_CASESENSITIVE)
		::SendDlgItemMessage(_hSelf, IDC_CHECKCASE, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKCASE, BM_SETCHECK, BST_UNCHECKED, 0);

	if ((_options->searchFlags & SEARCHFLAG_STARTONLY))
		::SendDlgItemMessage(_hSelf, IDC_CHECKANYPART, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKANYPART, BM_SETCHECK, BST_UNCHECKED, 0);

	if (_options->searchFlags & SEARCHFLAG_INCLUDEPATH)
		::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEPATH, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEPATH, BM_SETCHECK, BST_UNCHECKED, 0);

	if (_options->searchFlags & SEARCHFLAG_INCLUDEFILENAME)
		::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEFILENAME, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEFILENAME, BM_SETCHECK, BST_UNCHECKED, 0);

	if (_options->searchFlags & SEARCHFLAG_INCLUDEINDEX)
		::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEINDEX, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEINDEX, BM_SETCHECK, BST_UNCHECKED, 0);

	if (_options->searchFlags & SEARCHFLAG_INCLUDEVIEW)
		::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEVIEW, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEVIEW, BM_SETCHECK, BST_UNCHECKED, 0);

	if (_options->useHomeForEdit)
		::SendDlgItemMessage(_hSelf, IDC_CHECKUSEHOMEFOREDIT, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKUSEHOMEFOREDIT, BM_SETCHECK, BST_UNCHECKED, 0);

	if (_options->searchFlags & SEARCHFLAG_INCLUDEWILDCARD)
		::SendDlgItemMessage(_hSelf, IDC_CHECKWILDCARD, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKWILDCARD, BM_SETCHECK, BST_UNCHECKED, 0);

	switch(_options->disabledSelectedSortOrder)
	{
	case ALWAYSREMEMBER :
		{
			::EnableWindow(GetDlgItem(_hSelf, IDC_RADIOSORTREMEMBER), FALSE);
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTREMEMBER, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
	case FILENAME :
		{
			::EnableWindow(GetDlgItem(_hSelf, IDC_RADIOSORTFILENAME), FALSE);
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTFILENAME, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
	case PATH :
		{
			::EnableWindow(GetDlgItem(_hSelf, IDC_RADIOSORTPATH), FALSE);
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTPATH, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
	case INDEX :
		{
			::EnableWindow(GetDlgItem(_hSelf, IDC_RADIOSORTINDEX), FALSE);
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTINDEX, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
	}

	if(_options->configuredContextPath.length() > 0)
	{
		::SetDlgItemText(_hSelf, IDC_SEARCHCONTEXTPATH, _options->configuredContextPath.c_str());
	}

	contextConfiguredOnInit = _options->hasConfiguredContext;
	if (_options->hasConfiguredContext)
	{
		::SendDlgItemMessage(_hSelf, IDC_CHECKCONFIGURESEARCHCONTEXT, BM_SETCHECK, BST_CHECKED, 0);
		::EnableWindow(GetDlgItem(_hSelf, IDC_SEARCHCONTEXTPATH), TRUE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_MAXTRAVERSEFILES), TRUE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_MAXDISPLAYFILES), TRUE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_RADIOSORTINDEX), FALSE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKINCLUDEVIEW), FALSE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKINCLUDEINDEX), FALSE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKONLYCURRENTVIEW), FALSE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKSEPARATECOLUMNFORVIEW), FALSE);

		contextPathOnInit = _options->configuredContextPath;
		if(_options->configuredContextPath.length() > 0
			&& !(_options->ConfiguredContextPathIsValid()))
		{
			error = _T("Context path is invalid.  Please provide a valid context path.");
		}
	}
	else
	{
		::SendDlgItemMessage(_hSelf, IDC_CHECKCONFIGURESEARCHCONTEXT, BM_SETCHECK, BST_UNCHECKED, 0);
		::EnableWindow(GetDlgItem(_hSelf, IDC_SEARCHCONTEXTPATH), FALSE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_MAXTRAVERSEFILES), FALSE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_MAXDISPLAYFILES), FALSE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_RADIOSORTINDEX), TRUE);
		if(_options->disabledSelectedSortOrder != NONE)
		{
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTINDEX, BM_SETCHECK, BST_CHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTFILENAME, BM_SETCHECK, BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTPATH, BM_SETCHECK, BST_UNCHECKED, 0);
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTREMEMBER, BM_SETCHECK, BST_UNCHECKED, 0);
			_options->disabledSelectedSortOrder = NONE;
		}
	}

	TCHAR bufMax[20];
	_itot_s(_options->maxTraverseFiles, bufMax, 20, 10);
	::SetDlgItemText(_hSelf, IDC_MAXTRAVERSEFILES, bufMax);
	_itot_s(_options->maxDisplayFiles, bufMax, 20, 10);
	::SetDlgItemText(_hSelf, IDC_MAXDISPLAYFILES, bufMax);

	switch(LOBYTE(_options->defaultSortOrder))
	{
	case FILENAME:
		{
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTFILENAME, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
	case PATH:
		{
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTPATH, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
	case INDEX:
		{
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTINDEX, BM_SETCHECK, BST_CHECKED, 0);
			break;
		}
	case ALWAYSREMEMBER:
	default:
		{
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTREMEMBER, BM_SETCHECK, BST_CHECKED, 0);
			EnableWindow(GetDlgItem(_hSelf, IDC_CHECKRESETSORTORDER), FALSE);
			EnableWindow(GetDlgItem(_hSelf, IDC_CHECKSORTDESCENDING), FALSE);
			break;
		}
	}

	::SendDlgItemMessage(_hSelf, IDC_CHECKSORTDESCENDING, BM_SETCHECK, _options->reversedSortOrder ? BST_CHECKED : BST_UNCHECKED, 0);
	::SendDlgItemMessage(_hSelf, IDC_CHECKRESETSORTORDER, BM_SETCHECK, _options->resetSortOrder ? BST_CHECKED : BST_UNCHECKED, 0);

	::SendDlgItemMessage(_hSelf, IDC_CHECKONLYCURRENTVIEW, BM_SETCHECK, _options->onlyUseCurrentView ? BST_CHECKED : BST_UNCHECKED, 0);

	::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZECOLUMNS, BM_SETCHECK, _options->autoSizeColumns ? BST_CHECKED : BST_UNCHECKED, 0);
	::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKAUTOSIZEWINDOW), _options->autoSizeColumns);

	::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZEWINDOW, BM_SETCHECK, _options->autoSizeWindow ? BST_CHECKED : BST_UNCHECKED, 0);

	::SendDlgItemMessage(_hSelf, IDC_CHECKSEPARATECOLUMNFORVIEW, BM_SETCHECK, _options->columnForView ? BST_CHECKED : BST_UNCHECKED, 0);

	if (_options->emulateCtrlTab)
	{
		::SetDlgItemText(_hSelf, IDC_LABELCTRLTAB, _T("Ctrl-Tab functionality is enabled"));
		::ShowWindow(GetDlgItem(_hSelf, IDC_LABELCTRLTAB), SW_HIDE);

		::ShowWindow(GetDlgItem(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING), SW_SHOW);
		::ShowWindow(GetDlgItem(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING), SW_SHOW);
		::ShowWindow(GetDlgItem(_hSelf, IDC_CHECKDIALOGFORCTRLTAB), SW_SHOW);

		if (_options->overrideSortWhenTabbing)
		{
			::SendDlgItemMessage(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING, BM_SETCHECK, BST_CHECKED, 0);
			::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING), TRUE);
		}
		else
		{
			::SendDlgItemMessage(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING, BM_SETCHECK, BST_UNCHECKED, 0);
			::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING), FALSE);
		}

		if (_options->revertSortWhenTabbing)
			::SendDlgItemMessage(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING, BM_SETCHECK, BST_CHECKED, 0);
		else
			::SendDlgItemMessage(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING, BM_SETCHECK, BST_UNCHECKED, 0);

		::SendDlgItemMessage(_hSelf, IDC_CHECKDIALOGFORCTRLTAB, BM_SETCHECK, _options->showDialogForCtrlTab ?  BST_CHECKED : BST_UNCHECKED, 0);
	}
	else
	{
		::SetDlgItemText(_hSelf, IDC_LABELCTRLTAB, _T("Ctrl-Tab functionality is disabled. ")
			_T("To enable it, set the shortcuts for ")
			_T("\"Switch to next document\" and ")
			_T("\"Switch to previous document\" ")
			_T("to Ctrl-Tab and Ctrl-Shift-Tab respectively. ")
			_T("Remove the shortcuts from the Notepad++ defaults."));
		::ShowWindow(GetDlgItem(_hSelf, IDC_LABELCTRLTAB), SW_SHOW);

		::ShowWindow(GetDlgItem(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING), SW_HIDE);
		::ShowWindow(GetDlgItem(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING), SW_HIDE);
		::ShowWindow(GetDlgItem(_hSelf, IDC_CHECKDIALOGFORCTRLTAB), SW_HIDE);
	}

	if(error.length() > 0)
	{
		MessageBox(_hSelf, error.c_str(), _T("Error"), MB_OK);
	}
}

void ConfigDialog::initTooltips()
{
	TCHAR bufMaxTooltip[MAX_TOOLTIP_LENGTH];
	_tcscpy_s(bufMaxTooltip
		, MAX_TOOLTIP_LENGTH
		, _T("Maximum number of files to traverse.")
			_T("This is to prevent np++ crashes due to a directory with too ")
			_T("many files, and configurable due to its performance dependency on hardware.")
	);
	CreateToolTip(IDC_TOOLTIPMAXTRAVERSE
		, _hSelf
		, bufMaxTooltip
	);

	_tcscpy_s(bufMaxTooltip
		, MAX_TOOLTIP_LENGTH
		, _T("Maximum number of files to display.  A lower number will speed up the searching refresh rate for each keystroke")
	);
	CreateToolTip(IDC_TOOLTIPMAXDISPLAY
		, _hSelf
		, bufMaxTooltip
	);

	_tcscpy_s(bufMaxTooltip
		, MAX_TOOLTIP_LENGTH
		, _T("Type a working directory that you want to be your new search context.")
	);
	CreateToolTip(IDC_TOOLTIPCONFIGURECONTEXT
		, _hSelf
		, bufMaxTooltip
	);
}

void ConfigDialog::enableOrDisableSortCheckBoxes()
{
	bool result = BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTREMEMBER, BM_GETCHECK, 0, 0);
	EnableWindow(GetDlgItem(_hSelf, IDC_CHECKRESETSORTORDER), !result);
	EnableWindow(GetDlgItem(_hSelf, IDC_CHECKSORTDESCENDING), !result);
}

BOOL CALLBACK ConfigDialog::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;

	switch (Message)
	{
	case WM_INITDIALOG :
		{
			initializeOptions();
			goToCenter();

			Edit_LimitText(GetDlgItem(_hSelf, IDC_MAXDISPLAYFILES), 7);
			Edit_LimitText(GetDlgItem(_hSelf, IDC_MAXTRAVERSEFILES), 7);
			Edit_LimitText(GetDlgItem(_hSelf, IDC_SEARCHCONTEXTPATH), MAX_PATH);

			initTooltips();

			return TRUE;
		}

	case WM_COMMAND :
		{
			switch (LOWORD(wParam))
			{
			case IDC_RADIOSORTREMEMBER:
			case IDC_RADIOSORTINDEX:
			case IDC_RADIOSORTPATH:
			case IDC_RADIOSORTFILENAME:
				{
					enableOrDisableSortCheckBoxes();
					break;
				}
			case IDC_CHECKSORTDESCENDING :
			{
				_options->activeReversedSortOrder = (BST_CHECKED != ::SendDlgItemMessage(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING, BM_GETCHECK, 0, 0));
				break;
			}
			case IDC_CHECKDIALOGFORCTRLTAB :
				{
					bool checked = BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKDIALOGFORCTRLTAB, BM_GETCHECK, 0, 0);
					EnableWindow(GetDlgItem(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING), checked);
					break;
				}
			case IDC_CHECKCONFIGURESEARCHCONTEXT:
				{
					bool isChecked = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKCONFIGURESEARCHCONTEXT, BM_GETCHECK, 0, 0));
					EnableWindow(GetDlgItem(_hSelf, IDC_SEARCHCONTEXTPATH), isChecked);
					EnableWindow(GetDlgItem(_hSelf, IDC_MAXTRAVERSEFILES), isChecked);
					EnableWindow(GetDlgItem(_hSelf, IDC_MAXDISPLAYFILES), isChecked);
					EnableWindow(GetDlgItem(_hSelf, IDC_CHECKINCLUDEINDEX), !isChecked);
					EnableWindow(GetDlgItem(_hSelf, IDC_CHECKINCLUDEVIEW), !isChecked);
					EnableWindow(GetDlgItem(_hSelf, IDC_RADIOSORTINDEX), !isChecked);
					EnableWindow(GetDlgItem(_hSelf, IDC_CHECKONLYCURRENTVIEW), !isChecked);
					EnableWindow(GetDlgItem(_hSelf, IDC_CHECKSEPARATECOLUMNFORVIEW), !isChecked);

					if(BST_CHECKED && ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTINDEX, BM_GETCHECK, 0, 0))
					{
						::SendDlgItemMessage(_hSelf, IDC_RADIOSORTFILENAME, BM_SETCHECK, BST_CHECKED, 0);
					}
					else if(!BST_CHECKED && ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTINDEX, BM_GETCHECK, 0, 0))
					{
						::SendDlgItemMessage(_hSelf, IDC_RADIOSORTREMEMBER, BM_SETCHECK, BST_UNCHECKED, 0);
						::SendDlgItemMessage(_hSelf, IDC_RADIOSORTFILENAME, BM_SETCHECK, BST_UNCHECKED, 0);
						::SendDlgItemMessage(_hSelf, IDC_RADIOSORTPATH, BM_SETCHECK, BST_UNCHECKED, 0);
					}
					break;
				}
			case IDC_CHECKAUTOSIZECOLUMNS:
				{
					result = ::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZECOLUMNS, BM_GETCHECK, 0, 0);
					EnableWindow(GetDlgItem(_hSelf, IDC_CHECKAUTOSIZEWINDOW), BST_CHECKED == result);
					break;
				}
			case IDOK :
				{
					tstring error = _T("");
					_options->searchFlags = 0;

					result = ::SendDlgItemMessage(_hSelf, IDC_CHECKSTARTONLY, BM_GETCHECK, 0, 0);
					if (BST_CHECKED == result)
						_options->searchFlags |= SEARCHFLAG_STARTONLY;

					result = ::SendDlgItemMessage(_hSelf, IDC_CHECKCASE, BM_GETCHECK, 0, 0);
					if (BST_CHECKED == result)
						_options->searchFlags |= SEARCHFLAG_CASESENSITIVE;

					result = ::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEPATH, BM_GETCHECK, 0, 0);
					if (BST_CHECKED == result)
						_options->searchFlags |= SEARCHFLAG_INCLUDEPATH;

					result = ::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEFILENAME, BM_GETCHECK, 0, 0);
					if (BST_CHECKED == result)
						_options->searchFlags |= SEARCHFLAG_INCLUDEFILENAME;

					result = ::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEINDEX, BM_GETCHECK, 0, 0);
					if (BST_CHECKED == result)
						_options->searchFlags |= SEARCHFLAG_INCLUDEINDEX;

					result = ::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEVIEW, BM_GETCHECK, 0, 0);
					if (BST_CHECKED == result)
						_options->searchFlags |= SEARCHFLAG_INCLUDEVIEW;

					_options->hasConfiguredContext = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKCONFIGURESEARCHCONTEXT, BM_GETCHECK, 0, 0));

					if(
						(
							!_options->hasConfiguredContext
							&& !(_options->searchFlags & SEARCHFLAG_INCLUDEPATH)
							&& !(_options->searchFlags & SEARCHFLAG_INCLUDEFILENAME)
							&& !(_options->searchFlags & SEARCHFLAG_INCLUDEINDEX)
							&& !(_options->searchFlags & SEARCHFLAG_INCLUDEVIEW)
						)
						|| (
							_options->hasConfiguredContext
							&& !(_options->searchFlags & SEARCHFLAG_INCLUDEPATH)
							&& !(_options->searchFlags & SEARCHFLAG_INCLUDEFILENAME)
						)
					){
						tstring tmpMsg = _T("- Must select at least one item to include in your search");
						error.length() > 0
							? error.append(_T("\r\n")).append(tmpMsg)
							: error.append(tmpMsg);
					}

					_options->disabledSelectedSortOrder = NONE;
					if (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTREMEMBER, BM_GETCHECK, 0, 0))
					{
						if (IsWindowEnabled(GetDlgItem(_hSelf, IDC_RADIOSORTREMEMBER)))
							_options->defaultSortOrder = ALWAYSREMEMBER;
						else
							_options->disabledSelectedSortOrder = ALWAYSREMEMBER;
					}

					_options->reversedSortOrder = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKSORTDESCENDING, BM_GETCHECK, 0, 0));
					_options->resetSortOrder = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKRESETSORTORDER, BM_GETCHECK, 0, 0));

					if (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTFILENAME, BM_GETCHECK, 0, 0))
					{
						if (IsWindowEnabled(GetDlgItem(_hSelf, IDC_RADIOSORTFILENAME)))
						{
							_options->activeSortOrder = FILENAME;
							_options->defaultSortOrder = FILENAME;
						}
						else
							_options->disabledSelectedSortOrder = FILENAME;
					}

					if (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTPATH, BM_GETCHECK, 0, 0))
					{
						if (IsWindowEnabled(GetDlgItem(_hSelf, IDC_RADIOSORTPATH)))
						{
							_options->activeSortOrder = PATH;
							_options->defaultSortOrder = PATH;
						}
						else
							_options->disabledSelectedSortOrder = PATH;
					}

					if (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTINDEX, BM_GETCHECK, 0, 0))
					{
						if (IsWindowEnabled(GetDlgItem(_hSelf, IDC_RADIOSORTINDEX)))
						{
							_options->activeSortOrder = INDEX;
							_options->defaultSortOrder = INDEX;
						}
						else
							_options->disabledSelectedSortOrder = INDEX;
					}

					_options->overrideSortWhenTabbing = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING, BM_GETCHECK, 0, 0));

					_options->revertSortWhenTabbing = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING, BM_GETCHECK, 0, 0));

					_options->onlyUseCurrentView = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKONLYCURRENTVIEW, BM_GETCHECK, 0, 0));

					_options->autoSizeColumns = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZECOLUMNS, BM_GETCHECK, 0, 0));

					_options->autoSizeWindow = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZEWINDOW, BM_GETCHECK, 0, 0));

					_options->columnForView = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKSEPARATECOLUMNFORVIEW, BM_GETCHECK, 0, 0));

					_options->showDialogForCtrlTab = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKDIALOGFORCTRLTAB, BM_GETCHECK, 0, 0));

					_options->useHomeForEdit = (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKUSEHOMEFOREDIT, BM_GETCHECK, 0, 0));

					if(BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKWILDCARD, BM_GETCHECK, 0, 0))
						_options->searchFlags |= SEARCHFLAG_INCLUDEWILDCARD;

					if (BST_CHECKED == ::SendDlgItemMessage(_hSelf, IDC_CHECKCASE, BM_GETCHECK, 0, 0))
						_options->searchFlags |= SEARCHFLAG_CASESENSITIVE;

					bool isNumber = true;
					TCHAR bufMaxs[20];
					GetDlgItemText(_hSelf, IDC_MAXTRAVERSEFILES, bufMaxs, MAX_PATH);
					tstring tmpMaxs = bufMaxs;
					if(tmpMaxs.length() > 0 && is_number(tmpMaxs))
					{
						_options->maxTraverseFiles = _tstoi(tmpMaxs.c_str());
					}
					else
					{
						tstring tmpMsg = _T("- Maximum files traversed is not a valid number");
						error.length() > 0
							? error.append(_T("\r\n")).append(tmpMsg)
							: error.append(tmpMsg);
						isNumber = false;
					}

					GetDlgItemText(_hSelf, IDC_MAXDISPLAYFILES, bufMaxs, MAX_PATH);
					tmpMaxs = bufMaxs;
					if(tmpMaxs.length() > 0 && is_number(tmpMaxs))
					{
						_options->maxDisplayFiles = _tstoi(tmpMaxs.c_str());
					}
					else
					{
						tstring tmpMsg = _T("- Maximum files displayed is not a valid number");
						error.length() > 0
							? error.append(_T("\r\n")).append(tmpMsg)
							: error.append(tmpMsg);
						isNumber = false;
					}
					if(isNumber && _options->maxTraverseFiles < _options->maxDisplayFiles)
					{
						tstring tmpMsg = _T("- Max traverse files must be more than max display files.");
						error.length() > 0
							? error.append(_T("\r\n")).append(tmpMsg)
							: error.append(tmpMsg);
					}

					TCHAR tmpPath[MAX_PATH];
					result = ::GetDlgItemText(_hSelf, IDC_SEARCHCONTEXTPATH, tmpPath, MAX_PATH);
					tstring tstrPath = tmpPath;
					if(_options->hasConfiguredContext
						&& (tstrPath.length() > 0))
						_options->configuredContextPath = (tstrPath.back() == _T('\\')) ? tstrPath : tstrPath.append(_T("\\"));

					if(_options->hasConfiguredContext
						&& !(_options->ConfiguredContextPathIsValid())
						){
						tstring tmpMsg = _T("- Context path is invalid.  Please provide a valid context path.");
						error.length() > 0
							? error.append(_T("\r\n")).append(tmpMsg)
							: error.append(tmpMsg);
					}

					if(error.length() > 0)
					{
						MessageBox(_hSelf, error.c_str(), _T("Error"), MB_OK);
						return TRUE;
					}

					g_filesNeedToBeReloaded(
						(_tcscmp(contextPathOnInit.c_str(), _options->configuredContextPath.c_str()) != 0)
						|| (!contextConfiguredOnInit && _options->hasConfiguredContext));
					redisplaySwitchDialog();
				}
			case IDCANCEL :
				{
					if (_isModal)
						EndDialog(_hSelf, 0);
					else
						display(FALSE);

					redisplaySwitchDialog();
					return TRUE;
				}
			case IDABORT :
			case IDCLOSE :
				{
					redisplaySwitchDialog();
				}
			default :
				break;
			}
		}
	}
	return FALSE;
}