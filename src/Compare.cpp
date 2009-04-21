#include "stdafx.h"

#include "Compare.h"
#include <tchar.h>

LPTSTR _tcsistr(LPCTSTR string, LPCTSTR strCharSet)
{
	int nLen = _tcslen(strCharSet);

	while (*string)
	{
		if (_tcsnicmp(string, strCharSet, nLen) == 0)
			break;
		string++;
	}

	if (*string == _T('\0'))
		string = NULL;

	return (LPTSTR)string;
}