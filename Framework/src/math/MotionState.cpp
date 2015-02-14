#include "MotionState.h"

MotionState::MotionState()
{
	t = 0;
	x = 0;
	v = 0;
	a = 0;
}

MotionState::MotionState(double t, double x, double v)
{
	this->t = t;
	this->x = x;
	this->v = v;
	a = 0;
}

void MotionState::set(double x, double v)
{
	this->x = x;
	this->v = v;
	a = 0;
}

