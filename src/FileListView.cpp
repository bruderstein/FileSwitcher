#include "stdafx.h"

#include "FileListView.h"
#include "resource.h"

FileListView::FileListView()
{
	
}

void FileListView::init(options_t *options, HINSTANCE hInst, HWND hParent, HWND hListView)
{
	_hListView = hListView; 
	_options = options;
	_hInstance = hInst;
	_hParentWindow = hParent;
	
	
	setupListViewHeader();
	_hHeader = ListView_GetHeader(_hListView); 
	
	if (LOBYTE(options->defaultSortOrder) == ALWAYSREMEMBER)
        _currentSortOrder = options->activeSortOrder;

	if (gWinVersion < WV_XP)
	{
		_bmpSortUp	 = (HBITMAP)::LoadImage(_hInstance, MAKEINTRESOURCE(IDB_SORTUP), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
		_bmpSortDown = (HBITMAP)::LoadImage(_hInstance, MAKEINTRESOURCE(IDB_SORTDOWN), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
	}

	_currentView = 0;

	setColumnOrder(_options->columnOrder);
	setColumnWidths(_options->columnWidths);

	ListView_SetImageList(_hListView, ghImgList, LVSIL_SMALL);
	updateHeader();
}

int FileListView::getCurrentSelectedIndex()
{ 
	return SendMessage(_hListView, LVM_GETNEXTITEM, -1, LVIS_SELECTED); 
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
	
	if (lParamSort & REVERSE_SORT_ORDER)
	{
		returnValue = -returnValue;
	}

	return returnValue;
			
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
	TCHAR tmp[20];
	int index;
	switch (reinterpret_cast<LPNMHDR>(lParam)->code)
	{
		case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* plvdi = reinterpret_cast<NMLVDISPINFO*>(lParam); 
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
					plvdi->item.pszText = editFile->getIndexString(_options->columnForView ? FALSE : TRUE);
					break;

				case 3:
					plvdi->item.pszText = editFile->getViewString();

				default:
					break;
			}
			return 0;
		}
		case LVN_COLUMNCLICK:
		{
			NMLISTVIEW *pnmv = (NMLISTVIEW*)lParam;
			
			if (LOBYTE(_currentSortOrder) == pnmv->iSubItem)
				_currentSortOrder ^= REVERSE_SORT_ORDER;
			else 
				_currentSortOrder = pnmv->iSubItem;

			sortItems(_currentSortOrder);

			_options->activeSortOrder = _currentSortOrder;

			return 0;
		}

		case NM_DBLCLK:
		{
			::SendMessage(_hParentWindow, FSM_ITEMDBLCLK, 0, 0);
			return 0;
		}

		case NM_SETFOCUS:
			::SendMessage(_hParentWindow, FSN_LISTVIEWSETFOCUS, 0, 0);
			break;

	}
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
			if (i == LOBYTE(_currentSortOrder))
			{
				hdItem.mask |= HDI_BITMAP;
				hdItem.fmt  |= (HDF_BITMAP | HDF_BITMAP_ON_RIGHT);
				hdItem.hbm   = (_currentSortOrder & REVERSE_SORT_ORDER) ? _bmpSortDown : _bmpSortUp;
			}
			
		}
		else
		{
			hdItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
			if (i == LOBYTE(_currentSortOrder))
			{
				hdItem.fmt |= (_currentSortOrder & REVERSE_SORT_ORDER) ? HDF_SORTDOWN : HDF_SORTUP;
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

	ListView_SetExtendedListViewStyle(_hListView, LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_DOUBLEBUFFER);

}

void FileListView::updateColumns(void)
{
	if (_columnForViewAdded)
	{
		ListView_DeleteColumn(_hListView, _viewColumn);
		_columnForViewAdded = FALSE;
	}

	if (_options->columnForView)
	{
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

TCHAR *FileListView::getColumnOrderString(TCHAR *buffer, int bufferSize)
{
	int columnOrder[4];
	int columns;
	

	if (_options->columnForView)
		columns = 4;
	else
		columns = 3;

	

	ListView_GetColumnOrderArray(_hListView, columns, &columnOrder);
	
	// empty the buffer
	_tcscpy(buffer, _T(""));

	TCHAR tmp[2];

	for(int index = 0; index < columns && index < bufferSize; index++)
	{
		_tcscat(buffer, _itot(columnOrder[index], tmp, 10));
	}

	return buffer;
}


TCHAR *FileListView::getColumnWidths(TCHAR *buffer, int bufferSize)
{
	int columnWidth;
	int columns;
	

	if (_options->columnForView)
		columns = 4;
	else
		columns = 3;

	TCHAR currentColumn[10];
	_tcscpy(buffer, _T(""));

	int currentLength = 0;
	
	for(int columnIndex = 0; columnIndex < columns; columnIndex++)
	{
		columnWidth = ListView_GetColumnWidth(_hListView, columnIndex);
		
		if (bufferSize > (currentLength + 1))
		{
			if (columnIndex > 0)
			{
				_tcscat(buffer, _T(","));
				currentLength++;
			}
		}
		else 
			break;

		

		_itot(columnWidth, currentColumn, 10);
		
		if (bufferSize > (currentLength + _tcslen(currentColumn)))
		{
			_tcscat(buffer, currentColumn);
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
	int index = 0;
	width = _tcstok(columnWidths, _T(","));
	int iWidth;

	while(NULL != width)
	{
		iWidth = _ttoi(width);
		ListView_SetColumnWidth(_hListView, index, iWidth);

		width = _tcstok(NULL, _T(","));
		++index;
	}

}
	

void FileListView::setColumnOrder(TCHAR *columnOrder)
{
	int columnOrderInt[4];


	int columnsSupplied = _tcslen(columnOrder);
	TCHAR columnIndex[2];
	columnIndex[1] = '\0';

	for(int index = 0; index < columnsSupplied; index++)
	{
		columnIndex[0] = columnOrder[index];
		columnOrderInt[index] = _ttoi(columnIndex);
	}
	
	ListView_SetColumnOrderArray(_hListView, columnsSupplied, columnOrderInt);
}
