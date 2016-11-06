#include "precompiledHeaders.h"

#include "FileListView.h"
#include "resource.h"
#include <typeinfo>
#include <string>

FileListView::FileListView()
{
}

void FileListView::init(options_t *options, HINSTANCE hInst, HWND hParent, HWND hListView)
{
	_hListView = hListView;
	_options = options;
	_hInstance = hInst;
	_hParentWindow = hParent;

	if(_options->hasConfiguredContext
		&& !_ctrlTabSearch)
	{
		ListView_SetImageList(_hListView, NULL, LVSIL_SMALL);
	}
	else
	{
		ListView_SetImageList(_hListView, ghImgList, LVSIL_SMALL);
	}

	setupListViewHeader();
	_hHeader = ListView_GetHeader(_hListView);

	if (gWinVersion < WV_XP)
	{
		_bmpSortUp	 = (HBITMAP)::LoadImage(_hInstance, MAKEINTRESOURCE(IDB_SORTUP), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
		_bmpSortDown = (HBITMAP)::LoadImage(_hInstance, MAKEINTRESOURCE(IDB_SORTDOWN), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
	}

	_currentView = 0;

	setColumnOrder(_options->columnOrder);

	if(g_options.hasConfiguredContext
		&& !_ctrlTabSearch)
	{
		::GetPrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNWIDTHSSIMPLE, _T("200,300"), g_options.columnWidths, COLUMNWIDTH_LENGTH, iniFilePath);
	}
	else
	{
		::GetPrivateProfileString(LISTVIEW_SETTINGS, KEY_COLUMNWIDTHS, _T("160,100,60"), g_options.columnWidths, COLUMNWIDTH_LENGTH, iniFilePath);
	}
	setColumnWidths(_options->columnWidths);

	updateHeader();
}

int FileListView::getCurrentSelectedIndex()
{
	// This is always used as the value for LVITEM::iItem, which is int.
	return (int)SendMessage(_hListView, LVM_GETNEXTITEM, -1, LVIS_SELECTED);
}

SimpleFileInfo* FileListView::getCurrentSimpleFile(void)
{
	LVITEM item;
	item.mask = LVIF_PARAM;
	item.iItem = getCurrentSelectedIndex();

	if (item.iItem == -1)
		return NULL;

	::SendMessage(_hListView, LVM_GETITEM, 0, reinterpret_cast<LPARAM>(&item));
	return reinterpret_cast<SimpleFileInfo*>(item.lParam);
}

EditFile* FileListView::getCurrentEditFile(void)
{
	LVITEM item;
	item.mask = LVIF_PARAM;
	item.iItem = getCurrentSelectedIndex();

	if (item.iItem == -1)
		return NULL;

	::SendMessage(_hListView, LVM_GETITEM, 0, reinterpret_cast<LPARAM>(&item));
	return reinterpret_cast<EditFile*>(item.lParam);
}

int CALLBACK FileListView::listViewSimpleFileComparer(void* lParam1, void* lParam2, LPARAM lParamSort)
{
	SimpleFileInfo *edit1 = static_cast<SimpleFileInfo *>(lParam1);
	SimpleFileInfo *edit2 = static_cast<SimpleFileInfo *>(lParam2);

	int returnValue;

	switch(LOBYTE(lParamSort))
	{
		case PATH:
		{
			returnValue = _tcsicmp(edit1->filePath.c_str(), edit2->filePath.c_str());
			if (returnValue == 0)
				returnValue = _tcsicmp(edit1->fileName.c_str(), edit2->fileName.c_str());

			break;
		}
		case FILENAME:
		default:
		{
			returnValue = _tcsicmp(edit1->fileName.c_str(), edit2->fileName.c_str());
			if (returnValue == 0)
				returnValue = _tcsicmp(edit1->filePath.c_str(), edit2->filePath.c_str());

			break;
		}
	}

	return g_options.activeReversedSortOrder ? -returnValue : returnValue;
}

int CALLBACK FileListView::listViewComparer(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
	EditFile *edit1 = reinterpret_cast<EditFile *>(lParam1);
	EditFile *edit2 = reinterpret_cast<EditFile *>(lParam2);

	int returnValue;

	switch(LOBYTE(lParamSort))
	{
		case PATH:
			returnValue = _tcsicmp(edit1->getPath(), edit2->getPath());
			if (returnValue == 0)
				returnValue = _tcsicmp(edit1->getFilename(), edit2->getFilename());

			break;

		case INDEX:

			returnValue = edit1->getView() - edit2->getView();
			if (returnValue == 0)
			{
				returnValue = edit1->getIndex() - edit2->getIndex();
			}

			break;

		case 3:

			returnValue = edit1->getView() - edit2->getView();
			if (returnValue == 0)
			{
				returnValue = edit1->getIndex() - edit2->getIndex();
			}
			break;

		case FILENAME:

		default:
			returnValue = _tcsicmp(edit1->getFilename(), edit2->getFilename());
			if (returnValue == 0)
				returnValue = _tcsicmp(edit1->getPath(), edit2->getPath());
			break;
	}

	return g_options.activeReversedSortOrder ? -returnValue : returnValue;
}

void FileListView::sortSimpleFileItems(int sortOrder)
{
	_currentSortOrder = sortOrder;
	updateHeader();
	sortSimpleFileItems();
}

void FileListView::sortSimpleFileItems()
{
	ListView_SortItems(_hListView, FileListView::listViewSimpleFileComparer, _currentSortOrder);
}

void FileListView::sortItems(int sortOrder)
{
	_currentSortOrder = sortOrder;
	updateHeader();
	sortItems();
}

void FileListView::sortItems()
{
	ListView_SortItems(_hListView, FileListView::listViewComparer, _currentSortOrder);
}

void FileListView::setCurrentView(int currentView)
{
	_currentView = currentView;
}

LRESULT FileListView::notify(WPARAM wParam, LPARAM lParam)
{
	switch (reinterpret_cast<LPNMHDR>(lParam)->code)
	{
		case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* plvdi = reinterpret_cast<NMLVDISPINFO*>(lParam);
			if(_options->hasConfiguredContext
				&& !_ctrlTabSearch)
			{
				SimpleFileInfo *simpleFile = reinterpret_cast<SimpleFileInfo*>(plvdi->item.lParam);

				switch (plvdi->item.iSubItem)
				{
					case 0:
					{
						TCHAR result[MAX_PATH];
						_tcscpy_s(result, sizeof(result), simpleFile->fileName.c_str());
						plvdi->item.pszText = result;
						break;
					}
					case 1:
					{
						TCHAR result[MAX_PATH];
						_tcscpy_s(result, sizeof(result), simpleFile->filePath.c_str());
						plvdi->item.pszText = result;
						break;
					}
					default:
						break;
				}
				return 0;
			}
			else
			{
				EditFile *editFile = reinterpret_cast<EditFile*>(plvdi->item.lParam);

				switch (plvdi->item.iSubItem)
				{
					case 0:
						plvdi->item.pszText = editFile->getFilename();
						break;

					case 1:
						plvdi->item.pszText = editFile->getPath();
						break;

					case 2:
						plvdi->item.pszText = editFile->getIndexString(!_options->columnForView);
						break;

					case 3:
						plvdi->item.pszText = editFile->getViewString();

					default:
						break;
				}
				return 0;
			}
		}
		case LVN_COLUMNCLICK:
		{
			_options->activeReversedSortOrder = !_options->activeReversedSortOrder;

			NMLISTVIEW *pnmv = (NMLISTVIEW*)lParam;

			_options->activeSortOrder = _currentSortOrder = pnmv->iSubItem;

			if(_options->hasConfiguredContext
				&& !_ctrlTabSearch)
			{
				sortSimpleFileItems(_currentSortOrder);
			}
			else
			{
				sortItems(_currentSortOrder);
			}

			return 0;
		}

		case NM_DBLCLK:
		{
			::SendMessage(_hParentWindow, FSM_ITEMDBLCLK, 0, 0);
			return 0;
		}

		case NM_SETFOCUS:
			::SendMessage(_hParentWindow, FSN_LISTVIEWSETFOCUS, 0, 0);
			return 0;
	}
	return 0;
}

