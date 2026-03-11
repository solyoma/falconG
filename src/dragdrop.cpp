#include "dragdrop.h"

#include <QString>
#include <QStringList>
#include <QMessageBox>

#include "support.h"
#include "albums.h"
#include "thumbnailView.h"

extern QWidget* frmMain;	// global pointer to main window, for message boxes

/*------------------- global drop handler -----------------*/

DropHandler dropHandler;

/*=============================================================
 * TASK:	signal if suitable mime data to drop is available
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: does not check if the files or folders in an url list
 *          are acceptable, just looks at the allowed types
 *------------------------------------------------------------*/
bool DropHandler::IsAllowedTypeToDrop(const QDropEvent* event)
{
	// DEBUG
	//qDebug() << "Mime text: " << event->mimeData()->text() 
	//         << ", hasUrls ? " << event->mimeData()->hasUrls()
	//         << "mimeData is null?" << (event->mimeData() ? "no":"yes")
	//         << ", hasImage ? " << event->mimeData()->hasImage()
	//         << ", x-thumb ? " << event->mimeData()->hasFormat("application/x-thumb")
	//    ;
	// qDebug() << "IsAllowedTypeToDrop: row=" << indexAt(event->pos()).row();

	// DEBUG
	QString qs = event->mimeData()->text();
	/*bool b = event->mimeData()->hasUrls(),
		b1 = event->mimeData()->hasImage(),
		b3 = event->mimeData()->hasFormat("application/x-thumb");*/
		// /DEBUG
	return /* (indexAt(event->pos()).row() >= 0) && */
		(event->mimeData()->hasUrls() ||    // e.g. text() == file:///I:/alma.jpg, or text() == file:///I:/folderName, each nami in its own line
			event->mimeData()->hasImage() ||    // image/...
			event->mimeData()->hasFormat("application/x-thumb")     // drag and drop inside this application
			);
}

/*=============================================================
 * TASK:   add files to 
 *			_imageMap or _videoMap,
 *			_pDestAlbum->items
 * 
 * PARAMS:  qslFileNames - file names to add
 *          onlyNew - do not add duplicates to the same album
 * GLOBALS:	_pDestAlbum	- destination album, not an alias
 *          _row - add before this row
 * RETURNS:
 * REMARKS: - no thumbnail flag is set for any of the icons
 *          - the same image may appear in any number of albums,
 *             but not twice in the same album
 *------------------------------------------------------------*/
void DropHandler::_AddImagesAndVideosFromList(QStringList qslFileNames, bool onlyNew)
{
	albumgen.AddImagesAndVideosFromList(_albumPointers.destAlbumId, qslFileNames, onlyNew, _row);
}

/*=============================================================
 * TASK:   add folders to destination album from list of folder names
 * PARAMS:	qslFolders - list of folder names to add
 * GLOBALS:	_albumPointers.destBaseAlbumId 
 *			_albumPointers.row - if >=0 add before this row, if -1: add after existing rows
 * RETURNS:	if at least one folder added
 * REMARKS:	- destination album is a base (non alias) album
 *------------------------------------------------------------*/
bool DropHandler::_AddFoldersFromList(QStringList qslFolders)
{
	return _atLeastOneFolderWasAdded = albumgen.AddFoldersFromList(_albumPointers.destBaseAlbumId, qslFolders);
}
/*=============================================================
 * TASK:    internal function that adds a folder recursively
 *
 * PARAMS:  folderName - full path name of folder to add
 * GLOBALS: _pDestAlbum - destination album. Never an alias album,
 *			_row - if >=0 add before this row, if -1: add after existing rows
 *				should be set using 'selectionModel()->hasSelection() ? currentIndex().row() : -1'
 *				in ThumbNaliView::dropEvent() into DropHandler before calling this function
 * RETURNS: if folder was added
 * REMARKS:
 *------------------------------------------------------------*/
