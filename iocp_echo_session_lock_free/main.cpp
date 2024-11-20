#include "lan_server.h"
#include <conio.h>

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

class echo : public lan_server
{
	;
};

int wmain(void) noexcept
{
#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	echo _echo;

	_echo.start(L"0.0.0.0", 6000, 10000, 8, 0);

	// loop
	for (;;)
	{
		char c = _getch();

		if (c == 'q') break;
	}

	_echo.stop();

	return 0;
}
