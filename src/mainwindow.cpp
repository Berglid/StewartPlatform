#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stewartplatform.h"
#include "utilities.h"

#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qscatter3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtCore/qmath.h>
#include <QtCore/qrandom.h>
#include <QtWidgets/QComboBox>
#include <QMessageBox>

#include <QFileDialog>
#include <QTimer>

#include <QStyleFactory>


using namespace QtDataVisualization;


// TODO:
// Clean up naming and declaration/definition
// Must set this in stewart platform instead!!
const int MAX_XZ = 200;
const int MIN_XZ = -MAX_XZ;
const int MAX_Y = 510;
const int MIN_Y = 340;

const int MAX_RXZ = 25; // reduce this if joints cant handle it
const int MIN_RXZ = -MAX_RXZ;

// MainWindow::MainWindow(QWidget *parent, Mouse3DInput* mouse3D): ... , m_mouse(mouse3D)
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serial(new QSerialPort(this))
    , m_settings(new SerialSettingsDialog)
    , statusText(3)
    , isRecording(false)
    , isPlaying(false)
    , isPaused(false)
    , recordingInitiated(false)
    , printDebug(true)
    , darkTheme(false)
    , indexDemo(0)
    , demoSize(0)
    , status(0)
    , fps(60.0f)

{

    ui->setupUi(this);
    graph = new Q3DScatter();
    container = QWidget::createWindowContainer(graph);
    stewart = new StewartPlatform(graph);
    ui->graph_layout->addWidget(container,1);

    if (!graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
        // return -1;
    }


    ////////////////////////////////
    // Setup of recording/playing //
    ////////////////////////////////

    calcPlaybackTime(); // initialize
    ui->timeLabel->setText(playBackTime);

    // DELETE?
    // Initialize status texts
    // TODO: implement as dictionary
    statusText[0] = "Status: User input";
    statusText[1] = "Status: playback";
    statusText[2] = "Status: Recording...";

    // Set unloaded filenames
    // Used to check wether or not a filename has been loaded
    recordFilePath = "Save file not set";
    playFilePath = "Load file not set";

    // TODO:
    // Rename vectorRecording and vectorPlaying
    // Clear demo and recording
    vectorRecording.clear();
    vectorPlaying.clear();
    recordFileName = "Not loaded";
    playFileName = "Not loaded";

    // Setup timer
    float timeIntervall = 1000.0f/fps;
    QTimer* timer = new QTimer();
    timer->setInterval(timeIntervall);
    timer->start();

    /////////////////
    // Setup of boxes
    /////////////////

    // Add logo to play pause stop and record buttons
    QString resetLogo;
    ui->recordButton->setIcon(QIcon(":/logo/record.png"));
    if(darkTheme){
        ui->playButton->setIcon(QIcon(":/logo/play_white.png"));
        ui->pauseButton->setIcon(QIcon(":/logo/pause_white.png"));
        ui->stopButton->setIcon(QIcon(":/logo/stop_white.png"));
        resetLogo = ":/logo/reset_white.png";

        // For framelesswindow setup check here
        // https://stackoverflow.com/questions/2235360/making-a-borderless-window-with-for-qt

        // Set style sheet
        // From https://gist.github.com/Skyrpex/5547015
        qApp->setStyle(QStyleFactory::create("fusion"));
        QPalette palette;
        palette.setColor(QPalette::Window, QColor(53,53,53));
        palette.setColor(QPalette::WindowText, Qt::white);
        palette.setColor(QPalette::Base, QColor(15,15,15));
        palette.setColor(QPalette::AlternateBase, QColor(53,53,53));
        palette.setColor(QPalette::ToolTipBase, Qt::white);
        palette.setColor(QPalette::ToolTipText, Qt::white);
        palette.setColor(QPalette::Text, Qt::white);
        palette.setColor(QPalette::Button, QColor(53,53,53));
        palette.setColor(QPalette::ButtonText, Qt::white);
        palette.setColor(QPalette::BrightText, Qt::red);
        palette.setColor(QPalette::Highlight, QColor(142,45,197).lighter());
        palette.setColor(QPalette::HighlightedText, Qt::black);
        qApp->setPalette(palette);

    }
    else {
        ui->playButton->setIcon(QIcon(":/logo/play.png"));
        ui->pauseButton->setIcon(QIcon(":/logo/pause.png"));
        ui->stopButton->setIcon(QIcon(":/logo/stop.png"));
        resetLogo = ":/logo/reset.png";
    }

    //TODO:
    // LICENSE https://www.123rf.com/license.php#standard
    ui->xResetButton->setIcon(QIcon(resetLogo));
    ui->yResetButton->setIcon(QIcon(resetLogo));
    ui->zResetButton->setIcon(QIcon(resetLogo));
    ui->rxResetButton->setIcon(QIcon(resetLogo));
    ui->ryResetButton->setIcon(QIcon(resetLogo));
    ui->rzResetButton->setIcon(QIcon(resetLogo));



    // DELETE? invalid positions are not updated anyway
    // setRange for spinboxes
    ui->xSpinbox->setRange(MIN_XZ, MAX_XZ);
    ui->ySpinbox->setRange(MIN_Y, MAX_Y);
    ui->zSpinbox->setRange(MIN_XZ, MAX_XZ);
    ui->rxSpinbox->setRange(MIN_RXZ, MAX_RXZ);
    ui->rySpinbox->setRange(-(stewart->getMaxRy()), stewart->getMaxRy());
    ui->rzSpinbox->setRange(MIN_RXZ, MAX_RXZ);

    // TODO:
    // ADD: option to change this from the UI
    // Not prioritized as 3D mouse is the main controller, and accurate positions can be typed into the spinboxes
    // Single step size for spinboxes
    double xSingleStep = 5.0f;
    double ySingleStep = 5.0f;
    double zSingleStep = 5.0f;
    double rxSingleStep = 1.0f;
    double rySingleStep = 1.0f;
    double rzSingleStep = 1.0f;

    // set single step value for spinboxes
    ui->xSpinbox->setSingleStep(xSingleStep);
    ui->ySpinbox->setSingleStep(ySingleStep);
    ui->zSpinbox->setSingleStep(zSingleStep);
    ui->rxSpinbox->setSingleStep(rxSingleStep);
    ui->rySpinbox->setSingleStep(rySingleStep);
    ui->rzSpinbox->setSingleStep(rzSingleStep);

    // set current value for spinboxes
    ui->xSpinbox->setValue(stewart->getX());
    ui->ySpinbox->setValue(stewart->getY());
    ui->zSpinbox->setValue(stewart->getZ());
    ui->rxSpinbox->setValue(stewart->getRX());
    ui->rySpinbox->setValue(stewart->getRY());
    ui->rzSpinbox->setValue(stewart->getRZ());

    //////////////
    // Connections
    //////////////

    // Camera buttons
    QObject::connect(ui->cameraPresetButton, &QPushButton::clicked, stewart, &StewartPlatform::changePresetCamera);
    QObject::connect(ui->cameraHomeButton, &QPushButton::clicked, stewart, &StewartPlatform::setCameraHome);

    // DELETE
    // REPLACED IN TIMER
    // Set actuator home
    // QObject::connect(ui->actuatorsHomeButton, &QPushButton::clicked, stewart, &StewartPlatform::setActuatorsHome);

    // Check boxes
    QObject::connect(ui->gridCheckBox, &QCheckBox::stateChanged, stewart, &StewartPlatform::setGridEnabled);
    QObject::connect(ui->backgroundCheckBox, &QCheckBox::stateChanged, stewart, &StewartPlatform::setBackgroundEnabled);
    QObject::connect(stewart, &StewartPlatform::backgroundEnabledChanged, ui->backgroundCheckBox, &QCheckBox::setChecked);
    // TODO:
    // TODO: Implement show labels

    // Theme, shadow, gridbox
    QObject::connect(ui->themeListComboBox, SIGNAL(currentIndexChanged(int)), stewart, SLOT(changeTheme(int)));
    QObject::connect(ui->shadowQualityComboBox, SIGNAL(currentIndexChanged(int)), stewart, SLOT(changeShadowQuality(int)));
    QObject::connect(stewart, &StewartPlatform::gridEnabledChanged, ui->gridCheckBox, &QCheckBox::setChecked);
    QObject::connect(stewart, &StewartPlatform::shadowQualityChanged, ui->shadowQualityComboBox, &QComboBox::setCurrentIndex);
    QObject::connect(graph, &Q3DScatter::shadowQualityChanged, stewart, &StewartPlatform::shadowQualityUpdatedByVisual);

    QObject::connect(stewart, &StewartPlatform::labelEnabledChanged, ui->labelCheckBox, &QCheckBox::setChecked);
    QObject::connect(ui->labelCheckBox, &QCheckBox::stateChanged, stewart, &StewartPlatform::setLabelEnabled);

    // TODO:
    // Value is updated 4 times when this is connected back to stewart.
    // Stewart updates spinbox, spinbox update stewart, stewart update spinbox, spinbox update stewart? messes with recording
    // Update platform position if values in spinboxes are changed
    QObject::connect(ui->xSpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), stewart, &StewartPlatform::xMove);
    QObject::connect(ui->ySpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), stewart, &StewartPlatform::yMove);
    QObject::connect(ui->zSpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), stewart, &StewartPlatform::zMove);
    QObject::connect(ui->rxSpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), stewart, &StewartPlatform::xRot);
    QObject::connect(ui->rySpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), stewart, &StewartPlatform::yRot);
    QObject::connect(ui->rzSpinbox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), stewart, &StewartPlatform::zRot);

    // update value of spinboxes, if values from stewart is changed (when using 3D mouse)
    // QObject::connect(stewart, &StewartPlatform::xChanged, ui->xSpinbox, &QDoubleSpinBox::setValue);
    // QObject::connect(stewart, &StewartPlatform::yChanged, ui->ySpinbox, &QDoubleSpinBox::setValue);
    // QObject::connect(stewart, &StewartPlatform::zChanged, ui->zSpinbox, &QDoubleSpinBox::setValue);
    // QObject::connect(stewart, &StewartPlatform::rXChanged, ui->rxSpinbox, &QDoubleSpinBox::setValue);
    // QObject::connect(stewart, &StewartPlatform::rYChanged, ui->rySpinbox, &QDoubleSpinBox::setValue);
    // QObject::connect(stewart, &StewartPlatform::rZChanged, ui->rzSpinbox, &QDoubleSpinBox::setValue);

    // Connect mouse to stewart
    // QObject::connect(mouse3D, SIGNAL(Move3d(QVector<float>&)), stewart, SLOT(OnMove(QVector<float>&)));

    // Playback: Connect this to stewart. Send motion data from demo
    QObject::connect(this, SIGNAL(Move3d(QVector<float>&)), stewart, SLOT(OnMove(QVector<float>&)));

    //TODO:
    // Move all dependted things on valid move to this function
    QObject::connect(stewart, &StewartPlatform::validMove, this, &MainWindow::onValidMove);

    // Timeout
    QObject::connect(timer, &QTimer::timeout, this, &MainWindow::onTimeOut);

    // TODO:
    // Make enum or dictionary for status
    // Initialize status labels
    ui->labelSerialStatus->setText("Serial: Disconnected");
    ui->labelModeStatus->setText(  "Mode: User Input");
    ui->labelDemoPlaying->setText("Demo: " + playFileName);


    /** Serial-related signals **/
    // Serial logging
    connect(m_serial, &QSerialPort::readyRead, this, &MainWindow::readSerialData);

    // Serial Settings dialog
    connect(ui->actionSelect_COM_port, &QAction::triggered, m_settings, &SerialSettingsDialog::ShowAndUpdatePortInfo);

    // Connect to serial port
    connect(m_settings, &SerialSettingsDialog::SettingsUpdated, this, &MainWindow::openSerialPort);

    // Disconnect from serial port if something happens
    // todo: make dedicated error handling function, for now disconnecting the port is good enough
    connect(m_serial, &QSerialPort::errorOccurred, this, &MainWindow::closeSerialPort);

}

