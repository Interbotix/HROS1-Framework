/*
 *   LinuxArbotixPro.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _LINUX_ARBOTIXPRO_H_
#define _LINUX_ARBOTIXPRO_H_

#include <semaphore.h>
#include "ArbotixPro.h"


namespace Robot
{
	class LinuxArbotixPro : public PlatformArbotixPro
	{
		private:
			int m_Socket_fd;
			double m_PacketStartTime;
			double m_PacketWaitTime;
			double m_UpdateStartTime;
			double m_UpdateWaitTime;
			double m_ByteTransferTime;
			char m_PortName[20];

			sem_t m_LowSemID;
			sem_t m_MidSemID;
			sem_t m_HighSemID;

			double GetCurrentTime();

		public:
			bool DEBUG_PRINT;

			LinuxArbotixPro(const char* name);
			~LinuxArbotixPro();

			void SetPortName(const char* name);
			const char* GetPortName()		{ return (const char*)m_PortName; }

			///////////////// Platform Porting //////////////////////
			bool OpenPort();
			bool SetBaud(int baud);
			void ClosePort();
			void ClearPort();
			int WritePort(unsigned char* packet, int numPacket);
			int ReadPort(unsigned char* packet, int numPacket);
			void FlushPort();

			void LowPriorityWait();
			void MidPriorityWait();
			void HighPriorityWait();
			void LowPriorityRelease();
			void MidPriorityRelease();
			void HighPriorityRelease();

			void SetPacketTimeout(int lenPacket);
			bool IsPacketTimeout();
			double GetPacketTime();
			void SetUpdateTimeout(int msec);
			bool IsUpdateTimeout();
			double GetUpdateTime();

			virtual void Sleep(int Miliseconds);
			////////////////////////////////////////////////////////
	};
}

#endif