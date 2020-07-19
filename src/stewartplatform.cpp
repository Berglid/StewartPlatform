#include "stewartplatform.h"
#include "utilities.h"
#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qscatter3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtWidgets/QComboBox>
#include <QtMath>
#include <QMatrix4x4>
#include <Qt>

#include <QFile>
#include <QTextStream>
#include <QFileDialog>

#include <iostream>
#include <QDebug>

#include <QTime>

using namespace QtDataVisualization;

StewartPlatform::StewartPlatform(Q3DScatter *scatter)
    : m_graph(scatter),
      m_fontSize(12.0f),
      m_smooth(true),
      x(x_home),
      y(y_home),
      z(z_home),
      rx(rx_home),
      ry(ry_home),
      rz(rz_home),
      meshScale(2000.0f, 2000.0f, 2000.0f),
      itemUpperActuators(actuatorCount),
      itemLowerActuatorsLight(actuatorCount),
      itemLowerActuatorsMedium(actuatorCount),
      itemLowerActuatorsDark(actuatorCount),
      actuatorLengths(actuatorCount, actuatorMinLength),
      platformPointsInSFrame(actuatorCount),
      baseToPlatformInSFrame(actuatorCount),
      lowerActuatorQuaternion(actuatorCount)

{    
    initializeGraph();
    setCameraHome();
    initializeMeshes();
    updateMeshPoses(getPose());
}

// deconstructor
StewartPlatform::~StewartPlatform()
{
    delete m_graph;
}

// Initialize and configure graph
void StewartPlatform::initializeGraph(){
    m_graph->activeTheme()->setType(Q3DTheme::ThemeDigia);
    QFont font = m_graph->activeTheme()->font();
    font.setPointSize(m_fontSize);
    m_graph->activeTheme()->setFont(font);
    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);
    m_graph->activeTheme()->setBackgroundEnabled(false);
    m_graph->activeTheme()->setGridEnabled(false);
    m_graph->activeTheme()->setLabelBorderEnabled(false);
    setLabelEnabled(false);
    m_graph->axisX()->setTitle("X");
    m_graph->axisY()->setTitle("Y");
    m_graph->axisZ()->setTitle("Z");
    m_graph->setAspectRatio(1.0);
    m_graph->setHorizontalAspectRatio(1.0);

    m_graph->axisZ()->setReversed(true); // Seems like Z axis was flipped by default, creating an "invalid" coordinate frame, thus reverse the axis

    int plotSize = 840;
    int radius = plotSize/2; // keep aspect ratio to get correct scaling of meshes
    int yMin = - 120;
    int yMax = plotSize + yMin ;

    m_graph->axisX()->setMax(radius);
    m_graph->axisX()->setMin(-radius);
    m_graph->axisY()->setMax(yMax);
    m_graph->axisY()->setMin(yMin);
    m_graph->axisZ()->setMax(radius);
    m_graph->axisZ()->setMin(-radius);
}


// Adds mesh to graph
void StewartPlatform::addItem(QCustom3DItem*& rItem, const QString& rMeshFile, const QString& rTextureFile, const QVector3D& rPosition){
    rItem = new QCustom3DItem;
    rItem->setMeshFile(rMeshFile);
    rItem->setPosition(rPosition);
    rItem->setTextureFile(rTextureFile);
    rItem->setScalingAbsolute(false);
    rItem->setScaling(meshScale);
    m_graph->addCustomItem(rItem);
}

