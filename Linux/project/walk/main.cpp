#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <term.h>
#include <ncurses.h>
#include <signal.h>
#include <libgen.h>
#include "cmd_process.h"
#include "mjpg_streamer.h"
#include <iostream>

#define MOTION_FILE_PATH    "../../../Data/motion_4096.bin"
#define INI_FILE_PATH       "../../../Data/config.ini"

using namespace std;
using namespace Robot;

LinuxMotionTimer linuxMotionTimer;
LinuxArbotixPro linux_arbotixpro("/dev/ttyUSB0");
ArbotixPro arbotixpro(&linux_arbotixpro);
minIni* ini;

void walk(int direction, int second){
    printf("walking...");
    Walking::GetInstance()->LoadINISettings(ini);
    linuxMotionTimer.Start();
    MotionManager::GetInstance()->AddModule((MotionModule*)Walking::GetInstance());
    MotionManager::GetInstance()->SetEnable(true);
    MotionManager::GetInstance()->ResetGyroCalibration();
    Walking::GetInstance()->X_OFFSET = direction * -2;
    Walking::GetInstance()->A_MOVE_AMPLITUDE = 0;
    Walking::GetInstance()->X_MOVE_AMPLITUDE = direction * 10;
    Walking::GetInstance()->Start();
    usleep(1000000 * second);
    Walking::GetInstance()->Stop();
    Walking::GetInstance()->X_MOVE_AMPLITUDE = 0;
    Walking::GetInstance()->Y_MOVE_AMPLITUDE = 0;
    Walking::GetInstance()->A_MOVE_AMPLITUDE = 0;
    usleep(1000000);
    MotionManager::GetInstance()->SetEnable(false);
    MotionManager::GetInstance()->RemoveModule((MotionModule*)Walking::GetInstance());
    linuxMotionTimer.Stop();
    printf(" done\n");
}

void turn(int direction, int degree){
    printf("turning\n");
    int turn_degree;
    if(degree < 90)
        turn_degree = 23;
    else
        turn_degree = 45;

    int sec = degree/(turn_degree*2.0) * 4.0;

    Walking::GetInstance()->LoadINISettings(ini);
    linuxMotionTimer.Start();
    MotionManager::GetInstance()->AddModule((MotionModule*)Walking::GetInstance());
    MotionManager::GetInstance()->SetEnable(true);
    MotionManager::GetInstance()->ResetGyroCalibration();
    printf("turning...\n");
    Walking::GetInstance()->X_OFFSET = -4;
    Walking::GetInstance()->Y_OFFSET += 5;
    Walking::GetInstance()->A_MOVE_AMPLITUDE = turn_degree * direction;
    Walking::GetInstance()->X_MOVE_AMPLITUDE = 0;
    Walking::GetInstance()->Start();
    usleep(1000000 * sec);
    Walking::GetInstance()->Stop();
    Walking::GetInstance()->X_MOVE_AMPLITUDE = 0;
    Walking::GetInstance()->Y_MOVE_AMPLITUDE = 0;
    Walking::GetInstance()->A_MOVE_AMPLITUDE = 0;
    usleep(1000000);
    MotionManager::GetInstance()->SetEnable(false);
    MotionManager::GetInstance()->RemoveModule((MotionModule*)Walking::GetInstance());
    linuxMotionTimer.Stop();
    printf(" Done\n");
}

void motion(int page_num){

    printf("playing ");
    MotionManager::GetInstance()->LoadINISettings(ini);
    MotionManager::GetInstance()->SetEnable(false);
    MotionManager::GetInstance()->AddModule((MotionModule*)Action::GetInstance());
    linuxMotionTimer.Stop();
    PlayCmd(&arbotixpro, page_num);
    MotionManager::GetInstance()->RemoveModule((MotionModule*)Action::GetInstance());
}

void change_current_dir()
{
    char exepath[1024] = {0};
    if (readlink("/proc/self/exe", exepath, sizeof(exepath)) != -1)
        chdir(dirname(exepath));
}

void sighandler(int sig)
{
    struct termios term;
    tcgetattr( STDIN_FILENO, &term );
    term.c_lflag |= ICANON | ECHO;
    tcsetattr( STDIN_FILENO, TCSANOW, &term );

    exit(0);
}

int main(int argc, char *argv[])
{
    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGQUIT, &sighandler);
    signal(SIGINT, &sighandler);


//    mjpg_streamer * streamer = new mjpg_streamer(0, 0);
    /*
    if (argc == 2)
        ini = new minIni(argv[1]);
    else

    */

    ini = new minIni(INI_FILE_PATH);
    change_current_dir();


    if(Action::GetInstance()->LoadFile(MOTION_FILE_PATH)) printf("Motions Loaded\n");
    //mjpg_streamer* streamer = new mjpg_streamer(0, 0);
    //httpd::ini = ini;

    //////////////////// Framework Initialize ////////////////////////////
    if (MotionManager::GetInstance()->Initialize(&arbotixpro) == false)
        {
            printf("Fail to initialize Motion Manager!\n");
            return 0;
        }


    bool on = TRUE;
    int  input1;
    int  input2;
    int  sec = 0;
    int  sign;
    int  degree;
    int  prev_page = 0;
//Walking::GetInstance()->LoadINISettings(ini);

    while(on){

	system("clear");
	cout << "******************** Walking and Motion Manager ********************\n\n";
	cout << "    Input Format: 2 arguments\n";
	cout << "    The second argument:\n";
	cout << "        1. walk:   number of seconds (+/forward -/backward)\n";
	cout << "        2. turn:   degree (+/turn left -/turn right)\n";
	cout << "        3. motion: page_number\n\n";
        cout << "    Examples:\n";
	cout << "        1   5 for walking forward 5 seconds\n";
        cout << "        2 -45 for turning right by 45 degrees\n";
        cout << "        3   4 for playing page #4\n\n";
	cout << "    Input: ";

        cin >> input1 >> input2;
	linuxMotionTimer.Initialize(MotionManager::GetInstance());
	cout << "    Status: ";
        switch(input1){
            case 1:
		if(prev_page != 8){
			prev_page = 8;
			motion(8);
		}
		if(input2 < 0)
			sign = -1;
		else
			sign = 1;

                sec = sign * input2;
		walk(sign, sec);

                break;

	    case 2:
		if(prev_page != 8){
			prev_page = 8;
			motion(8);
		}
		if(input2 < 0)
			sign = -1;
		else
			sign = 1;

		if(input2 * sign < 90)
			degree = 23;
		else
			degree = 45;

		turn(sign, sign * input2);
	        break;

            case 3:
		prev_page = input2;
		motion(input2);
		break;

            default:
		printf("invalid input\n");
		usleep(1500000);
         }
    }
    exit(0);
}

