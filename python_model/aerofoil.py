"""Aerofoil class using natural cubic splines for CL, CD, CM vs Alpha (degrees).

The constructor reads a text file with a header and columns: Alpha, CL, CD, CM
Alpha in the file is expected in degrees. The public method `coeffs_at(alpha_rad)`
accepts alpha in radians and returns (CL, CD, CM) interpolated with cubic splines.
"""
from __future__ import annotations

from math import degrees, isclose
from typing import List, Tuple
from pathlib import Path
from bisect import bisect_right


class NaturalCubicSpline:
    """Simple natural cubic spline implementation for 1D data.

    Builds a natural cubic spline (second derivatives zero at endpoints).
    """

    def __init__(self, x: List[float], y: List[float]):
        if len(x) != len(y):
            raise ValueError("x and y must be arrays of equal length")
        # ensure floats
        pairs = sorted(((float(xx), float(yy)) for xx, yy in zip(x, y)), key=lambda p: p[0])
        self.x = [p[0] for p in pairs]
        self.y = [p[1] for p in pairs]
        self.n = len(self.x)
        if self.n < 2:
            raise ValueError("At least two data points required")

        self.h = [self.x[i + 1] - self.x[i] for i in range(self.n - 1)]
        self.M = self._compute_second_derivatives()

    def _compute_second_derivatives(self) -> np.ndarray:
        n = self.n
        if n == 2:
            return [0.0, 0.0]

        # build tridiagonal system for M[1..n-2]
        a = [0.0] * (n - 2)  # lower diag (a_1..a_{n-3})
        b = [0.0] * (n - 2)  # main diag
        c = [0.0] * (n - 2)  # upper diag
        rhs = [0.0] * (n - 2)
        for i in range(1, n - 1):
            idx = i - 1
            if i - 2 >= 0:
                a[idx] = self.h[i - 2]
            b[idx] = 2.0 * (self.h[i - 1] + self.h[i - 2])
            if i < n - 2 + 1:
                c[idx] = self.h[i - 1] if idx < n - 3 + 1 else 0.0
            rhs[idx] = 6.0 * (
                (self.y[i + 1] - self.y[i]) / self.h[i] - (self.y[i] - self.y[i - 1]) / self.h[i - 1]
            )

        # Solve tridiagonal system via Thomas algorithm
        # forward elimination
        for i in range(1, n - 2):
            m = a[i] / b[i - 1]
            b[i] = b[i] - m * c[i - 1]
            rhs[i] = rhs[i] - m * rhs[i - 1]

        sol = [0.0] * (n - 2)
        if b[-1] == 0:
            raise ValueError("Singular system while computing spline second derivatives")
        sol[-1] = rhs[-1] / b[-1]
        # back substitution
        for i in range(n - 4, -1, -1):
            sol[i] = (rhs[i] - c[i] * sol[i + 1]) / b[i]

        M = [0.0] * n
        for i in range(1, n - 1):
            M[i] = sol[i - 1]
        return M

    def __call__(self, xq: float) -> float:
        xq = float(xq)
        # clamp or locate interval using bisect
        if xq <= self.x[0]:
            i = 0
        elif xq >= self.x[-1]:
            i = self.n - 2
        else:
            i = bisect_right(self.x, xq) - 1

        dx = xq - self.x[i]
        h = self.h[i]
        y_i = self.y[i]
        y_ip1 = self.y[i + 1]
        M_i = self.M[i]
        M_ip1 = self.M[i + 1]
        term1 = (y_ip1 - y_i) / h - (h / 6.0) * (M_ip1 - M_i)
        term2 = (M_i / 2.0) + (dx * (M_ip1 - M_i) / (6.0 * h))
        return y_i + term1 * dx + term2 * dx * dx


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

        if len(alphas) < 2:
            raise ValueError(f"Not enough data points in {path}")

        # sort by alpha
        idx = sorted(range(len(alphas)), key=lambda i: alphas[i])
        alphas = [float(alphas[i]) for i in idx]
        cls = [float(cls[i]) for i in idx]
        cds = [float(cds[i]) for i in idx]
        cms = [float(cms[i]) for i in idx]

        # If data contains both 0 and 360 with same values, drop the final duplicate
        if isclose(alphas[0] % 360.0, 0.0) and isclose(alphas[-1] % 360.0, 0.0) and isclose(cls[0], cls[-1]) and isclose(cds[0], cds[-1]) and isclose(cms[0], cms[-1]):
            alphas = alphas[:-1]
            cls = cls[:-1]
            cds = cds[:-1]
            cms = cms[:-1]

        self.alphas = alphas
        self._cl_spline = NaturalCubicSpline(alphas, cls)
        self._cd_spline = NaturalCubicSpline(alphas, cds)
        self._cm_spline = NaturalCubicSpline(alphas, cms)

    def coefficients_at(self, alpha_rad: float) -> Tuple[float, float, float]:
        """Return (CL, CD, CM) for a given alpha in radians.

        Alpha is wrapped into [0, 360) degrees before interpolation.
        """
        deg = (degrees(alpha_rad) % 360.0)
        cl = float(self._cl_spline(deg))
        cd = float(self._cd_spline(deg))
        cm = float(self._cm_spline(deg))
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
