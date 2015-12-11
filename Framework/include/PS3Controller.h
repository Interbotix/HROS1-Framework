/*
 *   PS3Controller.h
 *
 *   Author: Farrell Robotics
 *
 */

#ifndef _PS3CONTROLLER_H_
#define _PS3CONTROLLER_H_

typedef unsigned char byte;
typedef union
{
	struct
	{
		unsigned char HIDA1;
		unsigned char Chan;
		unsigned char unk1;
		//b1
		unsigned char Select: 1;
		unsigned char LeftHat: 1;
		unsigned char RightHat: 1;
		unsigned char Start: 1;
		unsigned char Up: 1;
		unsigned char Right: 1;
		unsigned char Down: 1;
		unsigned char Left: 1;
		//b2
		unsigned char L2: 1;
		unsigned char R2: 1;
		unsigned char L1: 1;
		unsigned char R1: 1;
		unsigned char Triangle: 1;
		unsigned char Circle: 1;
		unsigned char Cross: 1;
		unsigned char Square: 1;
		//b3
		unsigned char PS: 1;
		unsigned char na1: 7;

		unsigned char unk2;
		//analog sticks
		unsigned char LJoyX;
		unsigned char LJoyY;
		unsigned char RJoyX;
		unsigned char RJoyY;

		unsigned char unk3[4];//15
		//pressure sensing buttons
		unsigned char presUp;
		unsigned char presRight;
		unsigned char presDown;
		unsigned char presLeft;
		unsigned char presL2;
		unsigned char presR2;
		unsigned char presL1;
		unsigned char presR1;
		unsigned char presTiangle;
		unsigned char presO;
		unsigned char presX;
		unsigned char presBox;//27

		unsigned char unk4[3];
		unsigned char status1;//should be 3 normally, 2 when cable plugged in
		unsigned char powerRating;//Seems to be power rating - 05=full, 02=dying, 01=just before shutdown, EE=charging
		unsigned char comStatus;//Status? 14 when operating by bluetooth, 10 when operating by bluetooth with cable plugged in

		unsigned char unk5[9];
		unsigned short accX;//Accelerator data - big-endian 0..1023, centred at 512, resolution 0.01g
		unsigned short accY;
		unsigned short accZ;
		unsigned short zGyro;//Z gyro - measures counter-clockwise rotation, 0..1023, nominally centred but see note below
	} key;
	unsigned char keys[50];
} _PS3;

typedef union
{
	struct
	{
		byte rjx;
		byte rjy;
		byte ljx;
		byte ljy;
	} key;
	byte keys[4];
} _JKeys;

extern volatile _PS3 PS3;
extern volatile _JKeys JKeys;
extern volatile int bPS3Active, gLeftJoyStickEnable, gRightJoyStickEnable, robotInStandby, gJoyIMUMixEnable;

int PS3Controller_Start();
void PS3Controller_Stop();

byte PS3KeyChanged(void);
void ClearPS3Keys(void);
void SetPS3LEDFlashRate(int rate);
int ToggleRobotStandby(void);
void PS3Vibrate(void);

#endif
