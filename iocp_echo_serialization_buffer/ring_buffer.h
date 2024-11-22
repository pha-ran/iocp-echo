#pragma once

#include <winsock2.h>

class ring_buffer
{
public:
	static constexpr int _size = 10000;
	static constexpr int _capacity = _size + 1;

public:
#pragma warning(suppress:26495)
	inline ring_buffer(void) noexcept
		: _front(0), _back(0) {}

public:
	int enqueue(const char* in, int size) noexcept;
	int dequeue(char* out, int size) noexcept;
	int peek(char* out, int size) noexcept;

public:
	inline void clear(void) noexcept
	{
		_front = 0;
		_back = 0;
	}

	inline bool empty(void) const noexcept
	{
		return _front == _back;
	}

	inline int use_size(void) const noexcept
	{
		int f = _front;
		int b = _back;

		if (f <= b)		return b - f;
		else			return _capacity - (f - b);
	}

	inline int free_size(void) const noexcept
	{
		int f = _front;
		int b = _back;

		if (f <= b)		return _size - (b - f);
		else			return _size - (_capacity - (f - b));
	}

	inline int direct_enqueue_size(void) const noexcept
	{
		int f = _front;
		int b = _back;

		if (f <= b)
		{
			if (f == 0) return _capacity - b - 1;
			else		return _capacity - b;
		}
		else			return f - b - 1;
	}

	inline int direct_dequeue_size(void) const noexcept
	{
		int f = _front;
		int b = _back;

		if (f <= b)		return b - f;
		else			return _capacity - f;
	}

	inline char* front(void) const noexcept
	{
		return (char*)(_data + _front);
	}

	inline char* back(void) const noexcept
	{
		return (char*)(_data + _back);
	}

	inline void move_front(int move) noexcept
	{
		_front = (_front + move) % _capacity;
	}

	inline void move_back(int move) noexcept
	{
		_back = (_back + move) % _capacity;
	}

	inline int set_wsabuf_recv(WSABUF wsabuf[2])
	{
		int f = _front;
		int b = _back;

		int free_size;
		int direct_enqueue_size;

		if (f <= b)
		{
			free_size = _size - (b - f);

			if (f == 0) direct_enqueue_size = _capacity - b - 1;
			else		direct_enqueue_size = _capacity - b;
		}
		else
		{
			free_size = _size - (_capacity - (f - b));
			direct_enqueue_size = f - b - 1;
		}

		if (free_size == direct_enqueue_size)
		{
			wsabuf[0].buf = _data + b;
			wsabuf[0].len = direct_enqueue_size;

			return 1;
		}

		wsabuf[0].buf = _data + b;
		wsabuf[0].len = direct_enqueue_size;

		wsabuf[1].buf = _data;
		wsabuf[1].len = free_size - direct_enqueue_size;

		return 2;
	}

	inline int set_wsabuf_send(WSABUF wsabuf[2])
	{
		int f = _front;
		int b = _back;

		int use_size;
		int direct_dequeue_size;

		if (f <= b)
		{
			use_size = b - f;
			direct_dequeue_size = use_size;
		}
		else
		{
			use_size = _capacity - (f - b);
			direct_dequeue_size = _capacity - f;
		}

		if (use_size == direct_dequeue_size)
		{
			wsabuf[0].buf = _data + f;
			wsabuf[0].len = direct_dequeue_size;

			return 1;
		}

		wsabuf[0].buf = _data + f;
		wsabuf[0].len = direct_dequeue_size;

		wsabuf[1].buf = _data;
		wsabuf[1].len = use_size - direct_dequeue_size;

		return 2;
	}

private:
	int _front;
	int _back;

	char _data[_capacity];

};
