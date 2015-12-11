#ifndef MOTIONSTATE_H_
#define MOTIONSTATE_H_

class MotionState
{
	public:
		MotionState();
		MotionState(double t, double x, double v);
		~MotionState() {};

		void set(double x, double v);

		double t;
		double x;
		double v;
		double a;
};

#endif /* MOTIONSTATE_H_ */
