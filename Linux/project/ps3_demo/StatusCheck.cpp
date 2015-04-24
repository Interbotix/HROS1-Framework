/*
 * StatusCheck.cpp
 *
 *  Created on: 2011. 1. 21.
 *      Author: zerom, Farrell Robotics
 */
 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "StatusCheck.h"
#include "Head.h"
#include "Action.h"
#include "Walking.h"
#include "MotionStatus.h"
#include "MotionManager.h"
#include "LinuxActionScript.h"
#include "PS3Controller.h"
#include "PS3BallFollower.h"
#include "LineFollower.h"
#include "RobotFollower.h"
#include "LinuxDARwIn.h"

using namespace Robot;

int StatusCheck::m_cur_mode     = READY;
int StatusCheck::m_old_btn      = 0;
int StatusCheck::m_is_started   = 0;
int StatusCheck::m_current_walk_speed = FAST_WALK;

bool bLJState=false,bRJState=false;
minIni* StatusCheck::m_ini;
minIni* StatusCheck::m_ini1;

void StatusCheck::mPlay(int motion_page, int mode, int wait)
{
  Walking::GetInstance()->Stop();
	while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);
	m_cur_mode = mode;      
	MotionManager::GetInstance()->Reinitialize();
  MotionManager::GetInstance()->SetEnable(true);
  m_is_started = 1;
  
  Action::GetInstance()->m_Joint.SetEnableBody(true, true);

  Action::GetInstance()->Start(motion_page);
  if(wait == WAIT)
		{
		while(Action::GetInstance()->IsRunning() == true) usleep(8000);
	
		if(mode != SITTING && mode != STAIRS)
			{
			Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
			Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);
			}
		}
	return;		
}

void StatusCheck::resetLEDs(CM730 &cm730)
{
	cm730.WriteWordDelayed(CM730::P_LED_EYE_L, cm730.MakeColor(3,3,3));//cm730.MakeColor(31,0,18));        
	cm730.WriteWordDelayed(CM730::P_LED_HEAD_L, cm730.MakeColor(1,1,1));
	return;
}


void StatusCheck::Check(CM730 &cm730)
{
	int value=0;
	//toggle stnadby		
	if(PS3.key.PS != 0)
		{
		if(Walking::GetInstance()->IsRunning() == true)
			{
			Walking::GetInstance()->Stop();
			while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);
			}
		
		if(ToggleRobotStandby()==1)
			{
			//LinuxActionScript::PlayMP3("../../../Data/mp3/standby.mp3");
			}		
		// wait for key release
		while(PS3.key.PS != 0) usleep(8000);		
		}
	if(robotInStandby == 1) return;	
	if((MotionStatus::FALLEN != STANDUP) && (m_cur_mode == STAIRS) && (m_is_started == 1) && (Action::GetInstance()->IsRunning() == true))
		{
		cm730.DXLPowerOn(false);
		Action::GetInstance()->Stop();
		while(Action::GetInstance()->IsRunning() == true) usleep(8000);
		m_is_started = 0;
		}
	if(MotionStatus::FALLEN != STANDUP && (m_cur_mode == SOCCER || m_cur_mode == LINE_FOLLOWING || m_cur_mode == ROBOT_FOLLOWING) && m_is_started == 1)
  	{
    Walking::GetInstance()->Stop();
		resetLEDs(cm730);
		while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);

    Action::GetInstance()->m_Joint.SetEnableBody(true, true);

