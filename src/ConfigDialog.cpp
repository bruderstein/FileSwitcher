#include "StdAfx.h"
#include "ConfigDialog.h"
#include "resource.h" 



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


void ConfigDialog::initialiseOptions()
{
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



	switch(LOBYTE(_options->defaultSortOrder))
	{
		case FILENAME:
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTFILENAME, BM_SETCHECK, BST_CHECKED, 0);
			break;

		case PATH:
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTPATH, BM_SETCHECK, BST_CHECKED, 0);
			break;

		case INDEX:
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTINDEX, BM_SETCHECK, BST_CHECKED, 0);
			break;

		case ALWAYSREMEMBER:
		default:
			::SendDlgItemMessage(_hSelf, IDC_RADIOSORTREMEMBER, BM_SETCHECK, BST_CHECKED, 0);
			EnableWindow(GetDlgItem(_hSelf, IDC_CHECKRESETSORTORDER), FALSE);
			EnableWindow(GetDlgItem(_hSelf, IDC_CHECKSORTDESCENDING), FALSE);
			
			break;    
	}

	if (_options->defaultSortOrder & REVERSE_SORT_ORDER && _options->defaultSortOrder != ALWAYSREMEMBER)
		::SendDlgItemMessage(_hSelf, IDC_CHECKSORTDESCENDING, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKSORTDESCENDING, BM_SETCHECK, BST_UNCHECKED, 0);

	if (_options->resetSortOrder)
		::SendDlgItemMessage(_hSelf, IDC_CHECKRESETSORTORDER, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKRESETSORTORDER, BM_SETCHECK, BST_UNCHECKED, 0);


	if (_options->onlyUseCurrentView)
		::SendDlgItemMessage(_hSelf, IDC_CHECKONLYCURRENTVIEW, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKONLYCURRENTVIEW, BM_SETCHECK, BST_UNCHECKED, 0);



	if (_options->autoSizeColumns)
	{
		::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZECOLUMNS, BM_SETCHECK, BST_CHECKED, 0);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKAUTOSIZEWINDOW), TRUE);
	}
	else
	{
		::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZECOLUMNS, BM_SETCHECK, BST_UNCHECKED, 0);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKAUTOSIZEWINDOW), FALSE);
	}


	if (_options->autoSizeWindow)
		::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZEWINDOW, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZEWINDOW, BM_SETCHECK, BST_UNCHECKED, 0);

	
	if (_options->columnForView)
		::SendDlgItemMessage(_hSelf, IDC_CHECKSEPARATECOLUMNFORVIEW, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKSEPARATECOLUMNFORVIEW, BM_SETCHECK, BST_UNCHECKED, 0);

	if (_options->emulateCtrlTab)
	{
		::SetDlgItemText(_hSelf, IDC_LABELCTRLTAB, _T("Ctrl-Tab functionality is enabled"));
		::ShowWindow(GetDlgItem(_hSelf, IDC_LABELCTRLTAB), SW_HIDE);

		::ShowWindow(GetDlgItem(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING), SW_SHOW);
		::ShowWindow(GetDlgItem(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING), SW_SHOW);
		::ShowWindow(GetDlgItem(_hSelf, IDC_CHECKNODIALOGFORCTRLTAB), SW_SHOW);

/*
		::EnableWindow(GetDlgItem(_hSelf, IDC_LABELCTRLTAB), FALSE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING), TRUE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING), TRUE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKNODIALOGFORCTRLTAB), TRUE);
*/	

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



		if (_options->noDialogForCtrlTab)
			::SendDlgItemMessage(_hSelf, IDC_CHECKDIALOGFORCTRLTAB, BM_SETCHECK, BST_UNCHECKED, 0);
		else
			::SendDlgItemMessage(_hSelf, IDC_CHECKDIALOGFORCTRLTAB, BM_SETCHECK, BST_CHECKED, 0);
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
		::ShowWindow(GetDlgItem(_hSelf, IDC_CHECKNODIALOGFORCTRLTAB), SW_HIDE);
		/*
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING), FALSE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING), FALSE);
		::EnableWindow(GetDlgItem(_hSelf, IDC_CHECKDIALOGFORCTRLTAB), FALSE);
		*/
	}

}




