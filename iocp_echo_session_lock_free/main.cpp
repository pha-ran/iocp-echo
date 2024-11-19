#include "lan_server.h"
#include <conio.h>

class echo : public lan_server
{
	;
};

int wmain(void)
{
	echo _echo;

	_echo.start(L"0.0.0.0", 20000, 10000, 8, 0);

	// loop
	for (;;)
	{
		char c = _getch();

		if (c == 'q') break;
	}

	_echo.stop();

	return 0;
}
