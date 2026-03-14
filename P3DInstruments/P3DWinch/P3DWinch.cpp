// P3DWinch.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <shlwapi.h>
#include <iostream>
#include <iostream>

#include "../P3DCommon/Prepar3D.h"
#include "../P3DCommon/SimObjectDataRequest.h"
#include "simulation.h"
#include "Joystick.h"
#include "state_input.h"

#ifndef NDEBUG
#include "TestFinEffect.h"
#include "TestTailplaneEffect.h"
#include "TestWingEffect.h"
#include "TestFuselageEffect.h"

#endif

int main(int argc, char* argv[])
{
    
                                                                                                                                                                                                        
    std::cout << "CGC Winch" << std::endl;
    std::cout << "-nodisengage  - don't disengage model on release" << std::endl;
    std::cout << "-vf           - use visual frame rate (default is sim rate)" << std::endl;
    std::cout << "-verbose      - print verbose debugging information" << std::endl;

    bool nodisengage = false;
    bool useVisualFrame = false;
    bool verbose = false;


    for (int i = 1; i < argc; ++i) {
        //std::cout << argv[i] << std::endl;
        if (argv[i] == "-nodisengage") {
            nodisengage = true;
        }

        if (argv[i] == "-vf") {
            useVisualFrame = true;
        }
    }
    
    // Full path  to program e.g. D:\Projects\Glider-Simulator\P3DInstruments\x64\Debug\P3DWinch.exe
    char* pszCommandPath = argv[0];


    // COM needed for sound playing
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

    //SimplePlayer player;
    //player.Play(L"music.m4a");

#ifndef NDEBUG
    {
        //Aerofoil_NACA0010 naca0010;
        //std::cout << "---- NACA0010 -----" << std::endl;
        //for (int i = 0; i < 360; ++i) {
        //    float aoa = float(i) * float(3.14159265358979 / 180); // in radians
        //    auto coeffs = naca0010.coefficients_at(aoa);
        //    std::cout << i << ',' << coeffs.Cl << ',' << coeffs.Cd << ',' << coeffs.Cm << std::endl;
        //}
        //std::cout << "---- NACA0010 -----" << std::endl;

    

        TestFinEffect testFinEffect;
        testFinEffect.test_all();

        TestTailplaneEffect testTailplaneEffect;
        testTailplaneEffect.test_all();

        TestWingEffect testWingEffect;
        testWingEffect.test_all();

        TestFuselageEffect testFuselageEffect;
        testFuselageEffect.test_all();

    }
#endif


    Simulation simulation;
    Prepar3D* p3D = new Prepar3D("Winch", verbose);

    std::cout << p3D->userAircraft().name() << "," << p3D->userAircraft().id() << std::endl;

    StateInput input(p3D, &simulation);
    input.setAutoDisengage(!nodisengage);

    SIMCONNECT_PERIOD rate = (useVisualFrame) ? SIMCONNECT_PERIOD_VISUAL_FRAME : SIMCONNECT_PERIOD_SIM_FRAME;
    SimObjectDataRequest request(p3D, &input, &p3D->userAircraft(), rate);


    p3D->DispatchLoop();

    delete p3D;

    MFShutdown();
    CoUninitialize();

    return 0;

}

