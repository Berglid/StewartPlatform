#ifndef STEWARTPLATFORM_H
#define STEWARTPLATFORM_H

#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtGui/QFont>
#include <QCustom3DItem>

// TODO:
// fix notation: consistent troughout with inverse kinematics

using namespace QtDataVisualization;

class StewartPlatform : public QObject
{
    Q_OBJECT

public:
    explicit StewartPlatform(Q3DScatter *scatter);
    ~StewartPlatform();

    void setCameraHome();
    void setActuatorsHome();
    void changePresetCamera();
    void setGridEnabled(int enabled);
    void setLabelEnabled(int enabled);
    void setBackgroundEnabled(int enabled);

    void xMove(double value);
    void yMove(double value);
    void zMove(double value);
    void xRot(double value);
    void yRot(double value);
    void zRot(double value);
    void move3D(QVector<float> motionData);

    double getX();
    double getY();
    double getZ();
    double getRX();
    double getRY();
    double getRZ();
    float getMaxRy(); // DELETE?
    QVector<float> getPose();
    QVector<float> getHomePose();
    QVector<int> getActuatorData();
    QVector<float> getMaxValues(); // Implement
    QVector<QVector<float>> generatePath(const QVector<float> startPose, const QVector<float> endPose, int FPS = 60);

public slots:
    void OnMove(QVector<float>& motionData);

public Q_SLOTS:
    void changeTheme(int theme);
    void changeShadowQuality(int quality);
    void shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality shadowQuality);

Q_SIGNALS:
    // place all signals to single function
    // must have poseChanged(Vector), and pass motion data
    // Then on connect function use this and add av variable to mainwindow that stores this vector
    // and connect spinboxes
    void poseChanged(); // TODO
    void xChanged(double value);
    void yChanged(double value);
    void zChanged(double value);
    void rXChanged(double value);
    void rYChanged(double value);
    void rZChanged(double value);
    void validMove(); // DELETE?


    void gridEnabledChanged(bool enabled);
    void shadowQualityChanged(int quality);
    void labelEnabledChanged(bool enabled);
    void backgroundEnabledChanged(bool enabled);

signals:

private:
    Q3DScatter *m_graph;
    int m_fontSize;
    bool m_smooth;
    bool isLabelEnabled;

    const float x_home = 0.0f;
    const float y_home = 360.0f;
    const float z_home = 0.0f;
    const float rx_home = 0.0f;
    const float ry_home = 0.0f;
    const float rz_home = 0.0f;
    const float ry_max = 60.0f;
    const float ry_min = -ry_max;
    const int actuatorCount = 6;
    const int actuatorMinLength = 372;
    const int actuatorMaxLength = 522;

    float x;
    float y;
    float z;
    float rx;
    float ry;
    float rz;

    // Item/meshes
    QVector3D meshScale;
    QCustom3DItem* itemTopPlateDark;
    QCustom3DItem* itemTopPlateLight;
    QCustom3DItem* itemBottomPlateLight;
    QCustom3DItem* itemBottomPlateDark;
    QVector<QCustom3DItem*> itemUpperActuators;
    QVector<QCustom3DItem*> itemLowerActuatorsLight;
    QVector<QCustom3DItem*> itemLowerActuatorsMedium;
    QVector<QCustom3DItem*> itemLowerActuatorsDark;

    // platform parameters
    QVector<float> actuatorLengths;
    QVector<QVector3D> platformPointsInSFrame; // position of upper joints relative to {s} frame
    QVector<QVector3D> baseToPlatformInSFrame; // Vectors from base to platform relative to body frame {s} frame
    QVector<QQuaternion> lowerActuatorQuaternion; // Quaternion representation oriantation of lower actuators

    QQuaternion getQuaternion();
    QMatrix3x3 getRotationMatrix();
    QMatrix4x4 getTransformationMatrix();
    QMatrix4x4 getTransformationMatrix(QVector<float> posAndRot);
    QMatrix3x3 getRotationMatrix(float rx, float ry, float rz);

    void initializeGraph();
    void initializeMeshes();
    void setPose(QVector<float> pose);
    bool isValidLengths(QVector<float> lengths);
    void updateMeshPoses(QVector<float> posAndRot);
    bool updatePlatformParameters(QVector<float> posAndRot);
    void addItem(QCustom3DItem* & rItem, const QString& rMeshFile, const QString& rTextureFile, const QVector3D& rPosition);

    // MOVE CONSTANTS TO GLOBAL VARIABLE??
    const QVector<QVector3D> basePoints
    {
        QVector3D(-164.97f, 0.0f, -114.29f),
        QVector3D(-181.47f, 0.0f, -85.72f ),
        QVector3D( -16.50f, 0.0f, 200.01f ),
        QVector3D(  16.50f, 0.0f, 200.01f ),
        QVector3D( 181.47f, 0.0f, -85.72f ),
        QVector3D( 164.97f, 0.0f, -114.29f)
    };
    // Position of upper joints relative to body frame
    const QVector<QVector3D> platformPoints
    {
        QVector3D( -16.50f, 0.0f, -145.01f),
        QVector3D(-133.83f, 0.0f,  58.22f ),
        QVector3D(-117.34f, 0.0f,  86.79f ),
        QVector3D( 117.34f, 0.0f,  86.79f ),
        QVector3D( 133.83f, 0.0f,  58.22f ),
        QVector3D(  16.50f, 0.0f, -145.01f)
    };

    // Quaternion representation for "initial" rotation of lower actuators (angles from solidworks model)
    const QVector<QQuaternion> lowerActuatorQuaternionInitialRot
    {
        QQuaternion(0.8872927f , 0.0f, 0.4612067f , 0.0f),  // 54.93
        QQuaternion(-0.4612067f, 0.0f, 0.8872927f , 0.0f),  // 234.93
        QQuaternion(0.0410909f , 0.0f, 0.9991554f , 0.0f),  // 175.29
        QQuaternion(-0.9991554f, 0.0f, 0.0410909f , 0.0f),  // 355.29
        QQuaternion(0.8447485f , 0.0f, -0.5351635f, 0.0f),  // -64.71
        QQuaternion(0.5351635f , 0.0f, 0.8447485f , 0.0f)   // 115.29
    };

    // Emits all signals if pose is changed
    void emitValueChanged();

    // DEBUG
    int tempCountFPS = 0; // DELETE LATER
    void printRotationMatrix(const QMatrix3x3& R);
    void printQuaternion(const QQuaternion& q);
    void printUpperJointPositions();
    void printActuatorLengths();
    void printQVector(QVector<float> vec);
    void printPose();
    void printLog();
};

#endif
