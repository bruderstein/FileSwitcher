#include "StdAfx.h"
#include <tchar.h>
#include "EditFile.h"

EditFile::EditFile(void)
{
	_dataSet = false;
	_indexString = NULL;
	_viewString = NULL;
}

EditFile::EditFile(int view, int index, CONST TCHAR* filename, int searchFlags, int bufferID, void* scintillaDoc)
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

	
	_dataSet = true;
	_indexString = NULL;
	_viewString = NULL;
	setIndex(view, index);
	_bufferID = bufferID;
	_fileStatus = SAVED;
	_scintillaDoc = scintillaDoc;

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

TCHAR *EditFile::getIndexString(BOOL includeView)
{
	if (_indexString == NULL)
	{
		TCHAR tmp[16];
		_itot(_index + 1, tmp, 10);
		int length = _tcslen(tmp);

		if (_view == 1 && includeView)
			length += 4;
	
		_indexString = new TCHAR[length + 1];

		if (_view == 1 && includeView)
		{
			_tcscpy(_indexString, _T("[2] "));
			_tcscat(_indexString, tmp);
		}
		else
			_tcscpy(_indexString, tmp);

	
	}
	return _indexString;
}



int EditFile::getView()
{
		return _view;
}

TCHAR *EditFile::getViewString()
{
	if (_viewString == NULL)
	{
		_viewString = new TCHAR[2];
		_itot(_view + 1, _viewString, 10);
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

void* EditFile::getScintillaDoc(void)
{
	return _scintillaDoc;
}

void EditFile::setScintillaDoc(void* scintillaDoc)
{
	_scintillaDoc = scintillaDoc;
}
