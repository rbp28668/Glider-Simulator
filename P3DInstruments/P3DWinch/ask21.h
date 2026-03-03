#pragma once
#include "aircraft_params.h"
#include "panel.h"
#include "Aerofoil_FX_02_196.h"
#include "Aerofoil_FX_60_126.h"
#include "Aerofoil_NACA0010.h"
#include "contact_point.h"
#include "local_math.h"

// Class representing the ASK21 glider model with its physical and aerodynamic properties.
// Note coordinate system.
// Origin: Datum at root leading edge on fuselage centerline
// X-axis: Forward through nose (positive forward)
// Y-axis: Right wing direction (starboard positive)
// Z-axis: Down through belly (perpendicular to XY plane, positive down)
class ASK21 : public AircraftParameters
{

public:
    // Initialize ASK21-specific parameters here

    // Moments of inertia about principal axes through center of gravity (roll, pitch, yaw)
    float Ixx = 1285.0f; // kg·m²
    float Iyy = 1824.0f; // kg·m²
    float Izz = 2663.0f; // kg·m²
    float Ixz = 100.0f;  // kg·m² Wild guess
    float moments_of_inertia[4] = {Ixx, Iyy, Izz, Ixz};

    float mass = 687.0f; // kg
    float cg = -0.30f; // m from datum (negative is aft of datum), was 30 originally


    // Wing information

    float wing_span = 17.0f;   // m
    float mean_chord = 1.121f; // m
    float S = 17.95f;            // m2(should be the same as 2x sum of panel areas + fuselage plug)

    Aerofoil_FX_60_126 root;
    Aerofoil_FX_02_196 tip;

    float incidenceDegrees = 0.6f; // was 3.192472613
    // Panels:  area, mid-span position from centreline, quarter-chord position (relative to datum), mean chord, incidence angle
    // mid-span is distance from centerline to panel center
    // quarter-chord is distance from root leading edge (datum) to quarter-chord of panel
    // root and tip are the root and tip sections and interp determines interpolation between them.
    // NOTE - washout hardwired into incidence of each panel.
    Panel *rootPanel = new Panel(3.215313306f, 1.545740741f, -0.3368518519f, 1.347407407f, incidenceDegrees, root, tip, 0.0f);
    Panel *airbrakePanel = new AirbrakePanel(1.648746982f, 3.440925926f, -0.2935648148f, 1.174259259f, incidenceDegrees - 0.2f, root, tip, 0.2f);
    Panel *outerPanel = new Panel(1.112233745f, 4.668703704f, -0.2644444444f, 1.057777778f, incidenceDegrees-0.4f, root, tip, 0.5f);
    Panel *aileronPanel = new AileronPanel(2.216578464f, 6.585925926f, -0.211712963f, 0.7964814815f, incidenceDegrees-0.8f, root, tip, 0.7f,
                                           18.0f, 12.0f, // differential aileron
                                            0.6f, // lift effectiveness
                                            -0.4f, // moment coeff
                                            0.01f  // profile drag coeff
                                        );
    Panel *tipPanel = new Panel(0.287909808f, 8.238703704f, -0.1629166667f, 0.5509259259f, incidenceDegrees-1.0f, root, tip, 1.0f);

    std::vector<Panel*> wing = {
        rootPanel,
        airbrakePanel,
        outerPanel,
        aileronPanel,
        tipPanel
    };

    float dihedral_angle = radians(4.0f); // degrees (under each tip)

    // Tailplane and fin aerofoils
    Aerofoil_NACA0010 tail;

    // Tailplane area and moment
    float tailplane_area = 1.796160768f;           // m²
    float tailplane_chord = 0.50f;                 // m - mean aerodynamic chord of tailplane
    float tailplane_incidence = radians(-2.5f);    // tailplane incidence relative to fuselage (negative = download at trim)
    float tailplane_quarter_chord = -5.210185185f; // m from datum
    float elevator_max_deflection = radians(20.0f); // max elevator deflection (±20°)
    Aerofoil& tailplane = tail;