// Default deconstructor
MainWindow::~MainWindow()
{
    delete ui;
}

// Load demo from file
void MainWindow::loadDemo(){

    QString filter = "Text file (*.txt)";
    playFilePath = QFileDialog::getOpenFileName(this, "Open demo file ", QDir::homePath(), filter);

    updatePlayFileName();


    QFile fileIn(playFilePath);
    // Change to QFie::DOUBLE
    if(!fileIn.open(QFile::ReadOnly | QFile::Text)){
        QMessageBox::warning(this, "title", "file not open");
        return;
    }



    // Clear vector before adding new recording
    vectorPlaying.clear();

    QVector<float> buffer(6);
    int count = 0;

    QTextStream in(&fileIn);
    while(!in.atEnd()){
        for(int i = 0; i < buffer.size(); i++){
            in >> buffer[i];
        }
        vectorPlaying.append(buffer);
        count++;
    }

    fileIn.flush();
    fileIn.close();

    demoSize = vectorPlaying.size();

    if(isPaused){
        resetPlayPauseStopRecordButtons();
    }

    indexDemo = 0;
    calcPlaybackTime();

}

void MainWindow::updatePlayFileName(){
    QString temp;
    for(int i = (playFilePath.size()-5); i > 0 ; i--){
        if(playFilePath[i] == "/"){
            break;
        }
        temp.prepend(playFilePath[i]);
    }
    playFileName = temp;
    ui->labelDemoPlaying->setText("Demo: " + playFileName);
}