bool DropHandler::_AddFolder(QString folderName)
{
	bool added, atLeastOneFolderWasAdded = false;
	emit SignalInProcessing(true);
	emit SignalSaveTreeViewExpandedState(); // save expanded state of tree view before changing album structure
	emit SignalAlbumStructWillChange();

	IDVal_t idVal = _albumPointers.pDestAlbum->ID.Val();
	ID_t id = albumgen.Albums().Add(idVal, folderName, added);     // set new dirIndex too
	if (added)
	{
		atLeastOneFolderWasAdded = true;
		_albumPointers.SetupPointers();		// album position may have changed when new album was added to map
		albumgen.AddToModifiedList(*_albumPointers.pDestAlbum);
		Album& album = *albumgen.AlbumForID(id);
		album.parentId = _albumPointers.pDestAlbum->ID.Val();
		albumgen.AddDirsRecursively(id);

		// already added _pDestAlbum->AddItem(id);

		ID_t idth = album.ThumbID();
		if (idth.Val())
			folderName.clear();
		else
			folderName = albumgen.Images()[idth.Val()].FullSourceName();
		(void)fileIcons.Insert(_row, folderName);

		albumgen.WriteDirStruct(AlbumGenerator::BackupMode::bmKeepBackupFile, AlbumGenerator::WriteMode::wmOnlyIfChanged);
	}

	emit SignalRestoreTreeViewExpandedState(); // restore expanded state of tree view after changing album structure
	emit SignalInProcessing(false);
	emit SignalAlbumStructChanged(true);

	return atLeastOneFolderWasAdded;
}

/*=============================================================
 * TASK   :	 Drop images from clipboard into any folder
 * PARAMS :
 * EXPECTS:	- _srcId is NO_ID,
 *			- event->mimeData() is not a ThumbMimeData and has urls
 * GLOBALS:	_pDestId - destination album id
 *			_row = -1
 *			thumbNailView->AlbumId()
 * RETURNS:	none
 * REMARKS:	- no need to check the source folder
 *			- can't drop items onto an alias album, but can
 *				drop them into its base album
 *			- items are dropped at a position after the
 *				existing items, even when those are folders
 *				As folders are saved after images this won't matter
 *			- if the destination album is the actual one in
 *				thumbnailView then the display must be refreshed
 *------------------------------------------------------------*/
void DropHandler::_FromExternalSource()
{
	QStringList qsl, qslF;      // for files and Folders
	auto file_type = [&](QString s) {
		QFileInfo fi(s);
		if (fi.isDir())
		{
			return 0;   // folder
		}
		else // file
		{
			QString ext;
			int ld = s.lastIndexOf('.');
			if (ld >= 0)
				ext = s.mid(ld + 1);
			return ext == "jpg" || ext == "png" || ext == "mp4" || ext == "ogg" ? 1 : -1;   // file
		}
		return -1; // can't use'
		};

	// prepare list of files and folders to add
	for (auto& u : _event->mimeData()->urls())
	{
		auto s = u.toLocalFile();
		int w = file_type(s);
		if (w > 0)      // file
			qsl << s;
		else if (!w)    // folder
			qslF << s;
	}

	IDVal_t idOfDisplayedAlbum = _pThumbnailView->AlbumID();	// although we are friends and could use _albumId of thumbView, but this is constexpr so why not
	Q_ASSERT(_albumPointers.pDestAlbum);

	if (_albumPointers.pDest != _albumPointers.pDestAlbum) // then it is an alias for another album
	{
		int res = QuestionDialog(tr("falconG - Warning"), tr("You cannot drop items into an alias album.\n"
			"Do you want to drop them into the base album\n'%1'\ninstead?")
			.arg(_albumPointers.pDestAlbum->name),
			dboAskToMoveIntoBaseAlbum,
			frmMain,
			"",
			QMessageBox::Yes | QMessageBox::No
		);
		if (res != QMessageBox::Yes)
			return;
	}
	// check if the destination album is the actual one in thumbnailView. If so then the display must be refreshed
	_AddImagesAndVideosFromList(qsl, true);	// do not add duplicates to the same album
	(void)_AddFoldersFromList(qslF);
}
/*=============================================================
 * TASK   :	move items from one album into the same album or into
 *			another
 * PARAMS :	bool fromThumbView: means a move inside thumbnailView
 *				so _srcId == _destId
 * EXPECTS:
 * GLOBALS:	_pSrc - source album, may be null for external source and
 *					 may be an alias album for internal source
 *			_pSrcAlbum - source album not an alias,
 *			_pDest - destination album, may be an alias album
 *			_pDestAlbum - destination album id may equal to _pSrcAlbum
 *					not an alias
 *			_row - row to move before (or -1 to move to the end)
 * RETURNS:
 * REMARKS:	- destination album can only differ from the source when
 *			  called from the tree View
 *			- when they are the same but fromThumbView is false
 *				the move isn't possible
 *------------------------------------------------------------*/