// Initializes meshes and add them to graph
void StewartPlatform::initializeMeshes()
{
    // Texture files
    const QString darkGray(":/textures/gray45.png");
    const QString mediumGray(":/textures/grey159.png");
    const QString mediumLightGray(":/textures/grey189.png");
    const QString lightGray(":/textures/gray210.png");

    // Mesh files
    const QString meshTopPlateLight(":/obj/top_plate_light.obj");
    const QString meshTopPlateDark(":/obj/top_plate_dark.obj");
    const QString meshBottomPlateLight(":/obj/bottom_plate_light.obj");
    const QString meshBottomPlateDark(":/obj/bottom_plate_dark.obj");
    const QString meshLowerActuatorLight(":/obj/Lower_actuator_light.obj");
    const QString meshLowerActuatorMedium(":/obj/lower_actuator_medium.obj");
    const QString meshLowerActuatorDark(":/obj/lower_actuator_dark.obj");
    const QString meshUpperActuator(":/obj/upper_actuator.obj");

    // Add top plate
    addItem(itemTopPlateDark, meshTopPlateDark, darkGray, QVector3D(x, y, z));
    addItem(itemTopPlateLight, meshTopPlateLight, mediumGray, QVector3D(x, y, z));

    // Add bottom plate
    addItem(itemBottomPlateLight, meshBottomPlateLight, mediumGray, QVector3D(0, 0, 0));
    addItem(itemBottomPlateDark, meshBottomPlateDark, darkGray, QVector3D(0, 0, 0));

    // Add upper/lower actuators
    for(int i = 0; i < actuatorCount; i++){
        addItem(itemLowerActuatorsLight[i], meshLowerActuatorLight, lightGray, basePoints[i]);
        addItem(itemLowerActuatorsMedium[i], meshLowerActuatorMedium, mediumLightGray, basePoints[i]);
        addItem(itemLowerActuatorsDark[i], meshLowerActuatorDark, darkGray, basePoints[i]);
        addItem(itemUpperActuators[i], meshUpperActuator, lightGray, platformPointsInSFrame[i]);
    }
}

// change theme
void StewartPlatform::changeTheme(int theme)
{
    Q3DTheme *currentTheme = m_graph->activeTheme();
    switch (theme) {
    case 0:  { currentTheme->setType(Q3DTheme::ThemeQt); break; }
    case 1:  { currentTheme->setType(Q3DTheme::ThemeArmyBlue); break; }
    case 2:  { currentTheme->setType(Q3DTheme::ThemeEbony); break; }
    default: { break; }
    }

    currentTheme->setBackgroundEnabled(false);
    currentTheme->setGridEnabled(false);
    setLabelEnabled(false);

    emit backgroundEnabledChanged(currentTheme->isBackgroundEnabled());
    emit gridEnabledChanged(currentTheme->isGridEnabled());
}

// change camera preset
void StewartPlatform::changePresetCamera()
{
    static int preset = Q3DCamera::CameraPresetFrontLow;
    m_graph->scene()->activeCamera()->setCameraPreset((Q3DCamera::CameraPreset)preset);
    if (++preset > Q3DCamera::CameraPresetDirectlyBelow){
        preset = Q3DCamera::CameraPresetFrontLow;
    }
}

// shadow quality changed
void StewartPlatform::shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality sq)
{
    int quality = int(sq);
    emit shadowQualityChanged(quality);
}

// change shadow quality
void StewartPlatform::changeShadowQuality(int quality)
{
    QAbstract3DGraph::ShadowQuality sq = QAbstract3DGraph::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
}

// enable background
void StewartPlatform::setBackgroundEnabled(int enabled) { m_graph->activeTheme()->setBackgroundEnabled((bool)enabled);}

// enable grid
void StewartPlatform::setGridEnabled(int enabled) { m_graph->activeTheme()->setGridEnabled((bool)enabled); }

// TODO:
// Very ugly solution
// Sets the label color to the background color
// Overwrite Q3Dtheme?
// enable label
void StewartPlatform::setLabelEnabled(int enabled){
    isLabelEnabled = enabled;
    emit(labelEnabledChanged(enabled));
    QColor labelColor;
    Q3DTheme *currentTheme = m_graph->activeTheme();

    if(isLabelEnabled == false){
        labelColor = currentTheme->backgroundColor();
    }
    else {
        if(currentTheme->type() == Q3DTheme::ThemeEbony){
            labelColor = QColor(170,170,170); // Set to gray
        }
        else {
            labelColor = QColor(0, 0, 0); // set to black
        }
    }
    currentTheme->setLabelTextColor(labelColor);
    currentTheme->setLabelBackgroundColor(currentTheme->backgroundColor());
    QFont arialfont("Arial", m_fontSize);
    currentTheme->setFont(arialfont);
    currentTheme->setLabelBorderEnabled(false);
}

