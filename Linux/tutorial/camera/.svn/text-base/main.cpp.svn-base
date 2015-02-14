/*
 * main.cpp
 *
 *  Created on: 2011. 1. 4.
 *      Author: robotis
 */

#include <unistd.h>
#include <libgen.h>

#include "Camera.h"
#include "mjpg_streamer.h"
#include "minIni.h"
#include "LinuxCamera.h"

#define INI_FILE_PATH       "config.ini"

void change_current_dir()
{
    char exepath[1024] = {0};
    if(readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        chdir(dirname(exepath));
}

int main(void)
{
    printf( "\n===== Camera Tutorial for DARwIn =====\n\n");

    change_current_dir();

    minIni* ini = new minIni(INI_FILE_PATH);

    LinuxCamera::GetInstance()->Initialize(0);
    LinuxCamera::GetInstance()->LoadINISettings(ini);

    mjpg_streamer* streamer = new mjpg_streamer(Camera::WIDTH, Camera::HEIGHT);

    while(1)
    {
        LinuxCamera::GetInstance()->CaptureFrame();
        streamer->send_image(LinuxCamera::GetInstance()->fbuffer->m_YUVFrame);
    }

    return 0;
}
