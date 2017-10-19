#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <term.h>
#include <ncurses.h>
#include <libgen.h>
#include <signal.h>
#include "cmd_process.h"
#include "PS3Controller.h"

#define MOTION_FILE_PATH    "../../../Data/motion_4096.bin"

#define INI_FILE_PATH       "../../../Data/config.ini"

using namespace Robot;

LinuxArbotixPro linux_arbotixpro("/dev/ttyUSB0");
ArbotixPro arbotixpro(&linux_arbotixpro);
LinuxMotionTimer linuxMotionTimer;


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

/*
 * This will automate saving a movement.
 * Page Step will also automatically increment
 */
static void AutoCreateMovements(Robot::ArbotixPro *arbotixpro, int num_param, int *list, char lists[30][10])
{
    char cmd;
    bool done = false;
    int column_num = 0;
//	Action::PAGE * Page = get_page();
    
    /* Flush "autorecord" command */    
    fflush(stdin);

    while (!done) {
		
		/* Clean command entry line */
		ClearCmd();
        /* Initialize to off */
        OnOffCmd(arbotixpro, false, num_param, list, lists);
        /* Receive input after robot is moved to position */
        cmd = getchar();
        fflush(stdin);

        if (cmd == 'd') 
		{
			done = true;
        }
	else{  /* Press any key and enter to perform movement save */
                OnOffCmd(arbotixpro, true, num_param, list, lists);
                WriteStepCmd(column_num);
                SaveCmd(IndexPage());

                /* Increments the column we are saving to */
                ++column_num;
				
			    /* Increment Page step count */
				Increment_Step(column_num);
                
                /* Check if all columns are full */
                if (column_num > 6) {
                    NextCmd();
					DrawPage();
                    column_num = 0;
                }
        }
  }
}

