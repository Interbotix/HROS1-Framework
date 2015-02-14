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
		static const double CAMERA_DISTANCE = 50.00; //mm
		static const double EYE_TILT_OFFSET_ANGLE = 10.0; //degree
		static const double LEG_SIDE_OFFSET = 40.0; //mm
		static const double THIGH_LENGTH = 76.00; //mm
		static const double CALF_LENGTH = 76.00; //mm
		static const double ANKLE_LENGTH = 35.0; //mm
		static const double LEG_LENGTH = 187.0; //mm (THIGH_LENGTH + CALF_LENGTH + ANKLE_LENGTH)

		~Kinematics();

		static Kinematics* GetInstance()			{ return m_UniqueInstance; }
	};
}

#endif