/*
    if(MotionStatus::FALLEN == FORWARD)
        Action::GetInstance()->Start(1);   // FORWARD GETUP 10
    else if(MotionStatus::FALLEN == BACKWARD)
        Action::GetInstance()->Start(1);   // BACKWARD GETUP 11
*/

    while(Action::GetInstance()->IsRunning() == 1) usleep(8000);

    Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
    Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);
		}
	if(PS3BallFollower::GetInstance()->bHeadAuto == false && PS3BallFollower::GetInstance()->bFullAuto == false && LineFollower::GetInstance()->bFullAuto == false && RobotFollower::GetInstance()->bFullAuto == false)
		{
		//shut it down		
		if(PS3.key.Cross != 0)
			{
			resetLEDs(cm730);
      Walking::GetInstance()->Stop();
			while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);
			m_is_started    = 0;
      m_cur_mode      = READY;
      LinuxActionScript::m_stop = 1;

      Head::GetInstance()->m_Joint.SetEnableBody(false);
      Walking::GetInstance()->m_Joint.SetEnableBody(false);
      Action::GetInstance()->m_Joint.SetEnableBody(true);

      while(Action::GetInstance()->Start(15) == false) usleep(8000);
      while(Action::GetInstance()->IsRunning() == true) usleep(8000);
			while(PS3.key.Cross != 0) usleep(8000);			
			}
		
		//start		
		if(PS3.key.Triangle != 0)
			{
			if(m_is_started == 0)
				{
				cm730.DXLPowerOn(true);
				}
			resetLEDs(cm730);
		  Walking::GetInstance()->Stop();
			while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);
			int lastMode = m_cur_mode;
			m_cur_mode = SOCCER;      
			MotionManager::GetInstance()->Reinitialize();
      MotionManager::GetInstance()->SetEnable(true);
      m_is_started = 1;
      bLJState = bRJState = false;
      Head::GetInstance()->m_Joint.SetEnableBody(false);
      Walking::GetInstance()->m_Joint.SetEnableBody(false);
      Action::GetInstance()->m_Joint.SetEnableBody(true);

      if(lastMode == SITTING)
				Action::GetInstance()->Start(8); //50
			else if(lastMode == STAIRS && MotionStatus::FALLEN != STANDUP)
				Action::GetInstance()->Start(8); //8
			else
				Action::GetInstance()->Start(9); //9
      while(Action::GetInstance()->IsRunning() == true) usleep(8000);

      Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true);
      Action::GetInstance()->m_Joint.SetEnableBody(false);
			usleep(50000);
      Head::GetInstance()->m_Joint.SetEnableHeadOnly(true);
			while(PS3.key.Triangle != 0) usleep(8000);			
			}



//////////////////////////////////////////////////////////////////////////////////////
/*
		//start line follow mode or robot following mode
		if(PS3.key.Circle != 0)
			{
			//resetLEDs(cm730);
			Walking::GetInstance()->Stop();
			while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);
    	switch (m_cur_mode)
				{
				case LINE_FOLLOWING:
					m_cur_mode = ROBOT_FOLLOWING;
					value = cm730.MakeColor(0,14,18);
					break;
				default:
					m_cur_mode = LINE_FOLLOWING;
					value = cm730.MakeColor(5,4,9);
					break;
				}
    	cm730.WriteWord(CM730::P_LED_EYE_L, value, 0);        
			MotionManager::GetInstance()->Reinitialize();
      MotionManager::GetInstance()->SetEnable(true);
      m_is_started = 1;
      bLJState = bRJState = false;
      Head::GetInstance()->m_Joint.SetEnableBody(false);
      Walking::GetInstance()->m_Joint.SetEnableBody(false);
      Action::GetInstance()->m_Joint.SetEnableBody(true);

      Action::GetInstance()->Start(8);
      while(Action::GetInstance()->IsRunning() == true) usleep(8000);

      Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true);
      Action::GetInstance()->m_Joint.SetEnableBody(false);
			usleep(50000);
      Head::GetInstance()->m_Joint.SetEnableHeadOnly(true);
			while(PS3.key.Circle != 0) usleep(8000);			
			}
*/
//////////////////////////////////////////////////////////////////////////////////////





//////////////////////////////////////////////////////////////////////////////////////
/*
		// load slow or normal walk 
		if(PS3.key.Square != 0)
			{
			switch(StatusCheck::m_current_walk_speed)
				{
				case SLOW_WALK:
					StatusCheck::m_current_walk_speed = FAST_WALK;
					value = cm730.MakeColor(9,6,1);
					cm730.WriteWord(CM730::P_LED_EYE_L, value, 0);
					Walking::GetInstance()->LoadINISettings(StatusCheck::m_ini);
					break;
				case FAST_WALK:
					value = cm730.MakeColor(0,14,2);
					cm730.WriteWord(CM730::P_LED_EYE_L, value, 0);
					StatusCheck::m_current_walk_speed = SLOW_WALK;
					Walking::GetInstance()->LoadINISettings(StatusCheck::m_ini1);
					break;
				}
			while(PS3.key.Square != 0) usleep(8000);			
			}
*/
//////////////////////////////////////////////////////////////////////////////////////





