#include "stewartplatform.h"
#include "mouse3dinput.h"
#include "mainwindow.h"
#include "splashscreen.h"


#include <includes/eigen-3.3.7/Eigen/Dense>
#include <includes/eigen-3.3.7/Eigen/Geometry>
#include <QQuaternion>

#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFontComboBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMessageBox>
#include <QtGui/QScreen>
#include <QtGui/QFontDatabase>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <QFile>
#include <QTextStream>
#include <QSplashScreen>
#include <QProgressBar>
#include <QTimer>

#include <chrono>

#include <QElapsedTimer>
#include <QThread>

#include <QDesktopWidget>
// https://forum.qt.io/topic/87654/building-multi-progress-bar-getting-slowdown/8


int main(int argc, char **argv)
{

    using namespace std::chrono;

    QApplication app(argc, argv);

    SplashScreen *splash = new SplashScreen(&app);
    splash->show();
    app.processEvents();
    splash->showMessage("Loading 3D controller...", Qt::AlignBottom);
    Mouse3DInput mouse;
    app.installNativeEventFilter(&mouse);


    MainWindow mainWin(nullptr, &mouse);

    mainWin.resize(splash->width(), splash->height()*0.9);

    mainWin.move(splash->x(), splash->y());


    splash->showMessage("Loading user interface...", Qt::AlignBottom);


    mainWin.show();
    splash->finish(&mainWin);




    return app.exec();

    //    QElapsedTimer timer;
    //    timer.start();
    //    //int t = timer.elapsed();
    //    // qDebug() << "t: " << t;
    //    for(int i = 0; i < 101; i++){
    //        // splash->progressBarUpdated(i);
    //        splash->progress->setValue(i);
    //        int t = timer.elapsed();
    //        qDebug() << "t: " << t;
    //        qDebug() << "timer.elapsed()" << timer.elapsed();
    //        while(timer.elapsed() < (t + 100)){
    //            app.processEvents();
    //        }
    //    }


    //    for(int i = 0; i < 10100; i++){

    //        splash->progressBarUpdated(i/100);
    //    }
    //     app.processEvents();

    //    QTimer* timer = new QTimer;
    //    QObject::connect(qApp, &QApplication::aboutToQuit, timer, &QTimer::deleteLater); // to delete the timer.

    //    QObject::connect(timer, &QTimer::timeout, [splash, timer]() {
    //      static int val = 0;

    //      splash->progressBarUpdated(val);

    //      qDebug() << "TIMER" << val;
    //      val++;
    //      if (val > 500 )   val = 500;
    //    });
    //    timer->start(1);


    //    std::chrono::steady_clock::time_point t1;
    //    std::chrono::steady_clock::time_point t2;

    //    for(int i = 0; i < 101; i++){
    //        t2 = std::chrono::steady_clock::now();
    //        duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
    //        if (time_span.count() > 2.0) {
    //            t1 = t2;
    //            splash->progressBarUpdated(i);
    //            qApp->processEvents();
    //        }
    //        qDebug() << "i:" << i;
    //    }
}



