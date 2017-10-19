#include "cmd_process.h"

#include <sys/time.h>

using namespace Robot;

#define VTANSI_ATTRIB_RESET 0
#define VTANSI_ATTRIB_BOLD 1
#define VTANSI_ATTRIB_DIM 2
#define VTANSI_ATTRIB_UNDERLINE 4
#define VTANSI_ATTRIB_BLINK 5
#define VTANSI_ATTRIB_REVERSE 7
#define VTANSI_ATTRIB_INVISIBLE 8

#define VTANSI_FG_BLACK 30
#define VTANSI_FG_RED 31
#define VTANSI_FG_GREEN 32
#define VTANSI_FG_YELLOW 33
#define VTANSI_FG_BLUE 34
#define VTANSI_FG_MAGENTA 35
#define VTANSI_FG_CYAN 36
#define VTANSI_FG_WHITE 37
#define VTANSI_FG_DEFAULT 39

#define VTANSI_BG_BLACK 40
#define VTANSI_BG_RED 41
#define VTANSI_BG_GREEN 42
#define VTANSI_BG_YELLOW 43
#define VTANSI_BG_BLUE 44
#define VTANSI_BG_MAGENTA 45
#define VTANSI_BG_CYAN 46
#define VTANSI_BG_WHITE 47
#define VTANSI_BG_DEFAULT 49

extern LinuxMotionTimer linuxMotionTimer;
int Col = STP7_COL;
int Row = ID_1_ROW;
int Old_Col;
int Old_Row;
bool bBeginCommandMode = false;
bool bEdited = false;
int indexPage = 1;


Action::PAGE Page;
Action::STEP Step;

int cf = VTANSI_FG_WHITE;//text that cannot be edited
int cf1 = VTANSI_FG_GREEN;//values that can be edited
int	cf2 = VTANSI_FG_CYAN;//page step and inicator
int	cb = VTANSI_BG_BLACK;
int	ca = VTANSI_ATTRIB_BOLD;//bold for value that can be edited
int	df = VTANSI_FG_WHITE;//return set
int	db = VTANSI_BG_BLACK;
int	da = VTANSI_ATTRIB_DIM;//dim for fixed vaules





Action::PAGE * get_page(){


return &Page;

}




int _getch()
{
	struct termios oldt, newt;
	int ch;
	tcgetattr( STDIN_FILENO, &oldt );
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	ch = getchar();
	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
	return ch;
}

int kbhit(bool bPushed)
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF && bPushed == true)
		{
			ungetc(ch, stdin);
			return 1;
		}

	if (ch != EOF && bPushed == false)
		return 1;

	return 0;
}

struct termios oldterm, new_term;
void set_stdin(void)
{
	tcgetattr(0, &oldterm);
	new_term = oldterm;
	new_term.c_lflag &= ~(ICANON | ECHO | ISIG); // 의미는 struct termios를 찾으면 됨.
	new_term.c_cc[VMIN] = 1;
	new_term.c_cc[VTIME] = 0;
	tcsetattr(0, TCSANOW, &new_term);
}

void reset_stdin(void)
{
	tcsetattr(0, TCSANOW, &oldterm);
}

void ReadStep(ArbotixPro *arbotixpro)
{
	int value;
	for (int id = 0; id < 31; id++)
		{
			if (id >= JointData::ID_MIN && id <= JointData::ID_MAX)
				{
					if (arbotixpro->ReadByte(id, AXDXL::P_TORQUE_ENABLE, &value, 0) == ArbotixPro::SUCCESS)
						{
							if (value == 1)
								{
									if (arbotixpro->ReadWord(id, AXDXL::P_GOAL_POSITION_L, &value, 0) == ArbotixPro::SUCCESS)
										Step.position[id] = value;
									else
										Step.position[id] = Action::INVALID_BIT_MASK;
								}
							else
								Step.position[id] = Action::TORQUE_OFF_BIT_MASK;
						}
					else
						Step.position[id] = Action::INVALID_BIT_MASK;
				}
			else
				Step.position[id] = Action::INVALID_BIT_MASK;
		}
}


bool AskSave()
{
	if (bEdited == true)
		{
			PrintCmd("Are you sure? (y/n)");
			if (_getch() != 'y')
				{
					ClearCmd();
					return true;
				}
		}

	return false;
}


void GoToCursor(int col, int row)
{
	char *cursor;
	char *esc_sequence;
	cursor = tigetstr("cup");
	esc_sequence = tparm(cursor, row, col);
	putp(esc_sequence);

	Col = col;
	Row = row;
}

void MoveUpCursor()
{
	if (Col >= STP7_COL && Col <= CCWSLOPE_COL)
		{
			if ( Row > ID_1_ROW )
				GoToCursor(Col, Row - 1);
		}
	else
		{
			if ( Row > PLAYCOUNT_ROW )
				GoToCursor(Col, Row - 1);
		}
}

void MoveDownCursor()
{
	if (Col >= STP7_COL && Col <= STP6_COL)
		{
			if ( Row < SPEED_ROW )
				GoToCursor(Col, Row + 1);
		}
	else if (Col <= CCWSLOPE_COL)
		{
			if ( Row < ID_20_ROW )
				GoToCursor(Col, Row + 1);
		}
	else
		{
			if ( Row < SEQCOUNT_ROW )
				GoToCursor(Col, Row + 1);
		}
}

void MoveLeftCursor()
{
	switch (Col)
		{
		case STP0_COL:
			GoToCursor(STP7_COL, Row);
			break;

		case STP1_COL:
			GoToCursor(STP0_COL, Row);
			break;

		case STP2_COL:
			GoToCursor(STP1_COL, Row);
			break;

		case STP3_COL:
			GoToCursor(STP2_COL, Row);
			break;

		case STP4_COL:
			GoToCursor(STP3_COL, Row);
			break;

		case STP5_COL:
			GoToCursor(STP4_COL, Row);
			break;

		case STP6_COL:
			GoToCursor(STP5_COL, Row);
			break;

		case CWSLOPE_COL:
			GoToCursor(STP6_COL, Row);
			break;

		case CCWSLOPE_COL:
			GoToCursor(CWSLOPE_COL, Row);
			break;

		case PAGEPARAM_COL:
			GoToCursor(CCWSLOPE_COL, Row);
			break;
		}
}

