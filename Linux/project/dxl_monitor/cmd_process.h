#ifndef _DXL_MONITOR_CMD_PROCESS_H_
#define _DXL_MONITOR_CMD_PROCESS_H_


#include "LinuxDARwIn.h"


void Prompt(int id);
void Help();
void Scan(Robot::ArbotixPro *arbotixpro);
void Dump(Robot::ArbotixPro *arbotixpro, int id);
void Reset(Robot::ArbotixPro *arbotixpro, int id);
void Write(Robot::ArbotixPro *arbotixpro, int id, int addr, int value);

#endif
