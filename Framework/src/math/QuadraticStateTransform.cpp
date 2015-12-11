#include "QuadraticStateTransform.h"
#include <math.h>

inline double sgn(double a)
{
	if (a < 0)
		return -1.0;
	else
		return 1.0;
}

// Instantiates a keyframe player with A=1.
QuadraticStateTransform::QuadraticStateTransform()
{
	A = 1;
}

// Sets the acceleration limit to A.
void QuadraticStateTransform::setA(double A)
{
	this->A = A;
}

// Sets the start state to position x and velocity v.
void QuadraticStateTransform::setStartState(double x, double v)
{
	startState.x = x;
	startState.v = v;
}

// Sets the target state to position x and velocity v.
void QuadraticStateTransform::setTargetState(double x, double v)
{
	targetState.x = x;
	targetState.v = v;
}

// Returns time needed for the state transform from startState to targetState.
double QuadraticStateTransform::totalTime()
{
	double x0 = startState.x;
	double v0 = startState.v;
	double x1 = targetState.x;
	double v1 = targetState.v;
	double dx = fabs(x1 - x0);
	v0 = sgn(x1 - x0) * v0;
	v1 = sgn(x1 - x0) * v1;

	int sign = (sqrt(v0 * v0 + 2.0 * A * dx) < v1 or sqrt(v1 * v1 + 2.0 * A * dx) < v0) ? -1 : 1;
	double tt = (sqrt(2.0 * (v0 * v0 + v1 * v1) + sign * 4.0 * A * dx) - sign * (v0 + v1)) / A;

//	qDebug() << A << sign << tt << tt1 << tt2;

	return tt;
}

// Evaluates the state transform at a given time t and returns the calculated motion state.
MotionState QuadraticStateTransform::evaluateAt(double t)
{
	MotionState state;
	state = startState;

	double x0 = startState.x;
	double v0 = startState.v;
	double x1 = targetState.x;
	double v1 = targetState.v;
	double dx = fabs(x1 - x0);
	int dxsign = sgn(x1 - x0);
	v0 = dxsign * v0;
	v1 = dxsign * v1;

	int sign = (sqrt(v0 * v0 + 2.0 * A * dx) < v1 or sqrt(v1 * v1 + 2.0 * A * dx) < v0) ? -1 : 1;
	double tt = (sqrt(2.0 * (v0 * v0 + v1 * v1) + sign * 4.0 * A * dx) - sign * (v0 + v1)) / A;
	double tstar = (v1 - v0 + sign * A * tt) / (2 * sign * A);

	if (t < tstar)
		{
			transformState(dxsign * sign * A, t, state);
		}
	else
		{
			transformState(dxsign * sign * A, tstar, state);
			transformState(dxsign * -sign * A, t - tstar, state);
		}

	state.t = tt;

	return state;
}

void QuadraticStateTransform::transformState(double a, double t, MotionState& st)
{
	st.a = a;
	st.t += t;
	st.x += 0.5 * st.a * t * t + st.v * t;
	st.v += st.a * t;
}