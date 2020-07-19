#include "stewartplatform.h"
// #include "mouse3dinput.h"
#include "mainwindow.h"
#include "splashscreen.h"


#include <Eigen/Dense>
#include <Eigen/Geometry>
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
    // Mouse3DInput mouse;
    // app.installNativeEventFilter(&mouse);


    // MainWindow mainWin(nullptr, &mouse);
    MainWindow mainWin(nullptr);

    mainWin.resize(splash->width(), splash->height()*0.9);

    mainWin.move(splash->x(), splash->y());

    splash->showMessage("Loading user interface...", Qt::AlignBottom);

    mainWin.show();
    splash->finish(&mainWin);

    return app.exec();

}



