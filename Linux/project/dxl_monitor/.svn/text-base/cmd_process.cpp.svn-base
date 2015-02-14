#include <stdio.h>
#include <string.h>
#include "cmd_process.h"

using namespace Robot;

extern int gID;

const char *GetIDString(int id)
{
	switch(id)
	{
	case JointData::ID_R_SHOULDER_PITCH:
		return "R_SHOULDER_PITCH";

	case JointData::ID_L_SHOULDER_PITCH:
		return "L_SHOULDER_PITCH";

	case JointData::ID_R_SHOULDER_ROLL:
		return "R_SHOULDER_ROLL";

	case JointData::ID_L_SHOULDER_ROLL:
		return "L_SHOULDER_ROLL";

	case JointData::ID_R_ELBOW:
		return "R_ELBOW";

	case JointData::ID_L_ELBOW:
		return "L_ELBOW";

	case JointData::ID_R_HIP_YAW:
		return "R_HIP_YAW";

	case JointData::ID_L_HIP_YAW:
		return "L_HIP_YAW";

	case JointData::ID_R_HIP_ROLL:
		return "R_HIP_ROLL";

	case JointData::ID_L_HIP_ROLL:
		return "L_HIP_ROLL";

	case JointData::ID_R_HIP_PITCH:
		return "R_HIP_PITCH";

	case JointData::ID_L_HIP_PITCH:
		return "L_HIP_PITCH";

	case JointData::ID_R_KNEE:
		return "R_KNEE";

	case JointData::ID_L_KNEE:
		return "L_KNEE";

	case JointData::ID_R_ANKLE_PITCH:
		return "R_ANKLE_PITCH";

	case JointData::ID_L_ANKLE_PITCH:
		return "L_ANKLE_PITCH";

	case JointData::ID_R_ANKLE_ROLL:
		return "R_ANKLE_ROLL";

	case JointData::ID_L_ANKLE_ROLL:
		return "L_ANKLE_ROLL";

	case JointData::ID_HEAD_PAN:
		return "HEAD_PAN";

	case JointData::ID_HEAD_TILT:
		return "HEAD_TILT";

	case CM730::ID_CM:
		return "SUB_BOARD";
	}

	return "UNKNOWN";
}

void Prompt(int id)
{
	printf( "\r[ID:%d(%s)] ", id, GetIDString(id) );
}

void Help()
{
	printf( "\n" );
	printf( " exit : Exits the program\n" );
	printf( " scan : Outputs the current status of all Dynamixels\n" );
	printf( " id [ID] : Go to [ID]\n" );
	printf( " d : Dumps the current control table of CM-730 and all Dynamixels\n" );
	printf( " reset : Defaults the value of current Dynamixel\n" );
	printf( " reset all : Defaults the value of all Dynamixels\n" );
	printf( " wr [ADDR] [VALUE] : Writes value [VALUE] to address [ADDR] of current Dynamixel\n" );
	printf( " on/off : Turns torque on/off of current Dynamixel\n" );
	printf( " on/off all : Turns torque on/off of all Dynamixels)\n" );
	printf( "\n       Copyright ROBOTIS CO.,LTD.\n\n" );
}

void Scan(CM730 *cm730)
{
	printf("\n");

	for(int id=1; id<254; id++)
	{
        if(cm730->Ping(id, 0) == CM730::SUCCESS)
        {
            printf("                                  ... OK\r");
            printf(" Check ID:%d(%s)\n", id, GetIDString(id));
        }
        else if(id < JointData::NUMBER_OF_JOINTS || id == CM730::ID_CM)
        {
            printf("                                  ... FAIL\r");
            printf(" Check ID:%d(%s)\n", id, GetIDString(id));
        }
	}

	printf("\n");
}

