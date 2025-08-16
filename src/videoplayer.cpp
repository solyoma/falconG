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

// code modified by ChatGPT 4.1 and A. Solyom
#include "videoplayer.h"

#include <QtWidgets>

VideoPlayer::VideoPlayer(VPurpose purpose, Common::VideoData* videoData, QWidget* parent)
    : _purpose(purpose), _pVideoData(videoData), QWidget(parent)
{
    _mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface); // The player is expected to be able to render to a QAbstractVideoSurface output.
    _Setup();
}

VideoPlayer::~VideoPlayer()
{
}

void VideoPlayer::SlotOpenFile()
{
    static QString lastPath = "I:/Filmek/photo/";
    QFileDialog fileDialog(this);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setWindowTitle(tr("Open Movie"));
    QStringList supportedMimeTypes = _mediaPlayer->supportedMimeTypes();
    if (!supportedMimeTypes.isEmpty())
        fileDialog.setMimeTypeFilters(supportedMimeTypes);
    fileDialog.setDirectory(lastPath/*QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).value(0, QDir::homePath())*/);
    if (fileDialog.exec() == QDialog::Accepted)
    {
        lastPath = fileDialog.directory().currentPath();
        SetUrl(fileDialog.selectedUrls().constFirst());   // also gets video size and frame rate
    }
}

void VideoPlayer::SetPurpose(VideoPlayer::VPurpose purpose)
{
    _purpose = purpose;
    _Setup();       // setup this (invisible) widget as needed. For extractor no widgets are added to this
}

bool VideoPlayer::SetUrl(const QUrl& url)      
{
    _sErrorString.clear();
	_status = vsNotReady;  // reset status

	_currentUrl = url;
    if (url.isEmpty())
        return false;

    if(_purpose != vpExtractFrame)
        setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
    _mediaPlayer->setMedia(url);

    QMetaObject::Connection conErr = connect(_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), [&](QMediaPlayer::Error err) {
        _sErrorString = _mediaPlayer->errorString();
        // Signal error to UI or log, but do not exit
        });

    if (!_sErrorString.isEmpty())
    {
        disconnect(conErr); // Disconnect the error connection to avoid memory leaks
        return false;
    }                                          //  need to specify which metaDataChanged() function to use
    // QMetaObject::Connection connection = QObject::connect(_mediaPlayer, (void(QMediaPlayer::*)()) &QMediaPlayer::metaDataChanged, [this]() {
                                                // 'QOverload' proposed by Qt help:

    QMetaObject::Connection connection = QObject::connect(_mediaPlayer, QOverload<>::of(&QMediaPlayer::metaDataChanged), [this]() {
        QSize size = _mediaPlayer->metaData(QMediaMetaData::Resolution).toSize();
        QVariant rateVar = _mediaPlayer->metaData(QMediaMetaData::VideoFrameRate);
        qreal frameRate = rateVar.isValid() ? rateVar.toReal() : 0.0;
        if (_pVideoData)
        {
            _pVideoData->frameSize = size;
            _pVideoData->frameRate = frameRate;
        }

        if (_purpose == vpPlay)      // otherwise _playButton is nullptr
        {
            _errorLabel->setText(QString());
            _playButton->setEnabled(true);
        }
        });

	_mediaPlayer->play(); // so as to get the parameters
    QTimer::singleShot(5000, [&]() {
        if (!_mediaPlayer->isMetaDataAvailable()) 
        {
            _sErrorString += tr(" - Timeout: No metadata available.");
        }
        });
	disconnect(conErr); // Disconnect the error connection to avoid memory leaks
	disconnect(connection); // Disconnect the metadata connection to avoid memory leaks

    if (_sErrorString.isEmpty())
        _status = vsOk;

    return _status == vsOk;
}

