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
	) noexcept;

	void stop(void) noexcept;

public:
	inline unsigned short session_count(void) const noexcept
	{
		return _session_count;
	}

public:
	bool send_message(unsigned long long key, unsigned long long payload) noexcept;

public:
	virtual void on_receive(unsigned long long key, unsigned long long payload) noexcept = 0;

private:
	long recv_post(session* s) noexcept;
	long send_post(session* s) noexcept;
	void delete_session(session* s) noexcept;

private:
	static unsigned __stdcall accept_worker(void* args) noexcept;
	static unsigned __stdcall iocp_worker(void* args) noexcept;

private:
	session* _sessions;
	unsigned short _session_max;
	unsigned short _session_count;

	HANDLE* _threads;
	unsigned short _thread_max;

	HANDLE _iocp;
	SOCKET _listen_socket;

};
