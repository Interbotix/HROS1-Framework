/*
 *   MotionStatus.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _MOTION_STATUS_H_
#define _MOTION_STATUS_H_

#include "JointData.h"


namespace Robot
{
	enum
	{
		BACKWARD    = -1,
		STANDUP     = 0,
		FORWARD     = 1
	};

	class MotionStatus
	{
		private:

		public:
			static const int FALLEN_F_LIMIT     = 440;
			static const int FALLEN_B_LIMIT     = 590;
			static const int FALLEN_MAX_COUNT   = 30;

			static JointData m_CurrentJoints;
			static double FB_GYRO;
			static double RL_GYRO;
			static int FB_ACCEL;
			static int RL_ACCEL;

			static double ANGLE_PITCH;  //!< pitch angle in radians
			static double ANGLE_ROLL;   //!< roll angle in radians


			static int BUTTON;
			static int FALLEN;
	};
}

#endif
