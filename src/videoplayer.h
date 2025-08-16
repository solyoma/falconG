#pragma once
// AI created code from the example in the Qt sources
// modified by A. Solyom

/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef VIDEOPLAYER_H
#define VIDEOPLAYER_H

#include <QWidget>
#include <QtMultimedia/QMediaPlayer>
#include <QtMultimedia/QMediaMetaData>
#include <QtMultimedia/QMediaMetaData>
#include <QtMultimedia/QVideoSurfaceFormat>
#include <QtMultimedia/QAbstractVideoSurface>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QMutex>
#include "common.h"

QT_BEGIN_NAMESPACE
class QAbstractButton;
class QSlider;
class QLabel;
class QUrl;
QT_END_NAMESPACE

//--------------------------------------------------
// VideoSurface: Captures frames and notifies when a new frame is available
class VideoSurface : public QAbstractVideoSurface
{
    Q_OBJECT
private:
	QImage& _currentFrame;  // reference to the current frame image
public:
    VideoSurface(QImage &currentFrame, QObject* parent = nullptr) : _currentFrame(currentFrame), QAbstractVideoSurface(parent) {}

    bool present(const QVideoFrame& frame) override {
        QVideoFrame cloneFrame(frame);
        if (!cloneFrame.map(QAbstractVideoBuffer::ReadOnly))
            return false;
        QImage::Format imgFormat = QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat());
        if (imgFormat == QImage::Format_Invalid) {
            cloneFrame.unmap();
            return false;
        }
        QImage img(cloneFrame.bits(), cloneFrame.width(), cloneFrame.height(), cloneFrame.bytesPerLine(), imgFormat);
        _currentFrame = img.copy();
        cloneFrame.unmap();
        emit frameAvailable(_currentFrame);
        return true;
    }

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType handleType) const override {
        if (handleType == QAbstractVideoBuffer::NoHandle) {
            return QList<QVideoFrame::PixelFormat>()
                << QVideoFrame::Format_RGB32
                << QVideoFrame::Format_ARGB32;
        }
        else {
            return QList<QVideoFrame::PixelFormat>();
        }
    }
signals:
    void frameAvailable(const QImage& image);
};

//--------------------------------------------------
// VideoWidget: Displays the current frame
class VideoWidget : public QWidget
{
    Q_OBJECT
public:
    VideoWidget(QWidget* parent = nullptr) : QWidget(parent) {}

public slots:
    void setFrame(const QImage& image) {
        _frame = image;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        if (!_frame.isNull())
            p.drawImage(rect(), _frame);
        else
            p.fillRect(rect(), Qt::black);
    }

private:
    QImage _frame;
};
//--------------------------------------------------
class FrameGrabber : public QAbstractVideoSurface 
{
    Q_OBJECT
private:
	QImage& _grabbedImage;  // reference to the current frame image
public:
    bool frameReceived = false;

    FrameGrabber(QImage& grabbedImage) : _grabbedImage(grabbedImage), QAbstractVideoSurface() {}

    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType) const override {
        return { QVideoFrame::Format_RGB32, QVideoFrame::Format_ARGB32, QVideoFrame::Format_RGB24 };
    }

    bool present(const QVideoFrame& frame) override {
        QVideoFrame copy(frame);
        copy.map(QAbstractVideoBuffer::ReadOnly);
        QImage img = QImage(copy.bits(), copy.width(), copy.height(), copy.bytesPerLine(), QVideoFrame::imageFormatFromPixelFormat(copy.pixelFormat())).copy();
        copy.unmap();
        _grabbedImage = img;
        frameReceived = true;
        emit frameReady();
        return true;
    }

signals:
    void frameReady();
};

class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    enum VPurpose {         vpPlay,          // play video
        vpExtractFrame,  // extract a frame from the video
        vpSnapshot       // take a snapshot from the video
	};
    enum VPStatus { 
		vsNoPlayer,      // player is not set up
		vsNotReady,      // player is not ready to play
        vsError,         // player encountered an error
		vsOk             // player is ready to play/grab
	};
public:
	VideoPlayer(VPurpose purpose, Common::VideoData *videoData, QWidget* parent = nullptr);  // creates a player or a frame grabber
    ~VideoPlayer();

	void SetPurpose(VPurpose purpose);  // setups the widget for the given purpose
	// Sets the video file to be played or processed.
    bool SetUrl(const QUrl& url);   // getss video parameters, returns status and possible error string
	VPStatus Status() const 
    { 
        return _mediaPlayer ? _status : vsNoPlayer; 
    }
    QString LastErrorString() // Status() returns the same results until this is called
    {
		QString s = _sErrorString;
		_sErrorString.clear(); // clear the error string after reading it
        return s;
    }

	const inline QImage& GrabbedImage() const { return _grabbedImage; }

    // Extracts a frame at the given time (milliseconds) and returns it as QImage.
	// only use this if the player is set up as a frame grabber and does not playe visible video
    QImage ExtractFrameAt(qint64 ms=-1);    // -1: use videoData, > 0 set videoData

public slots:
    void SlotOpenFile();
    void SlotSnapshotFromFrame();
    void SlotPlay();

private slots:
    void SlotMediaStateChanged(QMediaPlayer::State state);
    void SlotPositionChanged(qint64 position);
    void SlotDurationChanged(qint64 duration);
    void SlotSetPosition(int position);
    int  SlotGetPosition();
    void SlotHandleError();

private:
    // common
	VPurpose _purpose = vpExtractFrame; // i.e. no widgets created and/or used
    QMediaPlayer* _mediaPlayer      = nullptr;  // this plays the video  
	QImage _grabbedImage;           // used by both the player and the grabber

	QUrl _currentUrl;                          // current video URL
	VPStatus _status = vsNotReady;             // current status of the player
    Common::VideoData* _pVideoData = nullptr;

	// for _purpose == vpGrabber
	FrameGrabber* _frameGrabber     = nullptr;  // this is also a VideoSurface, but it grabs frames from the video too
    QString _sErrorString;
	// for _purpose == vpPlay or vpSnapshot
    VideoWidget* _videoWidget       = nullptr;  // this shows the video frames
    VideoSurface *_surface          = nullptr;  // using this video surface
    QAbstractButton* _playButton    = nullptr;
    QAbstractButton* _snapshotButton= nullptr;
    QSlider* _positionSlider        = nullptr;
    QLabel* _errorLabel             = nullptr;


	void _SetupAsPlayer();
	void _SetupAsGrabber();
    void _Setup();
};

#endif