void Dump(CM730 *cm730, int id)
{
	unsigned char table[128];
	int addr;
	int value;


	if(id == CM730::ID_CM) // Sub board
	{
		if(cm730->ReadTable(id, CM730::P_MODEL_NUMBER_L, CM730::P_RIGHT_MIC_H, &table[CM730::P_MODEL_NUMBER_L], 0) != CM730::SUCCESS)
		{
			printf(" Can not read table!\n");
			return;
		}

		printf( "\n" );
		printf( " [EEPROM AREA]\n" );
		addr = CM730::P_MODEL_NUMBER_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " MODEL_NUMBER            (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = CM730::P_VERSION; value = table[addr];
		printf( " VERSION                 (R) [%.3d]:%5d\n", addr, value);
		addr = CM730::P_ID; value = table[addr];
		printf( " ID                     (R/W)[%.3d]:%5d\n", addr, value);
		addr = CM730::P_BAUD_RATE; value = table[addr];
		printf( " BAUD_RATE              (R/W)[%.3d]:%5d\n", addr, value);
		addr = CM730::P_RETURN_DELAY_TIME; value = table[addr];
		printf( " RETURN_DELAY_TIME      (R/W)[%.3d]:%5d\n", addr, value);
		addr = CM730::P_RETURN_LEVEL; value = table[addr];
		printf( " RETURN_LEVEL           (R/W)[%.3d]:%5d\n", addr, value);
		printf( "\n" );
		printf( " [RAM AREA]\n" );
		addr = CM730::P_DXL_POWER; value = table[addr];
		printf( " DXL_POWER              (R/W)[%.3d]:%5d\n", addr, value);
		addr = CM730::P_LED_PANNEL; value = table[addr];
		printf( " LED_PANNEL             (R/W)[%.3d]:%5d\n", addr, value);
		addr = CM730::P_LED_HEAD_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " LED_HEAD               (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = CM730::P_LED_EYE_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " LED_EYE                (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = CM730::P_BUTTON; value = table[addr];
		printf( " BUTTON                  (R) [%.3d]:%5d\n", addr, value);
		addr = CM730::P_GYRO_Z_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " GYRO_Z                  (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = CM730::P_GYRO_Y_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " GYRO_Y                  (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = CM730::P_GYRO_X_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " GYRO_X                  (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = CM730::P_ACCEL_X_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " ACCEL_X                 (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = CM730::P_ACCEL_Y_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " ACCEL_Y                 (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = CM730::P_ACCEL_Z_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " ACCEL_Z                 (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
        addr = CM730::P_VOLTAGE; value = table[addr];
        printf( " VOLTAGE                 (R) [%.3d]:%5d\n", addr, value);
		addr = CM730::P_LEFT_MIC_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " LEFT_MIC                (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = CM730::P_RIGHT_MIC_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " RIGHT_MIC               (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);

		printf( "\n" );
	}
	else // Actuator
	{		
		if(cm730->ReadTable(id, MX28::P_MODEL_NUMBER_L, MX28::P_PUNCH_H, &table[MX28::P_MODEL_NUMBER_L], 0) != CM730::SUCCESS)
		{
			printf(" Can not read table!\n");
			return;
		}

		printf( "\n" );
		printf( " [EEPROM AREA]\n" );
		addr = MX28::P_MODEL_NUMBER_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " MODEL_NUMBER            (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = MX28::P_VERSION; value = table[addr];
		printf( " VERSION                 (R) [%.3d]:%5d\n", addr, value);
		addr = MX28::P_ID; value = table[addr];
		printf( " ID                     (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_BAUD_RATE; value = table[addr];
		printf( " BAUD_RATE              (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_RETURN_DELAY_TIME; value = table[addr];
		printf( " RETURN_DELAY_TIME      (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_CW_ANGLE_LIMIT_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " CW_ANGLE_LIMIT         (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = MX28::P_CCW_ANGLE_LIMIT_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " CCW_ANGLE_LIMIT        (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = MX28::P_HIGH_LIMIT_TEMPERATURE; value = table[addr];
		printf( " HIGH_LIMIT_TEMPERATURE (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_LOW_LIMIT_VOLTAGE; value = table[addr];
		printf( " LOW_LIMIT_VOLTAGE      (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_HIGH_LIMIT_VOLTAGE; value = table[addr];
		printf( " HIGH_LIMIT_VOLTAGE     (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_MAX_TORQUE_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " MAX_TORQUE             (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = MX28::P_RETURN_LEVEL; value = table[addr];
		printf( " RETURN_LEVEL           (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_ALARM_LED; value = table[addr];
		printf( " ALARM_LED              (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_ALARM_SHUTDOWN; value = table[addr];
		printf( " ALARM_SHUTDOWN         (R/W)[%.3d]:%5d\n", addr, value);
		printf( "\n" );
		printf( " [RAM AREA]\n" );
		addr = MX28::P_TORQUE_ENABLE; value = table[addr];
		printf( " TORQUE_ENABLE          (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_LED; value = table[addr];
		printf( " LED                    (R/W)[%.3d]:%5d\n", addr, value);
#ifdef MX28_1024
		addr = MX28::P_CW_COMPLIANCE_MARGIN; value = table[addr];
		printf( " CW_COMPLIANCE_MARGIN   (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_CCW_COMPLIANCE_MARGIN; value = table[addr];
		printf( " CCW_COMPLIANCE_MARGIN  (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_CW_COMPLIANCE_SLOPE; value = table[addr];
		printf( " CW_COMPLIANCE_SLOPE    (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_CCW_COMPLIANCE_SLOPE; value = table[addr];
		printf( " CCW_COMPLIANCE_SLOPE   (R/W)[%.3d]:%5d\n", addr, value);
#else
        addr = MX28::P_P_GAIN; value = table[addr];
        printf( " P_GAIN                 (R/W)[%.3d]:%5d\n", addr, value);
        addr = MX28::P_I_GAIN; value = table[addr];
        printf( " I_GAIN                 (R/W)[%.3d]:%5d\n", addr, value);
        addr = MX28::P_D_GAIN; value = table[addr];
        printf( " D_GAIN                 (R/W)[%.3d]:%5d\n", addr, value);
        addr = MX28::P_RESERVED; value = table[addr];
        printf( " RESERVED               (R/W)[%.3d]:%5d\n", addr, value);
#endif
		addr = MX28::P_GOAL_POSITION_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " GOAL_POSITION          (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = MX28::P_MOVING_SPEED_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " MOVING_SPEED           (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = MX28::P_TORQUE_LIMIT_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " TORQUE_LIMIT           (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = MX28::P_PRESENT_POSITION_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " PRESENT_POSITION       (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = MX28::P_PRESENT_SPEED_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " PRESENT_SPEED          (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = MX28::P_PRESENT_LOAD_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " PRESENT_LOAD           (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);
		addr = MX28::P_PRESENT_VOLTAGE; value = table[addr];
		printf( " PRESENT_VOLTAGE        (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_PRESENT_TEMPERATURE; value = table[addr];
		printf( " PRESENT_TEMPERATURE    (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_REGISTERED_INSTRUCTION; value = table[addr];
		printf( " REGISTERED_INSTRUC     (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_MOVING; value = table[addr];
		printf( " MOVING                 (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_LOCK; value = table[addr];
		printf( " LOCK                   (R/W)[%.3d]:%5d\n", addr, value);
		addr = MX28::P_PUNCH_L; value = CM730::MakeWord(table[addr], table[addr+1]);
		printf( " PUNCH                  (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr+1]);

		printf( "\n" );
	}
}

void Reset(Robot::CM730 *cm730, int id)
{
	int FailCount = 0;
	int FailMaxCount = 10;
	printf(" Reset ID:%d...", id);

	if(cm730->Ping(id, 0) != CM730::SUCCESS)
	{
		printf("Fail\n");
		return;
	}

	FailCount = 0;
	while(1)
	{
		if(cm730->WriteByte(id, MX28::P_RETURN_DELAY_TIME, 0, 0) == CM730::SUCCESS)
			break;

		FailCount++;
		if(FailCount > FailMaxCount)
		{
			printf("Fail\n");
			return;
		}
		usleep(10000);
	}

	FailCount = 0;
	while(1)
	{
		if(cm730->WriteByte(id, MX28::P_RETURN_LEVEL, 2, 0) == CM730::SUCCESS)
			break;

		FailCount++;
		if(FailCount > FailMaxCount)
		{
			printf("Fail\n");
			return;
		}
		usleep(10000);
	}

	if(id != CM730::ID_CM)
	{
		double cwLimit = MX28::MIN_ANGLE;
		double ccwLimit = MX28::MAX_ANGLE;

		switch(id)
		{
		case JointData::ID_R_SHOULDER_ROLL:
			cwLimit = -75.0;
			ccwLimit = 135.0;
			break;

		case JointData::ID_L_SHOULDER_ROLL:
			cwLimit = -135.0;
			ccwLimit = 75.0;
			break;

		case JointData::ID_R_ELBOW:
			cwLimit = -95.0;
			ccwLimit = 70.0;
			break;

		case JointData::ID_L_ELBOW:
			cwLimit = -70.0;
			ccwLimit = 95.0;
			break;

		case JointData::ID_R_HIP_YAW:
            cwLimit = -123.0;
            ccwLimit = 53.0;
            break;

		case JointData::ID_L_HIP_YAW:
			cwLimit = -53.0;
			ccwLimit = 123.0;
			break;

		case JointData::ID_R_HIP_ROLL:
			cwLimit = -45.0;
			ccwLimit = 59.0;
			break;

		case JointData::ID_L_HIP_ROLL:
			cwLimit = -59.0;
			ccwLimit = 45.0;
			break;

		case JointData::ID_R_HIP_PITCH:
			cwLimit = -100.0;
			ccwLimit = 29.0;
			break;

		case JointData::ID_L_HIP_PITCH:
			cwLimit = -29.0;
			ccwLimit = 100.0;
			break;

		case JointData::ID_R_KNEE:
			cwLimit = -6.0;
			ccwLimit = 130.0;
			break;

		case JointData::ID_L_KNEE:
			cwLimit = -130.0;
			ccwLimit = 6.0;
			break;

		case JointData::ID_R_ANKLE_PITCH:
			cwLimit = -72.0;
			ccwLimit = 80.0;
			break;

		case JointData::ID_L_ANKLE_PITCH:
			cwLimit = -80.0;
			ccwLimit = 72.0;
			break;

		case JointData::ID_R_ANKLE_ROLL:
			cwLimit = -44.0;
			ccwLimit = 63.0;
			break;

		case JointData::ID_L_ANKLE_ROLL:
			cwLimit = -63.0;
			ccwLimit = 44.0;
			break;

		case JointData::ID_HEAD_TILT:
			cwLimit = -25.0;
			ccwLimit = 55.0;
			break;
		}
		
		FailCount = 0;
		while(1)
		{
			if(cm730->WriteWord(id, MX28::P_CW_ANGLE_LIMIT_L, MX28::Angle2Value(cwLimit), 0) == CM730::SUCCESS)
				break;

			FailCount++;
			if(FailCount > FailMaxCount)
			{
				printf("Fail\n");
				return;
			}
			usleep(10000);
		}		
		FailCount = 0;
		while(1)
		{
			if(cm730->WriteWord(id, MX28::P_CCW_ANGLE_LIMIT_L, MX28::Angle2Value(ccwLimit), 0) == CM730::SUCCESS)
				break;

			FailCount++;
			if(FailCount > FailMaxCount)
			{
				printf("Fail\n");
				return;
			}
			usleep(10000);
		}		
		FailCount = 0;
		while(1)
		{
			if(cm730->WriteByte(id, MX28::P_HIGH_LIMIT_TEMPERATURE, 80, 0) == CM730::SUCCESS)
				break;

			FailCount++;
			if(FailCount > FailMaxCount)
			{
				printf("Fail\n");
				return;
			}
			usleep(10000);
		}
		FailCount = 0;
		while(1)
		{
			if(cm730->WriteByte(id, MX28::P_LOW_LIMIT_VOLTAGE, 60, 0) == CM730::SUCCESS)
				break;

			FailCount++;
			if(FailCount > FailMaxCount)
			{
				printf("Fail\n");
				return;
			}
			usleep(10000);
		}		
		FailCount = 0;
		while(1)
		{
			if(cm730->WriteByte(id, MX28::P_HIGH_LIMIT_VOLTAGE, 140, 0) == CM730::SUCCESS)
				break;

			FailCount++;
			if(FailCount > FailMaxCount)
			{
				printf("Fail\n");
				return;
			}
			usleep(10000);
		}
		FailCount = 0;
		while(1)
		{
			if(cm730->WriteWord(id, MX28::P_MAX_TORQUE_L, MX28::MAX_VALUE, 0) == CM730::SUCCESS)
				break;

			FailCount++;
			if(FailCount > FailMaxCount)
			{
				printf("Fail\n");
				return;
			}
			usleep(10000);
		}
		FailCount = 0;
		while(1)
		{
			if(cm730->WriteByte(id, MX28::P_ALARM_LED, 36, 0) == CM730::SUCCESS) // Overload, Overheat
				break;

			FailCount++;
			if(FailCount > FailMaxCount)
			{
				printf("Fail\n");
				return;
			}
			usleep(10000);
		}		 
		FailCount = 0;
		while(1)
		{
			if(cm730->WriteByte(id, MX28::P_ALARM_SHUTDOWN, 36, 0) == CM730::SUCCESS) // Overload, Overheat
				break;

			FailCount++;
			if(FailCount > FailMaxCount)
			{
				printf("Fail\n");
				return;
			}
			usleep(10000);
		}		 
	}

	printf("Success\n");
}

void Write(Robot::CM730 *cm730, int id, int addr, int value)
{
	if(addr == MX28::P_BAUD_RATE || addr == MX28::P_RETURN_DELAY_TIME || addr == MX28::P_RETURN_LEVEL)
	{
		printf( " Can not change this address[%d]\n", addr);
		return;
	}

	int error = 0;
	int res;
	if(id == CM730::ID_CM)
	{
		if(addr >= CM730::MAXNUM_ADDRESS)
		{
			printf( " Invalid address\n");
			return;
		}

		if(addr == CM730::P_DXL_POWER
			|| addr == CM730::P_LED_PANNEL)
		{
			res = cm730->WriteByte(addr, value, &error);
		}
		else
		{
			res = cm730->WriteWord(addr, value, &error);
		}
	}
	else
	{
		if(addr >= MX28::MAXNUM_ADDRESS)
		{
			printf( " Invalid address\n");
			return;
		}

		if(addr == MX28::P_ID)
		{
		    if(cm730->Ping(value, 0) == CM730::SUCCESS)
		    {
		        printf( " Can not change the ID. ID[%d] is in use.. \n", value);
		        return;
		    }
		    else
		    {
		        res = cm730->WriteByte(id, addr, value, &error);
		        gID = value;
		    }
		}
		else if(addr == MX28::P_HIGH_LIMIT_TEMPERATURE
            || addr == MX28::P_LOW_LIMIT_VOLTAGE
            || addr == MX28::P_HIGH_LIMIT_VOLTAGE
            || addr == MX28::P_ALARM_LED
            || addr == MX28::P_ALARM_SHUTDOWN
            || addr == MX28::P_TORQUE_ENABLE
            || addr == MX28::P_LED
#ifdef MX28_1024
            || addr == MX28::P_CW_COMPLIANCE_MARGIN
            || addr == MX28::P_CCW_COMPLIANCE_MARGIN
            || addr == MX28::P_CW_COMPLIANCE_SLOPE
            || addr == MX28::P_CCW_COMPLIANCE_SLOPE
#else
			|| addr == MX28::P_P_GAIN
			|| addr == MX28::P_I_GAIN
			|| addr == MX28::P_D_GAIN
			|| addr == MX28::P_RESERVED
#endif
			|| addr == MX28::P_LED
			|| addr == MX28::P_LED)
		{
			res = cm730->WriteByte(id, addr, value, &error);
		}
		else
		{
			res = cm730->WriteWord(id, addr, value, &error);
		}
	}

	if(res != CM730::SUCCESS)
	{
		printf( " Fail to write!\n");
		return;
	}

	if(error != 0)
	{
		printf( " Access or range error!\n");
		return;
	}

	printf(" Writing successful!\n");
}
