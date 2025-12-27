import unittest
from pathlib import Path
import math

from aerofoil import Aerofoil


DATA_FILE = Path(__file__).resolve().parent.parent / 'FX-60-126.txt'


class TestAerofoil(unittest.TestCase):
    def setUp(self):
        self.af = Aerofoil(DATA_FILE)

    def test_known_points(self):
        cl, cd, cm = self.af.coefficients_at(0.0)
        self.assertAlmostEqual(cl, 0.5101, places=6)
        self.assertAlmostEqual(cd, 0.00653, places=6)
        self.assertAlmostEqual(cm, -0.1163, places=6)

        # another known point from file (5 degrees)
        cl5, cd5, cm5 = self.af.coefficients_at(math.radians(5.0))
        self.assertAlmostEqual(cl5, 1.0602, places=4)

    def test_wrap_around(self):
        a0 = self.af.coefficients_at(0.0)
        a2pi = self.af.coefficients_at(2.0 * math.pi)
        for v0, v2 in zip(a0, a2pi):
            self.assertAlmostEqual(v0, v2, places=6)

    def test_negative_aoa(self):
        # -5 degrees should match 355 degrees
        neg = self.af.coefficients_at(math.radians(-5.0))
        pos = self.af.coefficients_at(math.radians(355.0))
        for vneg, vpos in zip(neg, pos):
            self.assertAlmostEqual(vneg, vpos, places=6)


if __name__ == '__main__':
    unittest.main()
