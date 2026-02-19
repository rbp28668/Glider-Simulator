import sys
import unittest
import os


#sys.path.append("..\\")  # To allow importing from parent directory
up = os.path.abspath('../python_model')
print(up)
sys.path.insert(0, up)

from state_vector import StateVector
from model import Model
from ask21 import ASK21
from world import World
from control_inputs import ControlInputs
from simulation import Simulation

class TestSimulation(unittest.TestCase):

    def setUp(self):
        self.state = StateVector()
        self.model = Model()
        self.aircraft = ASK21()
        self.world = World()
        self.controls = ControlInputs()
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_position((0.0, 0.0, -500.0))

        self.inertia = (self.aircraft.Ixx, self.aircraft.Iyy, self.aircraft.Izz, self.aircraft.Ixz)

    # Case 1 - Straight and level flight
    def test_straight_and_level(self):
        self.state.set_angular_velocity((0.0,0.0,0.0)) 

        moments_body = (0,0,0)
        omega_dot = Simulation.calculate_angular_acceleration(self.state, moments_body, self.inertia)
        print(f"S&L: {moments_body} -> {omega_dot}")
        # allow small numeric coupling; expect essentially zero
        self.assertAlmostEqual( omega_dot[0], 0.0, delta=1e-2, msg="Angular acceleration around X should be (near) zero")
        self.assertEqual( omega_dot[1], 0.0, "Angular acceleration around Y should be zero")
        self.assertEqual( omega_dot[2], 0.0, "Angular acceleration around Z should be zero")

    def test_rolling_right_zero_moments(self):
        self.state.set_angular_velocity((1.0,0.0,0.0)) 

        moments_body = (0,0,0)
        omega_dot = Simulation.calculate_angular_acceleration(self.state, moments_body, self.inertia)
        print(f"Rolling right: {moments_body} -> {omega_dot}")
        # allow tiny inertial coupling; expect near-zero roll accel
        self.assertAlmostEqual( omega_dot[0], 0.0, delta=1e-2, msg="Angular acceleration around X should be (near) zero")
        self.assertLess( omega_dot[1], 0.0, "Angular acceleration around Y should be negative") # Tail on top should result in nose-down when rolling (think of centrifugal force)
        self.assertEqual( omega_dot[2], 0.0, "Angular acceleration around Z should be zero")

    def test_rolling_left_zero_moments(self):
        self.state.set_angular_velocity((-1.0,0.0,0.0)) 

        moments_body = (0,0,0)
        omega_dot = Simulation.calculate_angular_acceleration(self.state, moments_body, self.inertia)
        print(f"Rolling left: {moments_body} -> {omega_dot}")
        self.assertAlmostEqual( omega_dot[0], 0.0, delta=1e-2, msg="Angular acceleration around X should be (near) zero")
        self.assertLess( omega_dot[1], 0.0, "Angular acceleration around Y should be negative") # Tail on top should result in nose-down when rolling (think of centrifugal force)
        self.assertEqual( omega_dot[2], 0.0, "Angular acceleration around Z should be zero")

    def test_rolling_right_positive_pitch(self):
        self.state.set_angular_velocity((1.0,0.0,0.0)) 

        moments_body = (0,1000.0,0) # pitch up moment

        omega_dot = Simulation.calculate_angular_acceleration(self.state, moments_body, self.inertia)
        print(f"Rolling right: {moments_body} -> {omega_dot}")

        self.assertEqual( omega_dot[0], 0.0, "Angular acceleration around X should be zero")
        self.assertGreater( omega_dot[1], 0.0, "Angular acceleration around Y should be positive")
        self.assertEqual( omega_dot[2], 0.0, "Angular acceleration around Z should be zero")

    def test_rolling_right_positive_yaw(self):
        self.state.set_angular_velocity((1.0,0.0,0.0)) 

        moments_body = (0,0,1000) # yaw right moment

        omega_dot = Simulation.calculate_angular_acceleration(self.state, moments_body, self.inertia)
        print(f"Rolling right: {moments_body} -> {omega_dot}")

        self.assertAlmostEqual( omega_dot[0], 0.0, delta=1e-2, msg="Angular acceleration around X should be (near) zero")
        self.assertLess( omega_dot[1], 0.0, "Angular acceleration around Y (pitch) should be negative given Ixz")
        self.assertGreater( omega_dot[2], 0.0, "Angular acceleration around Z should be positive given +ve yaw moment")



if __name__ == '__main__':
    
    unittest.main()