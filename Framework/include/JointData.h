/*
 *   JointData.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _JOINT_DATA_H_
#define _JOINT_DATA_H_


namespace Robot
{
	class JointData
	{
		public:
			enum
			{
				ID_MIN					= 1,
				ID_R_SHOULDER_PITCH     = 1,
				ID_L_SHOULDER_PITCH     = 2,
				ID_R_SHOULDER_ROLL      = 3,
				ID_L_SHOULDER_ROLL      = 4,
				ID_R_ELBOW              = 5,
				ID_L_ELBOW              = 6,
				ID_R_HIP_YAW            = 7,
				ID_L_HIP_YAW            = 8,
				ID_R_HIP_ROLL           = 9,
				ID_L_HIP_ROLL           = 10,
				ID_R_HIP_PITCH          = 11,
				ID_L_HIP_PITCH          = 12,
				ID_R_KNEE               = 13,
				ID_L_KNEE               = 14,
				ID_R_ANKLE_PITCH        = 15,
				ID_L_ANKLE_PITCH        = 16,
				ID_R_ANKLE_ROLL         = 17,
				ID_L_ANKLE_ROLL         = 18,
				ID_HEAD_PAN             = 19,
				ID_HEAD_TILT            = 20,
				ID_MAX					= 20,
				NUMBER_OF_JOINTS		= 21
			};

			enum
			{
				SLOPE_HARD			= 16,
				SLOPE_DEFAULT		= 32,
				SLOPE_SOFT			= 64,
				SLOPE_EXTRASOFT		= 128
			};

			enum
			{
				TEMP_DEFAULT	= 0
			};
			enum
			{
				P_GAIN_DEFAULT      = 32,
				I_GAIN_DEFAULT      = 0,
				D_GAIN_DEFAULT      = 0
			};

		private:

		protected:
			bool m_Enable[NUMBER_OF_JOINTS];
			int m_Value[NUMBER_OF_JOINTS];
			double m_Angle[NUMBER_OF_JOINTS];
			int m_CWSlope[NUMBER_OF_JOINTS];
			int m_CCWSlope[NUMBER_OF_JOINTS];
			int m_PGain[NUMBER_OF_JOINTS];
			int m_IGain[NUMBER_OF_JOINTS];
			int m_DGain[NUMBER_OF_JOINTS];
			int m_Temp[NUMBER_OF_JOINTS];

		public:
			JointData();
			~JointData();

			void SetEnable(int id, bool enable);
			void SetEnable(int id, bool enable, bool exclusive);
			void SetEnableHeadOnly(bool enable);
			void SetEnableHeadOnly(bool enable, bool exclusive);
			void SetEnableRightArmOnly(bool enable);
			void SetEnableRightArmOnly(bool enable, bool exclusive);
			void SetEnableLeftArmOnly(bool enable);
			void SetEnableLeftArmOnly(bool enable, bool exclusive);
			void SetEnableRightLegOnly(bool enable);
			void SetEnableRightLegOnly(bool enable, bool exclusive);
			void SetEnableLeftLegOnly(bool enable);
			void SetEnableLeftLegOnly(bool enable, bool exclusive);
			void SetEnableUpperBodyWithoutHead(bool enable);
			void SetEnableUpperBodyWithoutHead(bool enable, bool exclusive);
			void SetEnableLowerBody(bool enable);
			void SetEnableLowerBody(bool enable, bool exclusive);
			void SetEnableBodyWithoutHead(bool enable);
			void SetEnableBodyWithoutHead(bool enable, bool exclusive);
			void SetEnableBody(bool enable);
			void SetEnableBody(bool enable, bool exclusive);
			bool GetEnable(int id);

			void SetValue(int id, int value);
			int GetValue(int id);

			void SetAngle(int id, double angle);
			double GetAngle(int id);

			void SetRadian(int id, double radian);
			double GetRadian(int id);

			void SetSlope(int id, int cwSlope, int ccwSlope);
			void SetCWSlope(int id, int cwSlope);
			int  GetCWSlope(int id);
			void SetCCWSlope(int id, int ccwSlope);
			int  GetCCWSlope(int id);

			void SetPGain(int id, int pgain) { m_PGain[id] = pgain; }
			int  GetPGain(int id)            { return m_PGain[id]; }
			void SetIGain(int id, int igain) { m_IGain[id] = igain; }
			int  GetIGain(int id)            { return m_IGain[id]; }
			void SetDGain(int id, int dgain) { m_DGain[id] = dgain; }
			int  GetDGain(int id)            { return m_DGain[id]; }

			int GetTemp(int id)									{ return m_Temp[id]; }
			void SetTemp(int id, int value)			{ m_Temp[id] = value; }
	};
}

#endif
