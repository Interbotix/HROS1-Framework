/*
 *   BallFollower.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _PS3BALL_FOLLOWER_H_
#define _PS3BALL_FOLLOWER_H_

#include "Point.h"
#include "BallTracker.h"


namespace Robot
{

	class PS3BallFollower
	{
		private:
			static PS3BallFollower* m_UniqueInstance;
			int m_NoBallMaxCount;
			int m_NoBallCount;
			int m_KickBallMaxCount;
			int m_KickBallCount;

			double m_MaxFBStep;
			double m_MaxRLStep;
			double m_MaxDirAngle;

			double m_KickTopAngle;
			double m_KickRightAngle;
			double m_KickLeftAngle;

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

			PS3BallFollower();

		protected:

		public:
			bool DEBUG_PRINT;
			int KickBall;		// 0: No ball 1:Left -1:Right
			bool bHeadAuto;
			bool bFullAuto;
			bool bTracking;
			bool bScanning;

			virtual ~PS3BallFollower();

			static PS3BallFollower* GetInstance() { return m_UniqueInstance; }
			void Process(Point2D ball_pos);
			double GetTime();
			//void SetAuto(bool t) { bFullAuto = true; return;}
			//bool GetAuto(void) { return bFullAuto;}
	};
}
#endif
