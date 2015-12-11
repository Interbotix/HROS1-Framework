/*
 *   PS3Controller.c
 *
 *   Author: Farrell Robotics
 *	This source contians bits from in BlueZ and QTSixa
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <signal.h>

#include <malloc.h>

#include <sys/types.h>
#include <sys/poll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <sched.h>

#include <sys/unistd.h>
#include <pthread.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/sdp.h>
#include <bluetooth/hidp.h>
#include "../../include/PS3Controller.h"
// Maybe ifdef out unneded code
#define USE_JOY_MIXER

#define DEFAULT_VIBE_DURATION 0x0f
#define DEFAULT_VIBE_INTENSITY 0xff

#define HIDP_TRANS_HANDSHAKE			0x00
#define HIDP_TRANS_HID_CONTROL			0x10
#define HIDP_TRANS_GET_REPORT			0x40
#define HIDP_TRANS_SET_REPORT			0x50
#define HIDP_TRANS_GET_PROTOCOL			0x60
#define HIDP_TRANS_SET_PROTOCOL			0x70
#define HIDP_TRANS_GET_IDLE			0x80
#define HIDP_TRANS_SET_IDLE			0x90
#define HIDP_TRANS_DATA				0xa0
#define HIDP_TRANS_DATC				0xb0

#define HIDP_DATA_RTYPE_MASK			0x03
#define HIDP_DATA_RSRVD_MASK			0x0c
#define HIDP_DATA_RTYPE_OTHER			0x00
#define HIDP_DATA_RTYPE_INPUT			0x01
#define HIDP_DATA_RTYPE_OUPUT			0x02
#define HIDP_DATA_RTYPE_FEATURE			0x03

#define L2CAP_PSM_HIDP_CTRL 0x11
#define L2CAP_PSM_HIDP_INTR 0x13
#define SEND(sock, msg) do {				     \
			  int res = send(sock, msg, sizeof(msg), 0);	\
			  if ( res < 0 ) {/*perror("send");*/}		\
			} while ( 0 )

volatile _PS3 PS3, lastPS3;
volatile _JKeys JKeys, lastJKeys;
volatile int bPS3Active, gLeftJoyStickEnable, gRightJoyStickEnable, robotInStandby, gJoyIMUMixEnable;

static pthread_t PS3ThreadID, PS3KeyServerThreadID, JoyMixerThreadID, PS3LEDServerThreadID;
static int ctl, csk, isk, _ncsk;
static int gFlashWait;
static int rbt_state;
static byte debug_enable;

static void* PS3Thread( void *param );
static void* PS3KeyServer(void *param);
static void* PS3LEDServer(void *param);

#ifdef USE_JOY_MIXER
static void* JoyMixer(void *param);
#endif
static int l2cap_listen(const bdaddr_t *bdaddr, unsigned short psm, int lm, int backlog);
static int l2cap_accept(int sk, bdaddr_t *bdaddr);
static void flush1(int sock);
static int flush(int sock, int ncsk);
static void run_server(int csk, int isk);
static int StartPS3Server(void);
static void StopPS3Server(void);
static void StartPS3KeyServer(void);
static void StopPS3KeyServer(void);
#ifdef USE_JOY_MIXER
static void StartJoyMixer(void);
static void StopJoyMixer(void);
#endif
static void StartPS3LEDServer(void);
static void StopPS3LEDServer(void);

void setPS3LED(int ncsk, byte ledMask, byte ledDelay, byte vibeDuration, byte vibeIntensity);

void delayms(unsigned int ms)
{
	struct timespec t1;

	t1.tv_sec = 0;
	t1.tv_nsec = (unsigned long)((unsigned long)ms * 1e6);
	nanosleep(&t1, NULL);
	return;
}

void enable_sixaxis(int csk)
{
	char buf[128];

	unsigned char enable[] =
	{
		0x53, /* HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_FEATURE */
		0xf4, 0x42, 0x03, 0x00, 0x00
	};

	/* enable reporting */
	send(csk, enable, sizeof(enable), 0);
	recv(csk, buf, sizeof(buf), 0);
}

