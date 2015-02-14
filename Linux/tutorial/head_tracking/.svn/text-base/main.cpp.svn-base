/*
 * main.cpp
 *
 *  Created on: 2011. 1. 4.
 *      Author: robotis
 */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>

#include "Camera.h"
#include "mjpg_streamer.h"
#include "LinuxDARwIn.h"

#define INI_FILE_PATH       "config.ini"

#define U2D_DEV_NAME        "/dev/ttyUSB0"

void change_current_dir()
{
    char exepath[1024] = {0};
    if(readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        chdir(dirname(exepath));
}

int main(void)
{
    printf( "\n===== Head tracking Tutorial for DARwIn =====\n\n");

    change_current_dir();

    minIni* ini = new minIni(INI_FILE_PATH);
    Image* rgb_ball = new Image(Camera::WIDTH, Camera::HEIGHT, Image::RGB_PIXEL_SIZE);

    LinuxCamera::GetInstance()->Initialize(0);
    LinuxCamera::GetInstance()->LoadINISettings(ini);

    mjpg_streamer* streamer = new mjpg_streamer(Camera::WIDTH, Camera::HEIGHT);

    BallTracker tracker = BallTracker();
    tracker.LoadINISettings(ini);
    httpd::ball_finder = &tracker.finder;

	//////////////////// Framework Initialize ////////////////////////////
	LinuxCM730 linux_cm730(U2D_DEV_NAME);
	CM730 cm730(&linux_cm730);
	if(MotionManager::GetInstance()->Initialize(&cm730) == false)
	{
		printf("Fail to initialize Motion Manager!\n");
			return 0;
	}
	MotionManager::GetInstance()->AddModule((MotionModule*)Head::GetInstance());	
	LinuxMotionTimer::Initialize(MotionManager::GetInstance());

	MotionStatus::m_CurrentJoints.SetEnableBodyWithoutHead(false);
	MotionManager::GetInstance()->SetEnable(true);
	/////////////////////////////////////////////////////////////////////

	Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);

	Head::GetInstance()->m_Joint.SetPGain(JointData::ID_HEAD_PAN, 8);
	Head::GetInstance()->m_Joint.SetPGain(JointData::ID_HEAD_TILT, 8);

    while(1)
    {
        Point2D pos;
        LinuxCamera::GetInstance()->CaptureFrame();	

        tracker.Process(LinuxCamera::GetInstance()->fbuffer->m_HSVFrame);

		rgb_ball = LinuxCamera::GetInstance()->fbuffer->m_RGBFrame;
        for(int i = 0; i < rgb_ball->m_NumberOfPixels; i++)
        {
            if(tracker.finder.m_result->m_ImageData[i] == 1)
            {
                rgb_ball->m_ImageData[i*rgb_ball->m_PixelSize + 0] = 255;
                rgb_ball->m_ImageData[i*rgb_ball->m_PixelSize + 1] = 0;
                rgb_ball->m_ImageData[i*rgb_ball->m_PixelSize + 2] = 0;
            }
        }
        streamer->send_image(rgb_ball);
    }

    return 0;
}
