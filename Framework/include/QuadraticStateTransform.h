#ifndef QUADRATICSTATETRANSFORM_H_
#define QUADRATICSTATETRANSFORM_H_
#include "MotionState.h"

class QuadraticStateTransform
{
	public:

		QuadraticStateTransform();
		~QuadraticStateTransform() {};

		void setA(double A);
		void setStartState(double x, double v);
		void setTargetState(double x, double v);

		MotionState evaluateAt(double t);
		double totalTime();

		double A;
		MotionState startState;
		MotionState targetState;

	private:
		void transformState(double a, double t, MotionState& st);
};

#endif // QUADRATICSTATETRANSFORM_H_
