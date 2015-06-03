
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
//#indlude "SerialInputCommander.h"
#include "../../include/SerialInputCommander.h"


int fd;
FILE *f;

char szDevice[] = "/dev/ttyXBEE";

/* Constructor */
Commander::Commander(){
    index = -1;
    status = 0;
    fValidPacket = false;
}





/* process messages coming from Commander
 *  format = 0xFF RIGHT_H RIGHT_V LEFT_H LEFT_V BUTTONS EXT CHECKSUM */
int Commander::ReadMsgs(){
    if (fValidPacket) {
        pthread_mutex_lock(&lock);
        // We have a valid packet so lets use it.  But lets do that using our mutex to keep things consistent...
        if((status&0x01) > 0){     // SouthPaw
            RightV = (signed char)( (int)vals[0]-128 );
            RightH = (signed char)( (int)vals[1]-128 );
            LeftV =  (signed char)( (int)vals[2]-128 );
            LeftH =  (signed char)( (int)vals[3]-128 );
        }else{
            LeftV =  (signed char)( (int)vals[0]-128 );
            LeftH =  (signed char)( (int)vals[1]-128 );
            RightV = (signed char)( (int)vals[2]-128 );
            RightH = (signed char)( (int)vals[3]-128 );
        }
        pan = (vals[0]<<8) + vals[1];
        tilt = (vals[2]<<8) + vals[3];
        buttons = vals[4];
        ext = vals[5];
        fValidPacket = false;   // clear out so we know if something new comes in
        pthread_mutex_unlock(&lock);
        return 1;
    }
    return 0;
}

//=============================================================================
// Global - Local to this file only...
//=============================================================================

unsigned char    buttonsPrev;
unsigned char    extPrev;


//================================================================================
// This is The function that is called by the Main program to initialize Xbee port
//================================================================================

bool Commander::Init ( void )
{
	bool beginstatus;
	printf("Attempting to open Xbee port\n");
    // Lets try to open the XBee device...
    beginstatus = begin ( szDevice, B38400 );
    //command.begin(szDevice, B38400);
    if(beginstatus == 0){
    printf("Xbee FAILURE!\n");
	}
    //pass begin check through Init
    return beginstatus;
}


//==============================================================================
// This is The function that is called by the library to handle the port thread
//==============================================================================


bool Commander::begin(char *pszDevice,  speed_t baud){
    // Create our lock to make sure we can do stuff safely
    if (pthread_mutex_init(&lock, NULL) != 0)
        return false;

    _pszDevice = pszDevice;
    _baud = baud;
    // Now we need to create our thread for doing the reading from the Xbee
    int err = pthread_create(&tid, NULL, &XBeeThreadProc, this);
    if (err != 0)
        return false;
    return true;
printf( "Commander Begin Success!\n" );
}


//==============================================================================
// This is The main code to input function to read inputs from the Commander and then
// process any commands.
//==============================================================================

//void Commander::ControlInput ( void )
//{
   // Lets try to open the XBee device...
//    command.Init();
	
    //// See if we have a new command available...
    //if ( command.ReadMsgs() > 0)
    //{

        //if ( ( buttons & BUT_LT ) && ! ( buttonsPrev & BUT_LT ) )
        //{
 
        //}
        
        //if ( ( buttons & BUT_L6 ) && ! ( buttonsPrev & BUT_L6 ) )
        //{
 
        //}

        //if ( ( buttons & BUT_L5 ) && ! ( buttonsPrev & BUT_L5 ) )
        //{
 
		//}
		
        //if ( ( buttons & BUT_L4 ) && ! ( buttonsPrev & BUT_L4 ) )
        //{

        //}
        
        //if ( ( buttons & BUT_R3 ) && ! ( buttonsPrev & BUT_R3 ) )
        //{

        //}

        //if ( ( buttons & BUT_R2 ) && ! ( buttonsPrev & BUT_R2 ) )
        //{

        //}
        
        
        //if ( ( buttons & BUT_R1 ) && ! ( buttonsPrev & BUT_R1 ) )
        //{

        //}
        
        //if ( ( buttons & BUT_RT ) && ! ( buttonsPrev & BUT_RT ) )
        //{

        //}        
        
////        g_InControlState.travelLength.x = -LeftH; //example blah blah
////        g_InControlState.travelLength.z = -LeftV;
////        g_InControlState.travelLength.y = -RightH;
////        g_InControlState.travelLength.y = -RightV;
		

		//buttonsPrev = buttons; // store last state

    //}
//}

//void Commander::ControlTest ( void )
//{
	//printf("Arbotix Commander XBee Test!\n");
    //// Lets try to open the XBee device...
    //command.begin(szDevice, B38400);
    //printf("After Begin!\n");
	//while(1)
    //{
    //// loop simply echo what we receive from xbee to terminal
    //// Now lets try to get data from the commander.
        //if (command.ReadMsgs())
        //{
            //// We have data.  see if anything has changed before
            ////if ((command.RightV != rightV) || (command.RightH != rightH) ||
                ////(command.LeftV != leftV) || (command.LeftH != leftH) ||
                ////(command.buttons != buttons) || (command.ext != ext))
            ////{
                //// Something changed so print it out
                //rightV = command.RightV;
                //rightH = command.RightH;
                //leftV = command.LeftV;
                //leftH = command.LeftH;
                //buttons = command.buttons;
                //ext = command.ext;
                //printf("%x %x - %d %d %d %d\n", buttons, ext, rightV, rightH, leftV, leftH);
            ////}
        //}
        //usleep(100);
	//}
//}


