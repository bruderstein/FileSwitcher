#ifndef _EDITFILE_H
#define _EDITFILE_H

#pragma once
#include <map>
#include "SearchOptions.h"
#include "Globals.h"

enum FileStatus
{
	SAVED,
	UNSAVED,
    READONLY
};

class EditFile
{
public:
    EditFile(void);
	EditFile(int view, int index, CONST TCHAR *filename, int searchFlags, int bufferID);
	~EditFile(void);

	TCHAR* getFilename();
	TCHAR* getFullFilename();
	TCHAR* getPath();
	TCHAR* getIndexString(BOOL includeView);
	TCHAR* getViewString();
	int    getIndex();
	int    getView();
	int    getBufferID();
	FileStatus getFileStatus();

	void   setIndex(int view, int index);
	void   setFileStatus(FileStatus status);
	void   clearIndexes();

private:
	TCHAR* _fullFilename;
	TCHAR* _searchFilename;
	TCHAR* _path;
	TCHAR* _filename;
	TCHAR* _display;
	FileStatus _fileStatus;

	bool _dataSet;
	int _bufferID;
	int _view;
	int _index;

	TCHAR* _indexString;
	TCHAR* _viewString;

	bool HasMultipleViews();
};

#endif