// Move platform to home position
void StewartPlatform::setActuatorsHome(){
    // updateMeshPoses(QVector<float>{ x_home, y_home, z_home, rx_home, ry_home, rz_home }); // DELETE
    // updateMeshPoses(getHomePose());
    QVector<QVector<float>> homePath = generatePath(getPose(), getHomePose());
    for(int i = 0; i < homePath.size(); i++){
        move3D(homePath[i]);
    }

    /// MOVE TO TIMER IN MAINWINDOW TO UPDATE WITH FPS

}

// Move platform x direction
void StewartPlatform::xMove(double value){
    QVector<float> tempPose = getPose();
    tempPose[0] = value;
    updateMeshPoses(tempPose);
}

// Move platform y direction
void StewartPlatform::yMove(double value){
    QVector<float> tempPose = getPose();
    tempPose[1] = value;
    updateMeshPoses(tempPose);
}

// Move platform z direction
void StewartPlatform::zMove(double value){
    QVector<float> tempPose = getPose();
    tempPose[2] = value;
    updateMeshPoses(tempPose);
}

// Rotate platform about x axis (roll)
void StewartPlatform::xRot(double value){
    QVector<float> tempPose = getPose();
    tempPose[3] = value;
    updateMeshPoses(tempPose);
}

// Rotate platform about y axis (pitch)
void StewartPlatform::yRot(double value){
    QVector<float> tempPose = getPose();
    tempPose[4] = value;
    updateMeshPoses(tempPose);
}

// Rotate platform about z axis (yaw)
void StewartPlatform::zRot(double value){
    QVector<float> tempPose = getPose();
    tempPose[5] = value;
    updateMeshPoses(tempPose);
}

// get positions
double StewartPlatform::getX(){ return x;}
double StewartPlatform::getY(){ return y;}
double StewartPlatform::getZ(){ return z;}
double StewartPlatform::getRX(){ return rx;}
double StewartPlatform::getRY(){ return ry;}
double StewartPlatform::getRZ(){ return rz; }
float StewartPlatform::getMaxRy(){ return ry_max; }
QVector<float> StewartPlatform::getPose(){ return QVector<float>{x,y,z,rx,ry,rz}; }
QVector<float> StewartPlatform::getHomePose(){ return QVector<float> {x_home, y_home, z_home, rx_home, ry_home, rz_home}; }


// Returns pose in string format used for serial communication
QVector<int> StewartPlatform::getActuatorData(){

    float interScale = 1023.0f/150.0f;

    QVector<int> actuatorData(actuatorCount);

    for(int i = 0; i < actuatorCount; i++){
        actuatorData[i] = round((actuatorLengths[i] - actuatorMinLength)*interScale); // convert actuatorLengths to value between 0 and 1023
    }

    return actuatorData;
}

// Returns rotation matrix for platform
QMatrix3x3 StewartPlatform::getRotationMatrix(){
    float RxValues[] = {1,          0,           0,
                        0, cosd(this->rx), -sind(this->rx),
                        0, sind(this->rx), cosd(this->rx)};

    float RyValues[] = {cosd(this->ry), 0, sind(this->ry),
                                  0, 1,           0,
                       -sind(this->ry), 0, cosd(this->ry)};

    float RzValues[] = {cosd(this->rz), -sind(this->rz), 0,
                        sind(this->rz),  cosd(this->rz), 0,
                                0,          0, 1 };

    QMatrix3x3 Rx(RxValues);
    QMatrix3x3 Ry(RyValues);
    QMatrix3x3 Rz(RzValues);
    return Rx*Ry*Rz;
}

// Overload: Returns the composite rotation matrix of R(roll) R(pitch) R(yaw)
QMatrix3x3 StewartPlatform::getRotationMatrix(float roll, float pitch, float yaw){


    float RxValues[] = {1,          0,           0,
                        0, cosd(roll), -sind(roll),
                        0, sind(roll), cosd(roll)};

    float RyValues[] = {cosd(pitch), 0, sind(pitch),
                                  0, 1,           0,
                       -sind(pitch), 0, cosd(pitch)};

    float RzValues[] = {cosd(yaw), -sind(yaw), 0,
                        sind(yaw),  cosd(yaw), 0,
                                0,          0, 1 };

    QMatrix3x3 Rx(RxValues);
    QMatrix3x3 Ry(RyValues);
    QMatrix3x3 Rz(RzValues);
    return Rx*Ry*Rz;
}

