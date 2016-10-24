#include <3ds.h>
#include <3ds/services/apt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <scenic/proc.h>
#include <scenic/dma.h>

#define HID_PID 0x10

// 9.2 HID
//#define HID_PATCH1_LOC 0x101dbc
//#define HID_PATCH2_LOC 0x107924

// 11.0 HID
#define HID_PATCH1_LOC 0x101de0
#define HID_PATCH2_LOC 0x107acc
#define HID_PATCH3_LOC 0x106a74
#define HID_CAVE_LOC   0x1094B8

#define HID_DAT_LOC	   0x10df00
#define HID_TS_RD_LOC 0x10df04
#define HID_TS_WR_LOC 0x10df08

#ifndef NTR
void noop(const char *a, ...)
{ (void)a; }
#define OUTPUT noop
#else
#define OUTPUT printf
#endif

scenic_process *self;
scenic_process *hid;

void read_input();
extern u32 read_input_sz;

bool run = true;

void input_loop(void* a)
{
	u32 input_loc = 0x10df20;

	acInit();
	u32 *sockbuf = (u32*)memalign(0x1000, 0x10000);
	Result res = socInit(sockbuf, 0x10000);

	if(res != 0)
	{
		acExit();
		return;
	}

	struct sockaddr_in addr;
	int sockfd;
	int byte_count;
	char buf[512];

	addr.sin_family = AF_INET;
	addr.sin_port = htons(4950);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sockfd == -1)
	{
		socExit();
		acExit();
		return;
	}
	int r = bind(sockfd, (struct sockaddr*) &addr, sizeof(addr));
	if(r == -1)
	{
		close(sockfd);
		socExit();
		acExit();
		return;
	}

	while(run)
	{
		byte_count = recv(sockfd, buf, sizeof buf, 0);
		if(byte_count == -1)
		{
			break;
		}

		if(byte_count >= 12)
		{
			dma_copy(hid, (void*)input_loc, self, buf, 12);
		}
	}

	socExit();
	acExit();
}

int main()
{
	#ifdef NTR
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);
	#endif

	OUTPUT("injecting into hid..\n");

	self = proc_open((u32)-1, 0);
	hid = proc_open(0x10, 0);

	u32 new_loc = HID_DAT_LOC;
	u32 test = 0;

	int r = dma_copy(self, &test, hid, (void*)new_loc, 4);
	if (r)
	{
		OUTPUT("copy returned %08x\n", r);
		exit(0);
	}

	bool err = false;

	if (test != 0)
	{
		OUTPUT("Already patched? \n");
		err = true;
	}
	else
	{
		u32 f = 0xffffffff;
		r = dma_copy(hid, (void*)new_loc, self, &f, 4);
		if (r != 0)
		{
			OUTPUT("init copy failed\n");
			err = true;
		}

		if (!err && dma_protect(hid, (void*)(HID_PATCH1_LOC & (~0xfff)), 0x1000) != 0)
		{
			OUTPUT("patch 1 prot failed\n");
			err = true;
		}

		if (!err && dma_copy(hid, (void*)HID_PATCH1_LOC, self, &new_loc, 4) != 0)
		{
			OUTPUT("patch 1 copy failed\n");
			err = true;
		}

		if (!err && dma_protect(hid, (void*)(HID_PATCH2_LOC & (~0xfff)), 0x1000) != 0)
		{
			OUTPUT("patch 2 prot failed\n");
			err = true;
		}

		if (!err && dma_copy(hid, (void*)HID_PATCH2_LOC, self, &new_loc, 4) != 0)
		{
			OUTPUT("patch 2 copy failed\n");
			err = true;
		}

		if (!err && dma_protect(hid, (void*)(HID_PATCH3_LOC & (~0xfff)), 0x1000) != 0)
		{
			OUTPUT("patch 3 prot failed\n");
			err = true;
		}

		if(!err)
		{
			proc_hook(hid, HID_PATCH3_LOC, HID_CAVE_LOC, (u32*)&read_input, read_input_sz);
			dma_kill_cache();
		}
	}
#ifndef NTR
	input_loop(NULL);
	svcExitProcess();
#else
	proc_close(hid);
	proc_close(self);
	
	if(err)
	{
		printf("An error occured!\n");
		while (aptMainLoop())
		{
			gspWaitForVBlank();
			hidScanInput();

			u32 kDown = hidKeysDown();
			if (kDown & KEY_START)
				break; // break in order to return to hbmenu
					   // Flush and swap framebuffers
			gfxFlushBuffers();
			gfxSwapBuffers();
		}
	}
#endif

	return 0;
}
