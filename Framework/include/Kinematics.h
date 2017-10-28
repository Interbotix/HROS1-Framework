/*
 *   Kinematics.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _KINEMATICS_H_
#define _KINEMATICS_H_

#include "Matrix.h"
#include "JointData.h"

namespace Robot
{
	class Kinematics
	{
		private:
			static Kinematics* m_UniqueInstance;
			Kinematics();

		protected:

		public:
			static constexpr double CAMERA_DISTANCE = 50.00; //mm
			static constexpr double EYE_TILT_OFFSET_ANGLE = 10.0; //degree
			static constexpr double LEG_SIDE_OFFSET = 39.0; //mm
			static constexpr double THIGH_LENGTH = 77.00; //mm
			static constexpr double CALF_LENGTH = 73.00; //mm
			static constexpr double ANKLE_LENGTH = 31.6; //mm
			static constexpr double LEG_LENGTH = 181.6; //mm (THIGH_LENGTH + CALF_LENGTH + ANKLE_LENGTH)

			~Kinematics();

			static Kinematics* GetInstance()			{ return m_UniqueInstance; }
	};
}

#endif
