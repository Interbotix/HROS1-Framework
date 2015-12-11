#include <stdio.h>
#include <string.h>
#include "cmd_process.h"

using namespace Robot;

extern int gID;

const char *GetIDString(int id)
{
	switch (id)
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

		case ArbotixPro::ID_CM:
			return "ARBOTIX_PRO";

//	case JointData::ID_TORSO_ROTATE:
//		return "TORSO_ROTATE";

//	case JointData::ID_R_ELBOW_YAW:
//		return "R_ELBOW_YAW";

//	case JointData::ID_L_ELBOW_YAW:
//		return "L_ELBOW_YAW";

//	case JointData::ID_R_WRIST_YAW:
//		return "R_WRIST_YAW";

//	case JointData::ID_L_WRIST_YAW:
//		return "L_WRIST_YAW";
//
//	case JointData::ID_R_GRIPPER:
//		return "R_GRIPPER";
//
//	case JointData::ID_L_GRIPPER:
//		return "L_GRIPPER";

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
	printf( " d : Dumps the current control table of Arbotix Pro and all Dynamixels\n" );
	printf( " reset : Defaults the value of current Dynamixel\n" );
	printf( " reset all : Defaults the value of all Dynamixels\n" );
	printf( " wr [ADDR] [VALUE] : Writes value [VALUE] to address [ADDR] of current Dynamixel\n" );
	printf( " on/off : Turns torque on/off of current Dynamixel\n" );
	printf( " on/off all : Turns torque on/off of all Dynamixels)\n" );
	printf( "\n       Copyright ROBOTIS CO.,LTD.\n\n" );
}

void Scan(ArbotixPro *arbotixpro)
{
	printf("\n");

	for (int id = 1; id < 254; id++)
		{
			if (arbotixpro->Ping(id, 0) == ArbotixPro::SUCCESS)
				{
					usleep(150);
					printf("                                  ... OK\r");
					printf(" Check ID:%d(%s)\n", id, GetIDString(id));
				}
			else if (id < JointData::NUMBER_OF_JOINTS || id == ArbotixPro::ID_CM)
				{
					usleep(150);
					printf("                                  ... FAIL\r");
					printf(" Check ID:%d(%s)\n", id, GetIDString(id));
				}
		}

	printf("\n");
}

