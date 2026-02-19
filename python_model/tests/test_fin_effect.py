import sys
import unittest
import os

up = os.path.abspath('../python_model')
sys.path.insert(0, up)

from state_vector import StateVector
from model import Model
from ask21 import ASK21
from world import World
from control_inputs import ControlInputs

class TestFin(unittest.TestCase):

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
        fin_force, fin_moments = self.model.fin_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"S&L, Fin Force: {fin_force}, Fin moments: {fin_moments}")

        self.assertLess( fin_force[0], 0.0, "Fin force X component should be negative due to drag")
        self.assertEqual( fin_force[1], 0.0, "Fin force Y component should not be zero")
        self.assertEqual(fin_force[2], 0.0, "Fin force Z component should be zero")
        
        self.assertEqual( fin_moments[0], 0.0, "Fin moment roll should be zero")
        self.assertEqual( fin_moments[1], 0.0, "Fin moment pitch should be zero")
        self.assertEqual( fin_moments[2], 0.0, "Fin moment yaw should be zero")


    # Case 2 - aircraft with right velocity component in body frame (nose yawed left)
    def test_yawed_left(self):
        self.state.set_velocity((25.0, 1.0, 0.0))  # 25 m/s level flight
        fin_force, fin_moments = self.model.fin_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)   
        print(f"Yawed left, Fin Force: {fin_force}, Fin moments: {fin_moments}")

        self.assertLess(fin_force[1], 0.0, "Fin force Y component should be negative") # Should produce a left force to counter right velocity
        self.assertEqual(fin_force[2], 0.0, "Fin force Z component should be zero")

        self.assertGreater( fin_moments[2], 0.0, "Fin moment yaw should be positive ") # should yaw into oncoming airflow


    # Case 3 - aircraft with left velocity component in body frame(nose yawed right)
    def test_yawed_right(self):
        self.state.set_velocity((25.0, -1.0, 0.0))  # 25 m/s level flight
        fin_force, fin_moments = self.model.fin_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Yawed right, Fin Force: {fin_force}, Fin moments: {fin_moments}")

        self.assertGreater(fin_force[1], 0.0, "Fin force Y component should be positive") # Should produce a right force to counter left velocity
        self.assertEqual(fin_force[2], 0.0, "Fin force Z component should be zero")

        self.assertLess( fin_moments[2], 0.0, "Fin moment yaw should be negative ") # should yaw into oncoming airflow


    # Case 4 - aircraft with with right yaw rate
    # If nose to the right, tail will be going left, so fin should produce a right force to counteract
    def test_yawing_right(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, 0.1))  # Yaw rate to right
        fin_force, fin_moments = self.model.fin_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Yawing right, Fin Force: {fin_force}, Fin moments: {fin_moments}")

        self.assertGreater(fin_force[1], 0.0, "Fin force Y component should be positive") # Should produce a right force to counter right yaw rate
        self.assertEqual( fin_force[2], 0.0, "Fin force Z component should be zero")

        self.assertLess( fin_moments[2], 0.0, "Fin moment yaw should be negative ") # should oppose yaw rate


    # Case 5 - aircraft with with left yaw rate
    # If nose yawing to the left, tail will be going right, so fin should produce a left force to counteract
    def test_yawing_left(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, -0.1))  # Yaw rate to left
        fin_force, fin_moments = self.model.fin_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Yawing left, Fin Force: {fin_force}, Fin moments: {fin_moments}")

        self.assertLess(fin_force[1], 0.0, "Fin force Y component should be negative") # Should produce a left force to counter left yaw rate
        self.assertEqual( fin_force[2], 0.0, "Fin force Z component should be zero")

        self.assertGreater( fin_moments[2], 0.0, "Fin moment yaw should be negative ") # should oppose yaw rate

    # Case 6 - test increasing yaw - as per #2 but more so
    def test_increasing_yaw_angle_left(self):
  
        self.state.set_angular_velocity((0.0, 0.0, 0.0))  # No yaw rate

        # Just increasing yaw, constant Vx - i.e. 0 to 45 degrees
        for vy in range(1,25) :
            self.state.set_velocity((25.0, vy, 0.0))  # 25 m/s level flight

            fin_force, fin_moments = self.model.fin_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
            print(f"Yawing left: Vy={vy}, Fin Force: {fin_force}, Fin moments: {fin_moments}")

            self.assertLess(fin_force[1], 0.0, "Fin force Y component should be negative") # Should produce a left force to counter left yaw rate
            self.assertEqual( fin_force[2], 0.0, "Fin force Z component should be zero")

            self.assertGreater( fin_moments[2], 0.0, "Fin moment yaw should be negative ") # should oppose yaw rate


        # increasing side velocity, decreasing forward 0..90 degrees
        for vy in range(1,25) :
            self.state.set_velocity((25.0-vy, vy, 0.0))  # 25 m/s level flight

            fin_force, fin_moments = self.model.fin_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
            print(f"Yawing left: Vy={vy}, Fin Force: {fin_force}, Fin moments: {fin_moments}")

            self.assertLess(fin_force[1], 0.0, "Fin force Y component should be negative") # Should produce a left force to counter left yaw rate
            self.assertEqual( fin_force[2], 0.0, "Fin force Z component should be zero")

            self.assertGreater( fin_moments[2], 0.0, "Fin moment yaw should be negative ") # should oppose yaw rate
  

      # Case 3 - aircraft with left velocity component in body frame(nose yawed right)
    def test_increasing_yaw_angle_right(self):

        self.state.set_angular_velocity((0.0, 0.0, 0.0))  # No yaw rate

        for vy in range(1,25):
            self.state.set_velocity((25.0, -vy, 0.0))  # 25 m/s level flight
            fin_force, fin_moments = self.model.fin_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
            print(f"Yawed right, Vy={-vy}, Fin Force: {fin_force}, Fin moments: {fin_moments}")

            self.assertGreater(fin_force[1], 0.0, "Fin force Y component should be positive") # Should produce a right force to counter left velocity
            self.assertEqual(fin_force[2], 0.0, "Fin force Z component should be zero")

            self.assertLess( fin_moments[2], 0.0, "Fin moment yaw should be negative ") # should yaw into oncoming airflow

        for vy in range(1,25):
            self.state.set_velocity((25.0-vy, -vy, 0.0))  # 25 m/s level flight
            fin_force, fin_moments = self.model.fin_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
            print(f"Yawed right, Vy={-vy}, Fin Force: {fin_force}, Fin moments: {fin_moments}")

            self.assertGreater(fin_force[1], 0.0, "Fin force Y component should be positive") # Should produce a right force to counter left velocity
            self.assertEqual(fin_force[2], 0.0, "Fin force Z component should be zero")

            self.assertLess( fin_moments[2], 0.0, "Fin moment yaw should be negative ") # should yaw into oncoming airflow


if __name__ == '__main__':
    unittest.main()