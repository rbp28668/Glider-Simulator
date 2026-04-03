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
#include "../P3DCommon/Folder.h"

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
    std::cout << "Copyright (c) Cambridge Gliding Centre & R Bruce Porteous (2026)" << std::endl;
    std::cout << "-nodisengage  - don't disengage model on release" << std::endl;
    std::cout << "-vf           - use visual frame rate (default is sim rate)" << std::endl;
    std::cout << "-verbose      - print verbose debugging information" << std::endl;
    std::cout << "-test         - run model tests" << std::endl;
    std::cout << "-early        - sends model state to P3d early in loop" << std::endl;
    std::cout << "-throttle <pct> sets default throttle position for launch" << std::endl;


    bool nodisengage = false;
    bool useVisualFrame = false;
    bool verbose = false;
    bool early = false;
    bool test = false;
    NumberT targetThrottle = 0.8;

    for (int i = 1; i < argc; ++i) {
        //std::cout << argv[i] << std::endl;
        if (strcmp(argv[i], "-nodisengage") == 0) {
            nodisengage = true;
        }

        if (strcmp(argv[i], "-vf") == 0) {
            useVisualFrame = true;
        }

        if (strcmp(argv[i], "-verbose") == 0) {
            verbose = true;
        }

        if (strcmp(argv[i], "-test") == 0) {
            test = true;
        }

        if (strcmp(argv[i], "-early") == 0) {
            early = true;
        }


        if (strcmp(argv[i], "-throttle") == 0) {
            int idx = i + 1;
            if (idx < argc) {
                targetThrottle = atof(argv[idx]);
                if (targetThrottle < 0)
                    targetThrottle = 0;
                else if (targetThrottle > 1.0)
                    targetThrottle /= 100; // Assume percentage

                // Now clamp positive range
                if (targetThrottle > 1.0)
                    targetThrottle = 1.0;

                std::cout << "Target throttle set to " << targetThrottle * 100 << "%" << std::endl;
            }
            else {
                std::cout << "No throttle position percentage given" << std::endl;
            }
        }


    }


    if (nodisengage) std::cout << "NODISENGAGE set" << std::endl;
    if (useVisualFrame) std::cout << "Using visual frame" << std::endl;
    if (verbose) std::cout << "VERBOSE output" << std::endl;
    if (test) std::cout << "TEST model on startup" << std::endl;
    if (early) std::cout << "EARLY send of output data" << std::endl;

    std::cout << std::endl;
    std::cout << "Keyboard commands:" << std::endl;
    std::cout << "e: engage flight model" << std::endl;
    std::cout << "d: disengage flight model" << std::endl;
    std::cout << "w: winch" << std::endl;
    std::cout << "r: release" << std::endl;
    std::cout << "f: fade power" << std::endl;
    std::cout << "x: drop left wing" << std::endl;
    std::cout << "c: drop right wing" << std::endl;
    std::cout << "p: reduce target power" << std::endl;
    std::cout << "P: increase target power" << std::endl;
    std::cout << "0..9: set target power" << std::endl;


    // Full path  to program e.g. D:\Projects\Glider-Simulator\P3DInstruments\x64\Debug\P3DWinch.exe
    // Get the folder to allow access to sound files.
    File commandPath(argv[0]);
    Directory folder = commandPath.directory();


    // COM needed for sound playing
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);



    // Debug
    //Directory resourceFolder = folder.sub("resources");
    //File file = resourceFolder.file("cable_on_and_secure_black_link.m4a");
    //SimplePlayer player;
    //player.Play(file);



#ifndef NDEBUG // never in production!
    if(test){
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
    input.setEarly(early);
    input.setTargetThrottle(targetThrottle);
    input.setRootFolder(folder);

    SIMCONNECT_PERIOD rate = (useVisualFrame) ? SIMCONNECT_PERIOD_VISUAL_FRAME : SIMCONNECT_PERIOD_SIM_FRAME;
    SimObjectDataRequest request(p3D, &input, &p3D->userAircraft(), rate);

    p3D->DispatchLoop();

    delete p3D;

    MFShutdown();
    CoUninitialize();

    return 0;

}

