
#pragma once
#include <assert.h>
#include <float.h>
#include <math.h>
#include <exception>

class PointT;

template <class T>
class Spline
{
public:

	Spline(T* x, T* y, int n);
	~Spline();
	bool Bad() const {return m_blBad;}
	T point(T x);
	T point(T x, T& dydx, T& d2ydx2);

private:

	// Forbid copying
	Spline(const Spline& rhs) {}
	Spline& operator= (const Spline& rhs) {return *this;}
	
	void spline(T *x,T *y,int n, T *y2);
	T splint(T x, T& dydx, T& d2ydx2);

	bool m_blBad;
	T *xx;        /* array of x coordinates */
	T *yy;        /* array of y coordinates */
	T *yy2;       /* 2nd deriv of y wrt x */
	int npts;       /* number of points on curve */
	int klo;
	int khi;
};


/************************************************************/
/** SPLINE calculates a second derivative vector for    **/
/** calculating natural splines.              **/
/************************************************************/
template<class T>
void Spline<T>::spline(T* x, T* y, int n, T* y2)
{
	int i, k;
	T p, qn, sig, un, * u;

	u = new T[(size_t)n - 1];

	y2[0] = u[0] = 0.0f;        /* Set lower boundary for natural splines */

	/* Tridiagonal decomposition loop */
	for (i = 1; i <= n - 2; i++)
	{
		sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
		p = sig * y2[i - 1] + 2.0f;
		y2[i] = (sig - 1.0f) / p;
		u[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (y[i] - y[i - 1]) / (x[i] - x[i - 1]);
		u[i] = (6.0f * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
	}

	qn = un = 0.0f;         /* Set upper boundary for natural splines */

	y2[n - 1] = (un - qn * u[n - 2]) / (qn * y2[n - 2] + 1.0f);
	/* Back substitution for tridiagonal algorithm */
	for (k = n - 2; k >= 0; k--) {
		y2[k] = y2[k] * y2[k + 1] + u[k];
	}

	delete[] u;

	return;
}


/************************************************************/
/** SPLINT is the spline interpolation routine that     **/
/** calculates y and dy/dx at a given x.          **/
/************************************************************/
template<class T>
T Spline<T>::splint(T x, T& dydx, T& d2ydx2)
{
	bool blOK = true;
	T y;
	T h, b, a;

	// Maybe moved to next sample.
	if (x >= xx[khi]) {
		++klo;
		++khi;
	}
	// If not in correct range now revert to doing a binary
	// search to find the correct values for klo and khi to 
	// bracket x.
	if (x < xx[klo] || x >= xx[khi]) {
		klo = 0;
		khi = npts - 1;
		while (khi - klo > 1) {
			int k = (khi + klo) / 2;
			if (xx[k] > x)
				khi = k;
			else
				klo = k;
		}
	}
	assert(x >= xx[klo] && x < xx[khi]);

	h = xx[khi] - xx[klo];
	if (h == 0.0f) {
		throw std::exception("Invalid spline data");
	}

	// Now calculate y and its derivatives
	a = (xx[khi] - x) / h;
	b = (x - xx[klo]) / h;
	y = a * yy[klo] + b * yy[khi] + ((a * a * a - a) * yy2[klo] + (b * b * b - b) * yy2[khi]) * (h * h) / 6.0f;
	dydx = (yy[khi] - yy[klo]) / h -
		((3.0f * a * a - 1.0f) * yy2[klo] - (3.0f * b * b - 1.0f) * yy2[khi]) * h / 6.0f;
	// New improved d2ydx2 with added vitamins and possibly correctness:
	// Straight linear interpolation of yy2 between xx[klo] and xx[khi];
	d2ydx2 = a * yy2[klo] + b * yy2[khi] ;
	return y;
}

// Set up a spline to interpolate y = f(x).
// x and y are the coordinate arrays, n is the length of each array.
template<class T>
Spline<T>::Spline(T* x, T* y, int n)
	: xx(x)
	, yy(y)
	, npts(n)
	, klo(0) // assume spline started at the start
	, khi(1)
{
	yy2 = new T[n];
	spline(x, y, n, yy2);  /* calculate 2nd deriv array for y wrt x */
}

template<class T>
Spline<T>::~Spline()
{
	delete[] yy2;
}

template<class T>
T Spline<T>::point(T x)
{
	T dydx = 0;
	T d2ydx2 = 0;
	T y = splint(x, dydx, d2ydx2);
	return y;
}

template<class T>
T Spline<T>::point(T x, T& dydx, T& d2ydx2)
{
	T y = splint(x, dydx, d2ydx2);
	return y;
}


