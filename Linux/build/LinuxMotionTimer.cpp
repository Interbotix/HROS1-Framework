/*
 *   LinuxMotionTimer.cpp
 *
 *   Author: ROBOTIS
 *
 */

#include "MotionModule.h"
#include "LinuxMotionTimer.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/prctl.h>

#if defined(__arm__)
#define __NR_ioprio_set   314
#define __NR_ioprio_get   315
#elif defined(__i386__)
#define __NR_ioprio_set         289
#define __NR_ioprio_get         290
#elif defined(__ppc__)
#define __NR_ioprio_set         273
#define __NR_ioprio_get         274
#elif defined(__x86_64__)
#define __NR_ioprio_set         251
#define __NR_ioprio_get         252
#elif defined(__ia64__)
#define __NR_ioprio_set         1274
#define __NR_ioprio_get         1275
#else
#error "Unsupported arch"
#endif

static inline int ioprio_set(int which, int who, int ioprio)
{
  return syscall(__NR_ioprio_set, which, who, ioprio);
}

static inline int ioprio_get(int which, int who)
{
  return syscall(__NR_ioprio_get, which, who);
}

enum
{
  IOPRIO_CLASS_NONE,
  IOPRIO_CLASS_RT,
  IOPRIO_CLASS_BE,
  IOPRIO_CLASS_IDLE,
};

enum
{
  IOPRIO_WHO_PROCESS = 1,
  IOPRIO_WHO_PGRP,
  IOPRIO_WHO_USER,
};

#define IOPRIO_CLASS_SHIFT      13

using namespace Robot;

LinuxMotionTimer::LinuxMotionTimer()
{
  this->finish_thread = false;
  this->timer_running = false;
  this->manager = NULL;
}

void *LinuxMotionTimer::motion_timing(void *param)
{
  LinuxMotionTimer *timer = (LinuxMotionTimer *)param;
  struct timespec next_time;
  struct timespec current_time;

  clock_gettime(CLOCK_MONOTONIC, &next_time);

  char name [17];
  strcpy (name, "LinuxTimer");
  prctl (PR_SET_NAME, (unsigned long)&name);

  // Set I/O priority to realtime
  ioprio_set(IOPRIO_WHO_PROCESS, getpid(), (IOPRIO_CLASS_RT << 13) | 0);

  while (!timer->finish_thread)
    {
      if (timer->manager != NULL)
        timer->manager->Process();
      // Calculate the next reachable period
      clock_gettime(CLOCK_MONOTONIC, &current_time);
      do
        {
          next_time.tv_sec += (next_time.tv_nsec + MotionModule::TIME_UNIT * 1000000) / 1000000000;
          next_time.tv_nsec = (next_time.tv_nsec + MotionModule::TIME_UNIT * 1000000) % 1000000000;
        }
      while (current_time.tv_sec > next_time.tv_sec
             || (current_time.tv_sec == next_time.tv_sec && current_time.tv_nsec > next_time.tv_nsec));

      clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &next_time, NULL);
    }

  pthread_exit(NULL);
}

void LinuxMotionTimer::update_time(int interval_ns)
{
  if ((this->next_time.tv_nsec + interval_ns) > 1000000000)
    {
      this->next_time.tv_sec++;
      this->next_time.tv_nsec += (interval_ns) - 1000000000;
    }
  else
    this->next_time.tv_nsec += interval_ns;
}

void LinuxMotionTimer::Initialize(MotionManager* manager)
{
  this->manager = manager;
}

void LinuxMotionTimer::Start(void)
{
  int error;
  struct sched_param param;
  pthread_attr_t attr;

  pthread_attr_init(&attr);

  error = pthread_attr_setschedpolicy(&attr, SCHED_RR);
  if (error != 0) printf("error = %d\n", error);
  error = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  if (error != 0) printf("error = %d\n", error);

  memset(&param, 0, sizeof(param));
  param.sched_priority = 31;// RT
  error = pthread_attr_setschedparam(&attr, &param);
  if (error != 0) printf("error = %d\n", error);

  // create and start the thread
  if ((error = pthread_create(&this->thread, &attr, this->motion_timing, this)) != 0)
    exit(-1);

  this->timer_running = true;
}

void LinuxMotionTimer::Stop(void)
{
  int error = 0;

  // seti the flag to end the thread
  if (this->timer_running)
    {
      this->finish_thread = true;
      // wait for the thread to end
      if ((error = pthread_join(this->thread, NULL)) != 0)
        exit(-1);
      this->finish_thread = false;
      this->timer_running = false;
    }
}

bool LinuxMotionTimer::IsRunning(void)
{
  return this->timer_running;
}

LinuxMotionTimer::~LinuxMotionTimer()
{
  this->Stop();
  this->manager = NULL;
}