/*=============================================================
 * TASK   : extract a single video frame from the video at position
 *  	   ms (milliseconds)    or videoData's 'thumbPosition'
 * PARAMS : ms: position in msec or -1 to use videoData's thumbPosition
 * EXPECTS: _frameGrabber and _mediaPlayer must be set up before this is called
 *              togetherwith videoOutput set to the correct video surface
 * GLOBALS:  videoData, _mediaPlayer, _frameGrabber, _grabbedImage
 * RETURNS: image captured at the specified position in the video
 * REMARKS: - if ms < 0 videoData's thumbPosition is used, otherwise 
 *              it is set to ms
 *          - thumbPosition < 0 sets it to 0
 *------------------------------------------------------------*/
QImage VideoPlayer::ExtractFrameAt(qint64 ms)   // 
{
    if (!_mediaPlayer || !_frameGrabber)
        return QImage();

    if (ms < 0 && _pVideoData)
    {
        ms = _pVideoData->thumbPosition;
        if (ms < 0)
            ms = _pVideoData->thumbPosition = 0;
    }
    else if(_pVideoData)
        _pVideoData->thumbPosition = ms;
    else if (ms < 0)
		ms = 0; // otherwise use 0

    QEventLoop loop;
    QImage result;
    _frameGrabber->frameReceived = false;  
    QMetaObject::Connection connection = QObject::connect(_frameGrabber, &FrameGrabber::frameReady, &loop, [&]() {
                                        result = _grabbedImage;
                                        loop.quit();
                                        });

    // Seek to the requested position
    _mediaPlayer->setPosition(ms);

    // Timeout to avoid hanging if no frame arrives
    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    timer.start(2000); // 2 seconds timeout

    loop.exec();

	QObject::disconnect(connection); // Disconnect the connection to avoid memory leaks

    return result;
}

void VideoPlayer::SlotSnapshotFromFrame()
{
    if (_grabbedImage.isNull()) 
    {
        _errorLabel->setText(tr("Failed to capture frame."));
        return;
    }
    if(_pVideoData)
    {
        _pVideoData->thumbPosition = _mediaPlayer->position(); // save the position of the frame
        _pVideoData->frameSize = _grabbedImage.size();
        _pVideoData->frameRate = _mediaPlayer->metaData(QMediaMetaData::VideoFrameRate).toReal();
	}
}

void VideoPlayer::SlotPlay()
{
    switch (_mediaPlayer->state()) {
    case QMediaPlayer::PlayingState:
        _mediaPlayer->pause();
        // SNAP		_snapshotButton->setEnabled(true);
        break;
    default:
        _mediaPlayer->play();
        // SNAP     _snapshotButton->setEnabled(false);
        break;
    }
}

void VideoPlayer::SlotMediaStateChanged(QMediaPlayer::State state)
{
    switch (state) 
    {
        case QMediaPlayer::PlayingState:
            _playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
            break;
        default:
            _playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
            break;
    }
}

void VideoPlayer::SlotPositionChanged(qint64 position)
{
    _positionSlider->setValue(position);
}

void VideoPlayer::SlotDurationChanged(qint64 duration)
{
    _positionSlider->setRange(0, duration);
}

void VideoPlayer::SlotSetPosition(int position)
{
    _mediaPlayer->setPosition(position);
}

int VideoPlayer::SlotGetPosition()
{
    return _mediaPlayer->position();
}

void VideoPlayer::SlotHandleError()
{
    _playButton->setEnabled(false);
    const QString errorString = _mediaPlayer->errorString();
    QString message = "Error: ";
    if (errorString.isEmpty())
        message += " #" + QString::number(int(_mediaPlayer->error()));
    else
        message += errorString;
    _errorLabel->setText(message);
}

