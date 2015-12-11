/*
 *   BallTracker.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _BALL_TRACKER_H_
#define _BALL_TRACKER_H_

#include <string.h>

#include "Point.h"
#include "minIni.h"
#include "ColorFinder.h"

namespace Robot
{
	class BallTracker
	{
		private:
			bool m_trackingBall;
			int NoBallCount;
			static const int NoBallMaxCount = 15;

		public:
			ColorFinder finder;
			Point2D     ball_position;
			bool	bMotionEnable;
			bool	bMasked;
			int	m_maskLeft;
			int m_maskRight;
			int m_maskTop;
			int	m_maskBottom;
			BallTracker();
			~BallTracker();

			void LoadINISettings(minIni* ini);
			void LoadINISettings(minIni* ini, const std::string &section);
			void SaveINISettings(minIni* ini);
			void SaveINISettings(minIni* ini, const std::string &section);

			void Process(Image* camImg);
			void Process(Point2D pos);

			bool isTracking() { return m_trackingBall; }
	};
}

#endif
