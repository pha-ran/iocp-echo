#pragma once

#include "session.h"

class lan_server
{
public:
	lan_server(void) noexcept;
	~lan_server(void) noexcept;

public:
	bool start(
		const wchar_t* ip, unsigned short port, unsigned int session_max,
		unsigned short worker_count, unsigned short number_of_concurrent
	);

	void stop(void);

public:
	static unsigned __stdcall accept_worker(void* args);
	static unsigned __stdcall iocp_worker(void* args);

private:
	session* _sessions;
	unsigned short _session_max;
	unsigned short _session_size;

	HANDLE* _threads;
	unsigned short _thread_max;

	HANDLE _iocp;
	SOCKET _listen_socket;

};
