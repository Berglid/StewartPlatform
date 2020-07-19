#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <QSplashScreen>
#include <QProgressBar>
#include <QApplication>
#include <QtCore>

class SplashScreen : public QSplashScreen
{
    Q_OBJECT
public:
    explicit SplashScreen(QApplication *app, QWidget *parent = 0);
    // QProgressBar *progress;
    QWidget *spacer;
    QApplication *app;


    int resX;
    int resY;
    float scale;

public slots:
    // void progressBarUpdated();

protected:
    void drawContents(QPainter *painter);

};

#endif // SPLASHSCREEN_H
