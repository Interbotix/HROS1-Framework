/*
 *   Head.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _HEAD_H_
#define _HEAD_H_

#include <string.h>

#include "minIni.h"
#include "MotionModule.h"
#include "Point.h"

#define HEAD_SECTION    "Head Pan/Tilt"
#define INVALID_VALUE   -1024.0

namespace Robot
{
	class Head : public MotionModule
	{
		private:
			static Head* m_UniqueInstance;
			double m_LeftLimit;
			double m_RightLimit;
			double m_TopLimit;
			double m_BottomLimit;
			double m_Pan_Home;
			double m_Tilt_Home;
			double m_Pan_err;
			double m_Pan_err_diff;
			double m_Pan_p_gain;
			double m_Pan_d_gain;
			double m_Tilt_err;
			double m_Tilt_err_diff;
			double m_Tilt_p_gain;
			double m_Tilt_d_gain;
			double m_PanAngle;
			double m_TiltAngle;

			Head();
			void CheckLimit();

		public:
			static Head* GetInstance() { return m_UniqueInstance; }

			~Head();

			double m_TopLimit_soccer;
			double m_TopLimit_line_following;
			double m_TopLimit_robot_following;

			double m_LookPanRate;
			double m_LookTiltRate;
			double m_LookPanDirection;
			double m_LookTiltDirection;

			void Initialize();
			void Process();

			double GetTopLimitAngle()		{ return m_TopLimit; }
			double GetBottomLimitAngle()	{ return m_BottomLimit; }
			double GetRightLimitAngle()		{ return m_RightLimit; }
			double GetLeftLimitAngle()		{ return m_LeftLimit; }

			void SetTopLimitAngle(double d)		{ m_TopLimit = d; }
			void SetBottomLimitAngle(double d)	{ m_BottomLimit = d; }
			void SetRightLimitAngle(double d)		{ m_RightLimit = d; }
			void SetLeftLimitAngle(double d)		{ m_LeftLimit = d; }

			double GetPanAngle()		{ return m_PanAngle; }
			double GetTiltAngle()		{ return m_TiltAngle; }

			void MoveToHome();
			void MoveByAngle(double pan, double tilt);
			void MoveByAngleOffset(double pan, double tilt);
			void InitTracking();
			void MoveTracking(Point2D err); // For image processing
			void MoveTracking();
			void LookAround();

			void LoadINISettings(minIni* ini);
			void LoadINISettings(minIni* ini, const std::string &section);
			void SaveINISettings(minIni* ini);
			void SaveINISettings(minIni* ini, const std::string &section);
	};
}

#endif
