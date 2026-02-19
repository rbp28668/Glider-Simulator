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

class TestWing(unittest.TestCase):

    def setUp(self):
        self.state = StateVector()
        self.model = Model()
        self.aircraft = ASK21()
        self.world = World()
        self.controls = ControlInputs()
        self.state.set_position((0.0, 0.0, -500.0))

    # Case 1 - Straight and level flight
    def test_straight_and_level(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, 0.0)) # Zero yaw rate

        Wing_force, Wing_moments = self.model.wing_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"S&L, Wing Force: {Wing_force}, Wing moments: {Wing_moments}")

        self.assertLess( Wing_force[0], 0.0, "Wing force X component should be negative due to drag")
        self.assertEqual( Wing_force[1], 0.0, "Wing force Y component should be zero") # no sideforce
        self.assertLess(Wing_force[2], 0.0, "Wing force Z component should be negative (up)")
        
        self.assertEqual( Wing_moments[0], 0.0, "Wing moment roll should be zero")
        self.assertLess( Wing_moments[1], 0.0, "Wing moment pitch should be negative") # pitch nose down
        self.assertEqual( Wing_moments[2], 0.0, "Wing moment yaw should be zero")


    # Case 2 - aircraft with right velocity component in body frame (nose yawed left)
    def test_yawed_left(self):
        self.state.set_velocity((25.0, 1.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, 0.0)) # Zero yaw rate

        Wing_force, Wing_moments = self.model.wing_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)   
        print(f"Yawed left, Wing Force: {Wing_force}, Wing moments: {Wing_moments}")

        self.assertLess(Wing_force[0], 0.0, "Wing force X component should be negative") # drag
        self.assertEqual(Wing_force[1], 0.0, "Wing force Y component should be zero") 
        self.assertLess(Wing_force[2], 0.0, "Wing force Z component should be negative") # lift -ve is up

        self.assertLess( Wing_moments[0], 0.0, "Wing moment roll should be negative")  # Slip to the right should roll left
        self.assertLess( Wing_moments[1], 0.0, "Wing moment pitch should be negative") # pitch nose down
        self.assertGreater( Wing_moments[2], 0.0, "Wing moment yaw should be positive ") # should yaw into oncoming airflow


    # Case 3 - aircraft with left velocity component in body frame(nose yawed right)
    def test_yawed_right(self):
        self.state.set_velocity((25.0, -1.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, 0.0)) # Zero yaw rate

        Wing_force, Wing_moments = self.model.wing_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Yawed right, Wing Force: {Wing_force}, Wing moments: {Wing_moments}")

        self.assertLess(Wing_force[0], 0.0, "Wing force X component should be negative") # drag
        self.assertEqual(Wing_force[1], 0.0, "Wing force Y component should be zero")    # no sideforce (as a first approximation anyway)
        self.assertLess(Wing_force[2], 0.0, "Wing force Z component should be negative") # lift -ve is up

        self.assertGreater( Wing_moments[0], 0.0, "Wing moment roll should be negative")  # Slip to the left should roll right
        self.assertLess( Wing_moments[1], 0.0, "Wing moment pitch should be negative") # pitch nose down
        self.assertLess( Wing_moments[2], 0.0, "Wing moment yaw should be positive ") # should yaw into oncoming airflow


    # Case 4 - aircraft with with right yaw rate
    # If nose to the right, tail will be going left, so Wing should produce a right force to counteract
    def test_yawing_right(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, 0.1))  # Yaw rate to right

        Wing_force, Wing_moments = self.model.wing_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Yawing right, Wing Force: {Wing_force}, Wing moments: {Wing_moments}")

        self.assertLess(Wing_force[0], 0.0, "Wing force Y component should be negative") # Drag is -ve
        self.assertEqual(Wing_force[1], 0.0, "Wing force Y component should be zero") # No sideways component of wing 
        self.assertLess( Wing_force[2], 0.0, "Wing force Z component should be negative (up)") # lift -ve is up

        self.assertLess( Wing_moments[2], 0.0, "Wing moment yaw should be negative ") # differential drag should oppose yaw rate


    # Case 5 - aircraft with with left yaw rate
    # If nose yawing to the left, tail will be going right, so Wing should produce a left force to counteract
    def test_yawing_left(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, -0.1))  # Yaw rate to left

        Wing_force, Wing_moments = self.model.wing_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Yawing left, Wing Force: {Wing_force}, Wing moments: {Wing_moments}")

        self.assertLess(Wing_force[0], 0.0, "Wing force Y component should be negative") # Drag is -ve
        self.assertEqual(Wing_force[1], 0.0, "Wing force Y component should be zero") # No sideways component of wing 
        self.assertLess( Wing_force[2], 0.0, "Wing force Z component should be negative (up)") # lift -ve is up

        self.assertGreater( Wing_moments[2], 0.0, "Wing moment yaw should be positive ") # should oppose yaw rate


    


    # Case 6, as per #2 - aircraft with right velocity component in body frame (nose yawed left) but increasing angles
    def test_increasing_yaw_angle_left(self):

        self.state.set_angular_velocity((0.0, 0.0, 0.0)) # Zero yaw rate
        for vy in range(1,25):
            self.state.set_velocity((25.0, vy, 0.0))  # 25 m/s level flight, increasing vy
            Wing_force, Wing_moments = self.model.wing_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)   
            #print(f"Yawed left Vy:{vy} Wing Force: {Wing_force}, Wing moments: {Wing_moments}")

            self.assertLess(Wing_force[0], 0.0, "Wing force X component should be negative") # drag
            self.assertEqual(Wing_force[1], 0.0, "Wing force Y component should be zero") 
            self.assertLess(Wing_force[2], 0.0, "Wing force Z component should be negative") # lift -ve is up

            self.assertLess( Wing_moments[0], 0.0, "Wing moment roll should be negative")  # Slip to the right should roll left
            self.assertLess( Wing_moments[1], 0.0, "Wing moment pitch should be negative") # pitch nose down
            self.assertGreater( Wing_moments[2], 0.0, "Wing moment yaw should be positive ") # should yaw into oncoming airflow (lift dependent drag)


    # Case 7, as per #3 - aircraft with left velocity component in body frame(nose yawed right) but increasing angles
    def test_increasing_yaw_angle_right(self):
        self.state.set_angular_velocity((0.0, 0.0, 0.0)) # Zero yaw rate

        for vy in range(1,25):
            self.state.set_velocity((25.0, -vy, 0.0))  # 25 m/s level flight, increasing vy

            Wing_force, Wing_moments = self.model.wing_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
            #print(f"Yawed right, Vy={-vy}, Wing Force: {Wing_force}, Wing moments: {Wing_moments}")

            self.assertLess(Wing_force[0], 0.0, "Wing force X component should be negative") # drag
            self.assertEqual(Wing_force[1], 0.0, "Wing force Y component should be zero")    # no sideforce (as a first approximation anyway)
            self.assertLess(Wing_force[2], 0.0, "Wing force Z component should be negative") # lift -ve is up

            self.assertGreater( Wing_moments[0], 0.0, "Wing moment roll should be negative")  # Slip to the left should roll right
            self.assertLess( Wing_moments[1], 0.0, "Wing moment pitch should be negative") # pitch nose down
            self.assertLess( Wing_moments[2], 0.0, "Wing moment yaw should be positive ") # should yaw into oncoming airflow

   # Case 8 - Test rolling right
    def test_rolling_right(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.1, 0.0, 0.0)) # +ve roll rate

        Wing_force, Wing_moments = self.model.wing_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Rolling right, Wing Force: {Wing_force}, Wing moments: {Wing_moments}")

        self.assertLess( Wing_force[0], 0.0, "Wing force X component should be negative due to drag")
        self.assertEqual( Wing_force[1], 0.0, "Wing force Y component should be zero") # no sideforce
        self.assertLess(Wing_force[2], 0.0, "Wing force Z component should be negative (up)")
        
        self.assertLess( Wing_moments[0], 0.0, "Wing moment roll should be negative") # in opposition to +ve roll
        self.assertLess( Wing_moments[1], 0.0, "Wing moment pitch should be negative") # pitch nose down
        self.assertGreater( Wing_moments[2], 0.0, "Wing moment yaw should be positive") # small yaw due to differential left dependent drag

   # Case 9 - Test rolling left
    def test_rolling_left(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((-0.1, 0.0, 0.0)) # -ve roll rate

        Wing_force, Wing_moments = self.model.wing_forces_moments(self.state, self.aircraft, self.state.velocity(), self.controls, self.world)
        print(f"Rolling left, Wing Force: {Wing_force}, Wing moments: {Wing_moments}")

        self.assertLess( Wing_force[0], 0.0, "Wing force X component should be negative due to drag")
        self.assertEqual( Wing_force[1], 0.0, "Wing force Y component should be zero") # no sideforce
        self.assertLess(Wing_force[2], 0.0, "Wing force Z component should be negative (up)")
        
        self.assertGreater( Wing_moments[0], 0.0, "Wing moment roll should be positive") # in opposition to +ve roll
        self.assertLess( Wing_moments[1], 0.0, "Wing moment pitch should be negative") # pitch nose down
        self.assertLess( Wing_moments[2], 0.0, "Wing moment yaw should be negative") # small yaw due to differential left dependent drag


if __name__ == '__main__':
    unittest.main()