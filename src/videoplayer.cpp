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

VideoPlayer::VideoPlayer(QWidget* parent)
    : QWidget(parent)
{
    _mediaPlayer = new QMediaPlayer(this, QMediaPlayer::VideoSurface);
    _videoWidget = new VideoWidget; // Use custom VideoWidget

    QAbstractButton* openButton = new QPushButton(tr("Open..."));
    connect(openButton, &QAbstractButton::clicked, this, &VideoPlayer::openFile);
    _snapshotButton = new QPushButton(tr("Save PNG from frame.."));
    connect(_snapshotButton, &QAbstractButton::clicked, this, &VideoPlayer::snapshotFrame);

    _playButton = new QPushButton;
    _playButton->setEnabled(false);
    _playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(_playButton, &QAbstractButton::clicked, this, &VideoPlayer::play);

    _positionSlider = new QSlider(Qt::Horizontal);
    _positionSlider->setRange(0, 0);
    connect(_positionSlider, &QAbstractSlider::sliderMoved, this, &VideoPlayer::setPosition);

    _errorLabel = new QLabel;
    _errorLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);

    QBoxLayout* controlLayout = new QHBoxLayout;
    controlLayout->setContentsMargins(0, 0, 0, 0);
    controlLayout->addWidget(openButton);
    controlLayout->addWidget(_snapshotButton);
    controlLayout->addWidget(_playButton);
    controlLayout->addWidget(_positionSlider);

    QBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(_videoWidget);
    layout->addLayout(controlLayout);
    layout->addWidget(_errorLabel);

    setLayout(layout);

    // Set the video output to the custom surface
    _mediaPlayer->setVideoOutput(&_surface);

    // Connect the surface's frameAvailable signal to the widget's setFrame slot
    connect(&_surface, &VideoSurface::frameAvailable, _videoWidget, &VideoWidget::setFrame);

    connect(_mediaPlayer, &QMediaPlayer::stateChanged, this, &VideoPlayer::mediaStateChanged);
    connect(_mediaPlayer, &QMediaPlayer::positionChanged, this, &VideoPlayer::positionChanged);
    connect(_mediaPlayer, &QMediaPlayer::durationChanged, this, &VideoPlayer::durationChanged);
    connect(_mediaPlayer, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error), this, &VideoPlayer::handleError);
}

VideoPlayer::~VideoPlayer()
{
}

void VideoPlayer::openFile()
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
        setUrl(fileDialog.selectedUrls().constFirst());
    }
}


void VideoPlayer::setUrl(const QUrl& url)
{
    _errorLabel->setText(QString());
    setWindowFilePath(url.isLocalFile() ? url.toLocalFile() : QString());
    _mediaPlayer->setMedia(url);
    _playButton->setEnabled(true);
}

void VideoPlayer::snapshotFrame()
{
    //QString fileName = QFileDialog::getSaveFileName(this, tr("Save actual frame as a PNG),
    //                                                QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
    //                                                tr("PNG files (*.png)"));
    QString fileName = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/snaphot1.png";
    if (fileName.isEmpty())
        return;

    QPixmap pixmap = QPixmap::fromImage(_surface.currentFrame);
    if (pixmap.isNull()) {
        _errorLabel->setText(tr("Failed to capture frame."));
        return;
    }
    if (!pixmap.save(fileName, "PNG")) {
        _errorLabel->setText(tr("Failed to save image to %1.").arg(fileName));
    }
}

void VideoPlayer::play()
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

void VideoPlayer::mediaStateChanged(QMediaPlayer::State state)
{
    switch (state) {
    case QMediaPlayer::PlayingState:
        _playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        break;
    default:
        _playButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        break;
    }
}

void VideoPlayer::positionChanged(qint64 position)
{
    _positionSlider->setValue(position);
}

void VideoPlayer::durationChanged(qint64 duration)
{
    _positionSlider->setRange(0, duration);
}

void VideoPlayer::setPosition(int position)
{
    _mediaPlayer->setPosition(position);
}

void VideoPlayer::handleError()
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
