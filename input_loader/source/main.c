#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <3ds.h>

int main()
{
	gfxInitDefault();

	consoleInit(GFX_BOTTOM, NULL);

	u32 pid;

	nsInit();
	Result r = NS_LaunchTitle(0x0004000000123400LL, 3, &pid);
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

	sdmcExit();
	// Exit services
	gfxExit();
	return 0;
}

