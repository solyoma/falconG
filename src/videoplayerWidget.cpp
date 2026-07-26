#include <QtCore>
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
// VideoPlayerWidget.cpp
#include "VideoPlayerWidget.h"

#include <QFileInfo>
#include <QUrl>
#include <QVBoxLayout>
#include <QVideoSink>

VideoPlayerWidget::VideoPlayerWidget(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&video);

    video.setAspectRatioMode(Qt::KeepAspectRatioByExpanding); // fill + crop
    player.setVideoOutput(&video);

    connect(video.videoSink(), &QVideoSink::videoFrameChanged,
        this, [this](const QVideoFrame& frame) {
            if (frame.isValid())
                lastFrame = frame;
        });
}

void VideoPlayerWidget::setVideo(const QString& fullFilePath)
{
    filePath = fullFilePath;
    lastFrame = {};
    player.setSource(QUrl::fromLocalFile(filePath));
}

QString VideoPlayerWidget::savePausedFrame()
{
    if (lastFrame.isValid() == false)
        return {};

    QFileInfo videoFile(filePath);
    const QString imagePath =
        videoFile.absolutePath() + "/" + videoFile.completeBaseName() + ".png";

    return lastFrame.toImage().save(imagePath, "PNG") ? imagePath : QString{};
}
#endif