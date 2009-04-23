#include "stdafx.h"

#include "FileListView.h"
#include "resource.h"

FileListView::FileListView()
{
	
}

void FileListView::init(options_t *options, HINSTANCE hInst, HWND hParent, HWND hListView)
{
	_hListView = hListView; 
	_hHeader = ListView_GetHeader(_hListView); 
	_hInstance = hInst;
	_hParentWindow = hParent;
	
	if (LOBYTE(options->defaultSortOrder) == ALWAYSREMEMBER)
        _currentSortOrder = options->activeSortOrder;

	if (gWinVersion < WV_XP)
	{
		_bmpSortUp	 = (HBITMAP)::LoadImage(_hInstance, MAKEINTRESOURCE(IDB_SORTUP), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
		_bmpSortDown = (HBITMAP)::LoadImage(_hInstance, MAKEINTRESOURCE(IDB_SORTDOWN), IMAGE_BITMAP, 0, 0, LR_LOADMAP3DCOLORS);
	}


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
			returnValue = edit1->getIndex() - edit2->getIndex();
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

LRESULT FileListView::notify(WPARAM wParam, LPARAM lParam)
{
	switch (reinterpret_cast<LPNMHDR>(lParam)->code)
	{
		case LVN_GETDISPINFO:
		{
			NMLVDISPINFO* plvdi = reinterpret_cast<NMLVDISPINFO*>(lParam);    
			switch (plvdi->item.iSubItem)
			{
				case 0:
					plvdi->item.pszText = reinterpret_cast<EditFile*>(plvdi->item.lParam)->getFilename();
					break;
			        
				case 1:
					plvdi->item.pszText = reinterpret_cast<EditFile*>(plvdi->item.lParam)->getPath();
					break;
			
				case 2:
					TCHAR tmp[16];
					_itot(reinterpret_cast<EditFile*>(plvdi->item.lParam)->getIndex() + 1, tmp, 10);
					plvdi->item.pszText = tmp;
					break;
			
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
			return 0;
		}

		case NM_DBLCLK:
		{
			::SendMessage(_hParentWindow, FSM_ITEMDBLCLK, 0, 0);
			return 0;
		}
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
				hdItem.hbm   = (_currentSortOrder & REVERSE_SORT_ORDER) ? _bmpSortUp : _bmpSortDown;
			}
			
		}
		else
		{
			hdItem.fmt &= ~(HDF_SORTUP | HDF_SORTDOWN);
			if (i == LOBYTE(_currentSortOrder))
			{
				hdItem.fmt |= (_currentSortOrder & REVERSE_SORT_ORDER) ? HDF_SORTUP : HDF_SORTDOWN;
			}
		}
		Header_SetItem(_hHeader, i, &hdItem);
	}
}


int FileListView::getCurrentSortOrder()
{
	return _currentSortOrder;
}
