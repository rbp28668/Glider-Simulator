// Aerofoil class using natural cubic splines for CL, CD, CM vs Alpha (degrees).

#pragma once

#include <cmath>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

#include <iostream>
#include "Spline.h"

// Provides Coefficients of lift, drag and moment for a given alpha (AoA)
// The constructor expects data with columns: Alpha, CL, CD, CM
// Alpha in the data is expected in degrees. The public method `coeffs_at(alpha_rad)`
// accepts alpha in radians and returns (CL, CD, CM) interpolated with cubic splines.
class Aerofoil {

    // Lookup table
    float* alphas = nullptr; // in radians
    float* cls = nullptr;
    float* cds = nullptr;
    float* cms = nullptr;

    // Interpolation
    Spline<float>* _cl_spline = nullptr;
    Spline<float>* _cd_spline = nullptr;
    Spline<float>* _cm_spline = nullptr;

    const float two_pi = float(3.14159265358979323846 * 2); //double pi = 3.14159265358979323846;

public:

    // Tuple to hold the results at a given alpha
    struct Coefficients {
        float Cl;
        float Cd;
        float Cm;
        Coefficients(float cl, float cd, float cm) {
            Cl = cl;
            Cd = cd;
            Cm = cm;
        }
    };

protected:


    Aerofoil(float data[][4], int nlines) {
        assert(nlines > 0);

        alphas = new float[nlines];
        cls = new float[nlines];
        cds = new float[nlines];
        cms = new float[nlines];

        //std::cout << "START AEROFOIL" << std::endl;
        for (int i = 0; i < nlines; ++i) {
            float* line = data[i];
            //std::cout << line[0] << ',' << line[1] << ',' << line[2] << ',' << line[3] << std::endl;

            alphas[i] = line[0] * two_pi / 360; // store as radians
            cls[i] = line[1];
            cds[i] = line[2];
            cms[i] = line[3];
        }
        //std::cout << "END AEROFOIL" << std::endl;

        _cl_spline = new Spline<float>(alphas, cls, nlines);
        _cd_spline = new Spline<float>(alphas, cds, nlines);
        _cm_spline = new Spline<float>(alphas, cms, nlines);
    }

public:


    ~Aerofoil() {
        delete _cl_spline;
        delete _cd_spline;
        delete _cm_spline;

        _cl_spline = nullptr;
        _cd_spline = nullptr;
        _cm_spline = nullptr;

        delete[] alphas;
        delete[] cls;
        delete[] cds;
        delete[] cms;

        alphas = nullptr;
        cls = nullptr;
        cds = nullptr;
        cms = nullptr;
    }

    Coefficients coefficients_at(float alpha_rad) const {
        //Return (CL, CD, CM) for a given alpha in radians.
        //Alpha is wrapped into [0, 2pi) radians before interpolation.
        assert(!isnan(alpha_rad));
        assert(!isinf(alpha_rad));

        while (alpha_rad < 0.0f) alpha_rad += two_pi;
        while (alpha_rad >= two_pi) alpha_rad -= two_pi;
        auto cl = _cl_spline->point(alpha_rad);
        auto cd = _cd_spline->point(alpha_rad);
        auto cm = _cm_spline->point(alpha_rad);
        return Coefficients(cl, cd, cm);
    }

};