BOOL CALLBACK ConfigDialog::run_dlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;

	switch (Message) 
	{
        case WM_INITDIALOG :
		{
			initialiseOptions();
			goToCenter();
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
						result = ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTREMEMBER, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
						{
							EnableWindow(GetDlgItem(_hSelf, IDC_CHECKRESETSORTORDER), FALSE);
							EnableWindow(GetDlgItem(_hSelf, IDC_CHECKSORTDESCENDING), FALSE);
						}
						else
						{
							EnableWindow(GetDlgItem(_hSelf, IDC_CHECKRESETSORTORDER), TRUE);
							EnableWindow(GetDlgItem(_hSelf, IDC_CHECKSORTDESCENDING), TRUE);
						}

						break;

					case IDC_CHECKOVERRIDESORTWHENTABBING:
						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
						{
							EnableWindow(GetDlgItem(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING), TRUE);
						}
						else
						{
							EnableWindow(GetDlgItem(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING), FALSE);
						}
						break;

					case IDC_CHECKAUTOSIZECOLUMNS:
						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZECOLUMNS, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
						{
							EnableWindow(GetDlgItem(_hSelf, IDC_CHECKAUTOSIZEWINDOW), TRUE);
						}
						else
						{
							EnableWindow(GetDlgItem(_hSelf, IDC_CHECKAUTOSIZEWINDOW), FALSE);
						}
						break;
					case IDOK :
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


						result = ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTFILENAME, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->defaultSortOrder = FILENAME;

						result = ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTPATH, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->defaultSortOrder = PATH;

						result = ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTINDEX, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->defaultSortOrder = INDEX;

						result = ::SendDlgItemMessage(_hSelf, IDC_RADIOSORTREMEMBER, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->defaultSortOrder = ALWAYSREMEMBER;
						else 
						{
							result = ::SendDlgItemMessage(_hSelf, IDC_CHECKSORTDESCENDING, BM_GETCHECK, 0, 0);
							if (BST_CHECKED == result)
								_options->defaultSortOrder |= REVERSE_SORT_ORDER;
						}

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKRESETSORTORDER, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->resetSortOrder = TRUE;
						else
							_options->resetSortOrder = FALSE;

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKOVERRIDESORTWHENTABBING, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->overrideSortWhenTabbing = TRUE;
						else
							_options->overrideSortWhenTabbing = FALSE;

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKREVERTSORTORDERDURINGTABBING, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->revertSortWhenTabbing = TRUE;
						else
							_options->revertSortWhenTabbing = FALSE;

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKONLYCURRENTVIEW, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->onlyUseCurrentView = TRUE;
						else
							_options->onlyUseCurrentView = FALSE;

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZECOLUMNS, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->autoSizeColumns = TRUE;
						else
							_options->autoSizeColumns = FALSE;


						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKAUTOSIZEWINDOW, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->autoSizeWindow = TRUE;
						else
							_options->autoSizeWindow = FALSE;

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKSEPARATECOLUMNFORVIEW, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->columnForView = TRUE;
						else
							_options->columnForView = FALSE;

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKDIALOGFORCTRLTAB, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->noDialogForCtrlTab = FALSE;
						else
							_options->noDialogForCtrlTab = TRUE;

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKUSEHOMEFOREDIT, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->useHomeForEdit = TRUE;
						else
							_options->useHomeForEdit = FALSE;

						

					case IDCANCEL :
						if (_isModal)
							EndDialog(hWnd, 0);
						else
							display(FALSE);
						return TRUE;

					default :
						break;
				}
			
		}
	}
	return FALSE;
}