void MoveRightCursor()
{
	switch (Col)
		{
		case STP7_COL:
			GoToCursor(STP0_COL, Row);
			break;

		case STP0_COL:
			GoToCursor(STP1_COL, Row);
			break;

		case STP1_COL:
			GoToCursor(STP2_COL, Row);
			break;

		case STP2_COL:
			GoToCursor(STP3_COL, Row);
			break;

		case STP3_COL:
			GoToCursor(STP4_COL, Row);
			break;

		case STP4_COL:
			GoToCursor(STP5_COL, Row);
			break;

		case STP5_COL:
			GoToCursor(STP6_COL, Row);
			break;

		case STP6_COL:
			GoToCursor(CWSLOPE_COL, Row);
			break;

		case CWSLOPE_COL:
			GoToCursor(CCWSLOPE_COL, Row);
			break;

		case CCWSLOPE_COL:
			if (Row >= PLAYCOUNT_ROW && Row <= EXIT_ROW)
				GoToCursor(PAGEPARAM_COL, Row);
			break;
		}
}

void DrawIntro(ArbotixPro *arbotixpro)
{
	int res, nrows, ncolumns;
	setupterm(NULL, fileno(stdout), (int *)0);
	nrows = tigetnum("lines");
	ncolumns = tigetnum("cols");

	res = system("clear"); res = 0;
	/*
	printf("\n");
	printf("[Action Editor for DARwIn %s]\n", PROGRAM_VERSION);
	printf("\n");
	printf(" *Terminal screen size must be %d(col)x%d(row).\n", SCREEN_COL, SCREEN_ROW);
	printf(" *Current terminal has %d columns and %d rows.\n", ncolumns, nrows);
	printf("\n");
	printf("\n");
	printf("Press any key to start program...\n");
	_getch();
	*/
	Action::GetInstance()->LoadPage(indexPage, &Page);

	SetColor(VTANSI_FG_WHITE, VTANSI_BG_DEFAULT, VTANSI_ATTRIB_BOLD);
	ReadStep(arbotixpro);
	Step.pause = 0;
	Step.time = 0;

	DrawPage();
}

void DrawEnding()
{
	int res;

	SetColor(VTANSI_FG_WHITE, VTANSI_BG_DEFAULT, VTANSI_ATTRIB_RESET);
	res = system("clear"); res = 0;
	printf("\n");
	printf("Program Exit.");
	printf("\n");
}

void DrawPage()
{
	int old_col = Col;
	int old_row = Row;

	int res = system("clear"); res = 0;
	// 80    01234567890123456789012345678901234567890123456789012345678901234567890123456789     //24
	printf( "ID: 1(R_SHO_PITCH)[    ]                                                       \n" );//0
	printf( "ID: 2(L_SHO_PITCH)[    ]                                       Page Number:    \n" );//1
	printf( "ID: 3(R_SHO_ROLL) [    ]                                        Address:       \n" );//2
	printf( "ID: 4(L_SHO_ROLL) [    ]                                         Play Count:   \n" );//3
	printf( "ID: 5(R_ELBOW)    [    ]                                          Page Step:   \n" );//4
	printf( "ID: 6(L_ELBOW)    [    ]                                         Page Speed:   \n" );//5
	printf( "ID: 7(R_HIP_YAW)  [    ]                                         Accel Time:   \n" );//6
	printf( "ID: 8(L_HIP_YAW)  [    ]                                       Link to Next:   \n" );//7
	printf( "ID: 9(R_HIP_ROLL) [    ]                                       Link to Exit:   \n" );//8
	printf( "ID:10(L_HIP_ROLL) [    ]                                                       \n" );//9
	printf( "ID:11(R_HIP_PITCH)[    ]                                                       \n" );//0
	printf( "ID:12(L_HIP_PITCH)[    ]                                                       \n" );//1
	printf( "ID:13(R_KNEE)     [    ]                                                       \n" );//2
	printf( "ID:14(L_KNEE)     [    ]                                                       \n" );//3
	printf( "ID:15(R_ANK_PITCH)[    ]                                                       \n" );//4
	printf( "ID:16(L_ANK_PITCH)[    ]                                                       \n" );//5
	printf( "ID:17(R_ANK_ROLL) [    ]                                                       \n" );//6
	printf( "ID:18(L_ANK_ROLL) [    ]                                                       \n" );//7
	printf( "ID:19(HEAD_PAN)   [    ]                                                       \n" );//8
	printf( "ID:20(HEAD_TILT)  [    ]                                                       \n" );//9
	printf( "   PauseTime      [    ]                                                       \n" );//7

	if ( Page.header.schedule == Action::SPEED_BASE_SCHEDULE )
		printf( "   Speed          [    ]                                                       \n" );//1
	else if ( Page.header.schedule == Action::TIME_BASE_SCHEDULE )
		printf( "   Time(x 8msec)  [    ]                                                       \n" );//1

	printf( "                   STP7  STP0 STP1 STP2 STP3 STP4 STP5 STP6                    \n" );//2
	printf( "]                                                                              " );  // 3

	for (int i = 0; i <= Action::MAXNUM_STEP; i++ )
		DrawStep(i);

	// Draw Compliance slope
	for ( int id = JointData::ID_MIN; id <= JointData::ID_MAX; id++ )
		{
			GoToCursor(CWSLOPE_COL, id - 1);
			printf( "%.1d%.1d", Page.header.slope[id] >> 4, Page.header.slope[id] & 0x0f );
		}

	// Draw Page parameter
	GoToCursor( PAGEPARAM_COL, PLAYCOUNT_ROW );
	printf( "%.3d", Page.header.repeat );

	GoToCursor( PAGEPARAM_COL, STEPNUM_ROW );
	printf( "%.3d", Page.header.stepnum );

	GoToCursor( PAGEPARAM_COL, PLAYSPEED_ROW );
	printf( "%.3d", Page.header.speed );

	GoToCursor( PAGEPARAM_COL, ACCEL_ROW );
	printf( "%.3d", Page.header.accel );

	GoToCursor( PAGEPARAM_COL, NEXT_ROW );
	printf( "%.3d", Page.header.next );

	GoToCursor( PAGEPARAM_COL, EXIT_ROW );
	printf( "%.3d", Page.header.exit );

	GoToCursor( PAGEPARAM_COL, SEQCOUNT_ROW );
	printf( "%.3d", Page.header.seq_repeats );

	// Draw Page information
	DrawName();

	GoToCursor(PAGENUM_COL, PAGENUM_ROW);
	printf( "%.4d", indexPage );

	GoToCursor(ADDR_COL, ADDR_ROW);
	printf( "0x%.5X", (int)(indexPage * sizeof(Action::PAGE)) );

	DrawStepLine(false);

	GoToCursor(old_col, old_row);
}

