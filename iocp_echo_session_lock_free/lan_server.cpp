#include "lan_server.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <process.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32")

#pragma warning(suppress:26495)
lan_server::lan_server(void) noexcept
{
	WSADATA wsadata;

	if (WSAStartup(0x0202, &wsadata) != 0)
	{
		int error = WSAGetLastError();
		__debugbreak();
	}
}

lan_server::~lan_server(void) noexcept
{
	WSACleanup();
}

bool lan_server::start(
	const wchar_t* ip, unsigned short port, unsigned int session_max,
	unsigned short worker_count, unsigned short number_of_concurrent
)
{
	// session
	_session_max = session_max;
	_session_size = 0;
	_sessions = new session[_session_max];

	// todo

	// iocp
	_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, number_of_concurrent);

	if (_iocp == NULL)
	{
		int error = GetLastError();
		__debugbreak();
	}

	// socket
	_listen_socket = socket(AF_INET, SOCK_STREAM, 0);

	if (_listen_socket == INVALID_SOCKET)
	{
		int error = WSAGetLastError();
		__debugbreak();
	}

	// addr
	SOCKADDR_IN listen_addr;

	memset(&listen_addr, 0, sizeof(listen_addr));

	listen_addr.sin_family = AF_INET;
	int pton_ret = InetPtonW(AF_INET, ip, &listen_addr.sin_addr);
	listen_addr.sin_port = htons(port);

	if (pton_ret != 1)
	{
		int error = WSAGetLastError();
		__debugbreak();
	}

	// bind
	int bind_ret = bind(_listen_socket, (SOCKADDR*)&listen_addr, sizeof(listen_addr));

	if (bind_ret == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		__debugbreak();
	}

	// linger
	LINGER linger;

	linger.l_onoff = 1;
	linger.l_linger = 0;

	int linger_ret = setsockopt(_listen_socket, SOL_SOCKET, SO_LINGER, (char*)&linger, sizeof(linger));

	if (linger_ret == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		__debugbreak();
	}

	// send buffer
	int opt_val = 0;

	int size_ret = setsockopt(_listen_socket, SOL_SOCKET, SO_SNDBUF, (char*)&opt_val, sizeof(opt_val));

	if (size_ret == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		__debugbreak();
	}

	// listen
	int listen_ret = listen(_listen_socket, SOMAXCONN_HINT(USHRT_MAX));

	if (listen_ret == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		__debugbreak();
	}

	// threads
	_thread_max = worker_count + 1;
	_threads = new HANDLE[_thread_max];

	_threads[0] = (HANDLE)_beginthreadex(nullptr, 0, accept_worker, this, 0, nullptr);

	if (_threads[0] == NULL)
	{
		int error = GetLastError();
		__debugbreak();
	}

	for (int i = 1; i < _thread_max; ++i)
	{
		_threads[i] = (HANDLE)_beginthreadex(nullptr, 0, iocp_worker, this, 0, nullptr);

		if (_threads[i] == NULL)
		{
			int error = GetLastError();
			__debugbreak();
		}
	}

	return true;
}

void lan_server::stop(void)
{
	closesocket(_listen_socket);

	// todo

	for (int i = 1; i < _thread_max; ++i)
		PostQueuedCompletionStatus(_iocp, 0, 0, 0);

	DWORD wait_ret = WaitForMultipleObjects(_thread_max, _threads, TRUE, INFINITE);

	if (wait_ret != WAIT_OBJECT_0)
	{
		int error = GetLastError();
		__debugbreak();
	}

	printf("[return] main %d\n", GetCurrentThreadId());

	for (int i = 0; i < _thread_max; ++i)
		CloseHandle(_threads[i]);

	delete[] _threads;
	CloseHandle(_iocp);
	delete[] _sessions;
}

unsigned __stdcall lan_server::accept_worker(void* args)
{
	lan_server* _this = (lan_server*)args;
	SOCKADDR_IN client_addr;
	int addr_len = sizeof(client_addr);
	unsigned long long session_id = 1;

	// todo

	wprintf(L"[return] accept %d\n", GetCurrentThreadId());

	return 0;
}

unsigned __stdcall lan_server::iocp_worker(void* args)
{
	lan_server* _this = (lan_server*)args;
	int thread_id = GetCurrentThreadId();

	// todo

	wprintf(L"[return] iocp %d\n", thread_id);

	return 0;
}