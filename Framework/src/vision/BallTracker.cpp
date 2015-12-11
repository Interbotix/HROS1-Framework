/*
 *   BallTracker.cpp
 *
 *   Author: ROBOTIS
 *
 */

#include <math.h>
#include "Head.h"
#include "Camera.h"
#include "ImgProcess.h"
#include "BallTracker.h"

using namespace Robot;


BallTracker::BallTracker() :
	finder(ColorFinder()),
	ball_position(Point2D(-1.0, -1.0)),
	bMotionEnable(true),
	bMasked(false),
	m_maskLeft(80),
	m_maskRight(240),
	m_maskTop(20),
	m_maskBottom(200)
{
	NoBallCount = 0;
}

BallTracker::~BallTracker()
{
}

void BallTracker::LoadINISettings(minIni* ini)
{
	finder.LoadINISettings(ini);
}

void BallTracker::LoadINISettings(minIni* ini, const std::string &section)
{
	finder.LoadINISettings(ini, section);
}

void BallTracker::SaveINISettings(minIni* ini)
{
	finder.SaveINISettings(ini);
}

void BallTracker::SaveINISettings(minIni* ini, const std::string &section)
{
	finder.SaveINISettings(ini, section);
}

void BallTracker::Process(Image* camImg)
{
	if (bMasked == true)
		{
			for (int y = 0; y < camImg->m_Height; y++)
				{
					for (int x = 0; x < camImg->m_Width; x++)
						{
							if (x < m_maskLeft || x > m_maskRight || y < m_maskTop || y > m_maskBottom)
								camImg->m_ImageData[camImg->y_table[y] + x] = 0;
						}
				}
		}
	Point2D pos = finder.GetPosition(camImg);
	if (pos.X < 0 || pos.Y < 0)
		{
			ball_position.X = -1;
			ball_position.Y = -1;
			if (NoBallCount < NoBallMaxCount)
				{
					Head::GetInstance()->MoveTracking();
					NoBallCount++;
				}
			else
				{
					Head::GetInstance()->InitTracking();
					Head::GetInstance()->LookAround();
					m_trackingBall = 0;
				}
		}
	else
		{
			m_trackingBall = 1;
			NoBallCount = 0;
			Point2D center = Point2D(camImg->m_Width / 2, camImg->m_Height / 2);
			Point2D offset = pos - center;
			offset *= -1; // Inverse X-axis, Y-axis
			offset.X *= (Camera::VIEW_H_ANGLE / (double)camImg->m_Width); // pixel per angle
			offset.Y *= (Camera::VIEW_V_ANGLE / (double)camImg->m_Height); // pixel per angle
			ball_position = offset;
			if (bMotionEnable == true)  Head::GetInstance()->MoveTracking(ball_position);
		}
}

double fitRange(double input, double range)
{
	if (input > range)return range;
	if (input < -range)return -range;
	return input;
}

void BallTracker::Process(Point2D pos)
{
	if (pos.X < 0 || pos.Y < 0)
		{
			ball_position.X = -1;
			ball_position.Y = -1;
			if (NoBallCount < NoBallMaxCount)
				{
					Head::GetInstance()->MoveTracking();
					NoBallCount++;
				}
			else
				{
					m_trackingBall = 0;
					Head::GetInstance()->InitTracking();
					Head::GetInstance()->LookAround();
					//Head::GetInstance()->MoveToHome();
				}
		}
	else
		{
			m_trackingBall = 1;
			NoBallCount = 0;
			Point2D center = Point2D((double)Camera::WIDTH / 2.0, (double)Camera::HEIGHT / 2.0);
			Point2D offset = pos - center;
			offset *= -1.0; // Inverse X-axis, Y-axis

			offset.X *= (Camera::VIEW_H_ANGLE / (double)Camera::WIDTH); // pixel per angle
			offset.Y *= (Camera::VIEW_V_ANGLE / (double)Camera::HEIGHT); // pixel per angle

			ball_position = offset;
			Head::GetInstance()->MoveTracking(ball_position);
		}
}