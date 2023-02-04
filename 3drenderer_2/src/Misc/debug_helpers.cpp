#include "debug_helpers.h"

#include <crtdbg.h>

void debug_heap_check()
{
#ifndef NDEBUG
    int result = _CrtCheckMemory();
    if (result != 0)
    {
        // If heap is corrupted
        _CrtDbgReport(_CRT_WARN, NULL, 0, NULL, "Heap corruption detected\n");
    }
#endif
}