void DropHandler::_FromInternalSource(bool fromThumbView)
{
	IntList thl = ((const ThumbMimeData*)_mimeData)->thumbList;  // indices in source album's items to move

	Q_ASSERT(_albumPointers.pDestAlbum);

	if ( (_albumPointers.pSrc != _albumPointers.pSrcAlbum) || (_albumPointers.pDest != _albumPointers.pDestAlbum)) // then either pSrc or pDest or both is an alias for another album
	{
		int res = QuestionDialog(tr("falconG - Warning"), tr("You cannot drop items into or from an alias album.\n"
			"Do you want to drop them from %1 into '%2' instead?")
			.arg(_albumPointers.pSrcAlbum->name)
			.arg(_albumPointers.pDestAlbum->name),
			dboAskToMoveFromOrIntoBaseAlbum,
			frmMain,
			"",
			QMessageBox::Yes | QMessageBox::No
		);
		if (res != QMessageBox::Yes)
			return;
	}

	IdList& items = _albumPointers.pSrcAlbum->items;   // original ordered items

	int itemSize = items.size();   // only changes when items dropped on other albums

	QVector<int> itemOrder;        // new item order indexes

	// if the items are dropped to the tree view then _row = -1, so no question what to do
	bool moveItemsIntoFolder = _row >= 0 && (items[_row].IsAlbum());   // now just see if the items moved above an album

	bool doWriteStructFile = true;                   // default: true
	if (moveItemsIntoFolder) // then possibly move items into album with id items[row]
	{                        // or relocate them or cancel operation                     
		// only comes here if drop started from and ended in the thumbnail view
		// check if the destination album is also in the list
		if (thl.indexOf(_row) >= 0)      // then list contains the folder to drop into
		{
			doWriteStructFile = false;                   // default: true
			if (thl.size() > 1)         // otherwise a single album is dropped into itself, so the drop is just cancelled
			{
				QString msg = thl.size() == 1 ? tr("An album cannot be dropped into itself!") : tr("List of items to drop contains the album to drop into!");
				QMessageBox::warning(frmMain, tr("falconG - Warning"), msg);
			}
		}
		else
		{
			QMessageBox mb(frmMain);
			mb.setWindowTitle(tr("falconG - Question"));
			mb.setText(tr("Reposition selection before this folder or Move into it?"));
			mb.setInformativeText(tr("Press 'Cancel' to discard possible position changes."));
			// buttons added after the existing buttons
			QPushButton* pBeforeFolderBtn = mb.addButton(tr("Re&position"), QMessageBox::YesRole); // YesRole comes first, no after and cancel after that
			QPushButton* pIntoFolderBtn = mb.addButton(tr("&Move into"), QMessageBox::NoRole);

#ifdef DEBUG
			QPushButton* pCancelBtn =
#endif
				mb.addButton(tr("Cancel"), QMessageBox::RejectRole);
			mb.setDefaultButton(pBeforeFolderBtn);

			mb.exec();
			QAbstractButton* pResBtn = mb.clickedButton();

			if (pResBtn == pIntoFolderBtn)
				MoveItems(_albumPointers.pSrcAlbum, _albumPointers.pDestAlbum, thl); // mimeData->hasUrls() == true : drop from External source, already handled
			else if (pResBtn == pBeforeFolderBtn)
				moveItemsIntoFolder = false;                 // default: true
			else        // else cancel is pressed
				doWriteStructFile = false;                   // default: true
		}
	}

	if (doWriteStructFile)
	{
		if (!moveItemsIntoFolder)    // = just relocate, move before selected item (or after the last one)
		{
			itemOrder.resize(itemSize);     // and original indexes are 0,1,2...

			int si = 0,     // original index
				di = 0;     // index in idl

			// here row is: when >= 0 -> row to insert items before, when < 0 -> move to the end
			if (_row < 0)
				_row = itemSize;

			for (; si < itemSize && si < _row; ++si)
				if (thl.indexOf(si) < 0)
					itemOrder[di++] = si;

			for (int i = 0; i < thl.size(); ++i)
				itemOrder[di++] = thl[i];

			for (si = _row; si < itemSize; ++si)
				if (thl.indexOf(si) < 0)
					itemOrder[di++] = si;

			// new order in 'itemOrder' set
			IdList idl;                         // new ordered items
			idl.resize(itemSize);

			for (int i = 0; i < itemOrder.size(); ++i)
				idl[i] = items[itemOrder[i]];

			// modify original stored itemOrder
			const QVector<int>& origIconOrder = fileIcons.IconOrder();  // original order might have been changed
			// so we must rearrange that according to 'itemOrder'
			QVector<int> iconOrder;                          // new icon order indexes
			iconOrder.resize(itemSize);
			for (int i = 0; i < itemOrder.size(); ++i)
				iconOrder[i] = origIconOrder[itemOrder[i]];

			fileIcons.SetIconOrder(iconOrder);
			items = idl;
		}
		// common for relocate and move
		albumgen.AddToModifiedList(*_albumPointers.pDest);			// may  be an alias
		if(_albumPointers.pDest != _albumPointers.pDestAlbum)
			albumgen.AddToModifiedList(*_albumPointers.pDestAlbum);	// also add the base album when destination is an alias

		albumgen.WriteDirStruct(AlbumGenerator::BackupMode::bmKeepBackupFile, AlbumGenerator::WriteMode::wmOnlyIfChanged);
	}
}