// Returns transformation matrix for platform
QMatrix4x4 StewartPlatform::getTransformationMatrix(){

    QMatrix3x3 R = getRotationMatrix();
    QMatrix4x4 trans(R(0,0), R(0,1), R(0,2), x,
                     R(1,0), R(1,1), R(1,2), y,
                     R(2,0), R(2,1), R(2,2), z,
                          0,      0,      0, 1);
    return trans;
}

// Returns transformation matrix for given pose
QMatrix4x4 StewartPlatform::getTransformationMatrix(QVector<float> pose){
    QMatrix3x3 R = getRotationMatrix(pose[3], pose[4], pose[5]);
    QMatrix4x4 trans(R(0,0), R(0,1), R(0,2), pose[0],
                     R(1,0), R(1,1), R(1,2), pose[1],
                     R(2,0), R(2,1), R(2,2), pose[2],
                          0,      0,      0,           1);
    return trans;
}

// FIX: Set quaternion directly from euler angles
// Returns quaternion for platform
QQuaternion StewartPlatform::getQuaternion(){
    QMatrix3x3 R = getRotationMatrix();
    return  QQuaternion::fromRotationMatrix(R);
}



//TODO::
// ADD bool to function Return true if valid pose
// RENAME?
// calculates platform parameters for given pose. If leg lengths are valid, stewart platforms parameters is updated
bool StewartPlatform::updatePlatformParameters(QVector<float> pose){
    QMatrix4x4 trans = getTransformationMatrix(pose);

    QVector<QVector3D> tempPlatformPointsInSFrame(actuatorCount);
    QVector<QVector3D> tempBaseToPlatformInSFrame(actuatorCount);
    QVector<float> tempActuatorLengths(actuatorCount);

    // calculate temporary vectors and actuator lengths
    for(int i = 0; i < actuatorCount; i++){
        QVector4D temp = trans * platformPoints[i];
        tempPlatformPointsInSFrame[i] = temp.toVector3D();

        tempBaseToPlatformInSFrame[i] = (tempPlatformPointsInSFrame[i] - basePoints[i]);
        tempActuatorLengths[i] = static_cast<int>((tempPlatformPointsInSFrame[i] - basePoints[i]).length()); // leg lengths total in mm
    }


    // if actuator lenghs are valid, then update stewart platform
    if (isValidLengths(tempActuatorLengths)){
        setPose(pose); // actuator lenghts are valid, set coordinates

        // Points were valid, update parameters
        platformPointsInSFrame = tempPlatformPointsInSFrame;
        baseToPlatformInSFrame = tempBaseToPlatformInSFrame;
        actuatorLengths = tempActuatorLengths;

        // calculate lower actuators rotation
        for(int i = 0; i < actuatorCount; i++){
            QQuaternion q;
            QVector3D a;
            QVector3D v1(0, 1, 0); // Y-unit vector
            QVector3D v2 = baseToPlatformInSFrame[i];
            v1.normalize();
            v2.normalize();
            a = QVector3D::crossProduct(v1, v2);

            // qDebug() << "base point" << i; // << ":" << basePoints[i] << "platform point" << "i" << platformPointsInSFrame[i] << "vector" << a.x() << a.y << a.z;
            q.setVector(a);
            q.setScalar(qSqrt(qPow(v1.length(), 2)) * qSqrt(qPow(v2.length(), 2)) + QVector3D::dotProduct(v1, v2));
            q.normalize();
            lowerActuatorQuaternion[i] = q;
        }
        return true;
    }
    else {
        // Pose gave invalid leg lengths
        return false;
    }
}


