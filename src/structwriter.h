#pragma once

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

class AlbumStructWriterThread : public QThread
{
	Q_OBJECT

		QMutex _albumMapStructIsBeingWritten;

	TextMap& _textMap;		// all texts for all albums and inmages
	AlbumMap& _albumMap;	// all source albums
	ImageMap& _imageMap;	// all images for all albums
	VideoMap& _videoMap;	// all videos for all albums
	Album _root;			// top level album (first in '_albumMap', ID = 1|ALBUM_ID_FLAG)
	QTextStream _ofs;

	void _WriteStructImagesThenSubAlbums(Album& album, QString indent);
	void _WriteStructAlbums(Album& album, QString indent);

signals:
	void resultReady(QString s, QString sStructPath, QString sStructTmp);
public:
	void run() override;  // the actual writing
	AlbumStructWriterThread(AlbumGenerator& generator, QObject* parent = Q_NULLPTR);
};
//------------------------------------------
#endif