// Save recording to file
void MainWindow::saveRecording(QString filename){
    QFile fileOut(filename);

    if(!fileOut.open(QFile::WriteOnly | QFile::Text)){
        QMessageBox::warning(this, "title", "file not open");
        return;
    }

    QTextStream out(&fileOut);
    for(int i = 0; i < vectorRecording.size(); i++){
        QVector<float>::iterator it = vectorRecording[i].begin();
        while(it != vectorRecording[i].end()){
            out << *it;
            out << " ";
            it++;
        }
        out << "\n";
    }
    fileOut.flush();
    fileOut.close();
    vectorRecording.clear();
}

// Recording button toggled
void MainWindow::on_recordButton_toggled(bool checked)
{
    QString filter = "Text file (*.txt)";
    if(checked){
        isRecording = true;
        recordingInitiated = true;
        recordFilePath = QFileDialog::getSaveFileName(this, "Save recording as ", QDir::homePath(), filter);


        // Disable buttons
        ui->playButton->setDisabled(true);
        ui->pauseButton->setDisabled(true);
        ui->stopButton->setDisabled(true);

        ui->labelModeStatus->setText("Mode: Recording...");

    } else {
        isRecording = false;

        //
        if(recordingInitiated == true){
            saveRecording(recordFilePath);
            recordingInitiated = false;
        }
        resetPlayPauseStopRecordButtons();
    }
}