int PS3Controller_Start()
{
	bPS3Active = 0;
	gLeftJoyStickEnable = 1;
	gRightJoyStickEnable = 1;
	robotInStandby = 0;
	gJoyIMUMixEnable = 0;
	gFlashWait = 0;
	rbt_state = 0;
	debug_enable = 1;
	if (StartPS3Server() == 0)
		return 0;
#ifdef USE_JOY_MIXER
	StartJoyMixer();
#endif
	ClearPS3Keys();
	StartPS3KeyServer();
	return 1;
}

void PS3Controller_Stop()
{
	StopPS3KeyServer();
#ifdef USE_JOY_MIXER
	StopJoyMixer();
#endif
	StopPS3Server();
}

int l2cap_listen(const bdaddr_t *bdaddr, unsigned short psm, int lm, int backlog)
{
	struct sockaddr_l2 addr;
	struct l2cap_options opts;
	int sk;

	if ((sk = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP)) < 0)
		return -1;

	memset(&addr, 0, sizeof(addr));
	addr.l2_family = AF_BLUETOOTH;
	bacpy(&addr.l2_bdaddr, bdaddr);
	addr.l2_psm = htobs(psm);

	if (bind(sk, (struct sockaddr *) &addr, sizeof(addr)) < 0)
		{
			close(sk);
			return -1;
		}

	setsockopt(sk, SOL_L2CAP, L2CAP_LM, &lm, sizeof(lm));

	memset(&opts, 0, sizeof(opts));
	opts.imtu = 64;
	opts.omtu = HIDP_DEFAULT_MTU;
	opts.flush_to = 0xffff;

	setsockopt(sk, SOL_L2CAP, L2CAP_OPTIONS, &opts, sizeof(opts));

	if (listen(sk, backlog) < 0)
		{
			close(sk);
			return -1;
		}

	return sk;
}

int l2cap_accept(int sk, bdaddr_t *bdaddr)
{
	struct sockaddr_l2 addr;
	socklen_t addrlen;
	int nsk;

	memset(&addr, 0, sizeof(addr));
	addrlen = sizeof(addr);

	if ((nsk = accept(sk, (struct sockaddr *) &addr, &addrlen)) < 0)
		return -1;

	if (bdaddr)
		bacpy(bdaddr, &addr.l2_bdaddr);

	return nsk;
}

void flush1(int sock)
{
	struct pollfd p;

	p.fd = sock;
	p.events = POLLIN | POLLERR;

	int res = poll(&p, 1, 200);
	if ( res < 0 ) { /*perror("poll:flush1:");*/} // errors can happen here - deal with it
	if ( res )
		{
			unsigned char buf[8192];
			int nr = recv(sock, buf, sizeof(buf), 0);
			if ( nr < 0 ) { /*perror("read"); exit(1);*/ }
		}
}

void setPS3LED(int ncsk, byte ledMask, byte ledDelay, byte vibeDuration, byte vibeIntensity)
{
	char set0101[] = { HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_OUPUT, 0x01,
	                   0x00, 0x00, 0x00, 0x01, (char)0xff, 0x00, 0x00, 0x00,
	                   0x00, 0x00, (char)0xff, 0x10, 0x27, 0x32, 0x32, (char)0xff,
	                   0x10, 0x27, 0x32, 0x32, (char)0xff, 0x10, 0x27, 0x32,
	                   0x32, (char)0xff, 0x10, 0x27, 0x32, 0x32, 0x00, 0x00,
	                   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	                 };
	set0101[11] = ledMask;
	set0101[13] = set0101[18] = set0101[23] = set0101[28] = ledDelay;//off time
	set0101[14] = set0101[19] = set0101[24] = set0101[29] = (char)0xff;//on time
	set0101[5] = vibeDuration;
	set0101[6] = vibeIntensity;
	SEND(ncsk, set0101);

	flush1(ncsk);

	char set03f4[] = { HIDP_TRANS_SET_REPORT | HIDP_DATA_RTYPE_FEATURE, (char)0xf4, 0x42, 0x03, 0x00, 0x00 };
	SEND(ncsk, set03f4);
	flush1(ncsk);

	//printf("LED set to %02x %02x\n",ledMask,set0101[11]);
	return;
}

