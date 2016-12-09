/*
  LinuxJoyEx.h - Library for interfacing with ArbotiX LinuxJoy
  Copyright (c) 2009-2012 Michael E. Ferguson.  All right reserved.

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

// This version was modified from the original one by Kurt to run on Linux for Raspberry Pi.
// Also I renamed some members and the like to make it easier to understand...
// Also Added support to optionally use network sockets to
/*
 DS4 Controller Input mapping:

 axes[0] = Left Stick Horizontal
 axes[1] = Left Stick Vertical
 axes[2] = Right Stick Horizontal
 axes[3] = L2 Trigger
 axes[4] = R2 Trigger
 axes[5] = Right Stick Vertical
 axes[6] = D-Pad Left and Right
 axes[7] = D-Pad Up and Down
 buttons[0] = Square
 buttons[1] = X
 buttons[2] = Circle
 buttons[3] = Triangle
 buttons[4] = L1 Trigger
 buttons[5] = R1 Trigger
 buttons[6] = L2 Trigger
 buttons[7] = R2 Trigger
 buttons[8] = Share Button
 buttons[9] = Options Button
 buttons[10] = Left Stick Push
 buttons[11] = Right Stick Push
 buttons[12] = PS Button
 buttons[13] = Trackpad Push
 */

/* DS3 Controller Input Mappings...
    axes[0] = Left Stick Horizontal
    axes[1] = Left Stick Vertical
    axes[2] = Right Stick Horizontal
    axes[3] = Right Stick Vertical
    buttons[0] = select Button
    buttons[1] = Left Stick Push
    buttons[2] = Right Stick Push
    buttons[3] = Start Button
    buttons[4] = D-UP A8-
    buttons[5] = D-Rt A9
    buttons[6] = D-DN A10
    buttons[7] = D-LF
    buttons[8] = L2 Trigger A12
    buttons[9] = R2 Trigger A13
    buttons[10] = L1 Trigger A14
    buttons[11] = R1 Trigger A15
    buttons[12] = Triangle A16
    buttons[13] = Circle A17
    buttons[14] = X - A18
    buttons[15] = Square - a19
    buttons[16] = PS Button
 */

#ifndef LinuxJoy_h
#define LinuxJoy_h
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdint.h>

//==============================================================================
// Define enums for logical Axes and Button mappings
//==============================================================================

namespace JOYSTICK_AXES
{
    enum { LX = 0, LY, RX, RY };
}

namespace JOYSTICK_BUTTONS
{
    enum { SELECT_OPT = 0, L3, R3, START_SHARE, D_UP, D_RIGHT,
           D_DOWN, D_LEFT, L2, R2, L1, R1, TRI, CIRCLE,
           X, SQUARE, PS, TRACK
         };
}


//==============================================================================
// Define our Joystick class.
//==============================================================================
class LinuxJoy
{
    public:
        LinuxJoy();
        bool begin(const char *pszComm);
        void end();                              // release any resources.
        int  readMsgs();                         // must be called regularly to clean out Serial buffer
        void setRepeatDelay(unsigned int repeat_delay_ms) { repeat_delay_ms_ = repeat_delay_ms;};       // how long before allowing messages to repeat...
        bool connected() { return data_valid_ ;};   // Do we have an active joystick

        // methods for returning button states
        uint32_t buttonsDown()  {return button_values_;}; // return state of all buttons;
        bool button(int ibtn);
        bool buttonPressed(int ibtn);
        bool buttonReleased(int ibtn);

        // methods for returning axes states
        // Keep information about the different Axis of this Joystick.
        int axis(int index_axis);                // return logical axis value
        int axisJoystick(int joystick_axis);     // return actual axis by js index

        // For buttons, I am going to assume we have less than 32 buttons, so
        // keep all of them in a bitmask.
        // Likewise for buttons
        int joystickButtonCount()  {return num_of_buttons_;};
        int joystickAxisCount()  {return num_of_axis_;};
        const char *JoystickName(void) {return name_of_joystick_;};

        // Name of joystick...

        // 0x1 - print buttons, 0x2 - axes, 0x3 - both
        void setDebugPrintLevel(uint8_t print_level) {print_level_ = print_level;} ;

    private:
        int                 *axis_values_;           // current values for the different axis
        int                 num_of_buttons_;         // Number of buttons on physical
        int                 num_of_axis_;            // Number of Axis
        char                name_of_joystick_[80];

        uint32_t            button_values_;          // Current button values;
        uint32_t            previous_button_values_; // The previous button values.

        unsigned int        repeat_delay_ms_;        // What is our delay speed.
        unsigned long       last_message_time_;      // when did we last return a message?
        //Private stuff for Linux threading and file descriptor, and open file...
        pthread_mutex_t     lock_;                   // A lock to make sure we don't walk over ourself...
        bool                data_changed_;           // Do we have a valid packet?
        bool                data_valid_;             // Do we have some valid data?
        char                *input_device_name_;     // Our device name /dev/input/ps0
        bool                cancel_thread_;          // Cancel any input threads.
        uint8_t             print_level_;            // debug print level.
        pthread_t           tidLinuxJoy_;            // Thread Id of our reader thread...
        pthread_barrier_t   thread_barrier_;
        const uint8_t*      ltop_axis_mapping_;      // is there any axis mappings?
        int                 count_axis_mapping_;     // size of it

        // Values updated by worker thread.
        uint32_t            thread_buttons_;         // get bit map of all button states
        int                 *thread_axis_;           // Current values for the different axis

        static void *JoystickThreadProc(void *);

};

#endif
