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
	EditFile(int view, int index, CONST TCHAR *filename, int searchFlags, int bufferID);
	~EditFile(void);
	
	
	TCHAR* getFilename();
	TCHAR* getFullFilename();
	TCHAR* getPath();
	TCHAR* getIndexString();
	int getIndex();
	int getView();
	void setIndex(int view, int index);
	
	void clearIndexes();
	
	int getBufferID();
	FileStatus getFileStatus();
	void setFileStatus(FileStatus status);

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

};