void DrawStep(int index)
{
	int old_col = Col;
	int old_row = Row;
	Action::STEP *step;
	int col;

	switch (index)
		{
		case 0:
			col = STP0_COL;
			step = &Page.step[0];
			break;

		case 1:
			col = STP1_COL;
			step = &Page.step[1];
			break;

		case 2:
			col = STP2_COL;
			step = &Page.step[2];
			break;

		case 3:
			col = STP3_COL;
			step = &Page.step[3];
			break;

		case 4:
			col = STP4_COL;
			step = &Page.step[4];
			break;

		case 5:
			col = STP5_COL;
			step = &Page.step[5];
			break;

		case 6:
			col = STP6_COL;
			step = &Page.step[6];
			break;

		case 7:
			col = STP7_COL;
			step = &Step;
			break;

		default:
			return;
		}

	for ( int id = JointData::ID_MIN; id <= JointData::ID_MAX; id++ )
		{
			GoToCursor(col, id - 1);
			if (step->position[id] & Action::INVALID_BIT_MASK)
				printf("----");
			else if (step->position[id] & Action::TORQUE_OFF_BIT_MASK)
				printf("????");
			else
				printf("%.4d", step->position[id]);
		}

	GoToCursor(col, PAUSE_ROW);
	printf("%4.3d", step->pause);

	GoToCursor(col, SPEED_ROW);
	printf("%4.3d", step->time);

	GoToCursor( old_col, old_row );
}

void DrawStepLine(bool erase)
{
	int old_col = Col;
	int old_row = Row;
	int col;

	switch (Page.header.stepnum)
		{
		case 0:
			col = STP0_COL;
			break;

		case 1:
			col = STP1_COL;
			break;

		case 2:
			col = STP2_COL;
			break;

		case 3:
			col = STP3_COL;
			break;

		case 4:
			col = STP4_COL;
			break;

		case 5:
			col = STP5_COL;
			break;

		case 6:
			col = STP6_COL;
			break;

		case 7:
			col = CWSLOPE_COL;
			break;

		default:
			return;
		}
	col--;

	for ( int id = JointData::ID_MIN; id <= (JointData::ID_MAX + 2); id++ )
		{
			GoToCursor(col, id - 1);
			if (erase == true)
				printf( " " );
			else
				printf( "|" );
		}

	GoToCursor(old_col, old_row);
}

void DrawName()
{
	int old_col = Col;
	int old_row = Row;

	GoToCursor(NAME_COL, NAME_ROW);
	printf( "                " );
	GoToCursor(NAME_COL, NAME_ROW);

	for (int i = 0; i < Action::MAXNUM_NAME; i++)
		printf("%c", (char)Page.header.name[i]);

	GoToCursor( old_col, old_row );
}

void ClearCmd()
{
	PrintCmd("");
}

void PrintCmd(const char *message)
{
	int len = strlen(message);
	GoToCursor(0, CMD_ROW);

	printf( "] %s", message);
	for (int i = 0; i < (SCREEN_COL - (len + 2)); i++)
		printf(" ");

	GoToCursor(len + 2, CMD_ROW);
}

void UpDownValue(ArbotixPro *arbotixpro, int offset)
{
	SetValue(arbotixpro, GetValue() + offset);
}

int GetValue()
{
	int col;
	int row;
	if (bBeginCommandMode == true)
		{
			col = Old_Col;
			row = Old_Row;
		}
	else
		{
			col = Col;
			row = Row;
		}

	if ( col == STP7_COL )
		{
			if ( row == PAUSE_ROW )
				return Step.pause;
			else if ( row == SPEED_ROW )
				return Step.time;
			else
				return Step.position[row + 1];
		}
	else if ( col <= STP6_COL )
		{
			int i = 0;
			switch (col)
				{
				case STP0_COL:
					i = 0;
					break;

				case STP1_COL:
					i = 1;
					break;

				case STP2_COL:
					i = 2;
					break;

				case STP3_COL:
					i = 3;
					break;

				case STP4_COL:
					i = 4;
					break;

				case STP5_COL:
					i = 5;
					break;

				case STP6_COL:
					i = 6;
					break;
				}

			if ( row == PAUSE_ROW )
				return Page.step[i].pause;
			else if ( row == SPEED_ROW )
				return Page.step[i].time;
			else
				return Page.step[i].position[row + 1];
		}
	else if (col == CWSLOPE_COL)
		return (Page.header.slope[row + 1] >> 4);
	else if (col == CCWSLOPE_COL)
		return (Page.header.slope[row + 1] & 0x0f);
	else if (col == PAGEPARAM_COL)
		{
			if (row == PLAYCOUNT_ROW)
				return Page.header.repeat;
			else if (row == STEPNUM_ROW)
				return Page.header.stepnum;
			else if (row == PLAYSPEED_ROW)
				return Page.header.speed;
			else if (row == ACCEL_ROW)
				return Page.header.accel;
			else if (row == NEXT_ROW)
				return Page.header.next;
			else if (row == EXIT_ROW)
				return Page.header.exit;
			else if (row == SEQCOUNT_ROW)
				return Page.header.seq_repeats;
		}

	return -1;
}

