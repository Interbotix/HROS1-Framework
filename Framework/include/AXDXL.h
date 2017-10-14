/*
 *   AXDXL.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _AX_DXL_H_
#define _AX_DXL_H_

namespace Robot
{
    class AXDXL
    {
        public:
            static const int MIN_VALUE = 0;
            static const int CENTER_VALUE = 512;
            static const int MAX_VALUE = 1023;
            static constexpr double MIN_ANGLE = -150.0; // degree
            static constexpr double MAX_ANGLE = 150.0; // degree
            static constexpr double RATIO_VALUE2ANGLE = 0.293; // 300 / 1024
            static constexpr double RATIO_ANGLE2VALUE = 3.413; // 1024 / 300

            static const int PARAM_BYTES = 5;


            static int GetMirrorValue(int value)        { return MAX_VALUE + 1 - value; }
            static double GetMirrorAngle(double angle)  { return -angle; }
            static int Angle2Value(double angle) { return (int)(angle * RATIO_ANGLE2VALUE) + CENTER_VALUE; }
            static double Value2Angle(int value) { return (double)(value - CENTER_VALUE) * RATIO_VALUE2ANGLE; }

            // Address
            enum
            {
                P_MODEL_NUMBER_L            = 0,
                P_MODEL_NUMBER_H            = 1,
                P_VERSION                   = 2,
                P_ID                        = 3,
                P_BAUD_RATE                 = 4,
                P_RETURN_DELAY_TIME         = 5,
                P_CW_ANGLE_LIMIT_L          = 6,
                P_CW_ANGLE_LIMIT_H          = 7,
                P_CCW_ANGLE_LIMIT_L         = 8,
                P_CCW_ANGLE_LIMIT_H         = 9,
                P_HIGH_LIMIT_TEMPERATURE    = 11,
                P_LOW_LIMIT_VOLTAGE         = 12,
                P_HIGH_LIMIT_VOLTAGE        = 13,
                P_MAX_TORQUE_L              = 14,
                P_MAX_TORQUE_H              = 15,
                P_RETURN_LEVEL              = 16,
                P_ALARM_LED                 = 17,
                P_ALARM_SHUTDOWN            = 18,
                P_TORQUE_ENABLE             = 24,
                P_LED                       = 25,
                P_CW_COMPLIANCE_MARGIN      = 26,
                P_CCW_COMPLIANCE_MARGIN     = 27,
                P_CW_COMPLIANCE_SLOPE       = 28,
                P_CCW_COMPLIANCE_SLOPE      = 29,
                P_GOAL_POSITION_L           = 30,
                P_GOAL_POSITION_H           = 31,
                P_MOVING_SPEED_L            = 32,
                P_MOVING_SPEED_H            = 33,
                P_TORQUE_LIMIT_L            = 34,
                P_TORQUE_LIMIT_H            = 35,
                P_PRESENT_POSITION_L        = 36,
                P_PRESENT_POSITION_H        = 37,
                P_PRESENT_SPEED_L           = 38,
                P_PRESENT_SPEED_H           = 39,
                P_PRESENT_LOAD_L            = 40,
                P_PRESENT_LOAD_H            = 41,
                P_PRESENT_VOLTAGE           = 42,
                P_PRESENT_TEMPERATURE       = 43,
                P_REGISTERED_INSTRUCTION    = 44,
                P_MOVING                    = 46,
                P_LOCK                      = 47,
                P_PUNCH_L                   = 48,
                P_PUNCH_H                   = 49,
                MAXNUM_ADDRESS
            };

    };
}

#endif