void VideoPlayer::_SetupAsGrabber()
{
	if (_videoWidget) // then remove widget components
	{
		hide(); // hide the widget
		disconnect(_surface, &VideoSurface::frameAvailable, _videoWidget, &VideoWidget::setFrame);
		disconnect(_mediaPlayer, &QMediaPlayer::stateChanged, this, &VideoPlayer::SlotMediaStateChanged);
		disconnect(_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::SlotPositionChanged);
		disconnect(_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::SlotDurationChanged);
		disconnect(_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &VideoPlayer::SlotHandleError);
		disconnect(_snapshotButton, &QAbstractButton::clicked, this, &VideoPlayer::SlotSnapshotFromFrame);
		disconnect(_playButton, &QAbstractButton::clicked, this, &VideoPlayer::SlotPlay);
		disconnect(_positionSlider, &QAbstractSlider::sliderMoved, this, &VideoPlayer::SlotSetPosition);
		delete layout();  // this doesn't delete the widgets, just the layout
		delete _playButton;   _playButton = nullptr;
        delete _snapshotButton; _snapshotButton=nullptr;
        delete _positionSlider; _positionSlider = nullptr;
        delete _errorLabel; _errorLabel = nullptr;
        delete _videoWidget; _videoWidget = nullptr;
	}

	_frameGrabber = new FrameGrabber(_grabbedImage);
    _mediaPlayer->setVideoOutput(_frameGrabber);
    connect(_frameGrabber, &FrameGrabber::frameReady, this, &VideoPlayer::SlotSnapshotFromFrame);
}


void VideoPlayer::_SetupAsPlayer()    // widget has a play/pause button, a snapshot button and a slider to control the position
{                                   // video file must be set before this is called
    if (_videoWidget)
        return; // already set up
    if (_frameGrabber)
    {
        disconnect(_frameGrabber, &FrameGrabber::frameReady, this, &VideoPlayer::SlotSnapshotFromFrame);
		delete _frameGrabber; _frameGrabber = nullptr; // delete the frame grabber
    }

	_surface = new VideoSurface(_grabbedImage, this); // Create a custom video surface to capture frames
    // Set the video output to the custom surface
    _mediaPlayer->setVideoOutput(_surface);

	// Create the video widget and other controls
    _videoWidget = new VideoWidget; // Use custom VideoWidget to display the frame in

    _snapshotButton = new QPushButton;
    _snapshotButton->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
    _snapshotButton->setToolTip(tr("Set as thumbnail image"));
    connect(_snapshotButton, &QAbstractButton::clicked, this, &VideoPlayer::SlotSnapshotFromFrame);

    _playButton = new QPushButton;
    _playButton->setEnabled(false);
    _playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(_playButton, &QAbstractButton::clicked, this, &VideoPlayer::SlotPlay);

    _positionSlider = new QSlider(Qt::Horizontal);
    _positionSlider->setRange(0, 0);
    connect(_positionSlider, &QAbstractSlider::sliderMoved, this, &VideoPlayer::SlotSetPosition);

    _errorLabel = new QLabel;
    _errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    QBoxLayout* controlLayout = new QHBoxLayout;
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addWidget(_playButton);
    controlLayout->addWidget(_snapshotButton);
    controlLayout->addWidget(_positionSlider);

    QBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_videoWidget);
    layout->addLayout(controlLayout);
    layout->addWidget(_errorLabel);

    setLayout(layout);


    // Connect the surface's frameAvailable signal to the widget's setFrame slot
    connect(_surface, &VideoSurface::frameAvailable, _videoWidget, &VideoWidget::setFrame);

    connect(_mediaPlayer, &QMediaPlayer::stateChanged, this, &VideoPlayer::SlotMediaStateChanged);
    connect(_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::SlotPositionChanged);
    connect(_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::SlotDurationChanged);
    connect(_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &VideoPlayer::SlotHandleError);
}

void VideoPlayer::_Setup()
{
	_status = vsNotReady; // reset status
	_sErrorString.clear(); // reset error string

    if (_purpose == vpExtractFrame)
        _SetupAsGrabber();
    else
        _SetupAsPlayer();
}