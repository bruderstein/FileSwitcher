#include "StdAfx.h"
#include "ConfigDialog.h"
#include "resource.h" 



void ConfigDialog::doDialog()
{
    if (!isCreated())
        create(IDD_CONFIGDIALOG);

	if (_options->searchFlags & SEARCHFLAG_CASESENSITIVE)
		::SendDlgItemMessage(_hSelf, IDC_CHECKCASE, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKCASE, BM_SETCHECK, BST_UNCHECKED, 0);

	if ((_options->searchFlags & SEARCHFLAG_STARTONLY))
		::SendDlgItemMessage(_hSelf, IDC_CHECKANYPART, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKANYPART, BM_SETCHECK, BST_UNCHECKED, 0);


	::SendDlgItemMessage(_hSelf, IDC_CHECKCTRLTAB, BM_SETCHECK, _options->emulateCtrlTab ? BST_CHECKED : BST_UNCHECKED, 0);



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
			break;    
	}

	if (_options->defaultSortOrder & REVERSE_SORT_ORDER)
		::SendDlgItemMessage(_hSelf, IDC_CHECKSORTDESCENDING, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKSORTDESCENDING, BM_SETCHECK, BST_UNCHECKED, 0);

	if (_options->resetSortOrder)
		::SendDlgItemMessage(_hSelf, IDC_CHECKRESETSORTORDER, BM_SETCHECK, BST_CHECKED, 0);
	else
		::SendDlgItemMessage(_hSelf, IDC_CHECKRESETSORTORDER, BM_SETCHECK, BST_UNCHECKED, 0);


	goToCenter();
}


BOOL CALLBACK ConfigDialog::run_dlgProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result;

	switch (Message) 
	{
        case WM_INITDIALOG :
		{
			return TRUE;
		}
		


		case WM_COMMAND : 
		{
			
				
				switch (wParam)
				{
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

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKSORTDESCENDING, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->defaultSortOrder |= REVERSE_SORT_ORDER;

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKRESETSORTORDER, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->resetSortOrder = TRUE;
						else
							_options->resetSortOrder = FALSE;











						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKCTRLTAB, BM_GETCHECK, 0, 0);
						_options->emulateCtrlTab = (BST_CHECKED == result) ? TRUE : FALSE;


					case IDCANCEL :
						display(FALSE);
						return TRUE;

					default :
						break;
				}
			
		}
	}
	return FALSE;
}