void SetValue(ArbotixPro *arbotixpro, int value)
{
	int col;
	int row;
	if (bBeginCommandMode == true)
		{
			col = Old_Col;
			row = Old_Row;
		}
	else
		{
			col = Col;
			row = Row;
		}

	GoToCursor(col, row);

	if ( col == STP7_COL )
		{
			if ( row == PAUSE_ROW )
				{
					if (value >= 0 && value <= 255)
						{
							Step.pause = value;
							printf( "%4.3d", value );
							bEdited = true;
						}
				}
			else if ( row == SPEED_ROW )
				{
					if (value >= 0 && value <= 255)
						{
							Step.time = value;
							printf( "%4.3d", value );
							bEdited = true;
						}
				}
			else
				{
					if (value >= 0 && value <= AXDXL::MAX_VALUE)
						{
							if (!(Step.position[row + 1] & Action::INVALID_BIT_MASK) && !(Step.position[row + 1] & Action::TORQUE_OFF_BIT_MASK))
								{
									int error;
									if (arbotixpro->WriteWord(row + 1, AXDXL::P_GOAL_POSITION_L, value, &error) == ArbotixPro::SUCCESS)
										{
											if (!(error & ArbotixPro::ANGLE_LIMIT))
												{
													Step.position[row + 1] = value;
													printf( "%.4d", value );
													bEdited = true;
												}
										}
								}
						}
				}
		}
	else if ( col <= STP6_COL )
		{
			int i = 0;
			switch (col)
				{
				case STP0_COL:
					i = 0;
					break;

				case STP1_COL:
					i = 1;
					break;

				case STP2_COL:
					i = 2;
					break;

				case STP3_COL:
					i = 3;
					break;

				case STP4_COL:
					i = 4;
					break;

				case STP5_COL:
					i = 5;
					break;

				case STP6_COL:
					i = 6;
					break;
				}

			if ( row == PAUSE_ROW )
				{
					if (value >= 0 && value <= 255)
						{
							Page.step[i].pause = value;
							printf( "%4.3d", value );
							bEdited = true;
						}
				}
			else if ( row == SPEED_ROW )
				{
					if (value >= 0 && value <= 255)
						{
							Page.step[i].time = value;
							printf( "%4.3d", value );
							bEdited = true;
						}
				}
			else
				{
					//	printf("value = %d\n\n",Page.step[i].position[row + 1]);
					if (value >= 0 && value <= AXDXL::MAX_VALUE)
						{
							if (!(Page.step[i].position[row + 1] & Action::INVALID_BIT_MASK))
								{
									Page.step[i].position[row + 1] = value;
									printf( "%.4d", value );
									bEdited = true;
								}
						}
				}
		}
	else if (col == CWSLOPE_COL)
		{
			if (value >= 1 && value <= 7)
				{
					Page.header.slope[row + 1] = (value << 4) + (Page.header.slope[row + 1] & 0x0f);
					printf( "%.1d", value );
					bEdited = true;
				}
		}
	else if (col == CCWSLOPE_COL)
		{
			if (value >= 1 && value <= 7)
				{
					Page.header.slope[row + 1] = (Page.header.slope[row + 1] & 0xf0) + (value & 0x0f);
					printf( "%.1d", value );
					bEdited = true;
				}
		}
	else if (col == PAGEPARAM_COL)
		{
			if (row == PLAYCOUNT_ROW)
				{
					if (value >= 0 && value <= 255)
						{
							Page.header.repeat = value;
							printf( "%.3d", value );
							bEdited = true;
						}
				}
			else if (row == STEPNUM_ROW)
				{
					if (value >= 0 && value <= Action::MAXNUM_STEP)
						{
							if (Page.header.stepnum != value)
								{
									DrawStepLine(true);
									Page.header.stepnum = value;
									DrawStepLine(false);
									printf( "%.3d", value );
									bEdited = true;
								}
						}
				}
			else if (row == PLAYSPEED_ROW)
				{
					if (value >= 0 && value <= 255)
						{
							Page.header.speed = value;
							printf( "%.3d", value );
							bEdited = true;
						}
				}
			else if (row == ACCEL_ROW)
				{
					if (value >= 0 && value <= 255)
						{
							Page.header.accel = value;
							printf( "%.3d", value );
							bEdited = true;
						}
				}
			else if (row == NEXT_ROW)
				{
					if (value >= 0 && value <= 255)
						{
							Page.header.next = value;
							printf( "%.3d", value );
							bEdited = true;
						}
				}
			else if (row == EXIT_ROW)
				{
					if (value >= 0 && value <= 255)
						{
							Page.header.exit = value;
							printf( "%.3d", value );
							bEdited = true;
						}
				}
			else if (row == SEQCOUNT_ROW)
				{
					if (value >= 0 && value <= 255)
						{
							Page.header.seq_repeats = value;
							printf( "%.3d", value );
							bEdited = true;
						}
				}
		}

	GoToCursor(col, row);
}

void ToggleTorque(ArbotixPro *arbotixpro)
{
	if (Col != STP7_COL || Row > ID_20_ROW)
		return;

	int id = Row + 1;

	if (Step.position[id] & Action::TORQUE_OFF_BIT_MASK)
		{
			if (arbotixpro->WriteByte(id, AXDXL::P_TORQUE_ENABLE, 1, 0) != ArbotixPro::SUCCESS)
				return;

			int value;
			if (arbotixpro->ReadWord(id, AXDXL::P_PRESENT_POSITION_L, &value, 0) != ArbotixPro::SUCCESS)
				return;

			Step.position[id] = value;
			printf("%.4d", value);
		}
	else
		{
			if (arbotixpro->WriteByte(id, AXDXL::P_TORQUE_ENABLE, 0, 0) != ArbotixPro::SUCCESS)
				return;

			Step.position[id] = Action::TORQUE_OFF_BIT_MASK;
			printf("????");
		}

	GoToCursor(Col, Row);
}

void BeginCommandMode()
{
	Old_Col = Col;
	Old_Row = Row;
	ClearCmd();
	GoToCursor(CMD_COL, CMD_ROW);
	bBeginCommandMode = true;
}

void EndCommandMode()
{
	GoToCursor(Old_Col, Old_Row);
	bBeginCommandMode = false;
}

void HelpCmd()
{
	int res = system("clear"); res = 0;
	printf(" exit               Exits the program.\n");
	printf(" re                 Refreshes the screen.\n");
	printf(" b                  Move to previous page.\n");
	printf(" n                  Move to next page.\n");
	printf(" page [index]       Move to page [index].\n");
	printf(" list               View list of pages.\n");
	printf(" new                Clears data of current page and initializes page.\n");
	printf(" copy [index]       Copy data from page [index].\n");
	printf(" set [value]        Sets value on cursor [value].\n");
	printf(" save               Saves changes.\n");
	printf(" play               Motion playback of current page.\n");
	printf(" g [index]          Motion playback of STP[index].\n");
	printf(" name               Name for current page or changes the name of current page.\n");
	printf(" time               Change time base playing.\n");
	printf(" speed              Change speed base playing.\n");
	printf(" w [index]          Overwrites data from STP[index] with STP7.\n");
	printf(" i                  Inserts data from STP7 to STP0. \n" \
	       "                    Moves data from STP[x] to STP[x+1].\n");
	printf(" i [index]          Inserts data from STP7 to STP[index]. \n" \
	       "                    Moves data from STP[index] to STP[index+1].\n");
	printf(" m [index] [index2] Moves data from [index] to [index2] step.\n");
	printf(" d [index]          Deletes data from STP[index]. \n"
	       "                    Pushes data from STP[index] to STP[index-1].\n");
	printf(" on/off             Turn On/Off torque from ALL actuators.\n");
	printf(" on/off [index1] [index2] ...  \n" \
	       "        [index1]-[index2] ...  from one index to another\n" \
	       "        ra la rl ll h ... right arm, left arm, right leg, left leg, head\n\n");
	printf(" playc [value]      Change value of Play Count. Number of times to loop this page.\n");
	printf(" pstep [value]      Change value of Page Step. Up to what STP to play of this page.\n");
	printf(" pspeed [value]     Change value of Play Speed. Affects speed this page is played at.\n");
	printf(" accel [value]      Change value of Accel Time.\n");
	printf(" l2n [index]        Change value of Link to Next. Index of page to play next after this page.\n");
	printf("\n");
	printf(" autorecord         Starts autorecord mode. Saves current servo values\n"
		   "                    starting from STP0 to STP7 and will continue to the\n"
		   "                    next page.\n"
		   "                    Exit this mode with d. Any other button will record\n"
		   "                    and save.\n");
	printf("\n");
	printf(" Press any key to continue...");
	_getch();

	DrawPage();
}