// Stop button clicked
void MainWindow::on_stopButton_clicked()
{
    resetPlayPauseStopRecordButtons();
    if(ui->playButton->isChecked()){
        resetPlayPauseStopRecordButtons();
    }
    if(ui->actuatorsHomeButton->isChecked()){
        if(isFollowingGeneratedPath){
            resetHoming();
        }
    }
}

// Resets all buttons and states
void MainWindow::resetPlayPauseStopRecordButtons(){
    ui->playButton->setEnabled(true);
    ui->pauseButton->setEnabled(true);
    ui->stopButton->setEnabled(true);
    ui->recordButton->setEnabled(true);

    ui->playButton->setChecked(false);
    ui->recordButton->setChecked(false);
    ui->pauseButton->setChecked(false);

    isRecording = false;
    isPlaying = false;
    isPaused = false;

    indexDemo = 0; // reset demo
    calcPlaybackTime();

    ui->labelModeStatus->setText("Mode: User Input");
    resetHoming();
}



// Pause button clicked
void MainWindow::on_pauseButton_clicked()
{
    if(isPlaying){
        isPlaying = false;
        isPaused = true;
        ui->pauseButton->setChecked(true);
        ui->labelModeStatus->setText("Mode: Paused");
    }

    else if ((ui->playButton->isChecked() == true) && (isPaused == true)){
        isPlaying = true;
        isPaused = false;
        ui->pauseButton->setChecked(false);
        ui->labelModeStatus->setText("Mode: Playing demo...");
        MoveToPreviosDemoPose();
    }


    else {
        ui->pauseButton->setChecked(false);
    }
}



