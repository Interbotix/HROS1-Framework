
#ifndef _SERIAL_INPUT_COMMANDER_H_
#define _SERIAL_INPUT_COMMANDER_H_

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


#define TRAVEL_DEAD_ZONE 8      // The deadzone for the analog input from the remote

/* bitmasks for buttons array */
#define BUT_R1      0x01
#define BUT_R2      0x02
#define BUT_R3      0x04
#define BUT_L4      0x08
#define BUT_L5      0x10
#define BUT_L6      0x20
#define BUT_RT      0x40
#define BUT_LT      0x80

/* the Commander will send out a frame at about 30hz, this class helps decipher the output. */

class Commander
{
    public:
        Commander();
        bool Init ( void );
        bool begin( char *pszComm, speed_t baud );
        void ControlInput ( void );
        void ControlTest ( void );
        void TurnRobotOff ( void );
        void UseSouthPaw( void );     // enable southpaw configuration
        int ReadMsgs( void );         // must be called regularly to clean out Serial buffer
        // joystick values are -125 to 125
        signed char RightV;      // vertical stick movement
        signed char RightH;      // horizontal stick movement
        signed char LeftV;      // vertical stick movement
        signed char LeftH;      // horizontal stick movement
        // 0-1023, use in extended mode
        int pan;
        int tilt;
        // buttons are 0 or 1 (PRESSED), and bitmapped
        unsigned char buttons;  //
        unsigned char ext;      // Extended function set
    private:
        // internal variables used for reading messages
        unsigned char vals[6];  // temporary values, moved after we confirm checksum
        int index;              // -1 = waiting for new packet
        int checksum;
        unsigned char status;
        //Private stuff for Linux threading and file descriptor, and open file...
        int fd;         // file descriptor
        FILE *pfile;        // Pointer to file
        pthread_t tid;      // Thread Id of our reader thread...
        pthread_mutex_t lock;   // A lock to make sure we don't walk over ourself...
        bool fValidPacket;      // Do we have a valid packet?
        unsigned char bInBuf[7]; // Input buffer we use in the thread to process partial messages.
        char *_pszDevice;
        speed_t _baud;
        static void *XBeeThreadProc(void *);
};


#endif // _SERIAL_INPUT_COMMANDER_H_

