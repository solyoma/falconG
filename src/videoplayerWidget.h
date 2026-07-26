#pragma once
#include <QtCore>
#if QT_VERSION >= QT_VERSION_CHECK(6,0,0)
// VideoPlayerWidget.h

/* usage:
    * auto *player = new VideoPlayerWidget(this);
    * player->setVideo("C:/Videos/example.mp4");
    * player->play();
    *
    * // When paused:
    * player->pause();
    * const QString savedImage = player->savePausedFrame();
    * 
*/

#include <QWidget>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QVideoFrame>

class VideoPlayerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayerWidget(QWidget* parent = nullptr);

    void setVideo(const QString& fullFilePath);
    void play() { player.play(); }
    void pause() { player.pause(); }

    // Returns the saved PNG path, or an empty string if no frame is available.
    QString savePausedFrame();

    QSize sizeHint() const override { return { 640, 360 }; }

private:
    QMediaPlayer player;
    QVideoWidget video;
    QVideoFrame lastFrame;
    QString filePath;
};
#endif
