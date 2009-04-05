#pragma once


#include "SearchOptions.h"




class EditFile
{

public:
    EditFile(void);
	EditFile(int index, CONST TCHAR *filename, int searchFlags, int bufferID);
	~EditFile(void);
	
	
	TCHAR* getFilename();
	TCHAR* getFullFilename();
	TCHAR* getSearchFilename();
	TCHAR* getDisplayString();
	int getIndex();
	int getBufferID();

private:
	TCHAR* _fullFilename;
	TCHAR* _searchFilename;
	TCHAR* _path;
	TCHAR* _filename;
	TCHAR* _display;
	int _index;
	bool _dataSet;
	int _bufferID;
};