/*=============================================================
 * TASK   : moves items from source to destination album
 *
 * PARAMS :	pSrc - source album can't be nullptr or an alias album,
 *									  but can be the base album of an alias album
 *			pDest - destination album	- " -
 * EXPECTS:	all alias substitutions for source and destination albums are done
 * GLOBALS:
 * RETURNS:	true: move successfull, false move failed
 * REMARKS:	- doesn't called when image or video dropped from external source
 *			- pDest is checked against circular moves, the list may not contain
 *				the destination album or any of its aliases,
 *				?? but may contain the source album or any of its aliases ??
 *
 *			- only moves if the destination is a folder and the
 *				items are not already in the destination album
 *			- doesn't refresh the actual album's thumbnails even
 *				if the move is successfull, so the caller should do that
 *			- doesn't mark alias albums as changed, so the caller should do that if needed
 *
 *------------------------------------------------------------*/
bool DropHandler::MoveItems(Album* pSrc, Album* pDest, const IntList& thl) const
{
	IdList itemsDest = pDest->items;
#ifdef DEBUG
	int destItemSize = itemsDest.size();
#endif

	// -- check if any of the items to move is a folder and if it is then
	//    whether it or any of its siblings are already in the destination album
	for (int i = 0; i < thl.size(); ++i)
	{
		ID_t itemID = pSrc->items[thl[i]];
		if (itemID.IsAlbum())
		{
			Album* pab = albumgen.AlbumForID(itemID);
			Q_ASSERT(pab);

			if (albumgen.IsCircular(pab, pDest)) //         if (pab->BaseAlbum()->ID.Val() == pAlbum->BaseAlbum()->ID.Val())
			{
				QMessageBox::warning(frmMain, tr("falconG - Warning"), tr("Ivalid move!\n"
					"Album \n'%1'\n is either an alias for album\n'%2'\n"
					"or they are aliases of the same album.\nCancelling move.").arg(pab->name).arg(pSrc->name));
				return false;
			}

			// now the album to be moved is not an alias of the album to move to.
			IDVal_t isThere = NO_ID;
			IDValList idvl = pab->BaseAlbum()->aliasesList; // list of albums linked to this album
			if (idvl.isEmpty())     // no linked albums for this one
				isThere = pDest->items.indexOf(itemID) >= 0 ? itemID.Val() : NO_ID; // check if album is already in destination album
			else
				for (auto& v : idvl)
				{
					ID_t idv(ALBUM_ID_FLAG, v);
					if (pDest->items.indexOf(idv) >= 0)
					{
						isThere = v;
						break;  // found at least one linked album in destination album
					}
				}

			if (isThere != NO_ID)    // album is already in destination album
			{
				QString qs = tr("The album '%1' to be moved is already in the destination album '%2'.\n").arg(pab->name).arg(_albumPointers.pDest->name);
				if (isThere != itemID.Val())             // another alias for the same album
					qs += tr("under the name '%1'\n").arg(albumgen.AlbumForIDVal(isThere)->name);
				qs += tr("Please remove it from the selection!");
				QMessageBox::warning(frmMain, tr("falconG - Warning"), qs);
				return false;
			}
		}
	}

	// ------------- housekeeping -----------
	// add moved items to destination album  (which doesn't have icons yet )
	IdList& items = pSrc->items;   // original ordered items
	for (int si = 0; si < thl.size(); ++si)
	{
		ID_t itemID = items[thl[si]];
		if (itemID.IsAlbum())
		{
			Album* pab = albumgen.AlbumForID(itemID);
			Q_ASSERT(pab);
			pab->parentId = pDest->ID.Val();      // reparent album
			albumgen.AddToModifiedList(itemID.Val(), true);   // so that it gets written 
		}
		if (!itemsDest.contains(itemID))
			itemsDest.push_back(itemID);
	}
	pDest->items = itemsDest;

	// remove indexes of moved items from source gallery and icon indices
	const QVector<int>& origIconOrder = fileIcons.IconOrder();  // original order of icons for actual album
	QVector<int> newIconOrder;                       // new icon order indexes
	IdList newItems;                                 // w.o. moved items
	int itemSize = items.size();
	for (int si = 0; si < itemSize; ++si)            // origIconOrder has the same number of items as items itself
	{
		int o = origIconOrder[si];
		if (thl.indexOf(o) < 0)
		{
			newItems.push_back(items[o]);
			newIconOrder.push_back(o);
		}
	}
	// discard moved icons
	//for (int si = itemSize - 1; si << itemSize >= 0; --si)   // origIconOrder has the same number of items as items itself
	//    if (thl.indexOf(origIconOrder[si]) >= 0)
	//        fileIcons.Remove(origIconOrder[si]);
	items = newItems;
	fileIcons.SetIconOrder(newIconOrder);
	// ----- never  move items physically to virtual destination folder --------
	albumgen.AddToModifiedList(*pSrc);
	albumgen.AddToModifiedList(*pDest);
	// redisplaying actual album is done in caller
	return true;
}

