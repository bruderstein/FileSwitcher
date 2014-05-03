#pragma once

#include "StaticDialog.h"

#ifndef PLUGININTERFACE_H
#include "PluginInterface.h"
#endif

class AboutDialog : public StaticDialog
{
public:
	AboutDialog(void);
	~AboutDialog(void);

	void init(HINSTANCE hInst, NppData nppData)
	{
		_nppData = nppData;
		Window::init(hInst, nppData._nppHandle);
	};

    void doDialog();
    BOOL CALLBACK run_dlgProc(UINT Message, WPARAM wParam, LPARAM lParam);

private:
	NppData _nppData;
};
