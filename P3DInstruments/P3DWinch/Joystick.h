#pragma once

#include "../P3DCommon/Prepar3D.h"

class Joystick : public Prepar3D::SystemEventHandler 
{

    //enum INPUT_ID {
    //    INPUT_Z,
    //    INPUT_SLIDER,
    //    INPUT_XAXIS,
    //    INPUT_YAXIS,
    //    INPUT_RZAXIS,
    //    INPUT_HAT,
    //};

    enum EVENT_ID {
        EVENT_SLIDER = Prepar3D::EVENT_ID::LAST_P3D_EVENT,
        EVENT_XAXIS,
        EVENT_YAXIS,
        EVENT_RZAXIS,
        EVENT_HAT,
    };

    const int INPUT_GROUP = 0;
    const int GROUP_0 = 0;

    DWORD dwXAxis;
    DWORD dwYAxis;
    DWORD dwSlider;
    DWORD dwRzAxis;
    DWORD dwHat;

    virtual void handleEvent(SIMCONNECT_RECV_EVENT* evt);
    virtual void quitEvent();

public:
    void Register(Prepar3D* p3d);
    void Unregister(Prepar3D* p3d);
};

