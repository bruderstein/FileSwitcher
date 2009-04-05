#include "StdAfx.h"
#include <tchar.h>
#include "EditFile.h"

EditFile::EditFile(void)
{
	_dataSet = false;
}

EditFile::EditFile(int index, CONST TCHAR* filename, int searchFlags, int bufferID)
{
	// Copy the full filename
	int filenameLength = _tcslen(filename);
	_fullFilename = new TCHAR[filenameLength + 1];
	_tcscpy(_fullFilename, filename);
	
	int position, filenameOffset = 1;
	for (position = filenameLength; position >= 0 && _fullFilename[position] != '\\'; position--)
		;
	if (position < 0)
	{
		position = 0;
		filenameOffset = 0;
	}

	_path = new TCHAR[position + 1];
	int filenameOnlyLength = (filenameLength - position) + 1;
	_filename = new TCHAR[filenameOnlyLength]; 
	_display = new TCHAR[filenameLength + 5];
	_tcsncpy(_path, _fullFilename, position);
	_path[position] = '\0';
	_tcscpy(_filename, (_fullFilename + position + filenameOffset));


	_tcscpy(_display, _filename);

	if (_tcscmp(_path, _T("")))
	{
		_tcscat(_display, _T(" ("));
		_tcscat(_display, _path);
		_tcscat(_display, _T(")"));
	}

	if (searchFlags & SEARCHFLAG_INCLUDEPATH)
	{
		_searchFilename = new TCHAR[filenameLength + 1];
		_tcscpy(_searchFilename, _fullFilename);

	}
	else 
	{
		_searchFilename = new TCHAR[filenameOnlyLength];
		_tcscpy(_searchFilename, _filename);
	}

	if ((searchFlags & SEARCHFLAG_CASESENSITIVE) == 0)
	{
		_tcslwr(_searchFilename);
	}

	_dataSet = true;
	_index = index;
	_bufferID = bufferID;
}

EditFile::~EditFile(void)
{
	if (_dataSet)
	{
		delete[] _fullFilename;
		delete[] _path;
		delete[] _display;
		delete[] _filename;
		delete[] _searchFilename;
	}
}

TCHAR *EditFile::getDisplayString()
{
	return _display;
}

TCHAR *EditFile::getFilename()
{
	return _filename;
}

TCHAR *EditFile::getFullFilename()
{
	return _fullFilename;
}

TCHAR *EditFile::getSearchFilename()
{
	return _searchFilename;
}

int EditFile::getIndex()
{
	return _index;
}

int EditFile::getBufferID()
{
	return _bufferID;
}