void Dump(ArbotixPro *arbotixpro, int id)
{
	unsigned char table[128];
	int addr;
	int value;


	if (id == ArbotixPro::ID_CM) // Sub board
		{
			if (arbotixpro->ReadTable(id, ArbotixPro::P_MODEL_NUMBER_L, ArbotixPro::P_RIGHT_MIC_H, &table[ArbotixPro::P_MODEL_NUMBER_L], 0) != ArbotixPro::SUCCESS)
				{
					printf(" Can not read table!\n");
					return;
				}

			printf( "\n" );
			printf( " [EEPROM AREA]\n" );
			addr = ArbotixPro::P_MODEL_NUMBER_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " MODEL_NUMBER            (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = ArbotixPro::P_VERSION; value = table[addr];
			printf( " VERSION                 (R) [%.3d]:%5d\n", addr, value);
			addr = ArbotixPro::P_ID; value = table[addr];
			printf( " ID                     (R/W)[%.3d]:%5d\n", addr, value);
			addr = ArbotixPro::P_BAUD_RATE; value = table[addr];
			printf( " BAUD_RATE              (R/W)[%.3d]:%5d\n", addr, value);
			addr = ArbotixPro::P_RETURN_DELAY_TIME; value = table[addr];
			printf( " RETURN_DELAY_TIME      (R/W)[%.3d]:%5d\n", addr, value);
			addr = ArbotixPro::P_RETURN_LEVEL; value = table[addr];
			printf( " RETURN_LEVEL           (R/W)[%.3d]:%5d\n", addr, value);
			printf( "\n" );
			printf( " [RAM AREA]\n" );
			addr = ArbotixPro::P_DXL_POWER; value = table[addr];
			printf( " DXL_POWER              (R/W)[%.3d]:%5d\n", addr, value);
			addr = ArbotixPro::P_LED_PANNEL; value = table[addr];
			printf( " LED_PANNEL             (R/W)[%.3d]:%5d\n", addr, value);
			addr = ArbotixPro::P_LED_HEAD_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " LED_HEAD               (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = ArbotixPro::P_LED_EYE_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " LED_EYE                (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = ArbotixPro::P_BUTTON; value = table[addr];
			printf( " BUTTON                  (R) [%.3d]:%5d\n", addr, value);
			addr = ArbotixPro::P_GYRO_Z_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " GYRO_Z                  (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = ArbotixPro::P_GYRO_Y_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " GYRO_Y                  (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = ArbotixPro::P_GYRO_X_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " GYRO_X                  (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = ArbotixPro::P_ACCEL_X_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " ACCEL_X                 (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = ArbotixPro::P_ACCEL_Y_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " ACCEL_Y                 (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = ArbotixPro::P_ACCEL_Z_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " ACCEL_Z                 (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = ArbotixPro::P_VOLTAGE; value = table[addr];
			printf( " VOLTAGE                 (R) [%.3d]:%5d\n", addr, value);
			addr = ArbotixPro::P_LEFT_MIC_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " LEFT_MIC                (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = ArbotixPro::P_RIGHT_MIC_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " RIGHT_MIC               (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);

			printf( "\n" );
		}
	else // Actuator
		{
			if (arbotixpro->ReadTable(id, AXDXL::P_MODEL_NUMBER_L, AXDXL::P_PUNCH_H, &table[AXDXL::P_MODEL_NUMBER_L], 0) != ArbotixPro::SUCCESS)
				{
					printf(" Can not read table!\n");
					return;
				}

			printf( "\n" );
			printf( " [EEPROM AREA]\n" );
			addr = AXDXL::P_MODEL_NUMBER_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " MODEL_NUMBER            (R) [%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = AXDXL::P_VERSION; value = table[addr];
			printf( " VERSION                 (R) [%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_ID; value = table[addr];
			printf( " ID                     (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_BAUD_RATE; value = table[addr];
			printf( " BAUD_RATE              (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_RETURN_DELAY_TIME; value = table[addr];
			printf( " RETURN_DELAY_TIME      (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_CW_ANGLE_LIMIT_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " CW_ANGLE_LIMIT         (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = AXDXL::P_CCW_ANGLE_LIMIT_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " CCW_ANGLE_LIMIT        (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = AXDXL::P_HIGH_LIMIT_TEMPERATURE; value = table[addr];
			printf( " HIGH_LIMIT_TEMPERATURE (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_LOW_LIMIT_VOLTAGE; value = table[addr];
			printf( " LOW_LIMIT_VOLTAGE      (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_HIGH_LIMIT_VOLTAGE; value = table[addr];
			printf( " HIGH_LIMIT_VOLTAGE     (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_MAX_TORQUE_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " MAX_TORQUE             (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = AXDXL::P_RETURN_LEVEL; value = table[addr];
			printf( " RETURN_LEVEL           (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_ALARM_LED; value = table[addr];
			printf( " ALARM_LED              (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_ALARM_SHUTDOWN; value = table[addr];
			printf( " ALARM_SHUTDOWN         (R/W)[%.3d]:%5d\n", addr, value);
			printf( "\n" );
			printf( " [RAM AREA]\n" );
			addr = AXDXL::P_TORQUE_ENABLE; value = table[addr];
			printf( " TORQUE_ENABLE          (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_LED; value = table[addr];
			printf( " LED                    (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_CW_COMPLIANCE_MARGIN; value = table[addr];
			printf( " CW_COMPLIANCE_MARGIN   (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_CCW_COMPLIANCE_MARGIN; value = table[addr];
			printf( " CCW_COMPLIANCE_MARGIN  (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_CW_COMPLIANCE_SLOPE; value = table[addr];
			printf( " CW_COMPLIANCE_SLOPE    (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_CCW_COMPLIANCE_SLOPE; value = table[addr];
			printf( " CCW_COMPLIANCE_SLOPE   (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_GOAL_POSITION_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " GOAL_POSITION          (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = AXDXL::P_MOVING_SPEED_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " MOVING_SPEED           (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = AXDXL::P_TORQUE_LIMIT_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " TORQUE_LIMIT           (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = AXDXL::P_PRESENT_POSITION_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " PRESENT_POSITION       (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = AXDXL::P_PRESENT_SPEED_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " PRESENT_SPEED          (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = AXDXL::P_PRESENT_LOAD_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " PRESENT_LOAD           (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);
			addr = AXDXL::P_PRESENT_VOLTAGE; value = table[addr];
			printf( " PRESENT_VOLTAGE        (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_PRESENT_TEMPERATURE; value = table[addr];
			printf( " PRESENT_TEMPERATURE    (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_REGISTERED_INSTRUCTION; value = table[addr];
			printf( " REGISTERED_INSTRUC     (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_MOVING; value = table[addr];
			printf( " MOVING                 (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_LOCK; value = table[addr];
			printf( " LOCK                   (R/W)[%.3d]:%5d\n", addr, value);
			addr = AXDXL::P_PUNCH_L; value = ArbotixPro::MakeWord(table[addr], table[addr + 1]);
			printf( " PUNCH                  (R/W)[%.3d]:%5d (L:0x%.2X H:0x%.2X)\n", addr, value, table[addr], table[addr + 1]);

			printf( "\n" );
		}
}

void Reset(Robot::ArbotixPro *arbotixpro, int id)
{
	int FailCount = 0;
	int FailMaxCount = 10;
	printf(" Reset ID:%d...", id);

	if (arbotixpro->Ping(id, 0) != ArbotixPro::SUCCESS)
		{
			printf("Fail\n");
			return;
		}

	FailCount = 0;
	while (1)
		{
			if (arbotixpro->WriteByte(id, AXDXL::P_RETURN_DELAY_TIME, 0, 0) == ArbotixPro::SUCCESS)
				break;

			FailCount++;
			if (FailCount > FailMaxCount)
				{
					printf("Fail\n");
					return;
				}
			usleep(10000);
		}

	FailCount = 0;
	while (1)
		{
			if (arbotixpro->WriteByte(id, AXDXL::P_RETURN_LEVEL, 2, 0) == ArbotixPro::SUCCESS)
				break;

			FailCount++;
			if (FailCount > FailMaxCount)
				{
					printf("Fail\n");
					return;
				}
			usleep(10000);
		}

	if (id != ArbotixPro::ID_CM)
		{
			double cwLimit = AXDXL::MIN_ANGLE;
			double ccwLimit = AXDXL::MAX_ANGLE;

			switch (id)
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
					cwLimit = -30.0;
					ccwLimit = 30.0;
					break;

				case JointData::ID_L_HIP_YAW:
					cwLimit = -30.0;
					ccwLimit = 30.0;
					break;

				case JointData::ID_R_HIP_ROLL:
					cwLimit = -45.0;
					ccwLimit = 45.0;
					break;

				case JointData::ID_L_HIP_ROLL:
					cwLimit = -45.0;
					ccwLimit = 45.0;
					break;

				case JointData::ID_R_HIP_PITCH:
					cwLimit = -90.0;
					ccwLimit = 90.0;//29.0;
					break;

				case JointData::ID_L_HIP_PITCH:
					cwLimit = -90.0;//-29.0;
					ccwLimit = 90.0;
					break;

				case JointData::ID_R_KNEE:
					cwLimit = -160.0;
					ccwLimit = 160.0;
					break;

				case JointData::ID_L_KNEE:
					cwLimit = -160.0;
					ccwLimit = 160.0;
					break;

				case JointData::ID_R_ANKLE_PITCH:
					cwLimit = -90.0;
					ccwLimit = 90.0;
					break;

				case JointData::ID_L_ANKLE_PITCH:
					cwLimit = -90.0;
					ccwLimit = 90.0;
					break;

				case JointData::ID_R_ANKLE_ROLL:
					cwLimit = -45.0;
					ccwLimit = 45.0;
					break;

				case JointData::ID_L_ANKLE_ROLL:
					cwLimit = -45.0;
					ccwLimit = 45.0;
					break;

				case JointData::ID_HEAD_TILT:
					cwLimit = -45.0;
					ccwLimit = 45.0;
					break;


				}

			FailCount = 0;
			while (1)
				{
					if (arbotixpro->WriteWord(id, AXDXL::P_CW_ANGLE_LIMIT_L, AXDXL::Angle2Value(cwLimit), 0) == ArbotixPro::SUCCESS)
						break;

					FailCount++;
					if (FailCount > FailMaxCount)
						{
							printf("Fail\n");
							return;
						}
					usleep(10000);
				}
			FailCount = 0;
			while (1)
				{
					if (arbotixpro->WriteWord(id, AXDXL::P_CCW_ANGLE_LIMIT_L, AXDXL::Angle2Value(ccwLimit), 0) == ArbotixPro::SUCCESS)
						break;

					FailCount++;
					if (FailCount > FailMaxCount)
						{
							printf("Fail\n");
							return;
						}
					usleep(10000);
				}
			FailCount = 0;
			while (1)
				{
					if (arbotixpro->WriteByte(id, AXDXL::P_HIGH_LIMIT_TEMPERATURE, 80, 0) == ArbotixPro::SUCCESS)
						break;

					FailCount++;
					if (FailCount > FailMaxCount)
						{
							printf("Fail\n");
							return;
						}
					usleep(10000);
				}
			FailCount = 0;
			while (1)
				{
					if (arbotixpro->WriteByte(id, AXDXL::P_LOW_LIMIT_VOLTAGE, 60, 0) == ArbotixPro::SUCCESS)
						break;

					FailCount++;
					if (FailCount > FailMaxCount)
						{
							printf("Fail\n");
							return;
						}
					usleep(10000);
				}
			FailCount = 0;
			while (1)
				{
					if (arbotixpro->WriteByte(id, AXDXL::P_HIGH_LIMIT_VOLTAGE, 140, 0) == ArbotixPro::SUCCESS)
						break;

					FailCount++;
					if (FailCount > FailMaxCount)
						{
							printf("Fail\n");
							return;
						}
					usleep(10000);
				}
			FailCount = 0;
			while (1)
				{
					if (arbotixpro->WriteWord(id, AXDXL::P_MAX_TORQUE_L, AXDXL::MAX_VALUE, 0) == ArbotixPro::SUCCESS)
						break;

					FailCount++;
					if (FailCount > FailMaxCount)
						{
							printf("Fail\n");
							return;
						}
					usleep(10000);
				}
			FailCount = 0;
			while (1)
				{
					if (arbotixpro->WriteByte(id, AXDXL::P_ALARM_LED, 36, 0) == ArbotixPro::SUCCESS) // Overload, Overheat
						break;

					FailCount++;
					if (FailCount > FailMaxCount)
						{
							printf("Fail\n");
							return;
						}
					usleep(10000);
				}
			FailCount = 0;
			while (1)
				{
					if (arbotixpro->WriteByte(id, AXDXL::P_ALARM_SHUTDOWN, 36, 0) == ArbotixPro::SUCCESS) // Overload, Overheat
						break;

					FailCount++;
					if (FailCount > FailMaxCount)
						{
							printf("Fail\n");
							return;
						}
					usleep(10000);
				}
		}

	printf("Success\n");
}

void Write(Robot::ArbotixPro *arbotixpro, int id, int addr, int value)
{
	/*
	if(addr == AXDXL::P_RETURN_DELAY_TIME || addr == AXDXL::P_RETURN_LEVEL)
	{
		printf( " Can not change this address[%d]\n", addr);
		return;
	}
	*/
	int error = 0;
	int res;
	if (id == ArbotixPro::ID_CM)
		{
			if (addr >= ArbotixPro::MAXNUM_ADDRESS)
				{
					printf( " Invalid address\n");
					return;
				}

			if (addr == ArbotixPro::P_DXL_POWER
			        || addr == ArbotixPro::P_LED_PANNEL)
				{
					res = arbotixpro->WriteByte(addr, value, &error);
				}
			else
				{
					res = arbotixpro->WriteWord(addr, value, &error);
				}
		}
	else
		{
			if (addr >= AXDXL::MAXNUM_ADDRESS)
				{
					printf( " Invalid address\n");
					return;
				}

			if (addr == AXDXL::P_ID)
				{
					if (arbotixpro->Ping(value, 0) == ArbotixPro::SUCCESS)
						{
							printf( " Can not change the ID. ID[%d] is in use.. \n", value);
							return;
						}
					else
						{
							res = arbotixpro->WriteByte(id, addr, value, &error);
							gID = value;
						}
				}
			else if (addr == AXDXL::P_HIGH_LIMIT_TEMPERATURE
			         || addr == AXDXL::P_LOW_LIMIT_VOLTAGE
			         || addr == AXDXL::P_HIGH_LIMIT_VOLTAGE
			         || addr == AXDXL::P_ALARM_LED
			         || addr == AXDXL::P_ALARM_SHUTDOWN
			         || addr == AXDXL::P_TORQUE_ENABLE
			         || addr == AXDXL::P_LED
			         || addr == AXDXL::P_CW_COMPLIANCE_MARGIN
			         || addr == AXDXL::P_CCW_COMPLIANCE_MARGIN
			         || addr == AXDXL::P_CW_COMPLIANCE_SLOPE
			         || addr == AXDXL::P_CCW_COMPLIANCE_SLOPE
			         || addr == AXDXL::P_LED
			         || addr == AXDXL::P_LED
			         || addr == AXDXL::P_BAUD_RATE)
				{
					res = arbotixpro->WriteByte(id, addr, value, &error);
				}
			else
				{
					res = arbotixpro->WriteWord(id, addr, value, &error);
				}
		}

	if (res != ArbotixPro::SUCCESS)
		{
			printf( " Fail to write!\n");
			return;
		}

	if (error != 0)
		{
			printf( " Access or range error!\n");
			return;
		}

	//if(id == ArbotixPro::ID_CM && addr == AXDXL::P_BAUD_RATE)
	//    arbotixpro->ChangeBaud(value);

	printf(" Writing successful!\n");
}
