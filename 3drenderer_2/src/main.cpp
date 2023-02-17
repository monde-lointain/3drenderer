#include "Application.h"
#include <crtdbg.h>

int main(int argc, char* args[])
{
	// Setup the CRT automated memory leak checker
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	// Send all reports to STDOUT
	_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
	_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);

	// Set this to the allocation number given by the leak checker to break at
	// it
	//_CrtSetBreakAlloc(863);

	// Need to put braces here to contain the scope!
	{
		Application app;
		
		app.initialize();
		app.run();
		app.destroy();
	}

	// Perform the leak check
	_CrtDumpMemoryLeaks();

	return 0;
}