    // Fin area and moment
    float fin_area = 1.412849246f;           // m²
    float fin_incidence = 0.0f;              // degrees
    float fin_quarter_chord = -5.082685185f; // m from datum
    Aerofoil& fin = tail;

    // Fuselage coefficients
    // Aerodynamic Coefficients for ASK21 Fuselage
    // These are typical values for a high - performance(? ? a K21 ? ) tandem glider
    float C_d0 = 0.015f;       // Baseline parasite drag(positive)
    float C_y_beta = -0.12f;   // Side force coefficient per radian(low angle)
    float C_z_alpha = -0.10f;  // Vertical force coefficient(negligible lift, low angle
    float C_m_alpha = 0.05f;   // Pitching instability(destabilizing)
    float C_n_beta = -0.04f;   // Yawing instability(Munk moment)
    float C_mq_fus = -0.15f;   // Pitch damping(fuselage contribution) - with pitch rate
    float C_nr_fus = -0.10f;   // Yaw damping(fuselage contribution) - with yaw rate
    float S_side = 4.5f;    // Projected side area of ASK21 fuselage(m ^ 2)
    float S_plan = 3.8f;    // Projected top / bottom area(m ^ 2)
    float Cd_cylinder = 1.2f; // Drag coefficient of a cylinder - like body - used for cross - flow drag


    // Contact points for ground detection. Relative to datum. Fwd and down are +ve.
    // contact_type determines stiffness, damping, friction and max penetration:
    //   nose_wheel: 5cm max penetration
    //   main_wheel: 10cm max penetration
    //   tail_wheel: 5cm max penetration
    //   wingtip: 2cm max penetration

    ContactPoint *noseWheel = new ContactPoint(1.64962963f, 0.0f, 0.6737037037f, ContactPoint::ContactType::WHEEL, 80000.0f, 5000.0f, 0.8f, 0.6f, 0.05f);
    ContactPoint *mainWheel = new ContactPoint(-0.6611111111f, 0.0f, 0.7303703704f, ContactPoint::ContactType::WHEEL, 60000.0f, 6000.0f, 0.8f, 0.6f, 0.10f);
    ContactPoint *tailWheel = new ContactPoint(-5.263703704f, 0.0f, 0.2203703704f, ContactPoint::ContactType::WHEEL, 80000.0f, 5000.0f, 0.8f, 0.6f, 0.05f);
    ContactPoint *leftTip =   new ContactPoint(-0.4092592593f, -wing_span / 2, -0.4533333333f, ContactPoint::ContactType::WINGTIP, 5000.0f, 500.0f, 0.9f, 0.7f, 0.02f);
    ContactPoint *rightTip =  new ContactPoint(-0.4092592593f, wing_span / 2, -0.4533333333f, ContactPoint::ContactType::WINGTIP, 5000.0f, 500.0f, 0.9f, 0.7f, 0.02f);
    std::vector<const ContactPoint*> contact_points = {
        noseWheel,
        mainWheel,
        tailWheel,
        leftTip,
        rightTip
    };

    // Special contact points (i.e. just positions) for aerotow and winch hooks
    V3d<float> aerotow_hook = V3d<float>(2.291851852f, 0.0f, 0.447037037f);  // Approximate position of aerotow hook
    V3d<float> winch_hook = V3d<float>(0.1574074074f, -0.05f, 0.572962963f); // approximate position of winch hook, slightly left of centerline

    ASK21()
    {
        // Constructor
    }

    ~ASK21()
    {
        // Destructor - clean up dynamically allocated panels and contact points
        delete rootPanel;
        delete airbrakePanel;
        delete outerPanel;
        delete aileronPanel;
        delete tipPanel;

        delete noseWheel;
        delete mainWheel;
        delete tailWheel;
        delete leftTip;
        delete rightTip;
    }

    // --- AircraftParameters interface implementation ---
    virtual float CG() const 
    {
        return cg;
    }
    virtual float AR() const 
    {
        return (wing_span / mean_chord);
    }
    virtual float Oswald() const 
    {
        return 0.95f; // typical for gliders
    }
    virtual float DihedralAngle() const 
    {
        return dihedral_angle;
    }
};