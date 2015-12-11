/*
 *   LinuxMotionTimer.h
 *
 *   Author: ROBOTIS
 *
 */

#ifndef _LINUX_MOTION_MANAGER_H_
#define _LINUX_MOTION_MANAGER_H_

#include <pthread.h>
#include <time.h>
#include "MotionManager.h"

namespace Robot
{
  class LinuxMotionTimer
  {
    private:
      pthread_t thread;// thread structure
      struct timespec next_time;// next absolute time
      bool finish_thread;
      bool timer_running;
      MotionManager *manager;// reference to the motion manager class.

    protected:
      static void *motion_timing(void *param);// thread function
      void update_time(int interval_ns);
    public:
      LinuxMotionTimer();
      void Initialize(MotionManager* manager);
      void Start();
      void Stop();
      bool IsRunning();
      ~LinuxMotionTimer();
  };
}

#endif
