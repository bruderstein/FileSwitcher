#pragma once
#include <map>

#include "SearchOptions.h"


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
	EditFile(int view, int index, CONST TCHAR *filename, int searchFlags, int bufferID, void* scintillaDoc);
	~EditFile(void);
	
	
	TCHAR* getFilename();
	TCHAR* getFullFilename();
	TCHAR* getPath();
	TCHAR* getIndexString(BOOL includeView);
	TCHAR* getViewString();
	void*  getScintillaDoc();
	int    getIndex();
	int    getView();
	int    getBufferID();
	FileStatus getFileStatus();
	
	void   setIndex(int view, int index);
	void   setFileStatus(FileStatus status);
	void   setScintillaDoc(void* scintillaDoc);
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

	void* _scintillaDoc;

};