void NextCmd()
{
	PageCmd(indexPage + 1);
	DrawPage();
}

int IndexPage(void)
{
	return indexPage;
}

// Increment Page Step as you save movements
void Increment_Step(int col)
{
	if(col > Page.header.stepnum)
	{
		GoToCursor(PAGEPARAM_COL,STEPNUM_ROW);
		DrawStepLine(true);
		Page.header.stepnum++;
        DrawStepLine(false);
		printf("%.3d",Page.header.stepnum);
	}
}

void Set_PlayCount(int value)
{
	if(value >= 0 && value <= 255)
	{
		GoToCursor(PAGEPARAM_COL,PLAYCOUNT_ROW);
		Page.header.repeat = value;
		printf( "%.3d", value );
		bEdited = true;
	}
		
}

void Set_PageStep(int value)
{
	if(value >= 0 && value <= Action::MAXNUM_STEP)
	{	
		if(Page.header.stepnum != value)
		{
			GoToCursor(PAGEPARAM_COL,STEPNUM_ROW);
			DrawStepLine(true);
			Page.header.stepnum = value;
			DrawStepLine(false);
			printf("%.3d", value);
			bEdited = true;
		}
	}
}

void Set_PageSpeed(int value)
{
	if(value >= 0 && value <= 255)
	{
		GoToCursor(PAGEPARAM_COL,PLAYSPEED_ROW);
		Page.header.speed = value;
		printf( "%.3d", value );
		bEdited = true;
	}
}

void Set_AccelTime(int value)
{
	if (value >= 0 && value <= 255)
	{
		GoToCursor(PAGEPARAM_COL,ACCEL_ROW);
		Page.header.next = value;
		printf( "%.3d", value );
		bEdited = true;
	}
}

void Set_Link2Next(int value)
{
	if (value >= 0 && value <= 255)
	{
		GoToCursor(PAGEPARAM_COL,NEXT_ROW);
		Page.header.next = value;
		printf( "%.3d", value );
		bEdited = true;
	}
}
void Set_Link2Exit(int value)
{
	if (value >= 0 && value <= 255)
	{
		GoToCursor(PAGEPARAM_COL,EXIT_ROW);
		Page.header.exit = value;
		printf( "%.3d", value );
		bEdited = true;
	}
}

void PrevCmd()
{
	PageCmd(indexPage - 1);
}

void PageCmd(int index)
{
	if (AskSave() == true)
		return;

	if (index >= 0 && index < Action::MAXNUM_PAGE)
		{
			indexPage = index;
			Action::GetInstance()->LoadPage(indexPage, &Page);

			Col = STP7_COL;
			Row = ID_1_ROW;
			DrawPage();
		}
	else
		PrintCmd("Invalid page index");

	bEdited = false;
}

void TimeCmd()
{
	Page.header.schedule = Action::TIME_BASE_SCHEDULE;
	bEdited = true;
	DrawPage();
}

void SpeedCmd()
{
	Page.header.schedule = Action::SPEED_BASE_SCHEDULE;
	bEdited = true;
	DrawPage();
}

void MonitorServos(ArbotixPro *arbotixpro)
{
	int value, j;
	int colors[] = {VTANSI_FG_BLUE, VTANSI_FG_CYAN, VTANSI_FG_YELLOW, VTANSI_FG_RED};
	int t1[] = {35, 45, 55, 65};
	for (int id = JointData::ID_MIN; id <= JointData::ID_MAX; id++)
		{
			value = MotionStatus::m_CurrentJoints.GetTemp(id);
			for (j = 0; j < 3; j++)
				{
					if (value < t1[j])
						{
							break;
						}
				}
			GoToCursor(15, id - 1);
			SetColor(colors[j], VTANSI_BG_DEFAULT, VTANSI_ATTRIB_BOLD);
			printf("%2d", value);
			SetColor(VTANSI_FG_WHITE, VTANSI_BG_DEFAULT, VTANSI_ATTRIB_BOLD);
		}
	GoToCursor(CMD_COL, CMD_ROW);
}