int flush(int sock, int ncsk)
{
	struct pollfd p;

	p.fd = sock;
	p.events = POLLIN | POLLERR;

	int res = poll(&p, 1, 200);
	if ( res < 0 ) { /*perror("poll:flush:"); exit(1);*/return 1; }
	if ( res )
		{
			unsigned char buf[8192];
			int nr = recv(sock, buf, sizeof(buf), 0);
			if ( nr < 0 ) { /*perror("read"); exit(1);*/ return 1;}
			if (nr == 50 && buf[0] == 0xa1)
				{
					memcpy((void *)PS3.keys, (void *)buf, sizeof(PS3));
				}
		}
	return 0;
}

void run_server(int csk, int isk)
{
	struct pollfd p[2];
	short events;
	int err, ncsk, nisk;

	rbt_state = 0;
serv_restart:
	p[0].fd = csk;
	p[0].events = POLLIN | POLLERR | POLLHUP;

	p[1].fd = isk;
	p[1].events = POLLIN | POLLERR | POLLHUP;
	if (debug_enable == 1) printf("Starting PS3 l2cap server.\n");
	while ( 1 )
		{
			p[0].revents = 0;
			p[1].revents = 0;

			err = poll(p, 2, 100);
			if (err <= 0)
				continue;

			events = p[0].revents | p[1].revents;

			if (events & POLLIN)
				{
					ncsk = l2cap_accept(csk, NULL);
					nisk = l2cap_accept(isk, NULL);
					_ncsk = ncsk;
					bPS3Active = 1;
					enable_sixaxis(ncsk);
					setPS3LED(ncsk, 2 << rbt_state, 0, DEFAULT_VIBE_DURATION, DEFAULT_VIBE_INTENSITY);
					enable_sixaxis(ncsk);
					while ( 1 )
						{
							if (flush(nisk, ncsk) != 0) break;
						}

					if (debug_enable == 1) printf("closing\n");
					sleep(1);
					close(nisk);
					sleep(1);
					close(ncsk);
					goto serv_restart;
				}
		}
}

void* PS3Thread( void *param )
{
	run_server(csk, isk);
	return 0;
} // ReaderThread

// returns 0 on failure, 1 on success
int StartPS3Server(void)
{
	bdaddr_t bdaddr, bany = {{0, 0, 0, 0, 0, 0}};
	char addr[18];
	int lm = L2CAP_LM_MASTER;// was 0;
//	struct timespec     clock_resolution;
	int rc;

	bacpy(&bdaddr, &bany);
	ba2str(&bdaddr, addr);
	ctl = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HIDP);
	if (ctl < 0)
		{
			printf("Can't open HIDP control socket\n");
			return 0;
		}

	csk = l2cap_listen(&bdaddr, L2CAP_PSM_HIDP_CTRL, lm, 10);
	if (csk < 0)
		{
			printf("Can't listen on HID control channel\n");
			close(ctl);
			return 0;
		}

	isk = l2cap_listen(&bdaddr, L2CAP_PSM_HIDP_INTR, lm, 10);
	if (isk < 0)
		{
			printf("Can't listen on HID interrupt channel\n");
			close(ctl);
			close(csk);
			return 0;
		}

	if (bacmp(&bdaddr, &bany))
		printf("Bluetooth HID daemon (%s)\n", addr);
	else
		printf("Bluetooth HID daemon\n");

	rc = pthread_create( &PS3ThreadID, NULL, &PS3Thread, NULL);
	if ( rc != 0 )
		{
			printf("Error creating PS3Thread: %s\n", strerror( rc ));
			return 0;
		}
	// if all is well the server is running in the background now
	return 1;
}

