#include <3ds.h>
#include <stdio.h>
#include <scenic/proc.h>
#include <scenic/dma.h>

#define HID_PID 0x10

// 11.0 HID
#define HID_PATCH1_LOC 0x101de0
#define HID_PATCH2_LOC 0x107acc
#define HID_PATCH3_LOC 0x106a74
#define HID_CAVE_LOC   0x1094B8

#define HID_DAT_LOC	   0x10df00
#define HID_TS_RD_LOC 0x10df04
#define HID_TS_WR_LOC 0x10df08

scenic_process *hid;

void read_input();
extern u32 read_input_sz;

int main()
{
	gfxInitDefault();
	consoleInit(GFX_BOTTOM, NULL);

	printf("injecting into hid..\n");

	hid = proc_open(0x10, 0);

	u32 new_loc = HID_DAT_LOC;
	u32 test = 0;

	int r = dma_copy_to_self(&test, hid, (void*)new_loc, 4);
	if (r)
	{
		printf("copy returned %08x\n", r);
		exit(0);
	}

	bool err = false;

	if (test != 0)
	{
		printf("Already patched? \n");
		err = true;
	}
	else
	{
		u32 f = 0xffffffff;
		r = dma_copy_from_self(hid, (void*)new_loc, &f, 4);
		if (r != 0)
		{
			printf("init copy failed\n");
			err = true;
		}

		if(!err && dma_protect(hid, (void*)(HID_PATCH1_LOC & (~0xfff)), 0x1000) != 0)
		{
			printf("patch 1 prot failed\n");
			err = true;
		}

		if (!err && dma_copy_from_self(hid, (void*)HID_PATCH1_LOC, &new_loc, 4) != 0)
		{
			printf("patch 1 copy failed\n");
			err = true;
		}

		if(!err && dma_protect(hid, (void*)(HID_PATCH2_LOC & (~0xfff)), 0x1000) != 0)
		{
			printf("patch 2 prot failed\n");
			err = true;
		}

		if (!err && dma_copy_from_self(hid, (void*)HID_PATCH2_LOC, &new_loc, 4) != 0)
		{
			printf("patch 2 copy failed\n");
			err = true;
		}

		if(!err)
		{
			if(!proc_hook(hid, HID_PATCH3_LOC, HID_CAVE_LOC, (u32*)&read_input, read_input_sz))
			{
				printf("Hook failed!\n");
				err = true;
			}
			dma_kill_cache();
		}
	}

	proc_close(hid);

	if(err)
	{
		printf("An error occured! press START to exit!\n");
		while (aptMainLoop())
		{
			gspWaitForVBlank();
			hidScanInput();

			u32 kDown = hidKeysDown();
			if (kDown & KEY_START)
				break; 
		}
	}

	printf("Press HOME, or START to exit!\n");
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		hidScanInput();

		u32 kDown = hidKeysDown();
		if (kDown & KEY_START)
			break; 
	}

	//gfxExit();

	return 0;
}