// Play button clicked
void MainWindow::on_playButton_clicked()
{
    if((isPlaying == 0) && (isPaused == 0) && (isRecording == 0)){
        ui->playButton->setChecked(true);
        isPlaying = true;

        if(playFileName == "Not loaded"){
            loadDemo();
        }

        // Create path to beginning of demo
        // Timer loop moves the generated path before starting the demo
        vectorGeneratedPath.clear();
        isFollowingGeneratedPath = true;
        vectorGeneratedPath = stewart->generatePath(stewart->getPose(), vectorPlaying[indexDemo]);


        ui->recordButton->setDisabled(true);
        ui->actuatorsHomeButton->setDisabled(true);
        ui->timeLabel->setText(statusTextPlaying);
        ui->labelModeStatus->setText("Mode: Playing demo...");

    } else if (isPaused){
        ui->playButton->setChecked(true);
        ui->pauseButton->setChecked(false);
        isPlaying = true;
        isPaused = false;
        ui->labelModeStatus->setText("Mode: Playing demo...");
        MoveToPreviosDemoPose();

    } else if (isPlaying){
        ui->playButton->setChecked(true);
    } else {
        ui->playButton->setChecked(false);
        resetPlayPauseStopRecordButtons();
    }
}




void MainWindow::onValidMove(){
    // If valid move is registered from stewart platform, and isRecording is set to true, position is added to recording
    // This means that "Standstill moves" are not recorded.
    // Can change this behavior if wanted, but for now its ok, as this function is only made to test playback of demo
    if(isRecording){
        vectorRecording.push_back(stewart->getPose()); // Append vector to recording
    }

    if (ui->enableComButton->isChecked()){
        sendActuatorPositions();
    }
}


// Debug: Prints vector
void MainWindow::printVector(QVector<float> vec){

    QDebug deb = qDebug();
    for(int i = 0; i < vec.size() + 1; i++){
        if(i == 0){
        }else {
            deb << vec[i-1];
        }
    }
}

// returns a string containing the current playback time of the recording and total duration of demo
void MainWindow::calcPlaybackTime(){
    int totalDuration; // time in seconds
    int currentPlayBackTime; // time in seconds

    if(demoSize == 0){
        totalDuration = 0; // time in seconds
        currentPlayBackTime = 0; // time in seconds
        playBackPercentage = 0;
    } else {
        totalDuration = (int)(demoSize/fps); // time in seconds
        currentPlayBackTime = (int)(indexDemo/fps); // time in seconds
        playBackPercentage = (float)(currentPlayBackTime/totalDuration); // Use this to update slider
    }

    playBackTime = secondsToStringHoursMinSeconds(currentPlayBackTime) + "/" + secondsToStringHoursMinSeconds(totalDuration);
    ui->timeLabel->setText(playBackTime);
}


//////////////////////////////
/// COM
//////////////////////////////


void MainWindow::readSerialData()
{
    Q_ASSERT(m_serial->isOpen());
    log(m_serial->readAll());
    // ui->log->appendPlainText(m_serial->readAll());
}

void MainWindow::writeSerialData(const char* data)
{
    Q_ASSERT(m_serial->isOpen());
    m_serial->write(data);
}