void StopPS3Server(void)
{

	pthread_cancel( PS3ThreadID );
	//wait for it to end
	pthread_join(PS3ThreadID, NULL);
	// close connection
	close(csk);
	close(isk);
	close(ctl);
	return;
}

byte PS3KeyChanged(void)
{
	byte j = 0, i, t1, t2, c = 0;

	for (i = 3; i < 5; i++)
		{
			if (lastPS3.keys[i] != PS3.keys[i])
				{
					c = 1;
					//printf("PS3.keys[%d] = %d\n",i,PS3.keys[i]);
					break;
				}
		}
	if (lastPS3.key.PS != PS3.key.PS)
		{
			c = 1;
		}
	j = c;
	// map joy sticks to buttons if greater then threshold
	t1 = 40;
	t2 = 80;
	c = 128;

	JKeys.key.rjx = JKeys.key.rjy = JKeys.key.ljx = JKeys.key.ljy = 0;
	if (gRightJoyStickEnable == 1)
		{
			if (PS3.key.RJoyX < c - t1)
				{
					JKeys.key.rjx = 2;
					if (PS3.key.RJoyX < c - t2)
						{
							JKeys.key.rjx = 1;
						}
				}
			if (PS3.key.RJoyX > c + t1)
				{
					JKeys.key.rjx = 3;
					if (PS3.key.RJoyX > c + t2)
						{
							JKeys.key.rjx = 4;
						}
				}
			if (PS3.key.RJoyY < c - t1)
				{
					JKeys.key.rjy = 2;
					if (PS3.key.RJoyY < c - t2)
						{
							JKeys.key.rjy = 1;
						}
				}
			if (PS3.key.RJoyY > c + t1)
				{
					JKeys.key.rjy = 3;
					if (PS3.key.RJoyY > c + t2)
						{
							JKeys.key.rjy = 4;
						}
				}
		}
	if (gLeftJoyStickEnable == 1)
		{
			if (PS3.key.LJoyX < c - t1)
				{
					JKeys.key.ljx = 2;
					if (PS3.key.LJoyX < c - t2)
						{
							JKeys.key.ljx = 1;
						}
				}
			if (PS3.key.LJoyX > c + t1)
				{
					JKeys.key.ljx = 3;
					if (PS3.key.LJoyX > c + t2)
						{
							JKeys.key.ljx = 4;
						}
				}
			if (PS3.key.LJoyY < c - t1)
				{
					JKeys.key.ljy = 2;
					if (PS3.key.LJoyY < c - t2)
						{
							JKeys.key.ljy = 1;
						}
				}
			if (PS3.key.LJoyY > c + t1)
				{
					JKeys.key.ljy = 3;
					if (PS3.key.LJoyY > c + t2)
						{
							JKeys.key.ljy = 4;
						}
				}
		}
	if (memcmp((void *)JKeys.keys, (void *)lastJKeys.keys, 4) != 0)	// joysticks have changed
		{
			j |= 2;// set flag
		}

	memcpy((void *)&lastPS3, (void *)&PS3, sizeof(PS3));
	memcpy((void *)&lastJKeys, (void *)&JKeys, sizeof(JKeys));
	return j;
}

void ClearPS3Keys(void)
{
	byte i;

	for (i = 0; i < 50; i++)
		{
			PS3.keys[i] = 0;
			lastPS3.keys[i] = 0;
			if (i < 4)
				{
					JKeys.keys[i] = 0;
					lastJKeys.keys[i] = 0;
				}
		}
	return;
}

void StartPS3LEDServer(void)
{
	int rc;

	rc = pthread_create( &PS3LEDServerThreadID, NULL, &PS3LEDServer, NULL );
	if ( rc != 0 )
		{
			printf("Error creating thread for PS3LEDServer: %s\n", strerror( rc ));
			return;
		}
	return;
}

