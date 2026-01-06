"""Aerofoil class using natural cubic splines for CL, CD, CM vs Alpha (degrees).

The constructor reads a text file with a header and columns: Alpha, CL, CD, CM
Alpha in the file is expected in degrees. The public method `coeffs_at(alpha_rad)`
accepts alpha in radians and returns (CL, CD, CM) interpolated with cubic splines.
"""
from __future__ import annotations

from math import degrees, isclose, radians
from typing import List, Tuple
from pathlib import Path
from bisect import bisect_right

from spline import Spline


class Aerofoil:
    """Reads aerofoil polar file and provides interpolated CL, CD, CM.

    The file should contain a header line with column names including 'Alpha', 'CL', 'CD', 'CM'.
    Alpha is expected in degrees in the file. The user supplies alpha in radians.
    """

    def __init__(self, path: str | Path):
        path = Path(path)
        if not path.exists():
            raise FileNotFoundError(path)

        alphas = []
        cls = []
        cds = []
        cms = []

        with path.open('r', encoding='utf-8', errors='replace') as fh:
            # skip any leading non-data lines until header found
            header_found = False
            for raw in fh:
                line = raw.strip()
                if not line:
                    continue
                parts = line.split()
                if not header_found:
                    # look for header containing 'Alpha' and 'CL'
                    low = [p.lower() for p in parts]
                    if 'alpha' in low and 'cl' in low:
                        header_found = True
                        continue
                    else:
                        # maybe filename or title line; skip
                        continue

                # now parse numeric rows
                try:
                    a = float(parts[0])
                    c_l = float(parts[1])
                    c_d = float(parts[2])
                    c_m = float(parts[3])
                except Exception:
                    # skip malformed lines
                    continue

                alphas.append(a)
                cls.append(c_l)
                cds.append(c_d)
                cms.append(c_m)

                #print(f'{a},{c_l},{c_d},{c_m}')

        if len(alphas) < 2:
            raise ValueError(f"Not enough data points in {path}")

        # sort by alpha
        idx = sorted(range(len(alphas)), key=lambda i: alphas[i])
        alphas = [float(alphas[i]) for i in idx]
        cls = [float(cls[i]) for i in idx]
        cds = [float(cds[i]) for i in idx]
        cms = [float(cms[i]) for i in idx]

        self.alphas = alphas

        self._cl_spline = Spline(alphas, cls)
        self._cd_spline = Spline(alphas, cds)
        self._cm_spline = Spline(alphas, cms)

    def coefficients_at(self, alpha_rad: float) -> Tuple[float, float, float]:
        """Return (CL, CD, CM) for a given alpha in radians.

        Alpha is wrapped into [0, 360) degrees before interpolation.
        """
        deg = (degrees(alpha_rad) % 360.0)
        cl = self._cl_spline.point(deg)
        cd = self._cd_spline.point(deg)
        cm = self._cm_spline.point(deg)
        return cl, cd, cm


if __name__ == '__main__':
    # quick demo using the local file if present
    import sys
    p = Path('FX-60-126.txt')
    if not p.exists():
        print('Place a polar file named FX-60-126.txt in this folder for demo')
        sys.exit(0)
    af = Aerofoil(p)
    cl, cd, cm = af.coefficients_at(0.0)
    print('alpha=0 rad -> CL,CD,CM =', cl, cd, cm)
    for deg in range(0,360):
        alpha = radians(deg)
        cl, cd, cm = af.coefficients_at(alpha)
        print(f'{deg}, {cl}, {cd}, {cm}')