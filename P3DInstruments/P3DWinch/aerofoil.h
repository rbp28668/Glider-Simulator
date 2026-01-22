// Aerofoil class using natural cubic splines for CL, CD, CM vs Alpha (degrees).

#pragma once

#include <cmath>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

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

    const double pi = 3.14159265358979323846;

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

        alphas = new float[nlines];
        cls = new float[nlines];
        cds = new float[nlines];
        cms = new float[nlines];

        for (int i = 0; i < nlines; ++i) {
            auto line = data[i];
            alphas[i] = line[0] / float(2 * pi); // store as radians
            cls[i] = line[1];
            cds[i] = line[2];
            cms[i] = line[3];
        }

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

        while (alpha_rad < 0.0f) alpha_rad += float(2 * pi);
        while (alpha_rad >= float(2 * pi)) alpha_rad -= float(2 * pi);
        auto cl = _cl_spline->point(alpha_rad);
        auto cd = _cd_spline->point(alpha_rad);
        auto cm = _cm_spline->point(alpha_rad);
        return Coefficients(cl, cd, cm);
    }

};