void* PS3LEDServer(void *param)
{
	unsigned int n = 0;
	int z[] = {0x02, 0x06, 0x0c, 0x18, 0x10, 0x18, 0x0c, 0x06};
	while (1)
		{
			setPS3LED(_ncsk, z[n], 0, 0, 0);
			delayms(120);
			n++;
			n %= 8;
		}
	return 0;
}

void StopPS3LEDServer(void)
{
	pthread_cancel( PS3LEDServerThreadID );
	//wait for it to end
	pthread_join(PS3LEDServerThreadID, NULL);
	return;
}

void StartPS3KeyServer(void)
{
	pthread_attr_t tattr;
	int rc;

	pthread_attr_init(&tattr);
	pthread_attr_setschedpolicy(&tattr, SCHED_FIFO);
	rc = pthread_create( &PS3KeyServerThreadID, &tattr, &PS3KeyServer, NULL);
	if ( rc != 0 )
		{
			printf("Error creating thread for PS3KeyServer: %s\n", strerror( rc ));
			return;
		}
	return;
}

void* PS3KeyServer(void *param)
{
	unsigned int flashChange = 0;

	while (1)
		{
			delayms(1);
			if (PS3KeyChanged() != 0)
				{
					//printf("button pressed\n");
					/*
					if(PS3.key.Select != 0)	printf("Select pressed\n");
					if(PS3.key.LeftHat != 0) printf("LeftHat pressed\n");
					if(PS3.key.RightHat != 0)	printf("RightHat pressed\n");
					if(PS3.key.Start != 0) printf("Start pressed\n");
					if(PS3.key.Up != 0)	printf("Up pressed\n");
					if(PS3.key.Right != 0) printf("Right pressed\n");
					if(PS3.key.Down != 0)	printf("Down pressed\n");
					if(PS3.key.Left != 0)	printf("Left pressed\n");
					if(PS3.key.L2 != 0)	printf("L2 pressed\n");
					if(PS3.key.R2 != 0)	printf("R2 pressed\n");
					if(PS3.key.L1 != 0)	printf("L1 pressed\n");
					if(PS3.key.R1 != 0)	printf("R1 pressed\n");
					if(PS3.key.Triangle != 0) printf("Triangle pressed\n");
					if(PS3.key.Circle != 0) printf("Circle pressed\n");
					if(PS3.key.Cross != 0) printf("Cross pressed\n");
					if(PS3.key.Square != 0) printf("Square pressed\n");
					if(PS3.key.PS != 0) printf("PS pressed\n");
					if(JKeys.key.rjx != 0) printf("rjx = %d\n",JKeys.key.rjx);
					if(JKeys.key.rjy != 0) printf("rjy = %d\n",JKeys.key.rjy);
					if(JKeys.key.ljx != 0) printf("ljx = %d\n",JKeys.key.ljx);
					if(JKeys.key.ljy != 0) printf("ljy = %d\n",JKeys.key.ljy);
					*/
					int lcnt = 0;
					flashChange = 0;
					if (flashChange == 1)
						{
							SetPS3LEDFlashRate(gFlashWait);
							flashChange = 0;
						}
				}
		}
	return 0;
}

void StopPS3KeyServer(void)
{

	pthread_cancel( PS3KeyServerThreadID );
	//wait for it to end
	pthread_join(PS3KeyServerThreadID, NULL);
	return;
}

void SetPS3LEDFlashRate(int rate)
{
	int z;

	gFlashWait = rate;
	if (rate < 0 || rate > 255)
		return;

	switch (rbt_state)
		{
		case 0:
			z = 2;
			break;
		case 1:
			z = 6;
			break;
		case 2:
			z = 0xe;
			break;
		case 3:
			z = 0x1e;
			break;
		default:
			//case 4:
			z = 0x12;
			break;
		}

	setPS3LED(_ncsk, z, rate, 0, 0);

	return;
}

