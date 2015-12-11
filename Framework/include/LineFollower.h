/*
 *   LineFollower.h
 *
 *   Author: ROBOTIS, Farrell Robotics
 *
 */

#ifndef _LINE_FOLLOWER_H_
#define _LINE_FOLLOWER_H_

#include "Point.h"
#include "BallTracker.h"


namespace Robot
{

	class LineFollower
	{
		private:
			static LineFollower* m_UniqueInstance;
			int m_NoLineMaxCount;
			int m_NoLineCount;
			int m_LineMaxCount;
			int m_LineCount;
			int m_HeadScanCount;

			double m_MaxFBStep;
			double m_MaxRLStep;
			double m_MaxDirAngle;

			double m_TopAngle;
			double m_RightAngle;
			double m_LeftAngle;

			double m_FollowMaxFBStep;
			double m_FollowMinFBStep;
			double m_FollowMaxRLTurn;
			double m_FitFBStep;
			double m_FitMaxRLTurn;
			double m_UnitFBStep;
			double m_UnitRLTurn;

			double m_GoalFBStep;
			double m_GoalRLTurn;
			double m_FBStep;
			double m_RLTurn;
			double m_ScanStartTime;

			LineFollower();

		protected:

		public:
			bool DEBUG_PRINT;
			int OnLine;		// 0: No line 1:Left -1:Right
			bool bHeadAuto;
			bool bFullAuto;
			bool bTracking;
			bool bScanning;
			int m_Line;
			virtual ~LineFollower();

			static LineFollower* GetInstance() { return m_UniqueInstance; }
			void Process(BallTracker &tracker);
			double GetTime();
			//void SetAuto(bool t) { bFullAuto = true; return;}
			//bool GetAuto(void) { return bFullAuto;}
	};
}
#endif