// Select
/*
		if(PS3.key.Select != 0) 
		{
		if(LinuxActionScript::m_is_running == 0)
			{
			m_cur_mode = SOCCER;
		      LinuxActionScript::m_stop = 0;
		      Head::GetInstance()->m_Joint.SetEnableBody(false);
		      Walking::GetInstance()->m_Joint.SetEnableBody(false);
		      Action::GetInstance()->m_Joint.SetEnableBody(true);		
                	LinuxActionScript::ScriptStart(SCRIPT_FILE_PATH1);
		      while(Action::GetInstance()->IsRunning() == true) usleep(8000);		
			}
		while(PS3.key.Select != 0) usleep(8000);
		}
*/


// Square
		if(PS3.key.Square != 0) 
		{
		if(LinuxActionScript::m_is_running == 0)
			{
			m_cur_mode = SOCCER;
		      LinuxActionScript::m_stop = 0;
		      Head::GetInstance()->m_Joint.SetEnableBody(false);
		      Walking::GetInstance()->m_Joint.SetEnableBody(false);
		      Action::GetInstance()->m_Joint.SetEnableBody(true);		
                	LinuxActionScript::ScriptStart(SCRIPT_FILE_PATH1);
		      while(Action::GetInstance()->IsRunning() == true) usleep(8000);		
			}
		while(PS3.key.Square != 0) usleep(8000);
		}

// Circle
		if(PS3.key.Circle != 0) 
		{
		if(LinuxActionScript::m_is_running == 0)
			{
			m_cur_mode = SOCCER;
		      LinuxActionScript::m_stop = 0;
		      Head::GetInstance()->m_Joint.SetEnableBody(false);
		      Walking::GetInstance()->m_Joint.SetEnableBody(false);
		      Action::GetInstance()->m_Joint.SetEnableBody(true);		
                	LinuxActionScript::ScriptStart(SCRIPT_FILE_PATH2);
		      while(Action::GetInstance()->IsRunning() == true) usleep(8000);		
			}
		while(PS3.key.Circle != 0) usleep(8000);
		}



// R1
		if(PS3.key.R1 != 0) 
		{
		if(LinuxActionScript::m_is_running == 0)
			{
			m_cur_mode = SOCCER;
		      LinuxActionScript::m_stop = 0;
		      Head::GetInstance()->m_Joint.SetEnableBody(false);
		      Walking::GetInstance()->m_Joint.SetEnableBody(false);
		      Action::GetInstance()->m_Joint.SetEnableBody(true);		
                	LinuxActionScript::ScriptStart(SCRIPT_FILE_PATH3);
		      while(Action::GetInstance()->IsRunning() == true) usleep(8000);		
			}
		while(PS3.key.R1 != 0) usleep(8000);
		}


// R2
		if(PS3.key.R2 != 0) 
		{
		if(LinuxActionScript::m_is_running == 0)
			{
			m_cur_mode = SOCCER;
		      LinuxActionScript::m_stop = 0;
		      Head::GetInstance()->m_Joint.SetEnableBody(false);
		      Walking::GetInstance()->m_Joint.SetEnableBody(false);
		      Action::GetInstance()->m_Joint.SetEnableBody(true);		
                	LinuxActionScript::ScriptStart(SCRIPT_FILE_PATH4);
		      while(Action::GetInstance()->IsRunning() == true) usleep(8000);		
			}
		while(PS3.key.R2 != 0) usleep(8000);
		}


// L1
		if(PS3.key.L1 != 0) 
		{
		if(LinuxActionScript::m_is_running == 0)
			{
			m_cur_mode = SOCCER;
		      LinuxActionScript::m_stop = 0;
		      Head::GetInstance()->m_Joint.SetEnableBody(false);
		      Walking::GetInstance()->m_Joint.SetEnableBody(false);
		      Action::GetInstance()->m_Joint.SetEnableBody(true);		
                	LinuxActionScript::ScriptStart(SCRIPT_FILE_PATH5);
		      while(Action::GetInstance()->IsRunning() == true) usleep(8000);		
			}
		while(PS3.key.L1 != 0) usleep(8000);
		}


