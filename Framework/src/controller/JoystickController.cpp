/*
   JoystickController.cpp - This file contains the code that I use in my projects
   to handle the linux /dev/input/js0 input events and keep the appropriate
   information for me to then use in programs.

   This implemention will also try to handle information about different joysticks
   which currently includes DS3 and DS4 and map their Axes and buttons to a logical
   values, such that the using program does not have to worry about it.

   This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

// Note: This one is hacked up to try to compile under linux...

#include "JoystickController.h"
#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/joystick.h>


#define JOY_DEV "/dev/js0"

//====================================================================
// Define which axes and buttons map for different Joysticks
//====================================================================
// First define our logical names for buttons and indexes.
// Probably close to PS3 mappings.
// Now define mapping tables for which joysticks we know about...
// PS3 Don't need we are logically setup as PS3...
// Used later on maps logical to physical.
typedef struct
{
    uint8_t     map_axis;
    uint8_t     map_button_positive;
    uint8_t     map_button_negative;
} AXIS_BUTTON_MAP;

// DS4
static const uint8_t g_ltop_ds4_axis_mapping[] = {0, 1, 2, 5};
static const uint8_t g_ptol_ds4_button_mapping[] =
{
    15, 14, 13, 12, 10, 11, 8, 9, 3, 0, 1, 2, 16, 17
};

static const AXIS_BUTTON_MAP g_abmap_ds4[] =
{
    {7, JOYSTICK_BUTTONS::D_DOWN, JOYSTICK_BUTTONS::D_UP},
    {6, JOYSTICK_BUTTONS::D_RIGHT, JOYSTICK_BUTTONS::D_LEFT},
    {0xff, 0xff, 0xff}
};


//====================================================================
// Millis - helper function
//====================================================================
unsigned long millis(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC,  &ts );
    return ( ts.tv_sec * 1000 + ts.tv_nsec / 1000000L );
}


//====================================================================
// Constructor
//====================================================================
LinuxJoy::LinuxJoy()
{
    data_changed_ = false;
    print_level_ = 1;    // Will change to 0 later...

    // Set initial states
    thread_buttons_ = 0;
    button_values_ = 0;
    previous_button_values_ = 0;
    repeat_delay_ms_ = 50;              // Max 20 per second?
    data_valid_ = false;


}


//====================================================================
//====================================================================
static ssize_t _read(int fd, void *buf, size_t count)
{
    extern ssize_t read(int fd, void *buf, size_t count);
    return read(fd, buf, count);
}

//====================================================================
// Main thread procedure that handles the events coming from joystick
//====================================================================
void *LinuxJoy::JoystickThreadProc(void *pv)
{
    LinuxJoy *pljoy = (LinuxJoy*)pv;
    fd_set fdset;                                // file descriptor set to wait on.
    timeval tv;                                   // how long to wait.
    int joy_fd = -1;                              // File descriptor
    bool error_reported = false;
    int count_messages_to_ignore = 0;             // How many messages should we ignore
    int num_of_axis = 0, num_of_buttons = 0;

    const uint8_t*  ptol_button_mapping = NULL;    // is there any button mappings?
    AXIS_BUTTON_MAP const * axis_button_map = NULL;

    pthread_barrier_wait(&pljoy->thread_barrier_);

    printf("Thread start(%s)\n", pljoy->input_device_name_);

    int ret = 0;
    struct js_event js;
    int iIndex = 0; // index into which byte we should start reading into of the joystick stuff...

    while (!pljoy->cancel_thread_)
    {
        // First lets see if we have a joystick object yet, if not try to open.
        //
        if (joy_fd == -1)
        {
            if ((joy_fd = open(pljoy->input_device_name_, O_RDONLY)) != -1 )
            {
                ioctl(joy_fd, JSIOCGAXES , &num_of_axis);
                ioctl(joy_fd, JSIOCGBUTTONS , &num_of_buttons);
                ioctl(joy_fd, JSIOCGNAME(80), &(pljoy->name_of_joystick_));

                pljoy->num_of_axis_ = num_of_axis;
                pljoy->num_of_buttons_ = num_of_buttons;
                //printf("Buttons %d Axes %d\n", num_of_buttons, num_of_axis);
                // Keep our realtime cache of Axis and buttons
                pljoy->thread_axis_ = (int *) calloc( pljoy->num_of_axis_ , sizeof(int));

                // Plus our ReadMessage which caches values
                pljoy->axis_values_ = (int *) calloc(pljoy->num_of_axis_ , sizeof(int));

                if (! pljoy->thread_axis_ ||  ! pljoy->axis_values_)
                    return (void*) - 1;    // ran out of memory???

                memset(pljoy->thread_axis_, 0, pljoy->num_of_axis_ * sizeof(int));

                // Debug  will comment out later
                printf( " Joy stick detected : %s \n \t %d axis \n\t %d buttons \n\n" , pljoy->name_of_joystick_ ,  pljoy->num_of_axis_ ,  pljoy->num_of_buttons_);

                // Lets try to setup button and axes mappings here.
                pljoy->ltop_axis_mapping_ = NULL;
                pljoy->count_axis_mapping_ = 0;
                ptol_button_mapping = NULL;
                axis_button_map = NULL;

                if ((pljoy->num_of_axis_ == 27) && (pljoy->num_of_buttons_ == 19))
                {
                    printf("PS3!\n");
                }
                else if ((pljoy->num_of_axis_ == 8) && (pljoy->num_of_buttons_ == 14))
                {
                    printf("DS4!\n");
                    pljoy->ltop_axis_mapping_ = g_ltop_ds4_axis_mapping;
                    pljoy->count_axis_mapping_ = sizeof(g_ltop_ds4_axis_mapping);
                    ptol_button_mapping = g_ptol_ds4_button_mapping;
                    axis_button_map = g_abmap_ds4;
                }
                else
                {
                    printf("Unknown\n");
                }

                fcntl( joy_fd, F_SETFL , O_NONBLOCK ); // use non - blocking methods
                error_reported = false;                 // if we lose the device...
                pljoy->data_valid_ = true;                     // let system know we are valid...
                count_messages_to_ignore = pljoy->num_of_axis_ + pljoy->num_of_buttons_;    // sends us some initial messages
            }
            else
            {
                if (!error_reported)
                {
                    printf(" Error opening joystick \n " );
                    error_reported = true;
                }
                usleep(250000);  // try again in 1/4 second...
            }
        }

        if (joy_fd != -1)
        {
            // Lets try using select to block our thread until we have some input available...
            FD_ZERO(&fdset);
            FD_SET(joy_fd, &fdset);                   // Make sure we are set to wait for our descriptor
            tv.tv_sec = 0;
            tv.tv_usec = 250000;                          // 1/4 of a second...
            // wait until some input is available...
            select(joy_fd + 1, &fdset, NULL, NULL, &tv);


            if (FD_ISSET(joy_fd, &fdset))
            {
                // try to read in rest of structure...
                while ((!pljoy->cancel_thread_) && (ret = _read(joy_fd, ((char*)&js) + iIndex, sizeof(js) - iIndex)) > 0)
                {
                    iIndex += ret; // see how many bytes we have of the js event...
                    if (iIndex == sizeof(js))
                    {
                        // When joystick object is first opened it sends us a set of initial values.
                        // Sometimes the inital axis are bogus -32767 so best to ignore.
                        if (count_messages_to_ignore == 0)
                        {
                            pthread_mutex_lock(&pljoy->lock_);  // make sure we keep things consistent...
                            switch (js.type & ~ JS_EVENT_INIT)
                            {
                            case JS_EVENT_AXIS :
                                pljoy->thread_axis_ [ js.number ] = js.value;
                                if (pljoy->print_level_ & 2)
                                    printf("A %d : %d\n", js.number, js.value);

                                if (axis_button_map)
                                {
                                    AXIS_BUTTON_MAP const * abm = axis_button_map;
                                    while (abm->map_axis != 0xff)
                                    {
                                        if (abm->map_axis == js.number)
                                        {
                                            // Axis changed that has logical buttons associated.
                                            if (js.value > 0)
                                            {
                                                if (pljoy->print_level_ & 1)
                                                    printf("AB %d : 1\n", abm->map_button_positive);
                                                pljoy->thread_buttons_ |= (1 << abm->map_button_positive);
                                            }
                                            else if (js.value < 0)
                                            {
                                                if (pljoy->print_level_ & 1)
                                                    printf("AB %d : 1\n", abm->map_button_negative);
                                                pljoy->thread_buttons_ |= (1 << abm->map_button_negative);
                                            }
                                            else
                                            {
                                                if (pljoy->print_level_ & 1)
                                                    printf("AB %d %d: 0\n", abm->map_button_positive,
                                                           abm->map_button_negative);
                                                pljoy->thread_buttons_ &= ~(1 << abm->map_button_positive);
                                                pljoy->thread_buttons_ &= ~(1 << abm->map_button_negative);
                                            }
                                            break;
                                        }
                                        abm++;  // point to next ne
                                    }
                                }
                                break;

                            case JS_EVENT_BUTTON :
                                // Do logical button mapping if appropriate.
                                int log_button = (ptol_button_mapping) ? ptol_button_mapping[js.number] :
                                                 js.number;
                                if (pljoy->print_level_ & 1)
                                    printf("B %d %d: %d\n", js.number, log_button, js.value);

                                if (js.value)
                                    pljoy->thread_buttons_ |= (1 << log_button);
                                else
                                    pljoy->thread_buttons_ &= ~(1 << log_button);
                            }
                            pljoy->data_changed_ = true;
                            pthread_mutex_unlock(&pljoy->lock_);
                        }
                        else
                        {
                            count_messages_to_ignore--;
                            //printf("Ignore %c %d: %d\n", ((js.type & ~ JS_EVENT_INIT) == JS_EVENT_AXIS)? 'A' : 'B', js.number, js.value);
                        }

                        iIndex = 0;         // start over reading from start of data...

                    }
                }

                if ((ret < 0) && (errno != EAGAIN))
                {
                    printf("Error reading from Joystick %d\n", errno);
                    // lets free the memory we allocated for old joystick...
                    pthread_mutex_lock(&pljoy->lock_);  // make sure we keep things consistent...
                    free (pljoy->thread_axis_);
                    pljoy->thread_axis_ = NULL;     // and zero it out
                    free (pljoy->axis_values_);
                    pljoy->axis_values_ = NULL;
                    pljoy->thread_buttons_  = 0;    // clear out all button states
                    pljoy->data_changed_ = false;
                    pljoy->data_valid_ = false;
                    pthread_mutex_unlock(&pljoy->lock_);

                    // And close off the file
                    close(joy_fd);
                    joy_fd = -1;
                }
            }
            // If we get to here try sleeping for a little time
            //usleep(1000);                                 // Note: we could maybe simply block the thread until input available!
        }
    }
    if (joy_fd != -1)
    {
        // release anything that we allocated.
        close (joy_fd);
        free (pljoy->thread_axis_);
        free (pljoy->axis_values_);
    }
    printf("LinuxJoy - LinuxJoy thread exit\n");
    return 0;
}


//====================================================================
// Begin
//====================================================================
bool LinuxJoy::begin(const char *pszDevice)
{
    int err;
    // Create our lock to make sure we can do stuff safely
    if (pthread_mutex_init(&lock_, NULL) != 0)
        return false;

    cancel_thread_ = false; // Flag to let our thread(s) know to abort.
    last_message_time_ = millis();      // when did we last return a message?


    // remember the device name
    input_device_name_ = (char*)malloc(strlen(pszDevice) + 1);
    if (!input_device_name_)
    {
        printf("malloc of device name failed\n");
        return -1;
    }

    strcpy(input_device_name_, pszDevice);
    // Now we need to create our thread for receiving messages from the joystick...
    pthread_barrier_init(&thread_barrier_, 0, 2);
    err = pthread_create(&tidLinuxJoy_, NULL, &JoystickThreadProc, this);
    if (err != 0)
        return false;

    // sync startup
    pthread_barrier_wait(&thread_barrier_);

    return true;
}

//====================================================================
// End - release anything and kill off our worker thread
//====================================================================
void LinuxJoy::end()
{
    cancel_thread_ = true;

    // pthread_join to sync
    pthread_join(tidLinuxJoy_, NULL);

    // destroy barrier
    pthread_barrier_destroy(&thread_barrier_);
}


//====================================================================
// ReadMsgs - Main function that using program uses to say it is
//    ready to get the current state of the Axes and Buttons
//====================================================================
int LinuxJoy::readMsgs()
{
    // Probably should probably rename some of this...

    if (data_valid_ && (data_changed_ || ((millis() - last_message_time_) >= repeat_delay_ms_)))
    {
        pthread_mutex_lock(&lock_);

        memcpy(axis_values_, thread_axis_, sizeof(thread_axis_[0])*num_of_axis_);
        previous_button_values_ =  button_values_;
        button_values_ = thread_buttons_;
        last_message_time_ = millis();              // remember when we received the message.
        data_changed_ = false;                      // clear out so we know if something new comes in
        pthread_mutex_unlock(&lock_);
        return 1;
    }
    return 0;
}

// Decided to not inline these as may have special case code for psuedo buttons
//====================================================================
// button: is the button currently pressed
//====================================================================
bool LinuxJoy::button(int ibtn)
{
    return ((button_values_ &  (1 << ibtn)) != 0);
}


//====================================================================
// buttonPressed: Has the button just been pressed.
//====================================================================
bool LinuxJoy::buttonPressed(int ibtn)
{
    return ((button_values_ &  (1 << ibtn)) != 0) &&
           ((previous_button_values_ &  (1 << ibtn)) == 0 );
}

//====================================================================
// buttonReleased: Has the button just been released.
//====================================================================
bool LinuxJoy::buttonReleased(int ibtn)
{
    return ((button_values_ &  (1 << ibtn)) == 0) &&
           ((previous_button_values_ &  (1 << ibtn)) != 0 );
}

//====================================================================
// axis
//====================================================================
int LinuxJoy::axis(int index_axis)
{
    if (ltop_axis_mapping_)
    {
        if (index_axis >= count_axis_mapping_)
            return (0);    // out of range error out

        index_axis = ltop_axis_mapping_[index_axis];
    }
    return axisJoystick(index_axis);    // use the main one
}

//====================================================================
// axis
//====================================================================
int LinuxJoy::axisJoystick(int index_axis)
{
    if (index_axis >= num_of_axis_)
        return (0);    // out of range error out

    return axis_values_[index_axis];
}

