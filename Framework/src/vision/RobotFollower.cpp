/*
 *   RobotFollower.cpp
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
#include "RobotFollower.h"
#include "MotionStatus.h"
#include "PS3Controller.h"
#include "Kinematics.h"


using namespace Robot;
RobotFollower* RobotFollower::m_UniqueInstance = new RobotFollower();

RobotFollower::RobotFollower()
{
	m_NoRobotMaxCount = 10;
	m_NoRobotCount = m_NoRobotMaxCount;
	m_RobotMaxCount = 10;
	m_RobotCount = 0;

	m_TopAngle = 2.0;
	m_RightAngle = -30.0;
	m_LeftAngle = 30.0;

	m_FollowMaxFBStep = 25.0;//35.0(good);//45.0(too high);//25.0(default);
	m_FollowMinFBStep = 3.0;//was 5.0
	m_FollowMaxRLTurn = 35.0;//40.0;
	m_FitFBStep = -2.0;//3.0;
	m_FitMaxRLTurn = 35.0;//40.0;
	m_UnitFBStep = 0.3;//1.0;
	m_UnitRLTurn = 1.0;//5.0;
	m_GoalWidthPercent = 60.0;
	m_GoalHeightPercent = 44.0;
	m_GoalFBStep = 0;
	m_GoalRLTurn = 0;
	m_FBStep = 0;
	m_RLTurn = 0;
	m_ScanStartTime = 0;
	m_HeadScanCount = 0;
	DEBUG_PRINT = false;
	m_Robot = 0;
	bFullAuto = false;
	bHeadAuto = false;
	bTracking = false;
	bScanning = false;
}

RobotFollower::~RobotFollower()
{
}

double RobotFollower::GetTime()
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

void RobotFollower::Process(BallTracker &tracker)
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
			m_Robot = 0;
			bTracking = false;
			//tracker.bMotionEnable = true;
			if (bScanning == true)
				{
					if (GetTime() - m_ScanStartTime > 21.6)
						bScanning = false;
					else
						{
							double t = GetTime() - m_ScanStartTime;
							double pht = 2 * M_PI * (t) / 0.9;
							double php = 2 * M_PI * (t) / 3.6;
							double pan = 57.295 * (60 * M_PI / 180 * dir * asin(sin(php)));
							double tilt = 57.295 * (13 * M_PI / 180 + 25 * M_PI / 180 * sin(pht));
							//printf("scanning %0.2f %0.2f t = %0.2f\n",pan,tilt,t);
							Head::GetInstance()->MoveByAngle(pan, tilt);
						}
				}
			else if (m_NoRobotCount > m_NoRobotMaxCount)
				{
					// can not find a Robot
					m_GoalFBStep = 0;
					m_GoalRLTurn = 0;
					if (DEBUG_PRINT == true)
						fprintf(stderr, "[NO ROBOT]");
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
					m_NoRobotCount++;
					if (DEBUG_PRINT == true)
						fprintf(stderr, "[NO ROBOT COUNTING(%d/%d)]", m_NoRobotCount, m_NoRobotMaxCount);
				}
		}
	else
		{
			m_NoRobotCount = 0;
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
					//printf("width = %0.1f\n",tracker.finder.m_width_percent);
					if (tilt <= (tilt_min + AXDXL::RATIO_VALUE2ANGLE) || (tracker.finder.m_width_percent > m_GoalWidthPercent || tracker.finder.m_height_percent > m_GoalHeightPercent))
						{
							if (tracker.ball_position.Y > m_TopAngle || (tracker.finder.m_width_percent > m_GoalWidthPercent || tracker.finder.m_height_percent > m_GoalHeightPercent))
								{
									m_GoalFBStep = 0;
									m_GoalRLTurn = 0;
									//stop
								}
							else
								{
									m_RobotCount = 0;
									m_Robot = 0;
									m_GoalFBStep = m_FitFBStep;
									m_GoalRLTurn = m_FitMaxRLTurn * pan_percent;
									if (DEBUG_PRINT == true)
										fprintf(stderr, "[FIT(P:%.2f T:%.2f>%.2f)]", pan, tracker.ball_position.Y, m_TopAngle);
								}
						}
					else
						{
							m_RobotCount = 0;
							m_Robot = 0;
							double f1, f2;
							f1 = (m_GoalWidthPercent - tracker.finder.m_width_percent) / m_GoalWidthPercent;
							f2 = (m_GoalHeightPercent - tracker.finder.m_height_percent) / m_GoalHeightPercent;
							f1 = f2;
							if (f1 > 1.0) f1 = 1.0;
							if (f1 < -1.0) f1 = -1.0;
							//if(f2 < f1) f1 = f2;
							m_GoalFBStep = m_FollowMaxFBStep * tilt_percent * f1;
							if (m_GoalFBStep < m_FollowMinFBStep)
								m_GoalFBStep = m_FollowMinFBStep;
							m_GoalRLTurn = m_FollowMaxRLTurn * pan_percent;
							if (tracker.finder.m_width_percent > m_GoalWidthPercent || tracker.finder.m_height_percent > m_GoalHeightPercent)
								{
									m_GoalFBStep = 0;
									m_GoalRLTurn = 0;
								}
							if (DEBUG_PRINT == true)
								fprintf(stderr, "[FOLLOW(P:%.2f T:%.2f>%.2f]", pan, tilt, tilt_min);
						}
				}
			else
				{
					m_RobotCount = 0;
					m_Robot = 0;
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
							if (m_RobotCount < m_RobotMaxCount)
								m_RobotCount++;
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
							m_RobotCount = 0;
							m_Robot = 0;
							Walking::GetInstance()->speedAdj = 0;
							Walking::GetInstance()->X_MOVE_AMPLITUDE = m_FBStep;
							Walking::GetInstance()->A_MOVE_AMPLITUDE = m_RLTurn;
							Walking::GetInstance()->Start();
						}
					else
						{
							//printf("width1 = %0.1f\n",tracker.finder.m_width_percent);
							if (m_FBStep < m_GoalFBStep)
								m_FBStep += m_UnitFBStep;
							else if (m_FBStep > m_GoalFBStep)
								m_FBStep = m_GoalFBStep;//m_FBStep -= m_UnitFBStep;
							if (tracker.finder.m_width_percent > m_GoalWidthPercent || tracker.finder.m_height_percent > m_GoalHeightPercent)
								{
									m_FBStep = 0;
									m_RLTurn = 0;
								}
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
