#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtGui/QFont>
#include <QCustom3DItem>
#include <qfiledialog.h>
#include <QTime>
#include <QLabel>


#include "stewartplatform.h"
#include "mouse3dinput.h"
#include "serial_settings_dialog.h"




QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr, Mouse3DInput* mouse3D = nullptr );
    ~MainWindow();

    // FROM PROGRESSIVE
    // Writes string to logging window.
    // TODO: Implement log winodw
    void log(const QString& entry);

    // Constants
    const static int NUM_ACTUATORS      = 6;
    const static int MIN_ACTUATOR_VALUE = 0;
    const static int MAX_ACTUATOR_VALUE = 1024;

signals:
    void Move3d(QVector<float>& motionData); // Signals a move from demo playback

private slots:

    //TODO: Move stuff here
    void onValidMove();

    // Stop, play, pause, record signals
    void on_stopButton_clicked();
    void on_playButton_clicked();
    void on_pauseButton_clicked();
    void on_recordButton_toggled(bool checked);

    void printVector(QVector<float> vec);
    void on_actionLoad_from_file_triggered();

    void enableHomingButtons();
    void disableHomingButtons();

    void on_xResetButton_toggled(bool checked);
    void on_yResetButton_toggled(bool checked);
    void on_zResetButton_toggled(bool checked);
    void on_rxResetButton_toggled(bool checked);
    void on_ryResetButton_toggled(bool checked);
    void on_rzResetButton_toggled(bool checked);
    void on_actuatorsHomeButton_toggled(bool checked);


private:
    Ui::MainWindow *ui;
    Q3DScatter *graph;
    QWidget *container;
    Mouse3DInput* m_mouse;
    StewartPlatform *stewart;
    QSerialPort* m_serial = nullptr;
    SerialSettingsDialog *m_settings = nullptr;

    QString playBackTime;
    QString playFileName;
    QString playFilePath;
    QString recordFileName;
    QString recordFilePath;
    QString statusTextPlaying;
    QString statusTextRecording;
    QString statusTextUserInput;
    QVector<QString> statusText;
    QVector<QVector<float>> vectorPlaying;
    QVector<QVector<float>> vectorRecording;
    QVector<QVector<float>> vectorGeneratedPath;

    bool isRecording; // USE ENUMS INSTEAD
    bool isPlaying;
    bool isPaused;
    bool recordingInitiated;
    bool isFollowingGeneratedPath;
    bool isHoming;
    bool printDebug;
    bool darkTheme;

    int indexDemo;
    int demoSize;
    int indexGeneratedPath;
    int status; // DELTE, Implement as ENUM

    float fps;
    float playBackPercentage;

    void resetPlayPauseStopRecordButtons();
    void resetHoming();
    void MoveToPreviosDemoPose();
    void calcPlaybackTime();
    void onTimeOut(); // running demo and homing of actuators
    void saveRecording(QString filename);
    void loadDemo();
    void updatePlayFileName();
    void homingButtonXaction(QVector<bool> states);



    // COM //
    // Functions to open/close m_serial
    void openSerialPort();
    void closeSerialPort();

    // Reads serial data and outputs to log.
    // PRECONDITION: m_serial must be open
    void readSerialData();

    // Writes given data to serial port
    // PRECONDITION: m_serial must be open
    void writeSerialData(const char* data);

    // Sends actuator positions
    void sendActuatorPositions();


    // COM //

};


#endif // MAINWINDOW_H
