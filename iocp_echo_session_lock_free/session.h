#pragma once

#include "ring_buffer.h"
#include <winsock2.h>

class session final
{
	friend class lan_server;

private:
#pragma warning(suppress:26495)
	inline session(void) noexcept
		: _socket(INVALID_SOCKET), _io_count(-1), _io_send(-1), _port(0)
	{
		_key._all = -1;
		_ip[0] = L'\0';
		InitializeCriticalSection(&_cs);
	}

	inline ~session(void) noexcept
	{
		DeleteCriticalSection(&_cs);
	}

private:
	SOCKET _socket;
	
	union KEY
	{
		unsigned long long _all;
		struct
		{
			unsigned long long _id : 48;
			unsigned long long _index : 16;
		};
	} _key;

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
