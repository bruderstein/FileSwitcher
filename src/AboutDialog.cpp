#include <precompiledHeaders.h>
#include "AboutDialog.h"
#include "resource.h"

AboutDialog::AboutDialog(void)
{
}

AboutDialog::~AboutDialog(void)
{
}

void AboutDialog::doDialog()
{
    if (!isCreated())
        create(IDD_ABOUTDIALOG);

	goToCenter();
}

BOOL CALLBACK AboutDialog::run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam)
{
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