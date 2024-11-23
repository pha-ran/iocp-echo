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
) noexcept
{
	// session
	_session_max = session_max;
	_session_count = 0;
	_sessions = new session[_session_max];

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

void lan_server::stop(void) noexcept
{
	closesocket(_listen_socket);

	for (int i = 0; i < _session_max; ++i)
		delete_session(&_sessions[i]);

	for (int i = 0; i < _session_max; ++i)
		if (_sessions[i]._socket != INVALID_SOCKET) __debugbreak();

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

bool lan_server::send_message(unsigned long long key, serialization_buffer* message) noexcept
{
	session* s = &_sessions[((session::key*)&key)->_index];

	// todo

	message->set_header();

	int message_ret = s->_send_buffer.enqueue((const char*)&message, sizeof(serialization_buffer*));

	if (message_ret <= 0) __debugbreak();

	send_post(s);

	return true;
}

void lan_server::recv_post(session* s) noexcept
{
	InterlockedIncrement(&s->_io_count);

	WSABUF wb[2];
	DWORD flags = 0;

	int count = s->_recv_buffer.set_wsabuf_recv(wb);

	if (wb[0].len <= 0) __debugbreak();

	memset(&s->_recv_overlapped, 0, sizeof(s->_recv_overlapped));

	int recv_ret = WSARecv(s->_socket, wb, count, NULL, &flags, &s->_recv_overlapped, NULL);

	if (recv_ret == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		if (error != WSA_IO_PENDING)
		{
			//wprintf(L"WSARecv error %d\n", error);

			if (error == WSAENOTSOCK ||
				error == WSAECONNABORTED ||
				error == WSAECONNRESET)
				InterlockedDecrement(&s->_io_count);
			else __debugbreak();
		}

		//wprintf(L"[WSARecv] PENDING %d (io %d)\n", error, s->_io_count);
	}
}

void lan_server::send_post(session* s) noexcept
{
	WSABUF wb[2];
	int count;

	for (;;)
	{
		int use_size = s->_send_buffer.use_size();

		if (use_size <= 0) return;

		char prev_io_send = InterlockedExchange8(&s->_io_send, 1);

		if (prev_io_send) return;

		count = s->_send_buffer.set_wsabuf_send(wb);

		if (wb[0].len <= 0) InterlockedExchange8(&s->_io_send, 0);
		else				break;
	}

	InterlockedIncrement(&s->_io_count);

	memset(&s->_send_overlapped, 0, sizeof(s->_send_overlapped));

	int send_ret = WSASend(s->_socket, wb, count, NULL, 0, &s->_send_overlapped, NULL);

	if (send_ret == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		if (error != WSA_IO_PENDING)
		{
			//wprintf(L"WSASend error %d\n", error);

			if (error == WSAENOTSOCK ||
				error == WSAECONNABORTED ||
				error == WSAECONNRESET)
			{
				InterlockedExchange8(&s->_io_send, 0);
				InterlockedDecrement(&s->_io_count);
			}
			else __debugbreak();
		}

		//wprintf(L"[WSASend] PENDING %d (io %d)\n", error, s->_io_count);
	}
}

void lan_server::delete_session(session* s) noexcept
{
	SOCKET temp = s->_socket;
	s->_socket = INVALID_SOCKET;
	closesocket(temp);
	--_session_count;
}

unsigned __stdcall lan_server::accept_worker(void* args) noexcept
{
	lan_server* _this = (lan_server*)args;
	SOCKADDR_IN client_addr;
	int addr_len = sizeof(client_addr);
	unsigned long long session_id = 1;

	for (;;)
	{
		// accept
		SOCKET client_socket = accept(_this->_listen_socket, (SOCKADDR*)&client_addr, &addr_len);

		if (client_socket == INVALID_SOCKET)
		{
			int error = WSAGetLastError();

			if (error == WSAEINTR) break;
			else if (error == WSAENOTSOCK) break;
			else __debugbreak();
		}

		// session
		session* sessions = _this->_sessions;
		unsigned short session_max = _this->_session_max;

		int index;
		session* current = nullptr;

		for (index = 0; index < session_max; ++index)
		{
			if (sessions[index]._socket == INVALID_SOCKET)
			{
				current = &(sessions[index]);
				break;
			}
		}

		if (index >= session_max) __debugbreak();

		current->_socket = client_socket;

		current->_key._id = session_id;
		current->_key._index = index;

		current->_io_count = 1;
		current->_io_send = 0;

		PCWSTR ntop = InetNtopW(AF_INET, &client_addr.sin_addr, current->_ip, _countof(current->_ip));
		current->_port = ntohs(client_addr.sin_port);

		if (ntop == NULL)
		{
			int error = WSAGetLastError();
			__debugbreak();
		}

		current->_recv_buffer.clear();
		current->_send_buffer.clear();

		// iocp
		HANDLE iocp_ret = CreateIoCompletionPort((HANDLE)client_socket, _this->_iocp, (ULONG_PTR)current, 0);

		if (iocp_ret == NULL)
		{
			int error = GetLastError();
			__debugbreak();
		}

		++session_id;
		++(_this->_session_count);

		// login
		serialization_buffer* sb = new serialization_buffer;

		*sb << (unsigned long long)0x7fffffffffffffff;

		sb->set_header();

		int message_ret = current->_send_buffer.enqueue((const char*)&sb, sizeof(serialization_buffer*));

		if (message_ret <= 0) __debugbreak();

		// send
		_this->send_post(current);

		// recv
		_this->recv_post(current);

		// io count
		long io_count = InterlockedDecrement(&current->_io_count);

		if (io_count == 0) _this->delete_session(current);
	}

	wprintf(L"[return] accept %d\n", GetCurrentThreadId());

	return 0;
}

unsigned __stdcall lan_server::iocp_worker(void* args) noexcept
{
	lan_server* _this = (lan_server*)args;
	int thread_id = GetCurrentThreadId();

	for (;;)
	{
		DWORD transferred;
		session* s;
		OVERLAPPED* ptr;

		int gqcs_ret = GetQueuedCompletionStatus(_this->_iocp, &transferred, (PULONG_PTR)&s, &ptr, INFINITE);

		if (transferred == 0)
		{
			/*wprintf(
				L"[GQCS EXIT] %d : gqcs_ret %d / t %d / s %p / p %p\n",
				thread_id, gqcs_ret, transferred, s, ptr
			);*/

			if (s == 0 && ptr == 0) break;
		}
		else if (&s->_recv_overlapped == ptr)
		{
			/*wprintf(
				L"[GQCS RECV] %d : gqcs_ret %d / t %d / s %p / p %p / io %d\n",
				thread_id, gqcs_ret, transferred, s, ptr, s->_io_count
			);*/

			s->_recv_buffer.move_back(transferred);

			for (;;)
			{
				unsigned short header;
				unsigned long long payload;

				if (s->_recv_buffer.use_size() < sizeof(header)) break;

				s->_recv_buffer.peek((char*)&header, sizeof(header));

				if (s->_recv_buffer.use_size() < header + sizeof(header)) break;

				s->_recv_buffer.move_front(sizeof(header));

				s->_recv_buffer.dequeue((char*)&payload, header);

				_this->on_receive(s->_key._all, payload);
			}

			_this->recv_post(s);
		}
		else
		{
			/*wprintf(
				L"[GQCS SEND] %d : gqcs_ret %d / t %d / s %p / p %p / io %d\n",
				thread_id, gqcs_ret, transferred, s, ptr, s->_io_count
			);*/

			s->_send_buffer.move_front(transferred);

			InterlockedExchange8(&s->_io_send, 0);

			_this->send_post(s);
		}

#pragma warning(suppress:6011)
		long io_count = InterlockedDecrement(&s->_io_count);

		if (io_count == 0) _this->delete_session(s);
	}

	wprintf(L"[return] iocp %d\n", thread_id);

	return 0;
}