void DropHandler::Setup(QDropEvent* event, IDVal_t srcAlbumId, IDVal_t destAlbumId, int row)
{
	_event = event;		 // event MUST contain ThumbMimeData
	_mimeData = reinterpret_cast<const ThumbMimeData*>(event->mimeData()); // event MUST contain ThumbMimeData
	_row = row;
	_albumPointers.Setup(srcAlbumId, destAlbumId);
}

/*=============================================================
 * TASK   : drop items to album
 * PARAMS :	fromThumbView: true if the drop is from thumbnail view
 *						   false if the drop is from tree view
 * EXPECTS:	_event: a drop event with mime data containing urls
 *					 or images
 *			_srcId: id of album from which items are dragged
 *					 NO_ID if from outside of the application
 *			_destId: id of album to drop into
 * GLOBALS: _mimeData
 * RETURNS:
 * REMARKS: if _destId == srcId
 *------------------------------------------------------------*/
void DropHandler::DropItems(bool fromThumbView)
{
	if (!_event || !IsAllowedTypeToDrop(_event))
		return;

	const QMimeData* md = _event->mimeData();

	if (md)
	{
		if (_mimeData->hasUrls())	// then this isn't a "ThumbMimeData"
			_FromExternalSource();
		else
			_FromInternalSource(fromThumbView);
	}
	_event = nullptr;	// mark it isn't ready
}

