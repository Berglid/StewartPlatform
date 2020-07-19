#include "mouse3dinput.h"

#include <QDebug>

Mouse3DInput::Mouse3DInput(QWidget *parent)
    : QMainWindow(parent),
      QAbstractNativeEventFilter(),
      dx(0),
      dy(0),
      dz(0),
      droll(0),
      dpitch(0),
      dyaw(0),
      data(6)


{
    //ui.setupUi(this);

    init3DMouse();
}

bool Mouse3DInput::init3DMouse()
{
  SiOpenData oData;

  /*init the SpaceWare input library */
  if (SiInitialize() == SPW_DLL_LOAD_ERROR)
  {
    return false;
  }

  SiOpenWinInit(&oData, (HWND)winId());    /* init Win. platform specific data  */
  SiSetUiMode(m_mouse3DHandle, SI_UI_ALL_CONTROLS); /* Config SoftButton Win Display */

  /* open data, which will check for device type and return the device handle
  to be used by this function */
  if ( (m_mouse3DHandle = SiOpen ("Stewart Platform Controller", SI_ANY_DEVICE, SI_NO_MASK, SI_EVENT, &oData)) == NULL)
  {
    SiTerminate();  /* called to shut down the SpaceWare input library */
    qDebug() << "Could not open 3D Mouse";
    return false;       /* could not open device */
  }
  else
  {
    qDebug() << "Opened 3D Mouse sucessfully";
    return true; /* opened device succesfully */
  }
}


Mouse3DInput::~Mouse3DInput()
{

}

bool Mouse3DInput::nativeEventFilter(const QByteArray &eventType, void *msg, long *)
{

  Q_UNUSED(eventType);

  if(!m_mouse3DHandle)
    return false;

  MSG* winMSG = (MSG*)msg;

  bool handled = SPW_FALSE;
  SiSpwEvent     Event;    /* SpaceWare Event */
  SiGetEventData EData;    /* SpaceWare Event Data */

  /* init Window platform specific data for a call to SiGetEvent */
  SiGetEventWinInit(&EData, winMSG->message, winMSG->wParam, winMSG->lParam);

  /* check whether msg was a 3D mouse event and process it */
  if (SiGetEvent (m_mouse3DHandle, SI_AVERAGE_EVENTS, &EData, &Event) == SI_IS_EVENT)
  {
    if (Event.type == SI_MOTION_EVENT)
    {
//      qDebug() << "delta by X coordinate = " << Event.u.spwData.mData[SI_TX] << "\n";
//      qDebug() << "delta by Y coordinate = " << Event.u.spwData.mData[SI_TY] << "\n";
//      qDebug() << "delta by Z coordinate = " << Event.u.spwData.mData[SI_TZ] << "\n";

//      qDebug() << "delta by Yaw = " << Event.u.spwData.mData[SI_RX] << "\n";
//      qDebug() << "delta by Pitch = " << Event.u.spwData.mData[SI_RY] << "\n";
//      qDebug() << "delta by Roll = " << Event.u.spwData.mData[SI_RZ] << "\n";


        // CLEAN UP
      dx = Event.u.spwData.mData[SI_TX];
      dy = Event.u.spwData.mData[SI_TY];
      dz = Event.u.spwData.mData[SI_TZ];
      dyaw = Event.u.spwData.mData[SI_RX];
      dpitch = Event.u.spwData.mData[SI_RY];
      droll = Event.u.spwData.mData[SI_RZ];

      data[0] = dx;
      data[1] = dy;
      data[2] = dz;
      data[3] = dyaw;
      data[4] = dpitch;
      data[5] = droll;
        // CLEAN UP

      emit Move3d(data);

    }
    else if (Event.type == SI_ZERO_EVENT)
    {
      // ZERO event
    }
    else if (Event.type == SI_BUTTON_EVENT)
    {
      // misc button events
    }

    handled = SPW_TRUE;  /* 3D mouse event handled */
  }

  return handled;
}

void Move3d(QVector<float>& motionData){
    emit (motionData);
}
