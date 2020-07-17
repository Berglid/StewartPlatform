#ifndef MOUSE3DINPUT_H
#define MOUSE3DINPUT_H

#include <QtWidgets/QMainWindow>
#include <QAbstractNativeEventFilter>


#include <windows.h>
#include <tchar.h>

// Mouse 3D stuff
#include <spwmacro.h>  /* Common macros used by SpaceWare functions. */
#include <si.h>        /* Required for any SpaceWare support within an app.*/
#include <siapp.h>     /* Required for siapp.lib symbols */
#include "virtualkeys.hpp"


class Mouse3DInput : public QMainWindow
                          , public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    float dx;
    float dy;
    float dz;
    float droll;
    float dpitch;
    float dyaw;
    QVector<float> data;

    Mouse3DInput(QWidget *parent = 0);
    ~Mouse3DInput();

public: // QAbstractNativeEventFilter
    virtual bool nativeEventFilter(const QByteArray &eventType, void *message, long *) Q_DECL_OVERRIDE;

signals:
    void Move3d(QVector<float>& motionData);

private:
  bool init3DMouse();

private:

    // Ui::HabrahabrAnd3DMouseClass ui;
    SiHdl m_mouse3DHandle; // Handle to 3D Mouse Device
};

#endif // MOUSE3DINPUT_H
