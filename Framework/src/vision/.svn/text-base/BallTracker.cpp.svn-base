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
        ball_position(Point2D(-1.0, -1.0))
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
	Point2D pos = finder.GetPosition(camImg);
	if(pos.X < 0 || pos.Y < 0)
	{
		ball_position.X = -1;
		ball_position.Y = -1;
		if(NoBallCount < NoBallMaxCount)
		{
			Head::GetInstance()->MoveTracking();
			NoBallCount++;
		}
		else
			Head::GetInstance()->InitTracking();
	}
	else
	{
		NoBallCount = 0;
		Point2D center = Point2D(camImg->m_Width/2, camImg->m_Height/2);
		Point2D offset = pos - center;
		offset *= -1; // Inverse X-axis, Y-axis
		offset.X *= (Camera::VIEW_H_ANGLE / (double)camImg->m_Width); // pixel per angle
		offset.Y *= (Camera::VIEW_V_ANGLE / (double)camImg->m_Height); // pixel per angle
		ball_position = offset;
		Head::GetInstance()->MoveTracking(ball_position);
	}
}