void *Commander::XBeeThreadProc(void *pv) {
    Commander *pcmdr = (Commander*)pv;
    fd_set readfs; // file descriptor set to wait on.
    timeval tv; // how long to wait.

    printf("Thread start(%s)\n", pcmdr->_pszDevice);

    // Lets do our init of the xbee here.
    // We will do all of the stuff to intialize the serial port plus we will spawn off our thread.
    struct termios tc;

    if ((pcmdr->fd = open(pcmdr->_pszDevice, O_RDWR | O_NOCTTY | O_SYNC | O_NONBLOCK)) == -1) {
        printf("Open Failed\n");
        return (void*)1;
    }

    if ((pcmdr->pfile = fdopen(pcmdr->fd, "r+")) == NULL) {
        return (void*)1;
    }


    setvbuf(pcmdr->pfile, NULL, _IONBF, BUFSIZ);
    fflush(pcmdr->pfile);


    if (tcgetattr(pcmdr->fd, &tc)) {
        perror("tcgetattr()");
        return (void*)1;
    }

    /* input flags */
    tc.c_iflag &= ~ IGNBRK;           /* enable ignoring break */
    tc.c_iflag &= ~(IGNPAR | PARMRK); /* disable parity checks */
    tc.c_iflag &= ~ INPCK;            /* disable parity checking */
    tc.c_iflag &= ~ ISTRIP;           /* disable stripping 8th bit */
    tc.c_iflag &= ~(INLCR | ICRNL);   /* disable translating NL <-> CR */
    tc.c_iflag &= ~ IGNCR;            /* disable ignoring CR */
    tc.c_iflag &= ~(IXON | IXOFF);    /* disable XON/XOFF flow control */
    /* output flags */
    tc.c_oflag &= ~ OPOST;            /* disable output processing */
    tc.c_oflag &= ~(ONLCR | OCRNL);   /* disable translating NL <-> CR */
    /* not for FreeBSD */
    tc.c_oflag &= ~ OFILL;            /* disable fill characters */
    /* control flags */
    tc.c_cflag |=   CLOCAL;           /* prevent changing ownership */
    tc.c_cflag |=   CREAD;            /* enable reciever */
    tc.c_cflag &= ~ PARENB;           /* disable parity */
        tc.c_cflag &= ~ CSTOPB;         /* disable 2 stop bits */
    tc.c_cflag &= ~ CSIZE;            /* remove size flag... */
    tc.c_cflag |=   CS8;              /* ...enable 8 bit characters */
    tc.c_cflag |=   HUPCL;            /* enable lower control lines on close - hang up */
#ifdef XBEE_NO_RTSCTS
    tc.c_cflag &= ~ CRTSCTS;          /* disable hardware CTS/RTS flow control */
#else
    tc.c_cflag |=   CRTSCTS;          /* enable hardware CTS/RTS flow control */
#endif
    /* local flags */
    tc.c_lflag &= ~ ISIG;             /* disable generating signals */
    tc.c_lflag &= ~ ICANON;           /* disable canonical mode - line by line */
    tc.c_lflag &= ~ ECHO;             /* disable echoing characters */
    tc.c_lflag &= ~ ECHONL;           /* ??? */
    tc.c_lflag &= ~ NOFLSH;           /* disable flushing on SIGINT */
    tc.c_lflag &= ~ IEXTEN;           /* disable input processing */

    /* control characters */
    memset(tc.c_cc,0,sizeof(tc.c_cc));

    /* set i/o baud rate */
    if (cfsetspeed(&tc, pcmdr->_baud)) {
        perror("cfsetspeed()");
        return (void*)1;
    }

    if (tcsetattr(pcmdr->fd, TCSAFLUSH, &tc)) {
        perror("tcsetattr()");
        return (void*)1;
    }

    /* enable input & output transmission */
    if (tcflow(pcmdr->fd, TCOON | TCION)) {
        perror("tcflow()");
        return (void*)1;
    }

    fflush(pcmdr->pfile);
    printf("Comm Thread Initialized.\n");

    // May want to add end code... But for now don't have any defined...
    int ch;
    for(;;) {
        FD_ZERO(&readfs);
        FD_SET(pcmdr->fd, &readfs); // Make sure we are set to wait for our descriptor
        tv.tv_sec = 0;
        tv.tv_usec = 250000;    // 1/4 of a second...
        select(pcmdr->fd + 1, &readfs, NULL, NULL, &tv);    // wait until some input is available...

         while((ch = getc(pcmdr->pfile)) != EOF) {
            if(pcmdr->index == -1){         // looking for new packet
                if(ch == 0xff){
                    pcmdr->index = 0;
                    pcmdr->checksum = 0;
                }
            }else if(pcmdr->index == 0){
                pcmdr->bInBuf[pcmdr->index] = (unsigned char) ch;
                if(pcmdr->bInBuf[pcmdr->index] != 0xff){
                    pcmdr->checksum += ch;
                    pcmdr->index++;
                }
            }else{
                pcmdr->bInBuf[pcmdr->index] = (unsigned char) ch;
                pcmdr->checksum += ch;
                pcmdr->index++;
                if(pcmdr->index == 7){ // packet complete
                    if(pcmdr->checksum%256 == 255){
                         // Lets grab our mutex to keep things consistent
                pthread_mutex_lock(&pcmdr->lock);
                for (int i=0; i < 6; i++)
                   pcmdr->vals[i] = pcmdr->bInBuf[i];
                pcmdr->fValidPacket = true;
                pthread_mutex_unlock(&pcmdr->lock);
                    }
                    pcmdr->index = -1;  // Say we are ready to start looking for start of next message...
                }
            }
        }
        // If we get to here try sleeping for a little time
        usleep(5000);
    }
    return 0;
}