// L2
		if(PS3.key.L2 != 0) 
		{
		if(LinuxActionScript::m_is_running == 0)
			{
			m_cur_mode = SOCCER;
		      LinuxActionScript::m_stop = 0;
		      Head::GetInstance()->m_Joint.SetEnableBody(false);
		      Walking::GetInstance()->m_Joint.SetEnableBody(false);
		      Action::GetInstance()->m_Joint.SetEnableBody(true);		
                	LinuxActionScript::ScriptStart(SCRIPT_FILE_PATH6);
		      while(Action::GetInstance()->IsRunning() == true) usleep(8000);		
			}
		while(PS3.key.L2 != 0) usleep(8000);
		}






// Circle
	
/*
		if(PS3.key.Circle != 0) 
			mPlay(20,SOCCER,DONT_WAIT);



		if(PS3.key.Circle != 0) 
		{
		if(LinuxActionScript::m_is_running == 0)
			{
			m_cur_mode = SOCCER;
		      LinuxActionScript::m_stop = 0;
		      Head::GetInstance()->m_Joint.SetEnableBody(false);
		      Walking::GetInstance()->m_Joint.SetEnableBody(false);
		      Action::GetInstance()->m_Joint.SetEnableBody(true);		
                	LinuxActionScript::ScriptStart(SCRIPT_FILE_PATH);
		      while(Action::GetInstance()->IsRunning() == true) usleep(8000);		
			}
		while(PS3.key.Circle != 0) usleep(8000);
		}
*/



/*
		//right kick		
		if(PS3.key.L1 != 0) 
			mPlay(20,SOCCER,DONT_WAIT);
		//right kick		
		if(PS3.key.R1 != 0) 
			mPlay(25,SOCCER,DONT_WAIT);


/*
//////////////////////////////////////////////////////////////////////////////////////
/*
// Select

		if(PS3.key.Select != 0) 
		{
		if(LinuxActionScript::m_is_running == 0)
			{
			m_cur_mode = SOCCER;
		      LinuxActionScript::m_stop = 0;
		      Head::GetInstance()->m_Joint.SetEnableBody(false);
		      Walking::GetInstance()->m_Joint.SetEnableBody(false);
		      Action::GetInstance()->m_Joint.SetEnableBody(true);		
                	LinuxActionScript::ScriptStart(SCRIPT_FILE_PATH);
		      while(Action::GetInstance()->IsRunning() == true) usleep(8000);		
			}
		while(PS3.key.Select != 0) usleep(8000);
		}
		
		//right kick		
		if(PS3.key.R2 != 0) 
			mPlay(12);
		//left kick		
		if(PS3.key.L2 != 0) 
			mPlay(13);
		//right pass		
		if(PS3.key.R1 != 0) 
			mPlay(70);
		//left pass		
		if(PS3.key.L1 != 0) 
			mPlay(71);
		
		//go up stairs		
		if(PS3.key.L1 != 0) 
			mPlay(34,STAIRS,DONT_WAIT);
		//go down stairs first step		
		if(PS3.key.R1 != 0) 
			mPlay(40,STAIRS,DONT_WAIT);
		//go down stairs 2nd and beyond		
		if(PS3.key.R2 != 0) 
			mPlay(59,STAIRS,DONT_WAIT);

*/

//////////////////////////////////////////////////////////////////////////////////////
/*
		// sit down
		if(PS3.key.Right != 0) 
			{
			if(m_cur_mode == SITTING)
				{
				//get up from sitting
				//mPlay(50); 
				// it already sitting toggle dynamixel power on and off
				// power down dynamixels
				if(m_is_started == 1)
					{
					cm730.DXLPowerOn(false);
					m_is_started = 0;
					}
				else
					{
					cm730.DXLPowerOn(true);
					m_is_started = 0;
					}
				}
			else
				{
				// sit down
				mPlay(15, SITTING); //48
				}
			// wait for key release
			while(PS3.key.Right != 0) usleep(8000);
			}
		
		
		if(PS3.key.Left != 0) 
			mPlay(34);// 34 up stairs was 11 get up
		
		//wave right_side		
		
		if(PS3.key.Square != 0) 
			{
			mPlay(38);
			}
		

*/

