/*
 *   MotionModule.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _MOTION_MODULE_H_
#define _MOTION_MODULE_H_

#include "JointData.h"

namespace Robot
{
	class MotionModule
	{
	private:

	protected:

	public:
		JointData m_Joint;

		static const int TIME_UNIT = 8; //msec 

		virtual void Initialize() = 0;
		virtual void Process() = 0;
	};
}

#endif