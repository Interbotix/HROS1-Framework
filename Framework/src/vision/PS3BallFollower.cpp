/*
 *   PS3BallFollower.cpp
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
#include "PS3BallFollower.h"
#include "MotionStatus.h"
#include "PS3Controller.h"


using namespace Robot;
PS3BallFollower* PS3BallFollower::m_UniqueInstance = new PS3BallFollower();
int m_HeadScanCount;

PS3BallFollower::PS3BallFollower()
{
	m_NoBallMaxCount = 10;
	m_NoBallCount = m_NoBallMaxCount;
	m_KickBallMaxCount = 10;
	m_KickBallCount = 0;

	m_KickTopAngle = -5.0;
	m_KickRightAngle = -30.0;
	m_KickLeftAngle = 30.0;

	m_FollowMaxFBStep = 35.0;//35.0(good);//45.0(too high);//25.0(default);
	m_FollowMinFBStep = 5.0;
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
	KickBall = 0;
	bFullAuto = false;
	bHeadAuto = false;
	bTracking = false;
	bScanning = false;
}

PS3BallFollower::~PS3BallFollower()
{
}

double PS3BallFollower::GetTime()
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

void PS3BallFollower::Process(Point2D ball_pos)
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
			ball_pos = pos;
		}

	if (ball_pos.X == -1.0 || ball_pos.Y == -1.0)
		{
			KickBall = 0;
			bTracking = false;
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
			else if (m_NoBallCount > m_NoBallMaxCount)
				{
					// can not find a ball
					m_GoalFBStep = 0;
					m_GoalRLTurn = 0;
					if (DEBUG_PRINT == true)
						fprintf(stderr, "[NO BALL]");
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
					m_NoBallCount++;
					if (DEBUG_PRINT == true)
						fprintf(stderr, "[NO BALL COUNTING(%d/%d)]", m_NoBallCount, m_NoBallMaxCount);
				}
		}
	else
		{
			m_NoBallCount = 0;
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

			if (pan > m_KickRightAngle && pan < m_KickLeftAngle)
				{
					if (tilt <= (tilt_min + AXDXL::RATIO_VALUE2ANGLE))
						{
							if (ball_pos.Y < m_KickTopAngle)
								{
									m_GoalFBStep = 0;
									m_GoalRLTurn = 0;

									if (m_KickBallCount >= m_KickBallMaxCount)
										{
											m_FBStep = 0;
											m_RLTurn = 0;
											if (DEBUG_PRINT == true)
												fprintf(stderr, "[KICK]");

											if (pan > 0)
												{
													KickBall = 1; // Left
													if (DEBUG_PRINT == true)
														fprintf(stderr, " Left");
												}
											else
												{
													KickBall = -1; // Right
													if (DEBUG_PRINT == true)
														fprintf(stderr, " Right");
												}
										}
									else
										{
											KickBall = 0;
											if (DEBUG_PRINT == true)
												fprintf(stderr, "[KICK COUNTING(%d/%d)]", m_KickBallCount, m_KickBallMaxCount);
										}
								}
							else
								{
									m_KickBallCount = 0;
									KickBall = 0;
									m_GoalFBStep = m_FitFBStep;
									m_GoalRLTurn = m_FitMaxRLTurn * pan_percent;
									if (DEBUG_PRINT == true)
										fprintf(stderr, "[FIT(P:%.2f T:%.2f>%.2f)]", pan, ball_pos.Y, m_KickTopAngle);
								}
						}
					else
						{
							m_KickBallCount = 0;
							KickBall = 0;
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
					m_KickBallCount = 0;
					KickBall = 0;
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
							if (m_KickBallCount < m_KickBallMaxCount)
								m_KickBallCount++;
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
							m_KickBallCount = 0;
							KickBall = 0;
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
