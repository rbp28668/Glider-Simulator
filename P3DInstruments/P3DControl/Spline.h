
#pragma once

class PointT;

class Spline
{
public:

	Spline(float* x, float* y, int n);
	~Spline();
	bool Bad() const {return m_blBad;}
	float point(float x);
	float point(float x, float& dydx, float& d2ydx2);

private:

	// Forbid copying
	Spline(const Spline& rhs) {}
	Spline& operator= (const Spline& rhs) {return *this;}
	
	void spline(float *x,float *y,int n, float *y2);
	float splint(float x, float& dydx, float& d2ydx2);

	bool m_blBad;
	float *xx;        /* array of x coordinates */
	float *yy;        /* array of y coordinates */
	float *yy2;       /* 2nd deriv of y wrt x */
	int npts;       /* number of points on curve */
	int klo;
	int khi;
};


