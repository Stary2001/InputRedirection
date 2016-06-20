#include <3ds.h>
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

u32 b = 0;
FILE *log_file;

void thread_fun(void* a)
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

	while(true)
	{
		byte_count = recv(sockfd, buf, sizeof buf, 0);
		if(byte_count == -1)
		{
			break;
		}

		if(byte_count >= 4)
		{
			b = *(u32*)buf;
		}
	}

	socExit();
	acExit();
}

int main(int argc, const char* argv[])
{
/*	sdmcInit();
	log_file = fopen("service_log.log", "a");

	fprintf(log_file, "thread\n");
	fflush(log_file);*/

	Thread t = threadCreate(thread_fun, 0, 0x1000, 0x18, 0, false);
	/*fprintf(log_file, "t\n");
	fflush(log_file);*/

	Handle hid = open_process(0x10);
	Handle self = open_current_process();
	u32 loc = 0x0010dffc;
	while(true)
	{
		u32 a;
		copy_remote_memory(self, &a, hid, (void*)0x1ec46000, 4);
		a &= ~b; // Clear the bits set in B.
		copy_remote_memory(hid, (void*)loc, self, &a, 4);
	}

	threadJoin(t, U64_MAX);

	svcExitProcess();
	return 0;
}
