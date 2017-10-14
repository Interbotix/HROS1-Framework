/*
 *   Camera.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _CAMERA_H_
#define _CAMERA_H_


namespace Robot
{
	class Camera
	{
		public:
			static constexpr double VIEW_V_ANGLE = 33.0; //degree
			static constexpr double VIEW_H_ANGLE = 45.0; //degree

			static const int WIDTH  = 320;
			static const int HEIGHT = 240;
	};
}

#endif
