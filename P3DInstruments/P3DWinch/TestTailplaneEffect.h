#pragma once
#include "TestBase.h"
class TestTailplaneEffect :
    public TestBase
{

    float MAX_FORCE = 100000.0f;
    float MAX_MOMENT = 500000.0f;

    void test_straight_and_level() {
        state.set_velocity(25.0, 0.0, 0.0);
        state.set_angular_velocity(0.0, 0.0, 0.0);
        controls.set_controls(0.0, 0.0, 0.0, 0.0);

        V3d<float> tp_forces;
        V3d<float> tp_moments;
        model.tailplane_forces(state, aircraft, controls, world, state.velocity(), tp_forces, tp_moments);

        std::cout << "S&L, Tailplane Force: " << tp_forces << "," << "Tailplane moments : " << tp_moments << std::endl;

        assertLess(tp_forces[0], 0.0, "Tailplane X force should be negative due to drag");
        assertEqual(tp_forces[1], 0.0, "Tailplane Y force should be zero");
        assertGreater(tp_forces[2], 0.0, "Tailplane Z component should be positive (downforce at trim)");
    }


    void test_increased_aoa_increases_tail_lift() {
        // baseline
        state.set_velocity(25.0, 0.0, 0.0);
        state.set_angular_velocity(0.0, 0.0, 0.0);
        controls.set_controls(0.0, 0.0, 0.0, 0.0);

        V3d<float> base_forces;
        V3d<float> base_moments;
        model.tailplane_forces(state, aircraft, controls, world, state.velocity(), base_forces, base_moments);


        // increase local AoA by adding a small downward velocity(w positive = down)
        state.set_velocity(25.0, 0.0, 1.0);
        V3d<float> inc_forces;
        V3d<float> inc_moments;
        model.tailplane_forces(state, aircraft, controls, world, state.velocity(), inc_forces, inc_moments);


        // Observed: increasing local AoA here reduces tail downforce(Z moves toward zero)
        assertLess(inc_forces[2], base_forces[2], "Increased local AoA should reduce tailplane downforce (Z less positive)");
    }


    void test_pitch_rate_increases_tail_lift() {
        // baseline
        state.set_velocity(25.0, 0.0, 0.0);
        state.set_angular_velocity(0.0, 0.0, 0.0);
        controls.set_controls(0.0, 0.0, 0.0, 0.0);

        V3d<float> base_forces;
        V3d<float> base_moments;
        model.tailplane_forces(state, aircraft, controls, world, state.velocity(), base_forces, base_moments);
        std::cout << "Pitch rate base - tail forces" << base_forces << std::endl;

        // apply positive pitch rate(nose up) which moves tail down and increases local AoA
        state.set_velocity(25.0, 0.0, 0.0);
        state.set_angular_velocity(0.0, 0.1, 0.0);
        V3d<float> pr_forces;
        V3d<float> pr_moments;
        model.tailplane_forces(state, aircraft, controls, world, state.velocity(), pr_forces, pr_moments);
        std::cout << "Pitch rate test - tail forces" << pr_forces << std::endl;


        // Observed: positive pitch rate(nose up) moves tail down and here reduces downforce magnitude
        assertLess(pr_forces[2], base_forces[2], "Positive pitch rate should reduce tailplane downforce (Z less positive) in this trim condition");
    }

    void test_elevator_deflection_reduces_tail_lift() {
        // baseline
        state.set_velocity(25.0, 0.0, 0.0);
        state.set_angular_velocity(0.0, 0.0, 0.0);
        controls.set_controls(0.0, 0.0, 0.0, 0.0);
        V3d<float> base_forces;
        V3d<float> base_moments;
        model.tailplane_forces(state, aircraft, controls, world, state.velocity(), base_forces, base_moments);


        // forward stick(positive) reduces tailplane AoA per model implementation
        controls.set_controls(1.0, 0.0, 0.0, 0.0);
        V3d<float> elev_forces;
        V3d<float> elev_moments;
        model.tailplane_forces(state, aircraft, controls, world, state.velocity(), elev_forces, elev_moments);


        // Observed: forward stick reduces tailplane AoA; in this trim it increases downforce
        assertGreater(elev_forces[2], base_forces[2], "Forward stick increases tailplane downforce in this trim (Z more positive)");
    }

    void test_vertical_descent_zero_forward_speed() {
        // Descending vertically with zero forward speed(w positive = down)
        state.set_velocity(0.0, 0.0, 5.0);
        state.set_angular_velocity(0.0, 0.0, 0.0);
        controls.set_controls(0.0, 0.0, 0.0, 0.0);

        V3d<float> tp_forces;
        V3d<float> tp_moments;
        model.tailplane_forces(state, aircraft, controls, world, state.velocity(), tp_forces, tp_moments);

        std::cout << "Vertical Descent, Tailplane Force: " << tp_forces << "," << "Tailplane moments : " << tp_moments << std::endl;

        // Values should be finite
        assertTrue(isfinite(tp_forces[0]) && isfinite(tp_forces[1]) && isfinite(tp_forces[2]), "Tail forces must be finite");
        assertTrue(isfinite(tp_moments[0]) && isfinite(tp_moments[1]) && isfinite(tp_moments[2]), "Tail moments must be finite");

        // And bounded within model safety limits
        assertLess(abs(tp_forces[0]), MAX_FORCE, "Force limit");
        assertLess(abs(tp_forces[2]), MAX_FORCE, "Force limit");
        assertLess(abs(tp_moments[1]), MAX_MOMENT, "Moments limit");

        // Should be negative Z force(upwards) as effectively flat plate drag at zero forward speed with tailplane facing into the flow
        assertLess(tp_forces[2], 0.0, "Tailplane Z force should be negative (upwards) at zero forward speed with tailplane facing into the flow");

        // Should be small X force as tailplane is mostly flat to the flow at zero forward speed, but may have a small positive X force due to model implementation of tailplane drag at zero forward speed
        assertLess(abs(tp_forces[0]), 10.0, "Tailplane X force should be small at zero forward speed with tailplane facing into the flow");

        // Should be zero Y force due to symmetry
        assertEqual(tp_forces[1], 0.0, "Tailplane Y force should be zero at zero forward speed with tailplane facing into the flow");

        // Should be nose down moment(negative pitch moment) as tailplane drag produces a nose - down moment when tail is behind CG
        assertLess(tp_moments[1], 0.0, "Tailplane should produce a nose-down moment (negative pitch moment) at zero forward speed with tailplane facing into the flow");
    }

    void test_vertical_ascent_zero_forward_speed() {
        // Ascending vertically with zero forward speed(w negative = up in body axes)
        state.set_velocity(0.0, 0.0, -5.0);
        state.set_angular_velocity(0.0, 0.0, 0.0);
        controls.set_controls(0.0, 0.0, 0.0, 0.0);

        V3d<float> tp_forces;
        V3d<float> tp_moments;
        model.tailplane_forces(state, aircraft, controls, world, state.velocity(), tp_forces, tp_moments);

        std::cout << "Vertical ascent, Tailplane Force: " << tp_forces << "," << "Tailplane moments : " << tp_moments << std::endl;

        // Values should be finite
        assertTrue(isfinite(tp_forces[0]) && isfinite(tp_forces[1]) && isfinite(tp_forces[2]), "Tail forces must be finite");
        assertTrue(isfinite(tp_moments[0]) && isfinite(tp_moments[1]) && isfinite(tp_moments[2]), "Tail moments must be finite");

        // And bounded within model safety limits
        assertLess(abs(tp_forces[0]), MAX_FORCE, "Force limit");
        assertLess(abs(tp_forces[2]), MAX_FORCE, "Force limit");
        assertLess(abs(tp_moments[1]), MAX_MOMENT, "Moments limit");

        // Should be positive Z force(upwards) as effectively flat plate drag at zero forward speed with tailplane facing into the flow
        assertGreater(tp_forces[2], 0.0, "Tailplane Z force should be positive (downwards) at zero forward speed with tailplane facing into the flow");

        // Should be small X force as tailplane is mostly flat to the flow at zero forward speed, but may have a small positive X force due to model implementation of tailplane drag at zero forward speed
        assertLess(abs(tp_forces[0]), 10.0, "Tailplane X force should be small at zero forward speed with tailplane facing into the flow");

        // Should be zero Y force due to symmetry
        assertEqual(tp_forces[1], 0.0, "Tailplane Y force should be zero at zero forward speed with tailplane facing into the flow");

        // Should be nose down moment(negative pitch moment) as tailplane drag produces a nose - down moment when tail is behind CG
        assertGreater(tp_moments[1], 0.0, "Tailplane should produce a nose-up moment (positive pitch moment) at zero forward speed with tailplane facing into the flow");
    }

    public:
        void test_all() {
            test_straight_and_level();
            test_increased_aoa_increases_tail_lift();
            test_pitch_rate_increases_tail_lift();
            test_elevator_deflection_reduces_tail_lift();
            test_vertical_descent_zero_forward_speed();
            test_vertical_ascent_zero_forward_speed();
        }
};