// updates all meshes if given pose for platform is valid
void StewartPlatform::updateMeshPoses(QVector<float> pose){

    // dont update if roll is not within limits
    if((pose[4] > ry_max) || (pose[4] < ry_min)){
        return;
    }

    // Set new pose for meshes if pose result in valid parameters
    if(updatePlatformParameters(pose)){
        itemTopPlateDark->setPosition(QVector3D(x, y, z));
        itemTopPlateDark->setRotation(getQuaternion());

        itemTopPlateLight->setPosition(QVector3D(x, y, z));
        itemTopPlateLight->setRotation(getQuaternion());

        // update position and rotation for actuators
        for(int i = 0; i < actuatorCount; i++){
            itemLowerActuatorsLight[i]->setRotation(lowerActuatorQuaternion[i]*lowerActuatorQuaternionInitialRot[i]); // Lower actuator light rotation
            itemLowerActuatorsMedium[i]->setRotation(lowerActuatorQuaternion[i]*lowerActuatorQuaternionInitialRot[i]); // Lower actuator medium rotation
            itemLowerActuatorsDark[i]->setRotation(lowerActuatorQuaternion[i]*lowerActuatorQuaternionInitialRot[i]); // Lower actuator dark rotation

            itemUpperActuators[i]->setPosition(platformPointsInSFrame[i]); // upper actuator position
            itemUpperActuators[i]->setRotation(lowerActuatorQuaternion[i]); // upper actuator rotation
        }
    }

//    qDebug() << "updatePose finished";
//    qDebug() << "(" + QTime::currentTime().toString() + ") i:" << tempCountFPS << "   x:" << x << "y:" << y << "z:" << z << "rx:" << roll << "ry:" << pitch << "rz:" << yaw;
//    tempCountFPS++;
//    // 63 FPS ?
}

void StewartPlatform::setPose(QVector<float> pose){
    x = pose[0];
    y = pose[1];
    z = pose[2];
    rx = pose[3];
    ry = pose[4];
    rz = pose[5];
    emitValueChanged(); // Emit signals for values changed to update spinboxes
}



void StewartPlatform::OnMove(QVector<float>& motionData){
    if(motionData.size() != 6) {
        qDebug() << "StewartPLatform::onMove.. motionData.size() != 6";
        return;
    }

    // Adjust base sensitivity for 3D Mouse
    float xSens = 0.005f;
    float ySens = 0.005f;
    float zSens = 0.005f;
    float rxSens = 0.0008f;
    float rySens = 0.0008f;
    float rzSens = 0.0008f;

    QVector<float> tempPose
    {
        x - (motionData[0]*xSens),    // Reversed (-)
        y + (motionData[1]*ySens),
        z + (motionData[2]*zSens),
        rx - (motionData[3]*rxSens),  // Reversed (-)
        ry + (motionData[4]*rySens),
        rz + (motionData[5]*rzSens)
    };

    // function updates platform position and rotation if tempPose is valid
    updateMeshPoses(tempPose);

    // TODO:
    // ADD IF PRINT ACTUATOR LENGHS
    printActuatorLengths();
}


void StewartPlatform::move3D(QVector<float> motionData){    
    updateMeshPoses(motionData);
}


bool StewartPlatform::isValidLengths(QVector<float> lengths){
    bool valid = true;
    for(int i = 0; i < actuatorLengths.size(); i++){
        if((lengths[i] < actuatorMinLength) || (lengths[i] > actuatorMaxLength)) {
            return false;
        }
    }
    return valid;
}

void StewartPlatform::setCameraHome(){
    float xRotationHome = -180.0f;
    float yRotationHome = 12.0f;
    float zoomHome = 250.0f;

    m_graph->scene()->activeCamera()->setTarget(QVector3D(0.0f, -0.25f, 0.0f));

    m_graph->scene()->activeCamera()->setXRotation(xRotationHome);
    m_graph->scene()->activeCamera()->setYRotation(yRotationHome);
    m_graph->scene()->activeCamera()->setZoomLevel(zoomHome);
}


void StewartPlatform::emitValueChanged(){
    emit(xChanged(x));
    emit(yChanged(y));
    emit(zChanged(z));
    emit(rXChanged(rx));
    emit(rYChanged(ry));
    emit(rZChanged(rz));
    emit(validMove());
}

