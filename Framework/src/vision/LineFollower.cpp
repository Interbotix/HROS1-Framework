/*
 *   LineFollower.cpp
 *
 *   Author: ROBOTIS, Farrell Robotics
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include "ImgProcess.h"
#include "AXDXL.h"
#include "Head.h"
#include "Action.h"
#include "Walking.h"
#include "LineFollower.h"
#include "MotionStatus.h"
#include "PS3Controller.h"
#include "Kinematics.h"


using namespace Robot;
LineFollower* LineFollower::m_UniqueInstance = new LineFollower();

LineFollower::LineFollower()
{
	m_NoLineMaxCount = 10;
	m_NoLineCount = m_NoLineMaxCount;
	m_LineMaxCount = 10;
	m_LineCount = 0;

	m_TopAngle = -5.0;
	m_RightAngle = -20.0;
	m_LeftAngle = 20.0;

	m_FollowMaxFBStep = 25.0;//35.0(good);//45.0(too high);//25.0(default);
	m_FollowMinFBStep = 3.0;//was 5.0
	m_FollowMaxRLTurn = 35.0;//40.0;
	m_FitFBStep = 3.0;
	m_FitMaxRLTurn = 35.0;//40.0;
	m_UnitFBStep = 0.3;//1.0;
	m_UnitRLTurn = 1.0;//5.0;

	m_GoalFBStep = 0;
	m_GoalRLTurn = 0;
	m_FBStep = 0;
	m_RLTurn = 0;
	m_ScanStartTime = 0;
	m_HeadScanCount = 0;
	DEBUG_PRINT = false;
	m_Line = 0;
	bFullAuto = false;
	bHeadAuto = false;
	bTracking = false;
	bScanning = false;
}

LineFollower::~LineFollower()
{
}

double LineFollower::GetTime()
{
	struct timeval tvs;
	time_t curtime1;
	double ts;
	char buffer[30];

	gettimeofday(&tvs, NULL);
	curtime1 = tvs.tv_sec;
	strftime(buffer, 30, "%s.", localtime(&curtime1));
	ts = atof(buffer) + (double)(tvs.tv_usec) / 1e6;
	return ts;
}

void LineFollower::Process(BallTracker &tracker)
{
	int dir = 1;
	if (DEBUG_PRINT == true)
		fprintf(stderr, "\r                                                                               \r");

	if (bHeadAuto == false)
		{
			double pan, tilt;
			pan = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
			tilt = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_TILT);
			Point2D pos = Point2D(pan, tilt);
			tracker.ball_position = pos;
		}

	if (tracker.ball_position.X == -1.0 || tracker.ball_position.Y == -1.0)
		{
			m_Line = 0;
			bTracking = false;
			//tracker.bMotionEnable = true;
			if (bScanning == true)
				{
					if (GetTime() - m_ScanStartTime > 10.8)
						bScanning = false;
					else
						{
							double t = GetTime() - m_ScanStartTime;
							double php = 2 * M_PI * (t) / 0.9;
							double phy = 2 * M_PI * (t) / 3.6;
							double yaw = 57.295 * (60 * M_PI / 180 * dir * asin(sin(phy)));
							double pitch = 57.295 * (10 * M_PI / 180 + 20 * M_PI / 180 * sin(php));
							//printf("scanning %0.2f %0.2f t = %0.2f\n",yaw,pitch,t);
							Head::GetInstance()->MoveByAngle(yaw, pitch);
						}
				}
			else if (m_NoLineCount > m_NoLineMaxCount)
				{
					// can not find a Line
					m_GoalFBStep = 0;
					m_GoalRLTurn = 0;
					if (DEBUG_PRINT == true)
						fprintf(stderr, "[NO LINE]");
					if (m_HeadScanCount < 1)
						{
							bScanning = true;
							dir = rand() % 3 - 1;
							m_HeadScanCount++;
							m_ScanStartTime = GetTime();
						}
					else
						Head::GetInstance()->MoveToHome();
				}
			else
				{
					m_NoLineCount++;
					if (DEBUG_PRINT == true)
						fprintf(stderr, "[NO LINE COUNTING(%d/%d)]", m_NoLineCount, m_NoLineMaxCount);
				}
		}
	else
		{
			m_NoLineCount = 0;
			m_HeadScanCount = 0;
			bTracking = true;
			bScanning = false;
			double pan = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
			double pan_range = Head::GetInstance()->GetLeftLimitAngle();
			double pan_percent = pan / pan_range;

			double tilt = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_TILT);
			double tilt_min = Head::GetInstance()->GetBottomLimitAngle();
			double tilt_range = Head::GetInstance()->GetTopLimitAngle() - tilt_min;
			double tilt_percent = (tilt - tilt_min) / tilt_range;
			if (tilt_percent < 0)
				tilt_percent = -tilt_percent;

			if (pan > m_RightAngle && pan < m_LeftAngle)
				{
					//tracker.bMotionEnable = false;
					//Head::GetInstance()->MoveByAngle();
					if (tilt <= (tilt_min + AXDXL::RATIO_VALUE2ANGLE))
						{
							if (tracker.ball_position.Y < m_TopAngle)
								{
									m_GoalFBStep = 0;
									m_GoalRLTurn = 0;
									//stop
								}
							else
								{
									m_LineCount = 0;
									m_Line = 0;
									m_GoalFBStep = m_FitFBStep;
									m_GoalRLTurn = m_FitMaxRLTurn * pan_percent;
									if (DEBUG_PRINT == true)
										fprintf(stderr, "[FIT(P:%.2f T:%.2f>%.2f)]", pan, tracker.ball_position.Y, m_TopAngle);
								}
						}
					else
						{
							m_LineCount = 0;
							m_Line = 0;
							m_GoalFBStep = m_FollowMaxFBStep * tilt_percent;
							if (m_GoalFBStep < m_FollowMinFBStep)
								m_GoalFBStep = m_FollowMinFBStep;
							m_GoalRLTurn = m_FollowMaxRLTurn * pan_percent;
							if (DEBUG_PRINT == true)
								fprintf(stderr, "[FOLLOW(P:%.2f T:%.2f>%.2f]", pan, tilt, tilt_min);
						}
				}
			else
				{
					m_LineCount = 0;
					m_Line = 0;
					m_GoalFBStep = 0;
					m_GoalRLTurn = m_FollowMaxRLTurn * pan_percent;
					if (DEBUG_PRINT == true)
						fprintf(stderr, "[FOLLOW(P:%.2f T:%.2f>%.2f]", pan, tilt, tilt_min);
				}
		}

	if (bFullAuto == true)
		{
			if (m_GoalFBStep == 0 && m_GoalRLTurn == 0 && m_FBStep == 0 && m_RLTurn == 0)
				{
					if (Walking::GetInstance()->IsRunning() == true)
						Walking::GetInstance()->Stop();
					else
						{
							if (m_LineCount < m_LineMaxCount)
								m_LineCount++;
						}

					if (DEBUG_PRINT == true)
						fprintf(stderr, " STOP");
				}
			else
				{
					if (DEBUG_PRINT == true)
						fprintf(stderr, " START");

					if (Walking::GetInstance()->IsRunning() == false)
						{
							m_FBStep = 0;
							m_RLTurn = 0;
							m_LineCount = 0;
							m_Line = 0;
							Walking::GetInstance()->speedAdj = 0;
							Walking::GetInstance()->X_MOVE_AMPLITUDE = m_FBStep;
							Walking::GetInstance()->A_MOVE_AMPLITUDE = m_RLTurn;
							Walking::GetInstance()->Start();
						}
					else
						{
							if (m_FBStep < m_GoalFBStep)
								m_FBStep += m_UnitFBStep;
							else if (m_FBStep > m_GoalFBStep)
								m_FBStep = m_GoalFBStep;//m_FBStep -= m_UnitFBStep;
							Walking::GetInstance()->X_MOVE_AMPLITUDE = m_FBStep;

							if (m_RLTurn < m_GoalRLTurn)
								m_RLTurn += m_UnitRLTurn;
							else if (m_RLTurn > m_GoalRLTurn)
								m_RLTurn -= m_UnitRLTurn;
							Walking::GetInstance()->A_MOVE_AMPLITUDE = m_RLTurn;
							/*
							if(m_FBStep>30)
								Walking::GetInstance()->HIP_PITCH_OFFSET = 56 + 4*((float)(m_FBStep-30))/15.0;
							else
								Walking::GetInstance()->HIP_PITCH_OFFSET = 56;
							*/
							if (DEBUG_PRINT == true)
								fprintf(stderr, " (FB:%.1f RL:%.1f)", m_FBStep, m_RLTurn);
						}
				}
		}
}