void PlayCmd(ArbotixPro *arbotixpro, int pageNum)
{
  char *timestring;
	int value, oldIndex = 0;
  struct timeval t1, t2;
	Action::PAGE page;

  // start timer
  gettimeofday(&t1, NULL);

	oldIndex = indexPage;
	if (pageNum != indexPage)
		{
			memcpy(&page, &Page, sizeof(Action::PAGE));
			indexPage = pageNum;
			if (Action::GetInstance()->LoadPage(indexPage, &Page) != true)
				{
					memcpy(&Page, &page, sizeof(Action::PAGE));
					indexPage = oldIndex;
					return;
				}
		}
	for (int i = 0; i < Page.header.stepnum; i++)
		{
			for (int id = JointData::ID_MIN; id <= JointData::ID_MAX; id++)
				{
					if (Page.step[i].position[id] & Action::INVALID_BIT_MASK)
						{
							PrintCmd("Exist invalid joint value");
							//return;
						}
				}
		}

	for (int id = JointData::ID_MIN; id <= JointData::ID_MAX; id++)
		{
			if (arbotixpro->ReadByte(id, AXDXL::P_TORQUE_ENABLE, &value, 0) == ArbotixPro::SUCCESS)
				{
					if (value == 0)
						{
							if (arbotixpro->ReadWord(id, AXDXL::P_PRESENT_POSITION_L, &value, 0) == ArbotixPro::SUCCESS)
								MotionStatus::m_CurrentJoints.SetValue(id, value);
						}
					else
						{
							if (arbotixpro->ReadWord(id, AXDXL::P_GOAL_POSITION_L, &value, 0) == ArbotixPro::SUCCESS)
								MotionStatus::m_CurrentJoints.SetValue(id, value);
						}
				}
		}
	PrintCmd("Playing... ('s' to stop, 'b' to brake)");

	//MotionManager::GetInstance()->StartThread();
	linuxMotionTimer.Start();
	Action::GetInstance()->m_Joint.SetEnableBody(true, true);
	MotionManager::GetInstance()->SetEnable(true);
	if (Action::GetInstance()->Start(pageNum, &Page) == false)
		{
			PrintCmd("Failed to play this page!");
			MotionManager::GetInstance()->SetEnable(false);
			linuxMotionTimer.Stop();
			return;
		}

	set_stdin();
	while (1)
		{
			if (Action::GetInstance()->IsRunning() == false)
				break;

			if (kbhit(true))
				{
					int key = _getch();
					GoToCursor(CMD_COL, CMD_ROW);
					if (key == 's')
						{
							Action::GetInstance()->Stop();
							fprintf(stderr, "\r] Stopping...                                  ");
						}
					else if (key == 'b')
						{
							Action::GetInstance()->Brake();
							fprintf(stderr, "\r] Braking...                                   ");
						}
					else
						fprintf(stderr, "\r] Playing... ('s' to stop, 'b' to brake)");
				}

			usleep(8000);
		}
	reset_stdin();

	MotionManager::GetInstance()->SetEnable(false);
	linuxMotionTimer.Stop();
	//MotionManager::GetInstance()->StopThread();

	GoToCursor(CMD_COL, CMD_ROW);

  // stop timer
  gettimeofday(&t2, NULL);

  if (((t2.tv_usec - t1.tv_usec) / ((double)1000.0)) < 0) {
    t2.tv_sec -= 1;
    t1.tv_usec += 1000.0;
  }

  asprintf(&timestring, "Done. Took %d s %f ms.",
      (t2.tv_sec - t1.tv_sec),
      (t2.tv_usec - t1.tv_usec) / ((double)1000.0)
      );
	PrintCmd(timestring);
  free(timestring);

	usleep(10000);
	if (oldIndex != indexPage)
		{
			memcpy(&Page, &page, sizeof(Action::PAGE));
			indexPage = oldIndex;
		}

	ReadStep(arbotixpro);
	DrawStep(7);
}

struct DIR_RECORD
{
	char	name[20];
	byte	pageNum;
};

void ListCmd()
{
	int old_col = Col;
	int old_row = Row;
	int index = 0;
	Action::PAGE page;
	int x = 0, y = 0, width = 19;
	struct DIR_RECORD table[Action::MAXNUM_PAGE];
	int cf, cf1, cf2, cb, ca;
	int df, db, da, da1;
	bool bActive;

	cf = VTANSI_FG_WHITE;//text that cannot be edited
	cf1 = VTANSI_FG_GREEN;//values that can be edited
	cf2 = VTANSI_FG_CYAN;//page step and inicator
	cb = VTANSI_BG_DEFAULT;//VTANSI_BG_BLACK;
	ca = VTANSI_ATTRIB_RESET;//VTANSI_ATTRIB_BOLD;//bold for value that can be edited
	df = VTANSI_FG_WHITE;//return set
	db = VTANSI_BG_BLACK;
	da = VTANSI_ATTRIB_DIM;//dim for fixed vaules
	da1 = VTANSI_ATTRIB_REVERSE;
	SetColor(VTANSI_FG_WHITE, VTANSI_BG_DEFAULT, VTANSI_ATTRIB_RESET);
	while (1)
		{
			int res = system("clear"); res = 0;
			GoToCursor(0, 0);
			for (int i = 0; i < 22; i++)
				{
					for (int j = 0; j < 4; j++)
						{
							int k = (index * 88) + (j * 22 + i);
							if (k < Action::MAXNUM_PAGE)
								{
									if (Action::GetInstance()->LoadPage(k, &page) == true)
										{
											printf(" |%.3d.", k);
											table[k].pageNum = k;
											for (int n = 0; n < Action::MAXNUM_NAME; n++)
												{
													table[k].name[n] = page.header.name[n];
													if ((char)page.header.name[n] >= ' ' && (char)page.header.name[n] <= '~')
														printf("%c", (char)page.header.name[n]);
													else
														printf(" ");
												}
										}
									else
										{
											printf(" |                ");
										}
								}
						}
					printf("\n");
				}

			int pageNum = 0;
			GoToCursor(x * width, y);
			SetColor(cf, cb, da1);
			printf(" |");
			SetColor(cf2, cb, da1);
			printf("%.3d.", pageNum);
			SetColor(cf1, cb, da1);
			printf("%-14.14s", table[pageNum].name);
			SetColor(cf, cb, da1);
			GoToCursor(0, 23); //this cursor is at the right edge of the selected item

			printf("Action Page List (%d/3) - Press key n(Next), b(Prev), q(Quit)", index + 1);
			bActive = true;
			while (bActive == true)
				{
					int ch = _getch();
					pageNum = index * 88 + x * 22 + y;
					if (pageNum < Action::MAXNUM_PAGE)
						{
							GoToCursor(x * width, y);
							SetColor(cf, cb, ca);
							printf(" |");
							SetColor(cf2, cb, ca);
							printf("%.3d.", pageNum);
							SetColor(cf1, cb, ca);
							printf("%-14.14s", table[pageNum].name);
							SetColor(cf, cb, ca);
							GoToCursor(x * width, y); //this cursor is at the right edge of the selected item
						}
					switch (ch)
						{
						case 'n':
							if (index < 2)
								{
									index++;
								}
							bActive = false;
							break;
						case 'b':
							if (index > 0)
								{
									index--;
								}
							bActive = false;
							break;
						case 'q':
							DrawPage();
							GoToCursor(old_col, old_row);
							return;
						case 0x41: //up
							if (y > 0) y--;
							break;
						case 0x42: //dn
							if (y < 21) y++;
							break;
						case 0x43: //rt
							if (x < 3) x++;
							break;
						case 0x44: //lt
							if (x > 0) x--;
							break;
						case 10: //CR
							if (pageNum < Action::MAXNUM_PAGE)
								{
									indexPage = pageNum;
									Action::GetInstance()->LoadPage(indexPage, &Page);
									SetColor(VTANSI_FG_WHITE, VTANSI_BG_DEFAULT, VTANSI_ATTRIB_RESET);
									GoToCursor(0, 0);
									DrawPage();
									GoToCursor(old_col, old_row);
									return;
								}
							break;
						}
					pageNum = index * 88 + x * 22 + y;
					if (pageNum < Action::MAXNUM_PAGE)
						{
							GoToCursor(x * width, y);
							SetColor(cf, cb, da1);
							printf(" |");
							SetColor(cf2, cb, da1);
							printf("%.3d.", pageNum);
							SetColor(cf1, cb, da1);
							printf("%-14.14s", table[pageNum].name);
							SetColor(cf, cb, da1);
							GoToCursor(x * width, y); //this cursor is at the right edge of the selected item
						}
				}
			SetColor(VTANSI_FG_WHITE, VTANSI_BG_DEFAULT, VTANSI_ATTRIB_RESET);
		}
}

