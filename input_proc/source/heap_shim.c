#include <3ds/types.h>
#include <3ds/svc.h>
#include <3ds/env.h>
#include <3ds/os.h>
#include <3ds/srv.h>

extern char* fake_heap_start;
extern char* fake_heap_end;

u32 __ctru_heap;
u32 __ctru_heap_size = 0x40000; // arbitrary but small
u32 __ctru_linear_heap = 0; // We don't use the linear heap.
u32 __ctru_linear_heap_size = 0;

void __appInit() // dont't init unnecessary services
{
	srvInit();
}

void __appExit()
{
	srvExit();
}

void __system_allocateHeaps(void)
{
	u32 tmp=0;

	// Allocate the application heap
	__ctru_heap = 0x08000000;
	svcControlMemory(&tmp, __ctru_heap, 0x0, __ctru_heap_size, MEMOP_ALLOC, MEMPERM_READ | MEMPERM_WRITE);

	// Set up newlib heap
	fake_heap_start = (char*)__ctru_heap;
	fake_heap_end = fake_heap_start + __ctru_heap_size;
}