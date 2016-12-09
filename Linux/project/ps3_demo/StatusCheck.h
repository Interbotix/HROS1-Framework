/*
 * StatusCheck.h
 *
 *  Created on: 2011. 1. 21.
 *      Author: zerom
 */

#ifndef STATUSCHECK_H_
#define STATUSCHECK_H_

#include "ArbotixPro.h"
#include "minIni.h"
#include "JoystickController.h"

#define SCRIPT_FILE_PATH_TRIANGLE    "action_scripts/Triangle.asc"
#define SCRIPT_FILE_PATH_CIRCLE   "action_scripts/Circle.asc"
#define SCRIPT_FILE_PATH_CROSS   "action_scripts/Cross.asc"
#define SCRIPT_FILE_PATH_SQUARE   "action_scripts/Square.asc"
#define SCRIPT_FILE_PATH_R1   "action_scripts/R1.asc"
#define SCRIPT_FILE_PATH_R2   "action_scripts/R2.asc"
#define SCRIPT_FILE_PATH_L1   "action_scripts/L1.asc"
#define SCRIPT_FILE_PATH_L2   "action_scripts/L2.asc"
#define SCRIPT_FILE_PATH_SELECT   "action_scripts/SelectButton.asc"
#define SCRIPT_FILE_PATH_START	"action_scripts/StartButton.asc"

namespace Robot
{
	enum
	{
		INITIAL,
		READY,
		SITTING,
		SOCCER,
		LINE_FOLLOWING,
		ROBOT_FOLLOWING,
		MOTION,
		VISION,
		STAIRS,
		MAX_MODE
	};

	enum
	{
		BTN_MODE = 1,
		BTN_START = 2
	};

	enum
	{
		FAST_WALK,
		MEDIUM_WALK,
		SLOW_WALK
	};

	enum
	{
		WAIT,
		DONT_WAIT
	};

	class StatusCheck
	{
		private:
			static int m_old_btn;
			static void mPlay(int motion_page, int mode = SOCCER, int wait = WAIT);
		public:
			static int m_cur_mode;
			static int m_is_started;
			static int m_current_walk_speed;
			static minIni* m_ini;
			static minIni* m_ini1;

			static void Check(LinuxJoy &ljoy, ArbotixPro &arbotixpro);

	};
}
#endif /* STATUSCHECK_H_ */
