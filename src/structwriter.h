﻿#pragma once

#ifndef _STRUCTWRITER_H
	#define _STRUCTWRITER_H

#ifndef _ALBUMS_H
	#include "albums.h"
#endif

#include <QWidget>
#include <QMutex>
#include <QThread>

//------------------------------------------
// Thread to write album structure (simplest: no message loop inside thread)

class AlbumStructWriter : public QThread
{
	Q_OBJECT

	QMutex _albumMapStructIsBeingWritten;

	TextMap& _textMap;		// all texts for all albums and inmages
	AlbumMap& _albumMap;	// all source albums
	ImageMap& _imageMap;	// all images for all albums
	VideoMap& _videoMap;	// all videos for all albums
	Album _root;			// top level album (first in '_albumMap', ID = 1|ALBUM_ID_FLAG)
	bool _recordChanges=false;	// when true then all changed albums will be marked as such
	QTextStream _ofs;

	void _WriteImageRecord(Image* pImg, QString indent);
	void _WriteStructImage(Album &album, ID_t id, QString indent);
	void _WriteVideo(Album &album, ID_t id, QString indent);
	void _WriteImagesThenSubAlbums(Album& album, QString indent);
	void _WriteAlbums(Album& album, QString indent);
	void _WriteLanguageTable();
	void _WritePathsTable();	// ID - PATH
	void _WriteOrphanThumbnails();

signals:
	void SignalResultIsReady(QString s, QString sStructPath, QString sStructTmp);
public:
	void run() override;  // the actual writing happens here
	AlbumStructWriter(AlbumGenerator& generator, bool keepChanged = true, QObject* parent = Q_NULLPTR);
};
//------------------------------------------
#endif