void MainWindow::openSerialPort()
{
    // Close the existing serial handle
    closeSerialPort();

    // Set configuration received from the serial dialog box
    const SerialSettingsDialog::Settings p = m_settings->settings();
    m_serial->setPortName(p.name);
    m_serial->setBaudRate(p.baudRate);
    m_serial->setDataBits(p.dataBits);
    m_serial->setParity(p.parity);
    m_serial->setStopBits(p.stopBits);
    m_serial->setFlowControl(p.flowControl);

    // If able to connect to the port, update the status and enable the send button
    // Else, produce an error
    if (m_serial->open(QIODevice::ReadWrite))
    {
        // TODO
        // SPLIT LABEL STATUSES TO TWO LABESL)
        ui->labelSerialStatus->setText(tr("Serial: %1").arg(p.name));

        ui->enableComButton->setEnabled(true);
        ui->enableComButton->setChecked(false);
        ui->enableComButton->setText("Enable serial COM");


        log(
            tr("<COM>  Connected to %1 : %2, %3, %4, %5, %6")
                .arg(p.name)
                .arg(p.stringBaudRate)
                .arg(p.stringDataBits)
                .arg(p.stringParity)
                .arg(p.stringStopBits)
                .arg(p.stringFlowControl)
        );
    }
    else
    {
        log(tr("<COM>  Connection error: %1").arg(m_serial->errorString()));
    }
}

void MainWindow::closeSerialPort()
{
    if (m_serial->isOpen())
    {
        m_serial->close();
        log(tr("<COM>  Disconnected from %1").arg(m_serial->portName()));

        // Update status
        ui->labelSerialStatus->setText(tr("Serial: Disconnected"));

        ui->enableComButton->setChecked(false);
        ui->enableComButton->setEnabled(false);
        ui->enableComButton->setText("Enable Serial COM");
    }
}


/// //////////////////////////////
/// Progressive
//////////////////////////////


// FIX NEWLINE WITH qDebug and add log window
void MainWindow::log(const QString& entry)
{
    // ui->log->appendPlainText("(" + QTime::currentTime().toString() + ") " + entry);
    qDebug() << "(" + QTime::currentTime().toString() + ") " + entry;



}


//TODO:
// ADD BUTTON THAT ONLY ALLOWS SENDING WHILE THIS IS TRUE
// CONNECT SPINBOXES SUCH THAT WHEN THEY CHANGE THEY SEND ACTUATORPOSITIONS
void MainWindow::sendActuatorPositions()
{
    // TODO:
    // ONLY SEND IF CHECKBOX CHECKED
    // Send data if able to connect to the port
    if (ui->enableComButton->isChecked()){
        QVector<int> actuator_pos = stewart->getActuatorData();

        QString s = "";
        for(int i = 0; i < NUM_ACTUATORS; ++i)
        {
            // Convert each actuator int to string, add space if not last element, else new line
            s += QString::number(actuator_pos[i]) + ((i < (NUM_ACTUATORS - 1)) ? " " : "\n");
        }
        writeSerialData(qPrintable(s));

        qDebug() << "Sending data to arduino: " << s;

    }

}

/// //////////////////////////////
/// Progressive
//////////////////////////////



/// //////////////////////////////
/// COM
//////////////////////////////

void MainWindow::on_actionLoad_from_file_triggered()
{

    calcPlaybackTime();
    loadDemo();
}





void MainWindow::MoveToPreviosDemoPose(){
    // generate path if it have been moved while paused
    if(indexDemo != 0){
        if(stewart->getPose() != vectorPlaying[indexDemo - 1]){
            vectorGeneratedPath.clear();
            isFollowingGeneratedPath = true;
            vectorGeneratedPath = stewart->generatePath(stewart->getPose(), vectorPlaying[indexDemo-1]);
        }
    }
}

void MainWindow::resetHoming(){
    ui->actuatorsHomeButton->setEnabled(true);
    ui->actuatorsHomeButton->setChecked(false);
    indexGeneratedPath = 0;
    vectorGeneratedPath.clear();
    isFollowingGeneratedPath = false;
    ui->playButton->setEnabled(true);
    ui->pauseButton->setEnabled(true);
}

