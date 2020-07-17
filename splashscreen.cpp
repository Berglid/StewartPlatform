#include "splashscreen.h"

SplashScreen::SplashScreen(QApplication *app, QWidget *parent) :
    QSplashScreen(parent),
    resX(3840),
    resY(2160),
    scale(resY*0.71)

{
    //    QSplashScreen splash(pixmap.scaledToHeight(resY*0.71), Qt::WindowStaysOnTopHint);

    this->app = app;
    this->setWindowFlag(Qt::WindowStaysOnTopHint);


    this->setPixmap(QPixmap(":/logo/stewart.198.jpg").scaledToHeight(scale));
    this->setCursor(Qt::BusyCursor);


    // Stay on top hint




    QFont font = this->font();
    font.setPointSize(16);
    this->setFont(font);
    this->showMessage("Hello", Qt::AlignBottom);

//    progress = new QProgressBar(this);
//    progress->setRange(0, 0);
//    progress->setValue(0);
//    progress->setEnabled(true);
//    progress->setFixedWidth(this->width());
//    progress->setAlignment(Qt::AlignTop);

//    connect(progress, SIGNAL(valueChanged(int)), this, SLOT(progressBarUpdated(int)));
}

void SplashScreen::drawContents(QPainter *painter)
{
    QSplashScreen::drawContents(painter);
    // this->progress->render(painter);
}

//void SplashScreen::progressBarUpdated()
//{
//    int value = progress->value() + 1;
//    if(value > 100) {value = 100; }
//    this->repaint();
//    progress->setValue(value);
//    this->app->processEvents();
//}


