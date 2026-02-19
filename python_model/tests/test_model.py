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

class TestModel(unittest.TestCase):

    def setUp(self):
        self.state = StateVector()
        self.model = Model()
        self.aircraft = ASK21()
        self.world = World()
        self.controls = ControlInputs()

    # Case 1 - Straight and level flight
    def test_straight_and_level(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_position((0.0, 0.0, -500.0))
        Model_force, Model_moments = self.model.calculate_aerodynamics(self.state, self.aircraft, self.controls, self.world, self.state.velocity())

        print(f"S&L, Model Force: {Model_force}, Model moments: {Model_moments}")

        self.assertLess( Model_force[0], 0.0, "Model force X component should be negative due to drag")
        self.assertEqual( Model_force[1], 0.0, "Model force Y component should be zero") # no sideforce
        self.assertLess(Model_force[2], 0.0, "Model force Z component should be negative (up)")
        
        self.assertEqual( Model_moments[0], 0.0, "Model moment roll should be zero")
        #self.assertLess( Model_moments[1], 0.0, "Model moment pitch should be negative") # pitch nose down
        self.assertEqual( Model_moments[2], 0.0, "Model moment yaw should be zero")


    # Case 2 - aircraft with right velocity component in body frame (nose yawed left)
    def test_yawed_left(self):
        self.state.set_velocity((25.0, 1.0, 0.0))  # 25 m/s level flight
        Model_force, Model_moments = self.model.calculate_aerodynamics(self.state, self.aircraft, self.controls, self.world, self.state.velocity())
        print(f"Yawed left, Model Force: {Model_force}, Model moments: {Model_moments}")

        self.assertLess(Model_force[0], 0.0, "Model force X component should be negative") # drag
        self.assertLess(Model_force[1], 0.0, "Model force Y component should be negative") # fin and fuselage produce lift away from airflow
        self.assertLess(Model_force[2], 0.0, "Model force Z component should be negative") # lift -ve is up

        self.assertLess( Model_moments[0], 0.0, "Model moment roll should be negative")  # Slip to the right should roll left
        # will depend on speed whether pitches up or down
        self.assertGreater( Model_moments[2], 0.0, "Model moment yaw should be positive ") # should yaw into oncoming airflow


    # Case 3 - aircraft with left velocity component in body frame(nose yawed right)
    def test_yawed_right(self):
        self.state.set_velocity((25.0, -1.0, 0.0))  # 25 m/s level flight
        Model_force, Model_moments = self.model.calculate_aerodynamics(self.state, self.aircraft, self.controls, self.world, self.state.velocity())

        print(f"Yawed right, Model Force: {Model_force}, Model moments: {Model_moments}")

        self.assertLess(Model_force[0], 0.0, "Model force X component should be negative") # drag
        self.assertGreater(Model_force[1], 0.0, "Model force Y component should be negative") # fin and fuselage produce lift away from airflow
        self.assertLess(Model_force[2], 0.0, "Model force Z component should be negative") # lift -ve is up

        self.assertGreater( Model_moments[0], 0.0, "Model moment roll should be negative")  # Slip to the left should roll right
        # will depend on speed whether pitches up or down
        self.assertLess( Model_moments[2], 0.0, "Model moment yaw should be positive ") # should yaw into oncoming airflow


    # Case 4 - aircraft with with right yaw rate
    # If nose to the right, tail will be going left, so Model should produce a right force to counteract
    def test_yawing_right(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, 0.1))  # Yaw rate to right
        Model_force, Model_moments = self.model.calculate_aerodynamics(self.state, self.aircraft, self.controls, self.world, self.state.velocity())
        print(f"Yawing right, Model Force: {Model_force}, Model moments: {Model_moments}")

        self.assertLess(Model_force[0], 0.0, "Model force Y component should be negative") # Drag is -ve
        self.assertGreater(Model_force[1], 0.0, "Model force Y component should be positive") # fin component should against rotation (fin moving left)
        self.assertLess( Model_force[2], 0.0, "Model force Z component should be negative (up)") # lift -ve is up

        self.assertLess( Model_moments[2], 0.0, "Model moment yaw should be negative ") # differential drag should oppose yaw rate


    # Case 5 - aircraft with with left yaw rate
    # If nose yaModel to the left, tail will be going right, so Model should produce a left force to counteract
    def test_yawing_left(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, -0.1))  # Yaw rate to left
        Model_force, Model_moments = self.model.calculate_aerodynamics(self.state, self.aircraft, self.controls, self.world, self.state.velocity())
        print(f"Yawing left, Model Force: {Model_force}, Model moments: {Model_moments}")

        self.assertLess(Model_force[0], 0.0, "Model force Y component should be negative") # Drag is -ve
        self.assertLess(Model_force[1], 0.0, "Model force Y component should be negative") # fin component should be against rotation (fin moving right)
        self.assertLess( Model_force[2], 0.0, "Model force Z component should be negative (up)") # lift -ve is up

        self.assertGreater( Model_moments[2], 0.0, "Model moment yaw should be positive ") # should oppose yaw rate


    def test_increasing_yaw_angle_left(self):

        self.state.set_angular_velocity((0.0, 0.0, 0.0))  # No yaw rate


        for vy in range(1,25) :
            self.state.set_velocity((25.0, vy, 0.0))  # 25 m/s level flight
            Model_force, Model_moments = self.model.calculate_aerodynamics(self.state, self.aircraft, self.controls, self.world, self.state.velocity())
            print(f"Model Yawed left, Vy: {vy}, Model Force: {Model_force}, Model moments: {Model_moments}")

            self.assertLess(Model_force[0], 0.0, "Model force X component should be negative") # drag
            self.assertLess(Model_force[1], 0.0, "Model force Y component should be negative") # fin and fuselage produce lift away from airflow
            self.assertLess(Model_force[2], 0.0, "Model force Z component should be negative") # lift -ve is up

            self.assertLess( Model_moments[0], 0.0, "Model moment roll should be negative")  # Slip to the right should roll left
            # will depend on speed whether pitches up or down
            self.assertGreater( Model_moments[2], 0.0, "Model moment yaw should be positive ") # should yaw into oncoming airflow


if __name__ == '__main__':
    unittest.main()