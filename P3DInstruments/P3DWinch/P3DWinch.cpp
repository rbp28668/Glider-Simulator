// P3DWinch.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "../P3DCommon/Prepar3D.h"
//#include "simulation.h"
#include "Joystick.h"

int main(int argc, char* argv[])
{
    

    //Simulation simulation;


    std::cout << "Hello Winch!\n";


    Prepar3D* p3D = new Prepar3D("Winch");

    Joystick joystick;

    joystick.Register(p3D);

    p3D->DispatchLoop();

    joystick.Unregister(p3D);
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
