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
#include <signal.h>

#include "Camera.h"
#include "mjpg_streamer.h"
#include "LinuxDARwIn.h"

#ifdef AXDXL_1024
#define MOTION_FILE_PATH    ((char *)"../../../Data/motion_1024.bin")
#else
#define MOTION_FILE_PATH    ((char *)"../../../Data/motion_4096.bin")
#endif
#define INI_FILE_PATH       ((char *)"../../../Data/config.ini")

#define M_INI   ((char *)"../../../Data/slow-walk.ini")
#define SCRIPT_FILE_PATH    "script.asc"

#define U2D_DEV_NAME0       "/dev/ttyUSB0"
#define U2D_DEV_NAME1       "/dev/ttyUSB1"

int isRunning = 1;
LinuxArbotixPro linux_arbotixpro(U2D_DEV_NAME0);
ArbotixPro arbotixpro(&linux_arbotixpro);

// Define the exit signal handler
void signal_callback_handler(int signum)
{
    //LinuxCamera::~LinuxCamera();
    printf("Exiting program; Caught signal %d\r\n", signum);
    isRunning = 0;
}

void change_current_dir()
{
    char exepath[1024] = {0};
    if (readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        chdir(dirname(exepath));
}

int main(void)
{
    //Register signal and signal handler
    signal(SIGINT, signal_callback_handler);

    printf( "\n===== Head tracking Tutorial for DARwIn =====\n\n");

    change_current_dir();

    minIni* ini = new minIni(INI_FILE_PATH);
    Image* rgb_ball = new Image(Camera::WIDTH, Camera::HEIGHT, Image::RGB_PIXEL_SIZE);

    LinuxCamera::GetInstance()->Initialize(0);
    LinuxCamera::GetInstance()->LoadINISettings(ini);

    mjpg_streamer* streamer = new mjpg_streamer(Camera::WIDTH, Camera::HEIGHT);

    ColorFinder* ball_finder = new ColorFinder();
    ball_finder->LoadINISettings(ini);
    httpd::ball_finder = ball_finder;

    BallTracker tracker = BallTracker();

//////////////////// Framework Initialize ////////////////////////////
    if (MotionManager::GetInstance()->Initialize(&arbotixpro) == false)
        {
            linux_arbotixpro.SetPortName(U2D_DEV_NAME1);
            if (MotionManager::GetInstance()->Initialize(&arbotixpro) == false)
                {
                    printf("Fail to initialize Motion Manager!\n");
                    return 0;
                }
        }

    Walking::GetInstance()->LoadINISettings(ini);
    usleep(100);
    MotionManager::GetInstance()->LoadINISettings(ini);

    MotionManager::GetInstance()->AddModule((MotionModule*)Action::GetInstance());
    MotionManager::GetInstance()->AddModule((MotionModule*)Head::GetInstance());
    MotionManager::GetInstance()->AddModule((MotionModule*)Walking::GetInstance());
    //MotionManager::GetInstance()->StartThread();
    //LinuxMotionTimer::Initialize(MotionManager::GetInstance());
    LinuxMotionTimer linuxMotionTimer;
    linuxMotionTimer.Initialize(MotionManager::GetInstance());
    linuxMotionTimer.Start();

    MotionStatus::m_CurrentJoints.SetEnableBodyWithoutHead(false);
    MotionManager::GetInstance()->SetEnable(true);
    /////////////////////////////////////////////////////////////////////

    Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);

    //Head::GetInstance()->m_Joint.SetPGain(JointData::ID_HEAD_PAN, 8);
    //Head::GetInstance()->m_Joint.SetPGain(JointData::ID_HEAD_TILT, 8);

    while (isRunning)
        {
            //usleep(10000);
            Point2D pos;
            LinuxCamera::GetInstance()->CaptureFrame();

            tracker.Process(ball_finder->GetPosition(LinuxCamera::GetInstance()->fbuffer->m_HSVFrame));

            rgb_ball = LinuxCamera::GetInstance()->fbuffer->m_RGBFrame;
            for (int i = 0; i < rgb_ball->m_NumberOfPixels; i++)
                {
                    if (ball_finder->m_result->m_ImageData[i] == 1)
                        {
                            rgb_ball->m_ImageData[i * rgb_ball->m_PixelSize + 0] = 255;
                            rgb_ball->m_ImageData[i * rgb_ball->m_PixelSize + 1] = 0;
                            rgb_ball->m_ImageData[i * rgb_ball->m_PixelSize + 2] = 0;
                        }
                }
            streamer->send_image(rgb_ball);
        }

    return 0;
}
