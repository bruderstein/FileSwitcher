#pragma once

#include "SearchOptions.h"
#include "StaticDialog.h"
#include "FileSwitcher.h"

class ConfigDialog : public StaticDialog
{
public:
	ConfigDialog() : StaticDialog() {}
	void init(HINSTANCE hInst, NppData nppData, struct options_t *options)
	{
		_nppData = nppData;
		Window::init(hInst, nppData._nppHandle);
		_options = options;
	};

   	void doDialog();
	void doModal(HWND parent);

	void setOptions(int options);
	int getOptions(void);

	~ConfigDialog(void) {}

protected :
	virtual BOOL CALLBACK run_dlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	NppData			_nppData;
    HWND			_HSource;
	
	BOOL			_isModal;

	struct options_t *_options;

	void ConfigDialog::initialiseOptions();

};