// PS3 R/C Control code
//////////////////////////////////////////////////////////////////////////////////////
		if(Walking::GetInstance()->IsRunning() == true)
			{
			int x,y,x1,dead_band=5;			
			double FBStep=0,RLTurn=0,RLStep=0,xd,yd;
			static double speedAdjSum=0;			
			x = -(PS3.key.RJoyX-128);
			y = -(PS3.key.RJoyY-128);
			x1 = -(PS3.key.LJoyX-128);
			
			if(abs(x) > dead_band || abs(y) > dead_band || abs(x1) > dead_band)
				{
				xd = (double)(x-dead_band)/256;
				yd = (double)(y-dead_band)/256;
				RLTurn = 30*xd;	
				FBStep = 20*yd;//45	
				if(FBStep < 0)
//					FBStep = 15*yd;
					FBStep = 0;
//				if(bLJState == false)
//					RLStep = 52*xd;		
 				speedAdjSum += yd;
				if(speedAdjSum > Walking::GetInstance()->UPPER_VELADJ_LIMIT) 
					speedAdjSum = Walking::GetInstance()->UPPER_VELADJ_LIMIT;
				else if(speedAdjSum < Walking::GetInstance()->LOWER_VELADJ_LIMIT) 
					speedAdjSum = Walking::GetInstance()->LOWER_VELADJ_LIMIT;
				}
			else
				speedAdjSum = 0;
			Walking::GetInstance()->speedAdj = speedAdjSum;
			//Walking::GetInstance()->X_OFFSET = Walking::GetInstance()->X_OFFSET_START - speedAdj;
			Walking::GetInstance()->X_MOVE_AMPLITUDE = FBStep;
			Walking::GetInstance()->Y_MOVE_AMPLITUDE = RLStep;
			Walking::GetInstance()->A_MOVE_AMPLITUDE = RLTurn;			
			}
		}
	else //things only done in auto mode
		{
		}
//////////////////////////////////////////////////////////////////////////////////////



// STOP WALKING GAIT ENGINE
//////////////////////////////////////////////////////////////////////////////////////

	if(Walking::GetInstance()->IsRunning() == true && PS3.key.Down != 0)
		{
		//fprintf(stderr, "STOPPING\n");		
		resetLEDs(cm730);
		Walking::GetInstance()->Stop();
		while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);
		//while(Walking::GetInstance()->IsRunning() == true);
	
		}
//////////////////////////////////////////////////////////////////////////////////////



// START WALKING GAIT ENGINE
//////////////////////////////////////////////////////////////////////////////////////
	if(Walking::GetInstance()->IsRunning() == false && PS3.key.Up != 0)
		{
		//fprintf(stderr, "STARTING\n");		
		Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
		Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);
		
		Walking::GetInstance()->X_MOVE_AMPLITUDE = 0;
		Walking::GetInstance()->Y_MOVE_AMPLITUDE = 0;
		Walking::GetInstance()->A_MOVE_AMPLITUDE = 0;
		Walking::GetInstance()->Start();			
		}
//////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////////
	// toggle head from auto to not	

	if(PS3.key.LeftHat != 0)
		{
		if(bLJState == true)
			{
			bLJState = false;
			if(m_cur_mode == SOCCER)
				PS3BallFollower::GetInstance()->bHeadAuto = false;
			else if(m_cur_mode == SITTING)
				LineFollower::GetInstance()->bHeadAuto = false;
			else if(m_cur_mode == LINE_FOLLOWING)
				LineFollower::GetInstance()->bHeadAuto = false;
			else if(m_cur_mode == ROBOT_FOLLOWING)
				RobotFollower::GetInstance()->bHeadAuto = false;

			//double pan,tilt;
			//pan = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
			//tilt = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_TILT);	
			//Head::GetInstance()->MoveByAngle(pan,tilt);
			}
		else
			{
			bLJState = true;
			if(m_cur_mode == SOCCER)
				{
				if(PS3BallFollower::GetInstance()->bFullAuto == true)
					PS3BallFollower::GetInstance()->bHeadAuto = true;
				else
					PS3BallFollower::GetInstance()->bHeadAuto = false;
				}
			else if(m_cur_mode == SITTING)
				{
				LineFollower::GetInstance()->bHeadAuto = true;
				}
			else if(m_cur_mode == LINE_FOLLOWING)
				{
				if(LineFollower::GetInstance()->bFullAuto == true)
					LineFollower::GetInstance()->bHeadAuto = true;
				else
					LineFollower::GetInstance()->bHeadAuto = false;
				}
			else if(m_cur_mode == ROBOT_FOLLOWING)
				{
				if(RobotFollower::GetInstance()->bFullAuto == true)
					RobotFollower::GetInstance()->bHeadAuto = true;
				else
					RobotFollower::GetInstance()->bHeadAuto = false;
				}
			}
		PS3Vibrate();		
		// wait for key release
		while(PS3.key.LeftHat != 0) usleep(8000);		
		}


