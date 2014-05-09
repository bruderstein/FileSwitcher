#include "precompiledHeaders.h"
#include <tchar.h>
#include "EditFile.h"

EditFile::EditFile(void)
{
	_dataSet = false;
	_indexString = NULL;
	_viewString = NULL;
}

EditFile::EditFile(int view, int index, CONST TCHAR* filename, int searchFlags, int bufferID)
{
	// Copy the full filename
	int filenameLength = _tcslen(filename);
	_fullFilename = new TCHAR[filenameLength + 1];
	_tcscpy_s(_fullFilename, filenameLength + 1, filename);

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
	_tcsncpy_s(_path, position + 1, _fullFilename, position);
	_path[position] = '\0';
	_tcscpy_s(_filename, filenameOnlyLength, (_fullFilename + position + filenameOffset));

	_dataSet = true;
	_indexString = NULL;
	_viewString = NULL;
	setIndex(view, index);
	_bufferID = bufferID;
	_fileStatus = SAVED;
}

EditFile::~EditFile(void)
{
	if (_dataSet)
	{
		delete[] _fullFilename;
		delete[] _path;
		delete[] _filename;
		if (_indexString != NULL)
			delete[] _indexString;

		if (_viewString != NULL)
			delete[] _viewString;
	}
}

TCHAR *EditFile::getFilename()
{
	return _filename;
}

TCHAR *EditFile::getPath()
{
	return _path;
}

TCHAR *EditFile::getFullFilename()
{
	return _fullFilename;
}

int EditFile::getIndex()
{
	return _index;
}

bool EditFile::HasMultipleViews()
{
	bool result = true;

	int nbFile[2];
	nbFile[0] = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, 0, PRIMARY_VIEW);
	if(nbFile[0] == 1)
	{
		TCHAR **fileNames = (TCHAR **) new TCHAR *[1];
		fileNames[0] = new TCHAR [MAX_PATH];
		::SendMessage(nppData._nppHandle, NPPM_GETOPENFILENAMESPRIMARY, reinterpret_cast<WPARAM>(fileNames), (LPARAM)1);
		tstring fileA = fileNames[0];
		tstring newFile = _T("new  0");

		if (_tcscmp(fileA.c_str(), newFile.c_str()) == 0)
		{
			result = false;
		}

		delete fileNames[0];
		delete fileNames;
	}

	return result;
}

TCHAR *EditFile::getIndexString(BOOL includeView)
{
	if (_indexString == NULL)
	{
		includeView = includeView && HasMultipleViews();

		TCHAR tmp[16];
		_itot_s(_index + 1, tmp, 16, 10);
		int length = _tcslen(tmp);

		if (_view == 1 && includeView)
			length += 4;

		_indexString = new TCHAR[length + 1];

		if (_view == 1 && includeView)
		{
			_tcscpy_s(_indexString, length + 1, _T("[2] "));
			_tcscat_s(_indexString, length + 1, tmp);
		}
		else
			_tcscpy_s(_indexString, length + 1, tmp);
	}
	return _indexString;
}

int EditFile::getView()
{
	return HasMultipleViews() ? _view : 0;
}

TCHAR *EditFile::getViewString()
{
	if (_viewString == NULL)
	{
		_viewString = new TCHAR[2];
		_itot_s(getView() + 1, _viewString, 2, 10);
	}

	return _viewString;
}

void EditFile::setIndex(int view, int index)
{
	_view = view;
	_index = index;
	if (_indexString != NULL)
	{
		delete[] _indexString;
		_indexString = NULL;
	}

	if (_viewString != NULL)
	{
		delete[] _viewString;
		_viewString = NULL;
	}
}

void EditFile::clearIndexes()
{
	_view = -1;
	_index = -1;
}

int EditFile::getBufferID()
{
	return _bufferID;
}

FileStatus EditFile::getFileStatus()
{
	return _fileStatus;
}

void EditFile::setFileStatus(FileStatus status)
{
	_fileStatus = status;
}