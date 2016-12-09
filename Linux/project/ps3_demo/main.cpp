/*
* main.cpp
*
*  Created on: 2011. 1. 4.
*      Author: robotis
*/

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <libgen.h>

#include "mjpg_streamer.h"
#include "LinuxDARwIn.h"

#include "StatusCheck.h"

#ifdef AXDXL_1024
#define MOTION_FILE_PATH    ((char *)"../../../Data/motion_1024.bin")
#else
#define MOTION_FILE_PATH    ((char *)"../../../Data/motion_4096.bin")
#endif
#define INI_FILE_PATH       ((char *)"../../../Data/config_SMOOTH.ini")

#define M_INI   ((char *)"../../../Data/slow-walk.ini")
#define SCRIPT_FILE_PATH    "script.asc"

#define U2D_DEV_NAME0       "/dev/ttyUSB0"
#define U2D_DEV_NAME1       "/dev/ttyUSB1"

LinuxArbotixPro linux_arbotixpro(U2D_DEV_NAME0);
ArbotixPro arbotixpro(&linux_arbotixpro);
int GetCurrentPosition(ArbotixPro &arbotixpro);
////////////////////////////////////////////
Action::PAGE Page;
Action::STEP Step;
////////////////////////////////////////////
int change_current_dir()
{
    char exepath[1024] = {0};
    int status = 0;
    if (readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        status = chdir(dirname(exepath));
    return status;
}

int main(int argc, char *argv[])
{
    int trackerSel;
    change_current_dir();

    minIni* ini = new minIni(INI_FILE_PATH);
    minIni* ini1 = new minIni(M_INI);
    StatusCheck::m_ini = ini;
    StatusCheck::m_ini1 = ini1;

    LinuxJoy ljoy = LinuxJoy();     // create our joystick object

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
    /////////////////////////////////////////////////////////////////////
//  MotionManager::GetInstance()->LoadINISettings(ini);

    int firm_ver = 0, retry = 0;
    //important but allow a few retries
    while (arbotixpro.ReadByte(JointData::ID_HEAD_PAN, AXDXL::P_VERSION, &firm_ver, 0)  != ArbotixPro::SUCCESS)
    {
        fprintf(stderr, "Can't read firmware version from Dynamixel ID %d!! \n\n", JointData::ID_HEAD_PAN);
        retry++;
        if (retry >= 3) exit(1); // if we can't do it after 3 attempts its not going to work.
    }

    if (0 < firm_ver && firm_ver < 27)
    {
        Action::GetInstance()->LoadFile(MOTION_FILE_PATH);
    }
    else
        exit(0);

    //conversion! ////////////////
    /*
    Action::GetInstance()->LoadFile("../../../Data/motion.bin");
    int j,k,p,a;
    double f;
    for(k=0;k<Action::MAXNUM_PAGE;k++)
        {
        Action::GetInstance()->LoadPage(k, &Page);
        for(j=0;j<Action::MAXNUM_STEP;j++)
            {
            for(p=0;p<31;p++)
                {
                a = Page.step[j].position[p];
                if(a < 1024)
                    {
                    f = ((a-512)*10)/3+2048;
                    a = (int)f;
                    if(a<0) a =0;
                    if(a>4095) a = 4095;
                    Page.step[j].position[p] = a;
                    }
                }
            }
        Action::GetInstance()->SavePage(k, &Page);
        }
    exit(0);
    */
    //copy page ////////////////
    if (argc > 1 && strcmp(argv[1], "-copy") == 0)
    {
        printf("Page copy -- uses files motion_src.bin and motion_dest.bin\n");
        if (Action::GetInstance()->LoadFile((char *)"../../../Data/motion_src.bin") == false)
        {
            printf("Unable to open source file\n");
            exit(1);
        }
        int k;
        void *page1;

        page1 = malloc(sizeof(Robot::Action::PAGE));
        printf("Page to load:");
        if (scanf("%d", &k) != EOF)
        {
            if (Action::GetInstance()->LoadPage(k, (Robot::Action::PAGE *)page1) == false)
            {
                printf("Unable to load page %d\n", k);
                exit(1);
            }
            if (Action::GetInstance()->LoadFile((char *)"../../../Data/motion_dest.bin") == false)
            {
                printf("Unable to open destination file\n");
                exit(1);
            }
            if (Action::GetInstance()->SavePage(k, (Robot::Action::PAGE *)page1) == false)
            {
                printf("Unable to save page %d\n", k);
                exit(1);
            }
            printf("Completed successfully.\n");
            exit(0);
        }
    }
    /////////////////////////////

    Walking::GetInstance()->LoadINISettings(ini);
    MotionManager::GetInstance()->LoadINISettings(ini);

    Walking::GetInstance()->m_Joint.SetEnableBody(false);
    Head::GetInstance()->m_Joint.SetEnableBody(false);
    Action::GetInstance()->m_Joint.SetEnableBody(true);
    MotionManager::GetInstance()->SetEnable(true);




    // Start up our joystick. - It will also handle the cases where the joystick starts up after program
    ljoy.begin("/dev/input/js0");


    //determine current position
    StatusCheck::m_cur_mode = GetCurrentPosition(arbotixpro);
    //LinuxActionScript::PlayMP3("../../../Data/mp3/ready.mp3");
    if ((argc > 1 && strcmp(argv[1], "-off") == 0) || (StatusCheck::m_cur_mode == SITTING))
    {
        arbotixpro.DXLPowerOn(false);
        //for(int id=JointData::ID_R_SHOULDER_PITCH; id<JointData::NUMBER_OF_JOINTS; id++)
        //  arbotixpro.WriteByte(id, AXDXL::P_TORQUE_ENABLE, 0, 0);
    }
    else
    {
        Action::GetInstance()->Start(15);
        while (Action::GetInstance()->IsRunning()) usleep(8 * 1000);
    }

    if ( arbotixpro.WriteWord(ArbotixPro::ID_BROADCAST, AXDXL::P_MOVING_SPEED_L, 1023, 0) != ArbotixPro::SUCCESS )
    {
        printf( "Warning: TODO\r\n");
    }

    while (1)
    {
        StatusCheck::Check(ljoy, arbotixpro);
//        if(StatusCheck::m_is_started == 0)
//            continue;
    }

    return 0;
}

int GetCurrentPosition(ArbotixPro &arbotixpro)
{
    int m = Robot::READY, p, j, pos[31];
    int dMaxAngle1, dMaxAngle2, dMaxAngle3;
    double dAngle;
    int rl[6] = { JointData::ID_R_ANKLE_ROLL, JointData::ID_R_ANKLE_PITCH, JointData::ID_R_KNEE, JointData::ID_R_HIP_PITCH, JointData::ID_R_HIP_ROLL, JointData::ID_R_HIP_YAW };
    int ll[6] = { JointData::ID_L_ANKLE_ROLL, JointData::ID_L_ANKLE_PITCH, JointData::ID_L_KNEE, JointData::ID_L_HIP_PITCH, JointData::ID_L_HIP_ROLL, JointData::ID_L_HIP_YAW };

    for (p = 0; p < 31; p++)
    {
        pos[p]  = -1;
    }
    for (p = 0; p < 6; p++)
    {
        if (arbotixpro.ReadWord(rl[p], AXDXL::P_PRESENT_POSITION_L, &pos[rl[p]], 0) != ArbotixPro::SUCCESS)
        {
            printf("Failed to read position %d", rl[p]);
        }
        if (arbotixpro.ReadWord(ll[p], AXDXL::P_PRESENT_POSITION_L, &pos[ll[p]], 0) != ArbotixPro::SUCCESS)
        {
            printf("Failed to read position %d", ll[p]);
        }
    }
    // compare to a couple poses
    // first sitting - page 48
    Action::GetInstance()->LoadPage(48, &Page);
    j = Page.header.stepnum - 1;
    dMaxAngle1 = dMaxAngle2 = dMaxAngle3 = 0;
    for (p = 0; p < 6; p++)
    {
        dAngle = abs(AXDXL::Value2Angle(pos[rl[p]]) - AXDXL::Value2Angle(Page.step[j].position[rl[p]]));
        if (dAngle > dMaxAngle1)
            dMaxAngle1 = dAngle;
        dAngle = abs(AXDXL::Value2Angle(pos[ll[p]]) - AXDXL::Value2Angle(Page.step[j].position[ll[p]]));
        if (dAngle > dMaxAngle1)
            dMaxAngle1 = dAngle;
    }
    // squating - page 15
    Action::GetInstance()->LoadPage(15, &Page);
    j = Page.header.stepnum - 1;
    for (int p = 0; p < 6; p++)
    {
        dAngle = abs(AXDXL::Value2Angle(pos[rl[p]]) - AXDXL::Value2Angle(Page.step[j].position[rl[p]]));
        if (dAngle > dMaxAngle2)
            dMaxAngle2 = dAngle;
        dAngle = abs(AXDXL::Value2Angle(pos[ll[p]]) - AXDXL::Value2Angle(Page.step[j].position[ll[p]]));
        if (dAngle > dMaxAngle2)
            dMaxAngle2 = dAngle;
    }
    // walkready - page 9
    Action::GetInstance()->LoadPage(9, &Page);
    j = Page.header.stepnum - 1;
    for (int p = 0; p < 6; p++)
    {
        dAngle = abs(AXDXL::Value2Angle(pos[rl[p]]) - AXDXL::Value2Angle(Page.step[j].position[rl[p]]));
        if (dAngle > dMaxAngle3)
            dMaxAngle3 = dAngle;
        dAngle = abs(AXDXL::Value2Angle(pos[ll[p]]) - AXDXL::Value2Angle(Page.step[j].position[ll[p]]));
        if (dAngle > dMaxAngle3)
            dMaxAngle3 = dAngle;
    }
    if (dMaxAngle1 < 20 && dMaxAngle1 < dMaxAngle2 && dMaxAngle1 < dMaxAngle3)
        m = Robot::SITTING;
    if (dMaxAngle2 < 20 && dMaxAngle2 < dMaxAngle1 && dMaxAngle2 < dMaxAngle3)
        m = Robot::READY;
    if (dMaxAngle3 < 20 && dMaxAngle3 < dMaxAngle1 && dMaxAngle3 < dMaxAngle2)
        m = Robot::SOCCER;
    printf("Sitting = %d, Squating = %d, Standing = %d\n", dMaxAngle1, dMaxAngle2, dMaxAngle3);
    printf("Robot is %s\n", m == Robot::READY ? "Ready" : m == Robot::SOCCER ? "Soccer" : m == Robot::SITTING ? "Sitting" : "None");
    return m;
}