// Temporary
//TODO:
// Calculate max step based on actuators length at start and end pose, and the actuator speed instead.
QVector<QVector<float>> StewartPlatform::generatePath(const QVector<float> startPose, const QVector<float> endPose, int FPS){
    QVector<QVector<float>> path;
    QVector<float> diffPose(6, 0.0f);
    QVector<float> currenStep(startPose);

    float maxDiff = 0;
    float maxDiffIndex;

    float maxStep = 0.94;
    int numStep;

    if((startPose.size() == endPose.size()) && (startPose.size() == 6)){
         for(int i = 0; i < diffPose.size(); i++){
             diffPose[i] = endPose[i] - startPose[i];
             if(abs(diffPose[i]) > maxDiff){
                 maxDiff = abs(diffPose[i]);
                 maxDiffIndex = i;
             }
         }
    }

    numStep = round(maxDiff/maxStep);

    QVector<float> poseStep(6);
    for(int i = 0; i < poseStep.size(); i++){
        if(diffPose[i] == 0 || numStep == 0){
            poseStep[i] = 0;
        }else{
            poseStep[i] = diffPose[i]/numStep;
        }
    }

    path.append(startPose);
    for(int i = 0; i < numStep-1; i++){
        for(int j = 0; j < currenStep.size(); j++){
            currenStep[j] += poseStep[j];
        }
        path.append(currenStep);
    }
    path.append(endPose);

    return path;

    // Override +, -, *, / operators of QVector?
    // maxLengthActuator = 150
    // minLengthActuator = 0
    // max height platform (yMax) = 522
    // min heigh platform (yMin) = 340
    // range Y = 505.5-340 = 160.5mm
    // Max speed actuator 2"/s = 25.4mm*2/s = 50.8mm sec
    // time from minActLengt to MaxActLength: 150 - s*50.8 = 0 -> s = 150/50.8 = 2.953s
    // Number of steps s * fps = 2.953*FPS(60) = 177.2
    // round down to 170 steps at 60 fps from ymin to ymax
    // thus maxStep = 160/170 = 0.9411
    // but combined movements will give diferent maxsteps
    // aswell as rotation
    // so not at all correct speeds
}



///////////////
//// Debug ////
///////////////

//// prints a QMatrix3x3
//void StewartPlatform::printRotationMatrix(const QMatrix3x3& R){
//    QDebug deb = qDebug();
//    for(int i = 0; i < 3; ++i){
//        for(int j = 0; j < 3; ++j){
//            deb << R(i, j);
//        }
//        deb << "\n";
//    }
//}

//// Print quaternion
//void StewartPlatform::printQuaternion(const QQuaternion& q){
//    qDebug() << "q.x: " << q.x() << "q.y:" << q.y();
//}

//// Print upper joint positions
//void StewartPlatform::printUpperJointPositions(){
//    QVector3D temp;
//    qDebug() << "Upper joint positions";
//    for(int i = 0; i < NumberOfActuators; i++){
//        temp = platformPointsInSFrame[i];
//        qDebug() << temp.x() << temp.y() << temp.z();
//    }
//}

// Prints actuator lengths
void StewartPlatform::printActuatorLengths(){
    QVector<int> convertedLengths = getActuatorData();
    qDebug() << "Actuator  :" << " 1 " << " 2 " << " 3 "
                << " 4 " << " 5 " << " 6 ";
    QDebug deb = qDebug();
    for(int i = 0; i < actuatorCount + 1; i++){
        if(i == 0){
            deb << "Length mm       :";
        }else {
                   deb << actuatorLengths[i-1];
        }
    }

    for(int i = 0; i < actuatorCount + 1; i++){
        if(i == 0){
            deb << "\nLength [0-1023] :";
        }else {
                   deb << convertedLengths[i-1];
        }
    }


}

//// Prints pose (position and rotation)
//void StewartPlatform::printPose(){
//    qDebug() << "x:" << x << " y:" << y << " z:" << z
//             << " roll:" << roll << " pitch:" << pitch << " yaw:" << yaw;
//}

//// prints log
//void StewartPlatform::printLog(){
//    printPose();
//    printActuatorLengths();
//}

void StewartPlatform::printQVector(QVector<float> vec){
    qDebug() << "x: " << vec[0] << "Y: " << vec[1] << "z: " << vec[2] << "rx:" << vec[3] << "ry:" << vec[4] << "rz" << vec[5];

}


