#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>

#define HID_PID 0x10

// 9.2 HID
//#define HID_PATCH1_LOC 0x101dbc
//#define HID_PATCH2_LOC 0x107924

// 11.0 HID
#define HID_PATCH1_LOC 0x101de0
#define HID_PATCH2_LOC 0x107acc

extern void shit();
extern u32 shit_end;

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

	ret = svcFlushProcessDataCache(hSrc, ptrSrc, size);
	ret = svcFlushProcessDataCache(hDst, ptrDst, size);
	ret = svcStartInterProcessDma(&hDma, hDst, ptrDst, hSrc, ptrSrc, size, dmaConfig);
	state = 0;
	
	if(done_state == 0)
	{
		ret = svcGetDmaState(&state, hDma);
		svcSleepThread(1000000000);
		ret = svcGetDmaState(&state, hDma);
		done_state = state;
		printf("InterProcessDmaFinishState: %08lx\n", state);
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
		printf("readRemoteMemory time out %08lx\n", state);
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
	
s32 killCache_k()
{
	__asm__ volatile("cpsid aif");
	__asm__ volatile("mcr p15, 0, r0, c7, c5, 0"); // icache
	__asm__ volatile("mcr p15, 0, r0, c7, c14, 0"); // dcache
	return 0;
}

void killCache()
{
	svcBackdoor(killCache_k);
}

int main()
{
	gfxInitDefault();

	consoleInit(GFX_BOTTOM, NULL);

	printf("about to do fun shit\n");

	Handle self = open_current_process();

	u8 *buff = malloc(1024);
	memset(buff, 0, 1024);

	u32 new_loc = 0x0010dffc;

	Handle target = open_process(HID_PID);

	memset(buff, 0, 1024);
	Result r = copy_remote_memory(self, buff, target, (void*)new_loc, 4);
	if(r != 0)
	{
		printf("copy returned %08lx\n", r);
		exit(0);
	}

	if(*(u32*)buff != 0)
	{
		printf("!!!!!!\n");
	}
	else
	{
		u32 f = 0xffffffff;
		r = copy_remote_memory(target, (void*)new_loc, self, &f, 4);
		if(r != 0)
		{
			printf("init copy failed\n");
		}

		if(protect_remote_memory(target, (void*)(HID_PATCH1_LOC & (~0xfff)), 0x1000) != 0)
		{
			printf("patch 1 prot failed\n");
		}

		if(copy_remote_memory(target, (void*)HID_PATCH1_LOC, self, &new_loc, 4) != 0)
		{
			printf("patch 1 copy failed\n");
		}

		if(protect_remote_memory(target, (void*)(HID_PATCH2_LOC & (~0xfff)), 0x1000) != 0)
		{
			printf("patch 2 prot failed\n");
		}

		if(copy_remote_memory(target, (void*)HID_PATCH2_LOC, self, &new_loc, 4) != 0)
		{
			printf("patch 2 copy failed\n");
		}

		printf("hid done\n");
	
		killCache();

		svcCloseHandle(target);
		svcCloseHandle(self);
	}

	u32 pid;

	nsInit();
	r = NS_LaunchTitle(0x0004000000123400LL, 3, &pid);
	if(r != 0)
	{
		printf("NS returned error %08lx, do a hard reboot because hid is patched already\n", r);
	}
	else
	{
		printf("Background launched, pid %08lx\n", pid);
	}
	nsExit();

	// Main loop
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

	// Exit services
	gfxExit();
	return 0;
}