int main(int argc, char *argv[])
{
    signal(SIGABRT, &sighandler);
    signal(SIGTERM, &sighandler);
    signal(SIGQUIT, &sighandler);
    signal(SIGINT, &sighandler);

    int ch;
    char filename[128];

    minIni* ini = new minIni(INI_FILE_PATH);

    change_current_dir();
    if (argc < 2)
        strcpy(filename, MOTION_FILE_PATH); // Set default motion file path
    else
        strcpy(filename, argv[1]);

    /////////////// Load/Create Action File //////////////////
    if (Action::GetInstance()->LoadFile(filename) == false)
        {
            printf("Can not open %s\n", filename);
            printf("Do you want to make a new action file? (y/n) ");
            ch = _getch();
            if (ch != 'y')
                {
                    printf("\n");
                    exit(0);
                }

            if (Action::GetInstance()->CreateFile(filename) == false)
                {
                    printf("Fail to create %s\n", filename);
                    exit(0);
                }
        }
    ////////////////////////////////////////////////////////////

    //    PS3Controller_Start();
    //////////////////// Framework Initialize ////////////////////////////
    if (MotionManager::GetInstance()->Initialize(&arbotixpro) == false)
        {
            printf("Initializing Motion Manager failed!\n");
            exit(0);
        }
    MotionManager::GetInstance()->LoadINISettings(ini);
    MotionManager::GetInstance()->SetEnable(false);
    MotionManager::GetInstance()->AddModule((MotionModule*)Action::GetInstance());
    linuxMotionTimer.Initialize(MotionManager::GetInstance());
    linuxMotionTimer.Stop();
    //MotionManager::GetInstance()->StopThread();
    /////////////////////////////////////////////////////////////////////

    DrawIntro(&arbotixpro);

    char input[128] = {0,};
    char *token;
    int input_len;
    char cmd[80];
    int num_param;
    int iparam[30];
    char iparams[30][10];
    char s[60];
    int idx = 0;
    int apState[6] = {0, 0, 0, 0, 0, 0};

    while (1)
        {
            // while(!kbhit(true)) ProcessPS3(&arbotixpro,apState);
            ch = _getch();

            if (ch == 0x1b)
                {
                    ch = _getch();
                    if (ch == 0x5b)
                        {
                            ch = _getch();
                            if (ch == 0x41)     // Up arrow key
                                MoveUpCursor();
                            else if (ch == 0x42) // Down arrow key
                                MoveDownCursor();
                            else if (ch == 0x44) // Left arrow key
                                MoveLeftCursor();
                            else if (ch == 0x43) // Right arrow key
                                MoveRightCursor();
                        }
                }
            else if ( ch == '[' )
                UpDownValue(&arbotixpro, -1);
            else if ( ch == ']' )
                UpDownValue(&arbotixpro, 1);
            else if ( ch == '{' )
                UpDownValue(&arbotixpro, -10);
            else if ( ch == '}' )
                UpDownValue(&arbotixpro, 10);
            else if ( ch == ' ' )
                ToggleTorque(&arbotixpro);
            else if ((ch >= 'a' && ch <= 'z' ) || ( ch >= '0' && ch <= '9') )
                {

                    BeginCommandMode();

                    printf("%c", ch);
                    if (idx < 127) input[idx++] = (char)ch;

                    while (1)
                        {
                            //while(!kbhit(true))   ProcessPS3(&arbotixpro,apState);
                            ch = _getch();
                            if ( ch == 0x0A ) // newline
                                break;
                            else if ( ch == 0x7F || ch == 0x08) // delete key or bksp
                                {
                                    if (idx > 0)
                                        {
                                            ch = 0x08; // bksp
                                            printf("%c", ch);
                                            ch = ' ';
                                            printf("%c", ch);
                                            ch = 0x08;// bksp
                                            printf("%c", ch);
                                            input[--idx] = 0;
                                        }
                                }
                            else if ( ch == ' ' || ch == '-' || (ch >= 'a' && ch <= 'z' ) || ( ch >= '0' && ch <= '9') )
                                {
                                    if (idx < 127)
                                        {
                                            printf("%c", ch);
                                            input[idx++] = (char)ch;
                                        }
                                }
                        }

                    fflush(stdin);
                    input_len = strlen(input);
                    if (input_len > 0)
                        {
                            token = strtok( input, " " );
                            if (token != 0)
                                {
                                    strcpy( cmd, token );
                                    token = strtok( 0, " " );
                                    num_param = 0;
                                    while (token != 0)
                                        {
                                            iparam[num_param] = atoi(token);
                                            strncpy(iparams[num_param++], token, 10);
                                            token = strtok( 0, " " );
                                        }

                                    if (strcmp(cmd, "exit") == 0)
                                        {
                                            if (AskSave() == false)
                                                break;
                                        }
                                    else if (strcmp(cmd, "autorecord") == 0)
                                    {
                                        AutoCreateMovements(&arbotixpro, num_param, iparam, iparams);
                                    }
									else if(strcmp(cmd, "playc") == 0)
										Set_PlayCount(iparam[0]);
									else if(strcmp(cmd, "pstep") == 0)
										Set_PageStep(iparam[0]);
									else if(strcmp(cmd, "pspeed") == 0)
										Set_PageSpeed(iparam[0]);
									else if(strcmp(cmd, "accel") == 0)
										Set_AccelTime(iparam[0]);
									else if(strcmp(cmd, "l2n") == 0)
										Set_Link2Next(iparam[0]);
									else if(strcmp(cmd, "l2e") == 0)
										Set_Link2Exit(iparam[0]);
                                    else if (strcmp(cmd, "re") == 0)
                                        DrawPage();
                                    else if (strcmp(cmd, "help") == 0)
                                        HelpCmd();
                                    else if (strcmp(cmd, "n") == 0)
                                        NextCmd();
                                    else if (strcmp(cmd, "b") == 0)
                                        PrevCmd();
                                    else if (strcmp(cmd, "time") == 0)
                                        TimeCmd();
                                    else if (strcmp(cmd, "speed") == 0)
                                        SpeedCmd();
                                    else if (strcmp(cmd, "mon") == 0)
                                        {
                                            linuxMotionTimer.Start();
                                            arbotixpro.m_bIncludeTempData = true;
                                            arbotixpro.MakeBulkReadPacket();// force packet to get rebuilt
                                            while (!kbhit(true))
                                                {
                                                    MonitorServos(&arbotixpro);
                                                    usleep(10000);
                                                }
                                            linuxMotionTimer.Stop();
                                            arbotixpro.m_bIncludeTempData = false;
                                            arbotixpro.MakeBulkReadPacket();// force packet to be rebuilt
                                            GoToCursor(CMD_COL, CMD_ROW);
                                        }
                                    else if (strcmp(cmd, "page") == 0)
                                        {
                                            if (num_param > 0)
                                                PageCmd(iparam[0]);
                                            else
                                                PrintCmd("Need parameter");
                                        }
                                    else if (strcmp(cmd, "play") == 0)
                                        {
                                            if (num_param > 0)
                                                PlayCmd(&arbotixpro, iparam[0]);
                                            else
                                                PlayCmd(&arbotixpro, IndexPage());
                                        }
                                    else if (strcmp(cmd, "set") == 0)
                                        {
                                            if (num_param > 0)
                                                SetValue(&arbotixpro, iparam[0]);
                                            else
                                                PrintCmd("Need parameter");
                                        }
                                    else if (strcmp(cmd, "list") == 0)
                                        ListCmd();
                                    else if (strcmp(cmd, "on") == 0)
                                        OnOffCmd(&arbotixpro, true, num_param, iparam, iparams);
                                    else if (strcmp(cmd, "off") == 0)
                                        OnOffCmd(&arbotixpro, false, num_param, iparam, iparams);
                                    else if (strcmp(cmd, "w") == 0)
                                        {
                                            if (num_param > 0)
                                                WriteStepCmd(iparam[0]);
                                            else
                                                PrintCmd("Need parameter");
                                        }
                                    else if (strcmp(cmd, "d") == 0)
                                        {
                                            if (num_param > 0)
                                                DeleteStepCmd(iparam[0]);
                                            else
                                                PrintCmd("Need parameter");
                                        }
                                    else if (strcmp(cmd, "i") == 0)
                                        {
                                            if (num_param == 0)
                                                InsertStepCmd(0);
                                            else
                                                InsertStepCmd(iparam[0]);
                                        }
                                    else if (strcmp(cmd, "m") == 0)
                                        {
                                            if (num_param > 1)
                                                MoveStepCmd(iparam[0], iparam[1]);
                                            else
                                                PrintCmd("Need parameter");
                                        }
                                    else if (strcmp(cmd, "copy") == 0)
                                        {
                                            if (num_param > 0)
                                                CopyCmd(iparam[0]);
                                            else
                                                PrintCmd("Need parameter");
                                        }
                                    else if (strcmp(cmd, "new") == 0)
                                        NewCmd();
                                    else if (strcmp(cmd, "g") == 0)
                                        {
                                            if (num_param > 0)
                                                GoCmd(&arbotixpro, iparam[0]);
                                            else
                                                PrintCmd("Need parameter");
                                        }
                                    else if (strcmp(cmd, "poweroff") == 0)
                                        {
                                            arbotixpro.DXLPowerOn(false);
                                        }
                                    else if (strcmp(cmd, "poweron") == 0)
                                        {
                                            arbotixpro.DXLPowerOn(true);
                                        }
                                    else if (strcmp(cmd, "save") == 0)
                                        {
                                            if (num_param > 0)
                                                SaveCmd(iparam[0]);
                                            else
                                                SaveCmd(IndexPage());
                                        }
                                    else if (strcmp(cmd, "name") == 0)
                                        NameCmd();
                                    else
                                        {
                                            sprintf(s, "Bad command (%s:%d)! please input 'help'", cmd, strlen(cmd));
                                            PrintCmd(s);
                                        }
                                }
                        }
                    idx = 0;
                    memset(input, 0, 128);
                    EndCommandMode();
                }
        }

    DrawEnding();

    exit(0);
}