void OnOffCmd(ArbotixPro *arbotixpro, bool on, int num_param, int *list, char lists[30][10])
{
	char *token, token1[30];
	int startID, stopID, id;
	int rl[6] = { JointData::ID_R_ANKLE_ROLL, JointData::ID_R_ANKLE_PITCH, JointData::ID_R_KNEE, JointData::ID_R_HIP_PITCH, JointData::ID_R_HIP_ROLL, JointData::ID_R_HIP_YAW };
	int ll[6] = { JointData::ID_L_ANKLE_ROLL, JointData::ID_L_ANKLE_PITCH, JointData::ID_L_KNEE, JointData::ID_L_HIP_PITCH, JointData::ID_L_HIP_ROLL, JointData::ID_L_HIP_YAW };
	int ra[6] = { JointData::ID_R_SHOULDER_PITCH, JointData::ID_R_SHOULDER_ROLL, JointData::ID_R_ELBOW };
	int la[6] = { JointData::ID_L_SHOULDER_PITCH, JointData::ID_L_SHOULDER_ROLL, JointData::ID_L_ELBOW };
	int h[3] = { JointData::ID_HEAD_PAN, JointData::ID_HEAD_TILT };

	if (num_param == 0)
		{
			for (int id = JointData::ID_MIN; id <= JointData::ID_MAX; id++)
				arbotixpro->WriteByte(id, AXDXL::P_TORQUE_ENABLE, (int)on, 0);
		}
	else
		{
			for (int i = 0; i < num_param; i++)
				{
					token = strtok(lists[i], "-");
					if (token != NULL)
						{
							startID = atoi(token);
							strcpy(token1, token);
							token = strtok(0, "-");
							if (token != NULL)
								{
									stopID = atoi(token);
									if (stopID >= JointData::ID_MIN && stopID <= JointData::ID_MAX && startID >= JointData::ID_MIN && startID <= JointData::ID_MAX)
										{
											for (id = startID; id <= stopID; id++)
												arbotixpro->WriteByte(id, AXDXL::P_TORQUE_ENABLE, (int)on, 0);
										}
								}
							else
								{
									if (strcmp(token1, "rl") == 0)
										{
											for (id = 0; id < 6; id++ )
												arbotixpro->WriteByte(rl[id], AXDXL::P_TORQUE_ENABLE, (int)on, 0);
										}
									else if (strcmp(token1, "ll") == 0)
										{
											for (id = 0; id < 6; id++ )
												arbotixpro->WriteByte(ll[id], AXDXL::P_TORQUE_ENABLE, (int)on, 0);
										}
									else if (strcmp(token1, "ra") == 0)
										{
											for (id = 0; id < 6; id++ )
												arbotixpro->WriteByte(ra[id], AXDXL::P_TORQUE_ENABLE, (int)on, 0);
										}
									else if (strcmp(token1, "la") == 0)
										{
											for (id = 0; id < 6; id++ )
												arbotixpro->WriteByte(la[id], AXDXL::P_TORQUE_ENABLE, (int)on, 0);
										}
									else if (strcmp(token1, "h") == 0)
										{
											for (id = 0; id < 3; id++ )
												arbotixpro->WriteByte(h[id], AXDXL::P_TORQUE_ENABLE, (int)on, 0);
										}
									else if (list[i] >= JointData::ID_MIN && list[i] <= JointData::ID_MAX)
										arbotixpro->WriteByte(list[i], AXDXL::P_TORQUE_ENABLE, (int)on, 0);
								}
						}
				}
		}
	ReadStep(arbotixpro);
	DrawStep(7);
}

void WriteStepCmd(int index)
{
	for (int id = JointData::ID_MIN; id <= JointData::ID_MAX; id++)
		{
			if (Step.position[id] & Action::TORQUE_OFF_BIT_MASK)
				return;
		}

	if (index >= 0 && index < Action::MAXNUM_STEP)
		{
			Page.step[index] = Step;
			DrawStep(index);
			bEdited = true;
		}
	else
		PrintCmd("Invalid step index");
}

void DeleteStepCmd(int index)
{
	if (index >= 0 && index < Action::MAXNUM_STEP)
		{
			for (int i = index; i < Action::MAXNUM_STEP; i++)
				{
					if (i == (Action::MAXNUM_STEP - 1))
						{
							for (int id = JointData::ID_MIN; id <= JointData::ID_MAX; id++)
								Page.step[i].position[id] = Action::INVALID_BIT_MASK;

							Page.step[i].pause = 0;
							Page.step[i].time = 0;
						}
					else
						Page.step[i] = Page.step[i + 1];

					DrawStep(i);
				}

			if (index < Page.header.stepnum)
				{
					if (Page.header.stepnum != 0)
						{
							DrawStepLine(true);
							Page.header.stepnum--;
							DrawStepLine(false);
						}

					GoToCursor(PAGEPARAM_COL, STEPNUM_ROW);
					printf( "%.3d", Page.header.stepnum );
				}

			bEdited = true;
		}
	else
		PrintCmd("Invalid step index");
}

void InsertStepCmd(int index)
{
	for (int id = JointData::ID_MIN; id <= JointData::ID_MAX; id++)
		{
			if (Step.position[id] & Action::TORQUE_OFF_BIT_MASK)
				{
					PrintCmd("Exist invalid joint value");
					return;
				}
		}

	if (index >= 0 && index < Action::MAXNUM_STEP)
		{
			for (int i = Action::MAXNUM_STEP - 1; i > index; i-- )
				{
					Page.step[i] = Page.step[i - 1];
					DrawStep(i);
				}

			Page.step[index] = Step;
			DrawStep(index);

			if (index == 0 || index < Page.header.stepnum)
				{
					if (Page.header.stepnum != Action::MAXNUM_STEP)
						{
							DrawStepLine(true);
							Page.header.stepnum++;
							DrawStepLine(false);
						}

					GoToCursor(PAGEPARAM_COL, STEPNUM_ROW);
					printf( "%.3d", Page.header.stepnum );
				}

			bEdited = true;
		}
	else
		PrintCmd("Invalid step index");
}

