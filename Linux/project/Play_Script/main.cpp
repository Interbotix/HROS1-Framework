/*
	This program will open and interpret a text file to give operations
	to the HR-OS1 robot to perform. The text file format should be:
	
		<Command> <Param>
		
	where Command and Param are both integers. To perform a continuous
	sequence of operations, the text file needs more than one line of 
	commands.
	
	By default, a textfile named script.txt will automatically be read,
	but user has the option to use a differently named textfile by 
	including that file name on the commandline as an argument upon
	starting up this program.
	
	Avoid adding comments to the script/text file.
	
	Commands (param interpretation):	
	1 Walk 			(seconds to walk for)
	2 Turn			(degrees to turn)
	3 Play Motion 	(page of motion in rme)
	4 Sleep 		(milliseconds to sleep for)
	5 Play MP3		**not developed yet
	6 Send Midi		**not developed yet
	7 Exit			*no param needed. Exit program
	
	To be used for Portland Cyber Show.
	
	Updated: 10-14-2017
*/
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
#include <fstream>

#define MOTION_FILE_PATH    "../../../Data/motion_4096.bin"
#define INI_FILE_PATH       "../../../Data/config.ini"

using namespace std;
using namespace Robot;

LinuxMotionTimer linuxMotionTimer;
LinuxArbotixPro linux_arbotixpro("/dev/ttyUSB0");
ArbotixPro arbotixpro(&linux_arbotixpro);
minIni* ini;
void play_script(void);
void walk(int direction, int second){
//    printf("walking...\t");
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
//    printf(" Done\n");
}

void turn(int direction, int degree){
    printf("turning...\t");
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
    signal(SIGINT,  &sighandler);

    ini = new minIni(INI_FILE_PATH);
    change_current_dir();

    if(Action::GetInstance()->LoadFile(MOTION_FILE_PATH)) 
		printf("Motions Loaded\n");
    
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
	
	ifstream script;

    while(on){
		
		system("clear"); // clear screen
		cout << "******************** Walking and Motion Manager ********************\n";
		
		// Check to see if there is a specified file that user wants
		// to use. Default to script.txt
		if(argc > 1)
		{
			cout << "Using text script " << argv[1] << ".\n\n";
			script.open(argv[1]);
		}
		else
		{
			cout << "No text file specified. Playing default file ""script.txt"".\n\n";
			script.open("script.txt");
		}
		
		// If file opened successfully, start playing script
		if (script.is_open())
		{
			while(!script.eof())
			{
				script >> input1;
				script >> input2;
			
				linuxMotionTimer.Initialize(MotionManager::GetInstance());
				cout << "\nCommand is : " << input1 << "   |   ";
				cout << "Param   is : "   << input2 << endl;
				cout << "    Status: ";
				
				switch(input1){
				case 1: // Walking
					if(prev_page != 8)
					{
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

				case 2: // Turning
					if(prev_page != 8)
					{
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

				case 3: // Play RME page
					prev_page = input2;
					motion(input2);
					break;
					
				case 4: // Sleep
					printf("Sleeping...\t");
					usleep(input2*1000);
					printf("Done.\n");
					break;	
				
				case 5: // Play MP3
					printf("MP3 Function not developed yet.\n");
					break;
					
				case 6: // Send Midi
					printf("Midi Function not developed yet.\n");
					
				case 7: // End of Script, off
					printf("End of Script.\n");
					script.close();
					break;
					
				default:
					perror("invalid input\n");
					usleep(1500000);
				}
			}
			
			printf("\nClosing Walking and Motion Manager.\n\n");
			exit(0);
		}
		else
		{
			printf("Failed to open text file.\nClosing Walking and Motion Manager.\n\n");
			exit(0);
		}
	}
}