void FileListView::updateHeader(void)
{
	HDITEM	hdItem		= {0};
	UINT	uMaxHeader	= Header_GetItemCount(_hHeader);

	for (UINT i = 0; i < uMaxHeader; i++)
	{
		hdItem.mask	= HDI_FORMAT;
		Header_GetItem(_hHeader, i, &hdItem);

		if (gWinVersion < WV_XP)
		{
			hdItem.mask &= ~HDI_BITMAP;

			hdItem.fmt  &= ~(HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
			if (i == _currentSortOrder)
			{
				hdItem.mask |= HDI_BITMAP;
				hdItem.fmt  |= (HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
				hdItem.hbm = _options->reversedSortOrder ? _bmpSortDown : _bmpSortUp;
			}
		}
		else
		{
			hdItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
			if (i == _currentSortOrder)
			{
				hdItem.fmt |= _options->activeReversedSortOrder ? HDF_SORTDOWN : HDF_SORTUP;
			}
		}
		Header_SetItem(_hHeader, i, &hdItem);
	}
}

int FileListView::getCurrentSortOrder()
{
	return _currentSortOrder;
}

void FileListView::setupListViewHeader(void)
{
	LVCOLUMN col;
	col.mask		= LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
	col.fmt			= LVCFMT_LEFT;
	col.iSubItem	= 0;
	col.cx			= 150;
	col.pszText = _T("Filename");
	ListView_InsertColumn(_hListView, 0, &col);

	col.iSubItem	= 1;
	col.cx			= 200;
	col.pszText		= _T("Path");
	ListView_InsertColumn(_hListView, 1, &col);

	if(!_options->hasConfiguredContext
		|| _ctrlTabSearch)
	{
		col.iSubItem	= 2;
		col.cx			= 40;
		col.pszText		= _T("Index");
		col.fmt			= LVCFMT_RIGHT;
		ListView_InsertColumn(_hListView, 2, &col);

		if (_options->columnForView)
		{
			_columnForViewAdded = TRUE;
			_viewColumn			= 3;
			col.iSubItem	= _viewColumn;
			col.cx			= 40;
			col.pszText		= _T("View");
			col.fmt			= LVCFMT_RIGHT;
			ListView_InsertColumn(_hListView, _viewColumn, &col);
		}
	}

	ListView_SetExtendedListViewStyle(_hListView, LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_DOUBLEBUFFER);
}

void FileListView::updateColumns(void)
{
	if (_columnForViewAdded)
	{
		ListView_DeleteColumn(_hListView, _viewColumn);
		_columnForViewAdded = FALSE;
	}

	if (!_options->hasConfiguredContext
		&& _options->columnForView
	){
		LVCOLUMN col;
		col.mask		= LVCF_TEXT | LVCF_FMT | LVCF_WIDTH;
		col.fmt			= LVCFMT_RIGHT;

		_viewColumn = 3;
		_columnForViewAdded = TRUE;
		col.iSubItem	= _viewColumn;
		col.cx			= 40;
		col.pszText		= _T("View");
		col.fmt			= LVCFMT_RIGHT;
		ListView_InsertColumn(_hListView, _viewColumn, &col);
	}
}

int FileListView::getColumnSize()
{
	return Header_GetItemCount(_hListView);
}

TCHAR *FileListView::getColumnOrderString(TCHAR *buffer, int bufferSize)
{
	int columnOrder[4];
	int columns = getColumnSize();

	ListView_GetColumnOrderArray(_hListView, columns, &columnOrder);

	// empty the buffer
	_tcscpy_s(buffer, bufferSize, _T(""));

	TCHAR tmp[2];

	for(int index = 0; index < columns && index < bufferSize; index++)
	{
		_itot_s(columnOrder[index], tmp, 2, 10);
		_tcscat_s(buffer, bufferSize, tmp);
	}

	return buffer;
}

TCHAR *FileListView::getColumnWidths(TCHAR *buffer, size_t bufferSize)
{
	int columnWidth;
	int columns = getColumnSize();

	TCHAR currentColumn[10];
	_tcscpy_s(buffer, bufferSize, _T(""));

	size_t currentLength = 0;

	for(int columnIndex = 0; columnIndex < columns; columnIndex++)
	{
		columnWidth = ListView_GetColumnWidth(_hListView, columnIndex);

		if (bufferSize > (currentLength + 1))
		{
			if (columnIndex > 0)
			{
				_tcscat_s(buffer, bufferSize, _T(","));
				currentLength++;
			}
		}
		else
			break;

		_itot_s(columnWidth, currentColumn, 10, 10);

		if (bufferSize > (currentLength + _tcslen(currentColumn)))
		{
			_tcscat_s(buffer, bufferSize, currentColumn);
			currentLength += _tcslen(currentColumn);
		}
		else
			break;
	}

	return buffer;
}

void FileListView::setColumnWidths(TCHAR *columnWidths)
{
	TCHAR *width;
	TCHAR *context;
	int index = 0;
	width = _tcstok_s(columnWidths, _T(","), &context);
	int iWidth;

	while(NULL != width)
	{
		iWidth = _ttoi(width);
		ListView_SetColumnWidth(_hListView, index, iWidth);

		width = _tcstok_s(NULL, _T(","), &context);
		++index;
	}
}

void FileListView::setColumnOrder(TCHAR *columnOrder)
{
	int columnOrderInt[4];

	size_t columnsSupplied = _tcslen(columnOrder);
	TCHAR columnIndex[2];
	columnIndex[1] = '\0';

	for(size_t index = 0; index < columnsSupplied; index++)
	{
		columnIndex[0] = columnOrder[index];
		columnOrderInt[index] = _ttoi(columnIndex);
	}

	ListView_SetColumnOrderArray(_hListView, columnsSupplied, columnOrderInt);
}