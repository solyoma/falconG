#pragma once

#include <QObject>
#include <QtWidgets>

#include "support.h"

/*=============================================================
 * Thumbnail Mime Data Type for Drag & Drop
 * amends the QMimeData with the list of indices of selected
 * items in thumbnail view
 * Set only if the drag started from inside the applicaton,
 *  otherwise we use the url list in the QMimeData to get the
 *  file names from external source
 * Doesn't contain album IDs, they must be set in the DropHandler
 *------------------------------------------------------------*/
class ThumbMimeData : public QMimeData
{
	Q_OBJECT
public:

	IntList thumbList;  // indices in actual album's 'items' for all selected items
	// IDs must be are added to this list with the << operator
	ThumbMimeData() {
		QMimeData::setData("application/x-thumb", "thumb"); // my type
	}
};

/*=============================================================
 * Drop Handler to drop items from one album to another or from
 *	external sources like file explorer
 * a QObject derived class is needed for the message system to work,
 *------------------------------------------------------------*/
struct Album;	// (in albums.h) for pointers
class AlbumTreeView;
class ThumbnailView;

class DropHandler : public QObject
{
	Q_OBJECT
public:
public:
	DropHandler() :
		_pAlbumTreeView(nullptr), _pThumbnailView(nullptr),
		_row(-1), _mimeData(nullptr), _event(nullptr)
	{
	}
	void Init(AlbumTreeView* ptv, ThumbnailView* ptnv)
	{
		_pAlbumTreeView = ptv; 
		_pThumbnailView = ptnv;
	}
	// Setup must be called before drop
	void Setup(QDropEvent* event, IDVal_t srcAlbumId, IDVal_t destAlbumId, int row = -1);
	void DropItems(bool notFromTreeView);
	bool MoveItems(Album* pSrc, Album* pDest, const IntList& ids) const; // from pSrc to pdest, returns if any items were moved

	static bool IsAllowedTypeToDrop(const QDropEvent* event);

signals:
	void SignalAlbumStructWillChange();
	void SignalAlbumStructChanged(bool yes);
	void SignalInProcessing(bool yes);
	void SignalSaveTreeViewExpandedState(); // save expanded state of tree view before changing album structure
	void SignalRestoreTreeViewExpandedState(); // save expanded state of tree view before changing album structure
private:
	void _AddImagesAndVideosFromList(QStringList qslFileNames, bool onlyNew);
	bool _AddFoldersFromList(QStringList qslFolders);
	bool _AddFolder(QString folderName);	// returns if folder was added
	void _FromExternalSource();
	void _FromInternalSource(bool fromThumbnailView);

	AlbumTreeView* _pAlbumTreeView;			// we are friends ;)
	ThumbnailView* _pThumbnailView;

	AlbumPointers _albumPointers;

	int _row;			// drop position in destination album, -1 for drop at the end of the list

	const ThumbMimeData* _mimeData;
	const QDropEvent* _event;

	bool _atLeastOneFolderWasAdded = false;
};

extern DropHandler dropHandler;
