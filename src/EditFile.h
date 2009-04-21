#pragma once

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
	EditFile(int index, CONST TCHAR *filename, int searchFlags, int bufferID);
	~EditFile(void);
	
	
	TCHAR* getFilename();
	TCHAR* getFullFilename();
	TCHAR* getPath();
	int getIndex();
	void setIndex(int index);
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
	int _index;
	bool _dataSet;
	int _bufferID;
};