//////////////////////////////////////////////////////////////////////////////////////




// PS3 VISION GUIDE MODE
//////////////////////////////////////////////////////////////////////////////////////	

	if((PS3BallFollower::GetInstance()->bHeadAuto == false && (m_cur_mode == SOCCER || m_cur_mode == SITTING)) \
		|| (LineFollower::GetInstance()->bFullAuto == true && LineFollower::GetInstance()->bHeadAuto == false && m_cur_mode == LINE_FOLLOWING) \
		|| (RobotFollower::GetInstance()->bFullAuto == true && RobotFollower::GetInstance()->bHeadAuto == false && m_cur_mode == ROBOT_FOLLOWING))
		{
		int x,y,dead_band=6;
		double pan,tilt;
		pan = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_PAN);
		tilt = MotionStatus::m_CurrentJoints.GetAngle(JointData::ID_HEAD_TILT);	
		Point2D pos = Point2D(pan,tilt);
		x = -(PS3.key.LJoyX-128);
		y = -(PS3.key.LJoyY-128);
		if(abs(x) > dead_band || abs(y) > dead_band)
			{
			pos.X = pan + 0.2*Camera::VIEW_V_ANGLE*(x-dead_band)/256;
			pos.Y = tilt + 0.2*Camera::VIEW_H_ANGLE*(y-dead_band)/256;		
			}
		Head::GetInstance()->MoveByAngle(pos.X,pos.Y);			
		//Head::GetInstance()->MoveTracking(pos);			
		}

//////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////

	if(PS3.key.Start != 0)
		{
		resetLEDs(cm730);
		if(m_cur_mode == SOCCER || m_cur_mode == SITTING)
			{
			if(PS3BallFollower::GetInstance()->bFullAuto == true || PS3BallFollower::GetInstance()->bHeadAuto == true)
				{
				PS3BallFollower::GetInstance()->bFullAuto = false;
				PS3BallFollower::GetInstance()->bHeadAuto = false;
				SetPS3LEDFlashRate(0);
				bLJState = false;
				}
			else
				{
				Head::GetInstance()->SetTopLimitAngle(Head::GetInstance()->m_TopLimit_soccer);
				if(m_cur_mode == SOCCER) PS3BallFollower::GetInstance()->bFullAuto = true;
				PS3BallFollower::GetInstance()->bHeadAuto = true;
				SetPS3LEDFlashRate(3);
				bLJState = true;
				}
			}
		else if(m_cur_mode == LINE_FOLLOWING) 
			{
			if(LineFollower::GetInstance()->bFullAuto == true)
				{
				LineFollower::GetInstance()->bFullAuto = false;
				LineFollower::GetInstance()->bHeadAuto = false;
				SetPS3LEDFlashRate(0);
				bLJState = false;
				}
			else
				{
				Head::GetInstance()->SetTopLimitAngle(Head::GetInstance()->m_TopLimit_line_following);
				LineFollower::GetInstance()->bFullAuto = true;
				LineFollower::GetInstance()->bHeadAuto = true;
				SetPS3LEDFlashRate(3);
				bLJState = true;
				}
			}
		else if(m_cur_mode == ROBOT_FOLLOWING) 
			{
			if(RobotFollower::GetInstance()->bFullAuto == true)
				{
				RobotFollower::GetInstance()->bFullAuto = false;
				RobotFollower::GetInstance()->bHeadAuto = false;
				SetPS3LEDFlashRate(0);
				bLJState = false;
				}
			else
				{
				Head::GetInstance()->SetTopLimitAngle(Head::GetInstance()->m_TopLimit_robot_following);
				RobotFollower::GetInstance()->bFullAuto = true;
				RobotFollower::GetInstance()->bHeadAuto = true;
				SetPS3LEDFlashRate(3);
				bLJState = true;
				}
			}
		// wait for key release
		while(PS3.key.Start != 0) usleep(8000);		
		}

