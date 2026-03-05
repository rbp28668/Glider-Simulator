// P3DWinch.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

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
    
                                                                                                                                                                                                        
    std::cout << "Hello Winch!\n";

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
    bool verbose = true;
    Prepar3D* p3D = new Prepar3D("Winch", verbose);

    std::cout << p3D->userAircraft().name() << "," << p3D->userAircraft().id() << std::endl;

    StateInput input(p3D, &simulation);
    SimObjectDataRequest request(p3D, &input, &p3D->userAircraft(), SIMCONNECT_PERIOD_SIM_FRAME); // SIMCONNECT_PERIOD_SIM_FRAME


    p3D->DispatchLoop();

    delete p3D;


    return 0;

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