void MoveStepCmd(int src, int dst)
{
	if (src < 0 || src >= Action::MAXNUM_STEP)
		{
			PrintCmd("Invalid step index");
			return;
		}

	if (dst < 0 || dst >= Action::MAXNUM_STEP)
		{
			PrintCmd("Invalid step index");
			return;
		}

	if (src == dst)
		return;

	Action::STEP step = Page.step[src];
	if (src < dst)
		{
			for (int i = src; i < dst; i++)
				{
					Page.step[i] = Page.step[i + 1];
					DrawStep(i);
				}
		}
	else
		{
			for (int i = src; i > dst; i--)
				{
					Page.step[i] = Page.step[i - 1];
					DrawStep(i);
				}
		}

	Page.step[dst] = step;
	DrawStep(dst);
	bEdited = true;
}

void CopyCmd(int index)
{
	if (index == indexPage)
		return;

	if (Action::GetInstance()->LoadPage(index, &Page) == true)
		{
			DrawPage();
			bEdited = true;
		}
	else
		PrintCmd("Invalid page index");
}

void NewCmd()
{
	Action::GetInstance()->ResetPage(&Page);
	DrawPage();
	bEdited = true;
}

void GoCmd(ArbotixPro *arbotixpro, int index)
{
	if (index < 0 || index >= Action::MAXNUM_STEP)
		{
			PrintCmd("Invalid step index");
			return;
		}

	if (index > Page.header.stepnum)
		{
			PrintCmd("Are you sure? (y/n)");
			if (_getch() != 'y')
				{
					ClearCmd();
					return;
				}
		}

	int id;
	int wStartPosition;
	Action::PAGE tPage;

	Action::GetInstance()->ResetPage(&tPage);

	for (id = JointData::ID_MIN; id <= JointData::ID_MAX; id++)
		{
			if (Page.step[index].position[id] & Action::INVALID_BIT_MASK)
				{
					PrintCmd("Exist invalid joint value");
					return;
				}

			if (arbotixpro->ReadWord(id, AXDXL::P_PRESENT_POSITION_L, &wStartPosition, 0) != ArbotixPro::SUCCESS)
				{
					PrintCmd("Failed to read position");
					return;
				}
			MotionStatus::m_CurrentJoints.SetValue(id, wStartPosition);
			tPage.step[0].position[id] = wStartPosition;
			tPage.step[1].position[id] = Page.step[index].position[id];

		}
	tPage.step[0].time = tPage.step[1].time = 80;
	tPage.header.stepnum = 2;
	tPage.header.repeat = 1;
	Action::GetInstance()->m_Joint.SetEnableBody(true, true);
	MotionManager::GetInstance()->SetEnable(true);
	linuxMotionTimer.Start();

	if (Action::GetInstance()->Start(indexPage, &tPage) == false)
		{
			PrintCmd("Failed to play this page!");
			MotionManager::GetInstance()->SetEnable(false);
			return;
		}
	set_stdin();

	while (1)
		{
			if (Action::GetInstance()->IsRunning() == false)
				break;

			if (kbhit(true))
				{
					int key = _getch();
					GoToCursor(CMD_COL, CMD_ROW);
					if (key == 's')
						{
							Action::GetInstance()->Stop();
							fprintf(stderr, "\r] Stopping...                                  ");
						}
					else if (key == 'b')
						{
							Action::GetInstance()->Brake();
							fprintf(stderr, "\r] Braking...                                   ");
						}
					else
						fprintf(stderr, "\r] Playing... ('s' to stop, 'b' to brake)");
				}

			usleep(1000);
		}
	reset_stdin();

	MotionManager::GetInstance()->SetEnable(false);
	linuxMotionTimer.Stop();

	Step = Page.step[index];
	DrawStep(7);
}

void SaveCmd(int pageNum)
{
	if (bEdited == false)
		return;

	if (Action::GetInstance()->SavePage(pageNum, &Page) == true)
		bEdited = false;
}

void NameCmd()
{
	ClearCmd();
	GoToCursor(CMD_COL, CMD_ROW);
	printf("name: ");
	char name[80] = {0};
	if (fgets(name, 80, stdin) != NULL)
		fflush(stdin);
	for (int i = 0; i <= Action::MAXNUM_NAME; i++)
		Page.header.name[i] = name[i];
	DrawName();
	bEdited = true;
}

void SetColor(int fg, int bg, int attrib)
{
	printf("\x1b[%d;%d;%dm", fg, bg, attrib);
	return;
}

void ProcessPS3(Robot::ArbotixPro *arbotixpro, int *apState)
{
	int num_param = 1;
	int iparam[30];
	char iparams[30][10];

	iparam[0] = 0;

	if (PS3.key.PS != 0)
		{
			ToggleRobotStandby();
			while (PS3.key.PS != 0) usleep(8000);
		}

	if (robotInStandby == 1) return;
	if (PS3.key.R2 != 0)
		{
			strcpy(iparams[0], "rl");
			apState[0]++;
			apState[0] %= 2;
			OnOffCmd(arbotixpro, apState[0] ? true : false, num_param, iparam, iparams);
			while (PS3.key.R2 != 0) usleep(8000);
		}
	if (PS3.key.R1 != 0)
		{
			apState[1]++;
			apState[1] %= 2;
			strcpy(iparams[0], "ra");
			OnOffCmd(arbotixpro, apState[1] ? true : false, num_param, iparam, iparams);
			while (PS3.key.R1 != 0) usleep(8000);
		}
	if (PS3.key.L2 != 0)
		{
			apState[2]++;
			apState[2] %= 2;
			strcpy(iparams[0], "ll");
			OnOffCmd(arbotixpro, apState[2] ? true : false, num_param, iparam, iparams);
			while (PS3.key.L2 != 0) usleep(8000);
		}
	if (PS3.key.L1 != 0)
		{
			apState[3]++;
			apState[3] %= 2;
			strcpy(iparams[0], "la");
			OnOffCmd(arbotixpro, apState[3] ? true : false, num_param, iparam, iparams);
			while (PS3.key.L1 != 0) usleep(8000);
		}
	if (PS3.key.Up != 0)
		{
			apState[4]++;
			apState[4] %= 2;
			strcpy(iparams[0], "h");
			OnOffCmd(arbotixpro, apState[4] ? true : false, num_param, iparam, iparams);
			while (PS3.key.Up != 0) usleep(8000);
		}
	return;
}