//////////////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////////////	

	if(m_old_btn == MotionStatus::BUTTON)
      return;


  m_old_btn = MotionStatus::BUTTON;

  if(m_old_btn & BTN_MODE)
    {
    fprintf(stderr, "Mode button pressed.. \n");

    if(m_is_started == 1)
    	{
			resetLEDs(cm730);
			Walking::GetInstance()->Stop();
			while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);
			MotionManager::GetInstance()->Reinitialize();
			MotionManager::GetInstance()->SetEnable(true);

			m_is_started    = 0;
      m_cur_mode      = READY;
      LinuxActionScript::m_stop = 1;

      Action::GetInstance()->m_Joint.SetEnableBody(true, true);

      while(Action::GetInstance()->Start(15) == false) usleep(8000);
      while(Action::GetInstance()->IsRunning() == true) usleep(8000);
    	}
  	else
    	{
    	m_cur_mode++;
    	if(m_cur_mode >= MAX_MODE) m_cur_mode = READY;
    	}

		MotionManager::GetInstance()->SetEnable(false);
		usleep(10000);

    if(m_cur_mode == READY)
      {
      cm730.WriteByte(CM730::P_LED_PANNEL, 0x01|0x02|0x04, NULL);
			LinuxActionScript::PlayMP3("../../../Data/mp3/ready.mp3");
     	}
    else if(m_cur_mode == SOCCER)
      {
      cm730.WriteByte(CM730::P_LED_PANNEL, 0x01, NULL);
    	int value = cm730.MakeColor(14,14,0);
    	cm730.WriteWord(CM730::P_LED_EYE_L, value, 0);        
         LinuxActionScript::PlayMP3("../../../Data/mp3/Autonomous soccer mode.mp3");
			}
    else if(m_cur_mode == LINE_FOLLOWING)
      {
      cm730.WriteByte(CM730::P_LED_PANNEL, 0x02, NULL);
      LinuxActionScript::PlayMP3("../../../Data/mp3/line following mode.mp3");
			}

		else if(m_cur_mode == ROBOT_FOLLOWING)
      {
      cm730.WriteByte(CM730::P_LED_PANNEL, 0x03, NULL);
      LinuxActionScript::PlayMP3("../../../Data/mp3/robot following mode.mp3");
			}
    else if(m_cur_mode == MOTION)
      {
      cm730.WriteByte(CM730::P_LED_PANNEL, 0x04, NULL);
      LinuxActionScript::PlayMP3("../../../Data/mp3/Interactive motion mode.mp3");
      }
    else if(m_cur_mode == VISION)
      {
      cm730.WriteByte(CM730::P_LED_PANNEL, 0x05, NULL);
      LinuxActionScript::PlayMP3("../../../Data/mp3/Vision processing mode.mp3");
      }
		}

  if(m_old_btn & BTN_START)
    {
    if(m_is_started == 0)
      {
      fprintf(stderr, "Start button pressed.. & started is false.. \n");

      if(m_cur_mode == SOCCER)
        {
				MotionManager::GetInstance()->ResetGyroCalibration();
				while(1)
					{
					if(MotionManager::GetInstance()->GetCalibrationStatus() == 1)
						{
						LinuxActionScript::PlayMP3("../../../Data/mp3/Sensor calibration complete.mp3");
						break;
						}
					else if(MotionManager::GetInstance()->GetCalibrationStatus() == -1)
						{
						LinuxActionScript::PlayMP3Wait("../../../Data/mp3/Sensor calibration fail.mp3");
						MotionManager::GetInstance()->ResetGyroCalibration();
						}
					usleep(8000);
					}      
        MotionManager::GetInstance()->Reinitialize();
        MotionManager::GetInstance()->SetEnable(true);
        m_is_started = 1;
        LinuxActionScript::PlayMP3("../../../Data/mp3/Start soccer demonstration.mp3");

        Action::GetInstance()->m_Joint.SetEnableBody(true, true);

        Action::GetInstance()->Start(9);
        while(Action::GetInstance()->IsRunning() == true) usleep(8000);

        Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true, true);
				usleep(50000);
        Head::GetInstance()->m_Joint.SetEnableHeadOnly(true, true);
				Head::GetInstance()->SetTopLimitAngle(Head::GetInstance()->m_TopLimit_soccer);
				PS3BallFollower::GetInstance()->bFullAuto = true;
				PS3BallFollower::GetInstance()->bHeadAuto = true;
        }
      else if(m_cur_mode == MOTION)
        {
        MotionManager::GetInstance()->Reinitialize();
        MotionManager::GetInstance()->SetEnable(true);
        m_is_started = 1;
        LinuxActionScript::PlayMP3("../../../Data/mp3/Start motion demonstration.mp3");

        // Joint Enable..
        Action::GetInstance()->m_Joint.SetEnableBody(true, true);

        Action::GetInstance()->Start(1);
        while(Action::GetInstance()->IsRunning() == true) usleep(8000);
        }
      else if(m_cur_mode == VISION)
        {
        MotionManager::GetInstance()->Reinitialize();
        MotionManager::GetInstance()->SetEnable(true);
        m_is_started = 1;
        LinuxActionScript::PlayMP3("../../../Data/mp3/Start vision processing demonstration.mp3");

        // Joint Enable...
        Action::GetInstance()->m_Joint.SetEnableBody(true, true);

        Action::GetInstance()->Start(1);
        while(Action::GetInstance()->IsRunning() == true) usleep(8000);
        }
			else if(m_cur_mode == LINE_FOLLOWING)
				{
				resetLEDs(cm730);
				Walking::GetInstance()->Stop();
				while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);
				MotionManager::GetInstance()->Reinitialize();
				MotionManager::GetInstance()->SetEnable(true);
				m_is_started = 1;
				bLJState = bRJState = false;
				Head::GetInstance()->m_Joint.SetEnableBody(false);
				Walking::GetInstance()->m_Joint.SetEnableBody(false);
				Action::GetInstance()->m_Joint.SetEnableBody(true);

				Action::GetInstance()->Start(9);
				while(Action::GetInstance()->IsRunning() == true) usleep(8000);

				Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true);
				Action::GetInstance()->m_Joint.SetEnableBody(false);
				usleep(5000000);//50000
				Head::GetInstance()->m_Joint.SetEnableHeadOnly(true);
				// also place in full auto
				Head::GetInstance()->SetTopLimitAngle(Head::GetInstance()->m_TopLimit_line_following);
				LineFollower::GetInstance()->bFullAuto = true;
				LineFollower::GetInstance()->bHeadAuto = true;
				SetPS3LEDFlashRate(3);
				bLJState = true;
				}

			else if(m_cur_mode == ROBOT_FOLLOWING)
				{
				resetLEDs(cm730);
				Walking::GetInstance()->Stop();
				while(Walking::GetInstance()->IsRunning() == 1) usleep(8000);
				MotionManager::GetInstance()->Reinitialize();
				MotionManager::GetInstance()->SetEnable(true);
				m_is_started = 1;
				bLJState = bRJState = false;
				Head::GetInstance()->m_Joint.SetEnableBody(false);
				Walking::GetInstance()->m_Joint.SetEnableBody(false);
				Action::GetInstance()->m_Joint.SetEnableBody(true);

				Action::GetInstance()->Start(9);
				while(Action::GetInstance()->IsRunning() == true) usleep(8000);

				Walking::GetInstance()->m_Joint.SetEnableBodyWithoutHead(true);
				Action::GetInstance()->m_Joint.SetEnableBody(false);
				usleep(5000000);//50000
				Head::GetInstance()->m_Joint.SetEnableHeadOnly(true);
				// also place in full auto
				Head::GetInstance()->SetTopLimitAngle(Head::GetInstance()->m_TopLimit_robot_following);
				RobotFollower::GetInstance()->bFullAuto = true;
				RobotFollower::GetInstance()->bHeadAuto = true;
				SetPS3LEDFlashRate(3);
				bLJState = true;
				}
      }
    else
      {
      fprintf(stderr, "Start button pressed.. & started is true.. \n");
      }
    }
}