// function called at set interval (FPS)
// checks wether demo or homing operation should be run
void MainWindow::onTimeOut(){
    // Check if following generated path
    if(isFollowingGeneratedPath){

        // Check if the generated path is for homing
        if(isHoming){
            ui->playButton->setDisabled(true);
            ui->pauseButton->setDisabled(true);
        }
        if((indexGeneratedPath >= 0) && (indexGeneratedPath < vectorGeneratedPath.size())){
            stewart->move3D(vectorGeneratedPath[indexGeneratedPath]);
            indexGeneratedPath++;

            // reached end of generated path.. reset
            if(indexGeneratedPath == vectorGeneratedPath.size()){
                enableHomingButtons();

                indexGeneratedPath = 0;
                vectorGeneratedPath.clear();
                isFollowingGeneratedPath = false;
                ui->playButton->setEnabled(true);
                ui->pauseButton->setEnabled(true);
            }
        }
    }

    // else Check if we're currently running demo
    else if(isPlaying){
        if((indexDemo >= 0) && (indexDemo < demoSize)){
            stewart->move3D(vectorPlaying[indexDemo]);
            indexDemo++;
            calcPlaybackTime();

            // Reached end of demo. reset
            if(indexDemo == demoSize){
                resetPlayPauseStopRecordButtons();
                ui->actuatorsHomeButton->setEnabled(true);
            }
        }
    }
}

// DELETE
// states 0: actuatorshome, 1: xResetbutton .... 6: rzResetbutton
void MainWindow::homingButtonXaction(QVector<bool> states){
   ui->actuatorsHomeButton->setEnabled(states[0]);
}

// CLEAN UP AND REDUCE AMOUNT OF FUNCTION TO DO THIS
// LAMBDA EXPRESSION?
// OR EMIT SIGNAL?
void MainWindow::on_actuatorsHomeButton_toggled(bool checked)
{
    if(checked){
        // ui->actuatorsHomeButton->setEnabled(false);
        ui->xResetButton->setEnabled(false);
        ui->yResetButton->setEnabled(false);
        ui->zResetButton->setEnabled(false);
        ui->rxResetButton->setEnabled(false);
        ui->ryResetButton->setEnabled(false);
        ui->rzResetButton->setEnabled(false);

        vectorGeneratedPath.clear();

        ui->actuatorsHomeButton->setChecked(true);
        isFollowingGeneratedPath = true;
        isHoming = true;

        vectorGeneratedPath = stewart->generatePath(stewart->getPose(), stewart->getHomePose());
    }

}

void MainWindow::on_xResetButton_toggled(bool checked)
{
    if(checked){
        ui->actuatorsHomeButton->setEnabled(false);
        // ui->xResetButton->setEnabled(false);
        ui->yResetButton->setEnabled(false);
        ui->zResetButton->setEnabled(false);
        ui->rxResetButton->setEnabled(false);
        ui->ryResetButton->setEnabled(false);
        ui->rzResetButton->setEnabled(false);

        vectorGeneratedPath.clear();
        isFollowingGeneratedPath = true;
        isHoming = true;


        QVector<float> destinationPose = stewart->getPose();
        QVector<float> stewartHome = stewart->getHomePose();

        destinationPose[0] = stewartHome[0];

        vectorGeneratedPath = stewart->generatePath(stewart->getPose(), destinationPose);
    }
}

void MainWindow::on_yResetButton_toggled(bool checked)
{
    if(checked){
        ui->actuatorsHomeButton->setEnabled(false);
        ui->xResetButton->setEnabled(false);
        // ui->yResetButton->setEnabled(false);
        ui->zResetButton->setEnabled(false);
        ui->rxResetButton->setEnabled(false);
        ui->ryResetButton->setEnabled(false);
        ui->rzResetButton->setEnabled(false);
        vectorGeneratedPath.clear();
        isFollowingGeneratedPath = true;
        isHoming = true;


        QVector<float> destinationPose = stewart->getPose();
        QVector<float> stewartHome = stewart->getHomePose();

        destinationPose[1] = stewartHome[1];

        vectorGeneratedPath = stewart->generatePath(stewart->getPose(), destinationPose);
    }
}


void MainWindow::on_zResetButton_toggled(bool checked)
{
    if(checked){
        ui->actuatorsHomeButton->setEnabled(false);
        ui->xResetButton->setEnabled(false);
        ui->yResetButton->setEnabled(false);
        // ui->zResetButton->setEnabled(false);
        ui->rxResetButton->setEnabled(false);
        ui->ryResetButton->setEnabled(false);
        ui->rzResetButton->setEnabled(false);
        vectorGeneratedPath.clear();
        isFollowingGeneratedPath = true;
        isHoming = true;

        QVector<float> destinationPose = stewart->getPose();
        QVector<float> stewartHome = stewart->getHomePose();

        destinationPose[2] = stewartHome[2];

        vectorGeneratedPath = stewart->generatePath(stewart->getPose(), destinationPose);
    }
}

