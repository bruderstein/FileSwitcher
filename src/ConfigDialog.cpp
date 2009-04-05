#include "StdAfx.h"
#include "ConfigDialog.h"
#include "resource.h" 



void ConfigDialog::doDialog()
{
    if (!isCreated())
        create(IDD_CONFIGDIALOG);

	if (_options->searchFlags & SEARCHFLAG_CASESENSITIVE)
		::SendDlgItemMessage(_hSelf, IDC_CHECKCASE, BM_SETCHECK, BST_CHECKED, 0);

	if (_options->searchFlags & SEARCHFLAG_INCLUDEPATH)
		::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEPATH, BM_SETCHECK, BST_CHECKED, 0);

	if ((_options->searchFlags & SEARCHFLAG_STARTONLY) == 0)
		::SendDlgItemMessage(_hSelf, IDC_CHECKANYPART, BM_SETCHECK, BST_CHECKED, 0);

	::SendDlgItemMessage(_hSelf, IDC_CHECKCTRLTAB, BM_SETCHECK, _options->emulateCtrlTab ? BST_CHECKED : BST_UNCHECKED, 0);
    
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
						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKANYPART, BM_GETCHECK, 0, 0);
						if (BST_UNCHECKED == result)
							_options->searchFlags |= SEARCHFLAG_STARTONLY;

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKCASE, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->searchFlags |= SEARCHFLAG_CASESENSITIVE;

						result = ::SendDlgItemMessage(_hSelf, IDC_CHECKINCLUDEPATH, BM_GETCHECK, 0, 0);
						if (BST_CHECKED == result)
							_options->searchFlags |= SEARCHFLAG_INCLUDEPATH;

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

