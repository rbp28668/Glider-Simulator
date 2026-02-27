import sys
import unittest
import os


#sys.path.append("..\\")  # To allow importing from parent directory
up = os.path.abspath('../python_model')
sys.path.insert(0, up)

from state_vector import StateVector
from model import Model
from ask21 import ASK21
from world import World
from control_inputs import ControlInputs
import model as model_module
from math import isfinite


class TestTailplane(unittest.TestCase):

    def setUp(self):
        self.state = StateVector()
        self.model = Model()
        self.aircraft = ASK21()
        self.world = World()
        self.controls = ControlInputs()
        self.state.set_position((0.0, 0.0, -500.0))

    def test_straight_and_level(self):
        self.state.set_velocity((25.0, 0.0, 0.0))
        self.state.set_angular_velocity((0.0, 0.0, 0.0))


        tp_force, tp_moments = self.model.tailplane_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"S&L, Tail Force: {tp_force}, Tail moments: {tp_moments}")

        self.assertLess(tp_force[0], 0.0, "Tailplane X force should be negative due to drag")
        self.assertEqual(tp_force[1], 0.0, "Tailplane Y force should be zero")
        self.assertGreater(tp_force[2], 0.0, "Tailplane Z component should be positive (downforce at trim)")

    def test_increased_aoa_increases_tail_lift(self):
        # baseline
        self.state.set_velocity((25.0, 0.0, 0.0))
        self.state.set_angular_velocity((0.0, 0.0, 0.0))
        base_force, _ = self.model.tailplane_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)

        # increase local AoA by adding a small downward velocity (w positive = down)
        self.state.set_velocity((25.0, 0.0, 1.0))
        inc_force, _ = self.model.tailplane_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)

        # Observed: increasing local AoA here reduces tail downforce (Z moves toward zero)
        self.assertLess(inc_force[2], base_force[2], "Increased local AoA should reduce tailplane downforce (Z less positive)")

    def test_pitch_rate_increases_tail_lift(self):
        # baseline
        self.state.set_velocity((25.0, 0.0, 0.0))
        self.state.set_angular_velocity((0.0, 0.0, 0.0))
        base_force, _ = self.model.tailplane_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Pitch rate base, Tail Force: {base_force}")

        # apply positive pitch rate (nose up) which moves tail down and increases local AoA
        self.state.set_velocity((25.0, 0.0, 0.0))
        self.state.set_angular_velocity((0.0, 0.1, 0.0))
        pr_force, _ = self.model.tailplane_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Pitch rate test, Tail Force: {pr_force}")
        
        # Observed: positive pitch rate (nose up) moves tail down and here reduces downforce magnitude
        self.assertLess(pr_force[2], base_force[2], "Positive pitch rate should reduce tailplane downforce (Z less positive) in this trim condition")

    def test_elevator_deflection_reduces_tail_lift(self):
        # baseline
        self.state.set_velocity((25.0, 0.0, 0.0))
        self.state.set_angular_velocity((0.0, 0.0, 0.0))
        base_force, _ = self.model.tailplane_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)

        # forward stick (positive) reduces tailplane AoA per model implementation
        self.controls.set_controls(1.0, 0.0, 0.0, 0.0)
        elev_force, _ = self.model.tailplane_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)

        # Observed: forward stick reduces tailplane AoA; in this trim it increases downforce
        self.assertGreater(elev_force[2], base_force[2], "Forward stick increases tailplane downforce in this trim (Z more positive)")

    def test_vertical_descent_zero_forward_speed(self):
        # Descending vertically with zero forward speed (w positive = down)
        self.state.set_velocity((0.0, 0.0, 5.0))
        self.state.set_angular_velocity((0.0, 0.0, 0.0))

        tp_force, tp_moments = self.model.tailplane_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Vertical descent, Tail Force: {tp_force}, Tail moments: {tp_moments}")

        # Values should be finite
        self.assertTrue(isfinite(tp_force[0]) and isfinite(tp_force[1]) and isfinite(tp_force[2]), "Tail forces must be finite")
        self.assertTrue(isfinite(tp_moments[0]) and isfinite(tp_moments[1]) and isfinite(tp_moments[2]), "Tail moments must be finite")

        # And bounded within model safety limits
        self.assertLess(abs(tp_force[0]), model_module.MAX_FORCE)
        self.assertLess(abs(tp_force[2]), model_module.MAX_FORCE)
        self.assertLess(abs(tp_moments[1]), model_module.MAX_MOMENT)

        # Should be negative Z force (upwards) as effectively flat plate drag at zero forward speed with tailplane facing into the flow
        self.assertLess(tp_force[2], 0.0, "Tailplane Z force should be negative (upwards) at zero forward speed with tailplane facing into the flow")

        # Should be small X force as tailplane is mostly flat to the flow at zero forward speed, but may have a small positive X force due to model implementation of tailplane drag at zero forward speed
        self.assertLess(abs(tp_force[0]), 10.0, "Tailplane X force should be small at zero forward speed with tailplane facing into the flow")

        # Should be zero Y force due to symmetry
        self.assertEqual(tp_force[1], 0.0, "Tailplane Y force should be zero at zero forward speed with tailplane facing into the flow")

        # Should be nose down moment (negative pitch moment) as tailplane drag produces a nose-down moment when tail is behind CG
        self.assertLess(tp_moments[1], 0.0, "Tailplane should produce a nose-down moment (negative pitch moment) at zero forward speed with tailplane facing into the flow")


    def test_vertical_ascent_zero_forward_speed(self):
        # Ascending vertically with zero forward speed (w negative = up in body axes)
        self.state.set_velocity((0.0, 0.0, -5.0))
        self.state.set_angular_velocity((0.0, 0.0, 0.0))

        tp_force, tp_moments = self.model.tailplane_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Vertical ascent, Tail Force: {tp_force}, Tail moments: {tp_moments}")

        # Values should be finite
        self.assertTrue(isfinite(tp_force[0]) and isfinite(tp_force[1]) and isfinite(tp_force[2]), "Tail forces must be finite")
        self.assertTrue(isfinite(tp_moments[0]) and isfinite(tp_moments[1]) and isfinite(tp_moments[2]), "Tail moments must be finite")

        # And bounded within model safety limits
        self.assertLess(abs(tp_force[0]), model_module.MAX_FORCE)
        self.assertLess(abs(tp_force[2]), model_module.MAX_FORCE)
        self.assertLess(abs(tp_moments[1]), model_module.MAX_MOMENT)

        # Should be positive Z force (upwards) as effectively flat plate drag at zero forward speed with tailplane facing into the flow
        self.assertGreater(tp_force[2], 0.0, "Tailplane Z force should be positive (downwards) at zero forward speed with tailplane facing into the flow")

        # Should be small X force as tailplane is mostly flat to the flow at zero forward speed, but may have a small positive X force due to model implementation of tailplane drag at zero forward speed
        self.assertLess(abs(tp_force[0]), 10.0, "Tailplane X force should be small at zero forward speed with tailplane facing into the flow")

        # Should be zero Y force due to symmetry
        self.assertEqual(tp_force[1], 0.0, "Tailplane Y force should be zero at zero forward speed with tailplane facing into the flow")

        # Should be nose down moment (negative pitch moment) as tailplane drag produces a nose-down moment when tail is behind CG
        self.assertGreater(tp_moments[1], 0.0, "Tailplane should produce a nose-up moment (positive pitch moment) at zero forward speed with tailplane facing into the flow")



if __name__ == '__main__':
    unittest.main()
