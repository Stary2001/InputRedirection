#include <3ds.h>
#include <3ds/services/apt.h>
#include <stdio.h>
#include <string>
#include <string.h>

#include <malloc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

u32 protect_remote_memory(Handle hProcess, void* addr, u32 size)
{
	return svcControlProcessMemory(hProcess, (u32)addr, (u32)addr, size, 6, 7);
}

u32 copy_remote_memory(Handle hDst, void* ptrDst, Handle hSrc, void* ptrSrc, u32 size)
{
	static u32 done_state = 0;

	u32 ret, i, state;
	u32 dmaConfig[20] = {0};
	Handle hDma;

	if((u32)ptrSrc != 0x1ec46000)
	{
		ret = svcFlushProcessDataCache(hSrc, ptrSrc, size);
	}

	ret = svcFlushProcessDataCache(hDst, ptrDst, size);
	ret = svcStartInterProcessDma(&hDma, hDst, ptrDst, hSrc, ptrSrc, size, dmaConfig);

	state = 0;

	if(done_state == 0)
	{
		ret = svcGetDmaState(&state, hDma);
		svcSleepThread(1000000000);
		ret = svcGetDmaState(&state, hDma);
		done_state = state;
	}

	for (i = 0; i < 10000; i++ )
	{
		state = 0;
		ret = svcGetDmaState(&state, hDma);
		if (state == done_state)
		{
			break;
		}
		svcSleepThread(1000000);
	}

	if (i >= 10000)
	{
		return 1; // error
	}

	svcCloseHandle(hDma);
	ret = svcInvalidateProcessDataCache(hDst, ptrDst, size);
	if (ret != 0)
	{
		return ret;
	}
	return 0;
}

u32 open_current_process()
{
	u32 handle = 0;
	u32 ret;
	u32 hCurrentProcess;
	u32 currentPid;

	svcGetProcessId(&currentPid, 0xffff8001);
	ret = svcOpenProcess(&handle, currentPid);
	if (ret != 0)
	{
		return 0;
	}
	hCurrentProcess = handle;
	return hCurrentProcess;
}

u32 open_process(u32 pid)
{
	Handle hProcess;
	Result r = svcOpenProcess(&hProcess, pid);
	if(r != 0)
	{
		return 0;
	}
	return hProcess;
}

u32 hid_in = 0;
u32 circle_in = 0xffffffff;
u32 ts_in = 0xffffffff;

bool run = true;

FILE *log_file;

void input_loop(void* a)
{
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
			u32 *b = (u32*)buf;
			hid_in = b[0];
			circle_in = b[1];
			ts_in = b[2];
		}
	}

	socExit();
	acExit();
}

void transport_loop(void *unused)
{

	Handle hid = open_process(0x10);
	Handle self = open_current_process();
	u32 hid_loc = 0x0010df00;
	u32 ts_rd_loc = 0x0010df08;
	u32 ts_wr_loc = 0x0010df10;

	while(run)
	{
		u32 orig_hid;
		u32 orig_ts_and_circle[2];

		copy_remote_memory(self, &orig_hid, hid, (void*)0x1ec46000, 4);
		copy_remote_memory(self, &orig_ts_and_circle, hid,  (void*)ts_wr_loc, 8);

		if(ts_in != 0xFFFFFFFF)
		{
			orig_ts_and_circle[0] = ts_in;
		}

		if(circle_in != 0xFFFFFFFF)
		{
			orig_ts_and_circle[1] = circle_in;
		}

		orig_hid &= ~hid_in; // Clear the bits set in B.
		copy_remote_memory(hid, (void*)hid_loc, self, &orig_hid, 4);
		copy_remote_memory(hid, (void*)ts_rd_loc, self, &orig_ts_and_circle, 8);

		svcSleepThread(10000000ULL); // Free up some CPU time. (run at 100Hz)
	}
}

int main(int argc, const char* argv[])
{
/*	sdmcInit();
	log_file = fopen("service_log.log", "w");*/

	Thread inp_thread = threadCreate(input_loop, 0, 0x1000, 0x30, 0, false);
	//Thread transport_thread = threadCreate(transport_loop, 0, 0x1000, 0x30, 1, false); // on the syscore!

	transport_loop(NULL);

	threadJoin(inp_thread, U64_MAX);
	//threadJoin(transport_thread, U64_MAX);

	svcExitProcess();
	return 0;
}
