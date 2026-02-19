#include "Joystick.h"


void Joystick::handleEvent(SIMCONNECT_RECV_EVENT* evt) {
    printf("\nEvent ID %d", evt->uEventID);
    switch (evt->uEventID) {
    case EVENT_SLIDER:
        printf("\nSlider value:%d", evt->dwData);
        dwSlider = evt->dwData;
        break;

    case EVENT_XAXIS:
        printf("\nX Axis value:%d", evt->dwData);
        dwXAxis = evt->dwData;
        break;

    case EVENT_YAXIS:
        printf("\nY Axis value:%d", evt->dwData);
        dwYAxis = evt->dwData;
        break;

    case EVENT_RZAXIS:
        printf("\nRotate Z axis value:%d", evt->dwData);
        dwRzAxis = evt->dwData;
        break;

    case EVENT_HAT:
        printf("\nHat value:%d", evt->dwData);
        dwHat = evt->dwData;
        break;
    }
}

void Joystick::quitEvent() {

}



void Joystick::Register(Prepar3D* p3d) {

    p3d->registerSystemEventHandler(this);

    HRESULT hr;
    auto hSimConnect = p3d->getHandle();
    hr = ::SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ID::EVENT_SLIDER);
    hr = ::SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ID::EVENT_XAXIS);
    hr = ::SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ID::EVENT_YAXIS);
    hr = ::SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ID::EVENT_RZAXIS);
    hr = ::SimConnect_MapClientEventToSimEvent(hSimConnect, EVENT_ID::EVENT_HAT);

    // Add all the private events to a notifcation group
    hr = ::SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_0, EVENT_ID::EVENT_SLIDER);
    hr = ::SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_0, EVENT_ID::EVENT_XAXIS);
    hr = ::SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_0, EVENT_ID::EVENT_YAXIS);
    hr = ::SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_0, EVENT_ID::EVENT_RZAXIS);
    hr = ::SimConnect_AddClientEventToNotificationGroup(hSimConnect, GROUP_0, EVENT_ID::EVENT_HAT);

    // Set a high priority for the group
    hr = ::SimConnect_SetNotificationGroupPriority(hSimConnect, GROUP_0, SIMCONNECT_GROUP_PRIORITY_HIGHEST);

    // Map input events to the private client events
    hr = ::SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT_GROUP, "joystick:0:slider", EVENT_ID::EVENT_SLIDER);
    hr = ::SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT_GROUP, "joystick:0:XAxis", EVENT_ID::EVENT_XAXIS);
    hr = ::SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT_GROUP, "joystick:0:YAxis", EVENT_ID::EVENT_YAXIS);
    hr = ::SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT_GROUP, "joystick:0:RzAxis", EVENT_ID::EVENT_RZAXIS);
    hr = ::SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT_GROUP, "joystick:0:POV", EVENT_ID::EVENT_HAT);

}

void Joystick::Unregister(Prepar3D* p3d) {
    HRESULT hr;
    auto hSimConnect = p3d->getHandle();

    // Turn all the joystick events off
    hr = SimConnect_SetInputGroupState(hSimConnect, INPUT_GROUP, SIMCONNECT_STATE_OFF);
    
    //The SimConnect_ClearInputGroup function is used to remove all the input events from a specified input group object.
    hr = ::SimConnect_ClearInputGroup(hSimConnect, INPUT_GROUP);

    //The SimConnect_ClearNotificationGroup function is used to remove all the client defined events from a notification group.
    hr = ::SimConnect_ClearNotificationGroup(hSimConnect, INPUT_GROUP);

    p3d->unRegisterSystemEventHandler(this);

}