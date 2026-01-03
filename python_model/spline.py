
class Spline:

    """
    Docstring for Spline
    Direct translation of C++ Spline class from simulator
    (Based on NRIC spline / splint)
    """

    def __init__(self, x: list[float], y: list[float]):
 
        if len(x) != len(y):
            raise Exception("Spline:  X & Y lists must be same length")
 
        self.m_blBad = False
        self.xx = x   #     # array of x coordinates 
        self.yy = y   #     # array of y coordinates 
        self.yy2 = [0.0] * len(x) #     # 2nd deriv of y wrt x 
        self.npts:int = len(x)     #  # number of points on curve 
        self.klo = 0
        self.khi = 1
        
        Spline._spline( self.xx, self.yy, len(self.xx), self.yy2)


    #**********************************************************
    #* SPLINE calculates a second derivative vector for    *
    #* calculating natural splines.              *
    #**********************************************************
    @staticmethod
    def _spline( x: list[float], y: list[float], n: int, y2: list[float]):

        #int i, k;
        #T p, qn, sig, un, * u;

        u = [0.0] * (n-1) # new T[(size_t)n - 1];

        y2[0] = u[0] = 0.0;        # Set lower boundary for natural splines 

        # Tridiagonal decomposition loop 
        for i in range(1,(n-1)): #for (i = 1; i <= n - 2; i++)
            sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1])
            p = sig * y2[i - 1] + 2.0
            y2[i] = (sig - 1.0) / p
            u[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1])
            u[i] = (6.0 * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p
        

        qn = un = 0.0;         # Set upper boundary for natural splines 

        y2[n - 1] = (un - qn * u[n - 2]) / (qn * y2[n - 2] + 1.0)
        # Back substitution for tridiagonal algorithm 
        for k in range(n-2,-1,-1) : #for (k = n - 2; k >= 0; k--) {
            y2[k] = y2[k] * y2[k + 1] + u[k]
        

        #delete[] u;

        return



    #**********************************************************
    #* SPLINT is the spline interpolation routine that     *
    #* calculates y and dy/dx at a given x.          *
    #**********************************************************
    def _splint(self, x : float) -> tuple[float,float,float]: # y, dydx, d2ydx2)
    
        blOK = True
        #T y;
        #T h, b, a;

        klo = self.klo
        khi = self.khi

        # Generally successive calls to a given spline will be in the same or
        # neighbouring ranges.  Test for these before the full binary search.

        # Maybe moved to next sample.
        if (x >= self.xx[khi]):
            klo += 1
            khi += 1
        # or previous one?
        elif (x < self.xx[klo]):
            klo -= 1
            khi -= 1

        
        # If not in correct range now revert to doing a binary
        # search to find the correct values for klo and khi to 
        # bracket x.
        if x < self.xx[klo] or x >= self.xx[khi] : 
            klo = 0
            khi = self.npts - 1
            while khi - klo > 1 :
                k = (khi + klo) // 2
                if self.xx[k] > x :
                    khi = k
                else :
                    klo = k

        #assert(x >= xx[klo] && x < xx[khi]);

        h = self.xx[khi] - self.xx[klo]
        if h == 0.0:
            raise Exception("Invalid spline data")
        

        # Now calculate y and its derivatives
        a = (self.xx[khi] - x) / h
        b = (x - self.xx[klo]) / h
        y = a * self.yy[klo] + b * self.yy[khi] + ((a * a * a - a) * self.yy2[klo] + (b * b * b - b) * self.yy2[khi]) * (h * h) / 6.0
        dydx = (self.yy[khi] - self.yy[klo]) / h - ((3.0 * a * a - 1.0) * self.yy2[klo] - (3.0 * b * b - 1.0) * self.yy2[khi]) * h / 6.0
        # New improved d2ydx2 with added vitamins and possibly correctness:
        # Straight linear interpolation of yy2 between xx[klo] and xx[khi];
        d2ydx2 = a * self.yy2[klo] + b * self.yy2[khi] 

        self.klo = klo
        self.khi = khi

        return y, dydx, d2ydx2
    

    def point( self, x : float) -> float :
        y, _, _ = self._splint(x)
        return y
    
    
    def values(self, x : float) -> tuple[float,float,float]: # y, dydx, d2ydx2)
        return self._splint(x)


