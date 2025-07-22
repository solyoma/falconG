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

#include <QMediaPlayer>
#include <QWidget>
#include <QAbstractVideoSurface>
#include <QVideoSurfaceFormat>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QMutex>

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
public:
    VideoSurface(QObject* parent = nullptr) : QAbstractVideoSurface(parent) {}

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
        currentFrame = img.copy();
        cloneFrame.unmap();
        emit frameAvailable(currentFrame);
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

    QImage currentFrame;

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

class VideoPlayer : public QWidget
{
    Q_OBJECT
public:
    VideoPlayer(QWidget* parent = nullptr);
    ~VideoPlayer();

    void setUrl(const QUrl& url);

public slots:
    void openFile();
    void snapshotFrame();
    void play();

private slots:
    void mediaStateChanged(QMediaPlayer::State state);
    void positionChanged(qint64 position);
    void durationChanged(qint64 duration);
    void setPosition(int position);
    void handleError();

private:
    QMediaPlayer* _mediaPlayer;
    VideoWidget* _videoWidget;
    VideoSurface _surface;
    QAbstractButton* _playButton;
    QAbstractButton* _snapshotButton;
    QSlider* _positionSlider;
    QLabel* _errorLabel;
};

#endif
