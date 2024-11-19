#pragma once

#include "ring_buffer.h"
#include <winsock2.h>

class session final
{
	friend class session_manager;

private:
#pragma warning(suppress:26495)
	inline session(void) noexcept
		: _id(-1), _socket(INVALID_SOCKET), _io_count(-1), _io_send(-1), _port(0)
	{
		_ip[0] = L'\0';
		InitializeCriticalSection(&_cs);
	}

	inline ~session(void) noexcept
	{
		DeleteCriticalSection(&_cs);
	}

private:
	unsigned long long _id;
	SOCKET _socket;

	volatile LONG _io_count;
	volatile CHAR _io_send;

	unsigned short _port;
	WCHAR _ip[16];

	CRITICAL_SECTION _cs;

	WSAOVERLAPPED _recv_overlapped;
	WSAOVERLAPPED _send_overlapped;

	ring_buffer _recv_buffer;
	ring_buffer _send_buffer;
};
