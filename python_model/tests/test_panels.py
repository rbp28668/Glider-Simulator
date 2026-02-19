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
from panel import Panel

class TestPanel(unittest.TestCase):

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

        panel = self.aircraft.wing[2] # outer between brakes and ailerons
        forces_r, moments_r = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,1.0)
        forces_l, moments_l = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,-1.0)

        print(f"S&L Forces left {forces_l}, Forces right {forces_r}")
        self.assertLess( forces_r[0], 0.0, "R panel produces drag")
        self.assertLess( forces_l[0], 0.0, "L panel produces drag")
        self.assertLess( forces_r[2], 0.0, "R panel produces lift")
        self.assertLess( forces_l[2], 0.0, "L panel produces lift")

        self.assertEqual( forces_l[0], forces_r[0], "Panels have equal drag")
        self.assertEqual( forces_l[2], forces_r[2], "Panels have equal lift")

        self.assertLess( moments_r[0], 0.0, "R panel rolls left")
        self.assertLess( moments_r[1], 0.0, "R panel pitches down")
        self.assertGreater( moments_r[2], 0.0, "R panel yaws right")

        self.assertGreater( moments_l[0], 0.0, "L panel rolls right")
        self.assertLess( moments_l[1], 0.0, "L panel pitches down")
        self.assertLess( moments_l[2], 0.0, "L panel yaws left")

        self.assertEqual( moments_l[0], -moments_r[0], "Panels have opposite roll moments")
        self.assertEqual( moments_l[1], moments_r[1], "Panels have equal pitch moments")
        self.assertEqual( moments_l[2], -moments_r[2], "Panels have opposite yaw moments")



    # Case 2 - aircraft with right velocity component in body frame (nose yawed left)
    # Check forces and moments make sense
    def test_yawed_left(self):
        self.state.set_velocity((25.0, 1.0, 0.0))  # 25 m/s level flight

        panel = self.aircraft.wing[2] # outer between brakes and ailerons
        forces_r, moments_r = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,1.0)
        forces_l, moments_l = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,-1.0)

        print(f"Yawed left: Forces left {forces_l}, Forces right {forces_r}")

        mx = moments_l[0] + moments_r[0] # moments about x - roll
        my = moments_l[1] + moments_r[1] # moments about y - pitch
        mz = moments_l[2] + moments_r[2] # moments about z - yaw

        self.assertLess(mx, 0.0, "Yawed left, roll left from dihedral")
        self.assertLess(my, 0.0,  "Nose down Cm still when yawed")
        self.assertGreater(mz, 0.0, "Yawed left, differential drag yaws right")

    # Case 3 - aircraft with left velocity component in body frame(nose yawed right)
    def test_yawed_right(self):
        self.state.set_velocity((25.0, -1.0, 0.0))  # 25 m/s level flight

        panel = self.aircraft.wing[2] # outer between brakes and ailerons
        forces_r, moments_r = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,1.0)
        forces_l, moments_l = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,-1.0)

        print(f"Yawed right: Forces left {forces_l}, Forces right {forces_r}")

        mx = moments_l[0] + moments_r[0] # moments about x - roll
        my = moments_l[1] + moments_r[1] # moments about y - pitch
        mz = moments_l[2] + moments_r[2] # moments about z - yaw

        self.assertGreater(mx, 0.0, "Yawed right, roll right from dihedral")
        self.assertLess(my, 0.0,  "Nose down Cm still when yawed")
        self.assertLess(mz, 0.0, "Yawed right, differential drag yaws left")


    # Case 4 - aircraft with with right yaw rate
    # If nose to the right, tail will be going left, so fin should produce a right force to counteract
    def test_yawing_right(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, 0.1))  # Yaw rate to right

        panel = self.aircraft.wing[2] # outer between brakes and ailerons
        forces_r, moments_r = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,1.0)
        forces_l, moments_l = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,-1.0)

        print(f"Yawing right: Forces left {forces_l}, Forces right {forces_r}")

        mx = moments_l[0] + moments_r[0] # moments about x - roll
        my = moments_l[1] + moments_r[1] # moments about y - pitch
        mz = moments_l[2] + moments_r[2] # moments about z - yaw

        self.assertGreater(mx, 0.0, "Yawing right, roll right from differential speed")
        self.assertLess(my, 0.0,  "Nose down Cm still when yawing")
        self.assertLess(mz, 0.0, "Yaweding right, differential drag yaws left") # must oppose yaw rate


    # Case 5 - aircraft with with left yaw rate
    # If nose yawing to the left, tail will be going right, so fin should produce a left force to counteract
    def test_yawing_left(self):
        self.state.set_velocity((25.0, 0.0, 0.0))  # 25 m/s level flight
        self.state.set_angular_velocity((0.0, 0.0, -0.1))  # Yaw rate to left

        panel = self.aircraft.wing[2] # outer between brakes and ailerons
        forces_r, moments_r = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,1.0)
        forces_l, moments_l = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,-1.0)

        print(f"Yawing left: Forces left {forces_l}, Forces right {forces_r}")

        mx = moments_l[0] + moments_r[0] # moments about x - roll
        my = moments_l[1] + moments_r[1] # moments about y - pitch
        mz = moments_l[2] + moments_r[2] # moments about z - yaw

        self.assertLess(mx, 0.0, "Yawing left, roll left from differential speed")
        self.assertLess(my, 0.0,  "Nose down Cm still when yawing")
        self.assertGreater(mz, 0.0, "Yawing left, differential drag yaws right") # must oppose yaw rate

    # Case 6 - aircraft descending vertically with zero forward speed (w positive = down)
    def test_vertical_descent_zero_forward_speed(self):
        self.state.set_velocity((0.0, 0.0, 5.0))  # descending vertically at 5 m/s

        panel = self.aircraft.wing[2] # outer between brakes and ailerons
        forces_r, moments_r = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,1.0)
        forces_l, moments_l = panel.process(self.state, self.state.velocity(), self.aircraft.get_params(), self.world, self.controls,-1.0)

        print(f"Vertical descent: Forces left {forces_l}, Forces right {forces_r}")

        fx = forces_l[0] + forces_r[0]
        fy = forces_l[1] + forces_r[1]
        fz = forces_l[2] + forces_r[2]

        self.assertLess(abs(fx), 10.0, "Vertical descent with zero forward speed should produce low forward force")
        self.assertEqual(fy, 0.0, "Vertical descent with zero forward speed should produce no side force")
        self.assertLess(fz, 0.0, "Vertical descent with zero forward speed should produce upward (-ve) force from drag of flat plate facing into the flow")

        mx = moments_l[0] + moments_r[0] # moments about x - roll
        my = moments_l[1] + moments_r[1] # moments about y - pitch
        mz = moments_l[2] + moments_r[2] # moments about z - yaw

        self.assertEqual(mx, 0.0, "Vertical descent with zero forward speed should produce no roll moment")
        self.assertEqual(mz, 0.0, "Vertical descent with zero forward speed should produce no yaw moment")
 

if __name__ == '__main__':
    unittest.main()