void MainWindow::on_rxResetButton_toggled(bool checked)
{
    if(checked){
        ui->actuatorsHomeButton->setEnabled(false);
        ui->xResetButton->setEnabled(false);
        ui->yResetButton->setEnabled(false);
        ui->zResetButton->setEnabled(false);
        // ui->rxResetButton->setEnabled(false);
        ui->ryResetButton->setEnabled(false);
        ui->rzResetButton->setEnabled(false);

        vectorGeneratedPath.clear();
        isFollowingGeneratedPath = true;
        isHoming = true;

        QVector<float> destinationPose = stewart->getPose();
        QVector<float> stewartHome = stewart->getHomePose();

        destinationPose[3] = stewartHome[3];

        vectorGeneratedPath = stewart->generatePath(stewart->getPose(), destinationPose);
    }
}

void MainWindow::on_ryResetButton_toggled(bool checked)
{
    if(checked){
        ui->actuatorsHomeButton->setEnabled(false);
        ui->xResetButton->setEnabled(false);
        ui->yResetButton->setEnabled(false);
        ui->zResetButton->setEnabled(false);
        ui->rxResetButton->setEnabled(false);
        // ui->ryResetButton->setEnabled(false);
        ui->rzResetButton->setEnabled(false);

        vectorGeneratedPath.clear();
        isFollowingGeneratedPath = true;
        isHoming = true;

        QVector<float> destinationPose = stewart->getPose();
        QVector<float> stewartHome = stewart->getHomePose();

        destinationPose[4] = stewartHome[4];

        vectorGeneratedPath = stewart->generatePath(stewart->getPose(), destinationPose);
    }
}

// TODO:
// Make vector that stores pointer for these buttons
// reimplement with loop
void MainWindow::on_rzResetButton_toggled(bool checked)
{
    if(checked){
        ui->actuatorsHomeButton->setEnabled(false);
        ui->xResetButton->setEnabled(false);
        ui->yResetButton->setEnabled(false);
        ui->zResetButton->setEnabled(false);
        ui->rxResetButton->setEnabled(false);
        ui->ryResetButton->setEnabled(false);
        // ui->rzResetButton->setEnabled(false);



        vectorGeneratedPath.clear();
        isFollowingGeneratedPath = true;
        isHoming = true;

        QVector<float> destinationPose = stewart->getPose();
        QVector<float> stewartHome = stewart->getHomePose();

        destinationPose[5] = stewartHome[5];

        vectorGeneratedPath = stewart->generatePath(stewart->getPose(), destinationPose);
    }
}


// TODO:
// Make vector that stores pointer for these buttons
// reimplement with loop
void MainWindow::disableHomingButtons(){
    ui->actuatorsHomeButton->setEnabled(false);
    ui->xResetButton->setEnabled(false);
    ui->yResetButton->setEnabled(false);
    ui->zResetButton->setEnabled(false);
    ui->rxResetButton->setEnabled(false);
    ui->ryResetButton->setEnabled(false);
    ui->rzResetButton->setEnabled(false);
}

// TODO:
// Make vector that stores pointer for these buttons
// reimplement with loop
void MainWindow::enableHomingButtons(){
    ui->actuatorsHomeButton->setChecked(false);
    ui->xResetButton->setChecked(false);
    ui->yResetButton->setChecked(false);
    ui->zResetButton->setChecked(false);
    ui->rxResetButton->setChecked(false);
    ui->ryResetButton->setChecked(false);
    ui->rzResetButton->setChecked(false);

    ui->actuatorsHomeButton->setEnabled(true);
    ui->xResetButton->setEnabled(true);
    ui->yResetButton->setEnabled(true);
    ui->zResetButton->setEnabled(true);
    ui->rxResetButton->setEnabled(true);
    ui->ryResetButton->setEnabled(true);
    ui->rzResetButton->setEnabled(true);
}