#ifdef USE_JOY_MIXER
void StartJoyMixer(void)
{
	int rc;

	rc = pthread_create( &JoyMixerThreadID, NULL, &JoyMixer, NULL);
	if ( rc != 0 )
		{
			printf("Error creating thread for JoyMixer: %s\n", strerror( rc ));
			return;
		}
	printf("joymixer started.\n");
	return;
}

void* JoyMixer(void *param)
{
//	int c=128;
//	long n=0;
//	double FBStep;
	double /*RLTurn,*/x, y;
	/*
	double FollowMaxFBStep = 25.0;
	double FollowMinFBStep = 5.0;
	double FollowMaxRLTurn = 40.0;
	double FitFBStep = 3.0;
	double FitMaxRLTurn = 40.0;
	*/

	while (1)
		{
			//apply mixing factors
			//joystick center is 128
			if (gRightJoyStickEnable == 1)
				{
					// x mixing
					// y mixing
					x = PS3.key.RJoyX - 128;
					y = PS3.key.RJoyY - 128;
					if (abs(x) > 10 || abs(y) > 10)
						{
							//RLTurn = 20*(x)/256;
							//FBStep = 25*(y)/256;
							//Walking::GetInstance()->X_MOVE_AMPLITUDE = FBStep;
							//Walking::GetInstance()->A_MOVE_AMPLITUDE = RLTurn;
						}
				}
			if (gLeftJoyStickEnable == 1)
				{
					// x mixing
					// y mixing
				}

			if (gJoyIMUMixEnable == 1)
				{
					//n = 0;
					//int accX,accY,accZ,zGyro;
					//have to change endianess
					//accX = ((PS3.key.accX & 0x3)<<8) | ((PS3.key.accX & 0xff00)>>8);
					//accY = ((PS3.key.accY & 0x3)<<8) | ((PS3.key.accY & 0xff00)>>8);
					//accZ = ((PS3.key.accZ & 0x3)<<8) | ((PS3.key.accZ & 0xff00)>>8);
					//zGyro = ((PS3.key.zGyro & 0x3)<<8) | ((PS3.key.zGyro & 0xff00)>>8);

					// xAccel mixing
					// yAccel mixing
					// zAccel mixing
					// zGyro mixing
				}
			//printf("\raccX = %5d, accY = %5d, accZ = %5d, zGyro = %5d  ",accX,accY,accZ,zGyro);
			delayms(2);
		}
	return 0;
}

void StopJoyMixer(void)
{
	pthread_cancel( JoyMixerThreadID );
	//wait for it to end
	pthread_join(JoyMixerThreadID, NULL);
	return;
}
#endif

void PS3Vibrate(void)
{
	int z;

	switch (rbt_state)
		{
		case 0:
			z = 2;
			break;
		case 1:
			z = 6;
			break;
		case 2:
			z = 0xe;
			break;
		case 3:
			z = 0x1e;
			break;
		default:
			//case 4:
			z = 0x12;
			break;
		}
	setPS3LED(_ncsk, z, gFlashWait, DEFAULT_VIBE_DURATION, DEFAULT_VIBE_INTENSITY);
	return;
}

int ToggleRobotStandby(void)
{
	int z;

	if (robotInStandby == 1)
		{
			StopPS3LEDServer();
			switch (rbt_state)
				{
				case 0:
					z = 2;
					break;
				case 1:
					z = 6;
					break;
				case 2:
					z = 0xe;
					break;
				case 3:
					z = 0x1e;
					break;
				default:
					//case 4:
					z = 0x12;
					break;
				}
			for (int i = 0; i < 5; i++) delayms(100);
			setPS3LED(_ncsk, z, gFlashWait, DEFAULT_VIBE_DURATION, DEFAULT_VIBE_INTENSITY);
			robotInStandby = 0;
		}
	else
		{
			// flash LED's and wait till release from standby - locked till unlocked
			StartPS3LEDServer();
			robotInStandby = 1;
		}
	return robotInStandby;
}

/*
main()
{
  printf("PS3 Controller %s Started\n",PS3Controller_Start()==1?"":"NOT");

	printf("Ready.\n");
  while(1);
}

*/

