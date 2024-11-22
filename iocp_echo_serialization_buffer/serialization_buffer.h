#pragma once

#include <string>

class serialization_buffer final
{
public:
	static constexpr int _capacity = 1024;

public:
#pragma warning(suppress:26495)
	inline serialization_buffer(void) noexcept
		: _front(0), _back(0), _fail(false) {}

public:
	inline void clear(void) noexcept
	{
		_front = 0;
		_back = 0;
		_fail = false;
	}

	inline int size(void) const noexcept
	{
		return _back - _front;
	}

	inline void push(const char* in, int size)
	{
		if (_back + size > _capacity)
		{
			_fail = true;
			return;
		}

		memcpy(_buffer + _back, in, size);
		_back += size;
	}

	inline void pop(char* out, int size)
	{
		if (_front + size > _back)
		{
			_fail = true;
			return;
		}

		memcpy(out, _buffer + _front, size);
		_front += size;
	}

public:
	inline bool fail(void) const noexcept
	{
		return _fail;
	}

	inline operator bool(void) const noexcept
	{
		return _fail;
	}

private:
	inline char* buffer(void) noexcept
	{
		return _buffer;
	}

	inline void move_back(int size) noexcept
	{
		if (_back + size > _capacity)
		{
			_fail = true;
			return;
		}

		_back += size;
	}

	inline void move_front(int size) noexcept
	{
		if (_front + size > _back)
		{
			_fail = true;
			return;
		}

		_front += size;
	}

public:
	// bool
	inline serialization_buffer& operator<<(bool in) noexcept
	{
		if (_back + sizeof(bool) > _capacity)
		{
			_fail = true;
			return *this;
		}

		_buffer[_back] = (char)in;
		++_back;

		return *this;
	}

	inline serialization_buffer& operator>>(bool& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(bool) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (bool)_buffer[_front];
		++_front;

		return *this;
	}

	// char
	inline serialization_buffer& operator<<(char in) noexcept
	{
		if (_back + sizeof(char) > _capacity)
		{
			_fail = true;
			return *this;
		}

		_buffer[_back] = in;
		++_back;

		return *this;
	}

	inline serialization_buffer& operator>>(char& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(char) > _back)
		{
			_fail = true;
			return *this;
		}

		out = _buffer[_front];
		++_front;

		return *this;
	}

	inline serialization_buffer& operator<<(unsigned char in) noexcept
	{
		if (_back + sizeof(unsigned char) > _capacity)
		{
			_fail = true;
			return *this;
		}

		_buffer[_back] = (char)in;
		++_back;

		return *this;
	}

	inline serialization_buffer& operator>>(unsigned char& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(unsigned char) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (unsigned char)_buffer[_front];
		++_front;

		return *this;
	}

	// wchar_t
	inline serialization_buffer& operator<<(wchar_t in) noexcept
	{
		if (_back + sizeof(wchar_t) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(wchar_t*)(_buffer + _back)) = in;
		_back += sizeof(wchar_t);

		return *this;
	}

	inline serialization_buffer& operator>>(wchar_t& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(wchar_t) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(wchar_t*)(_buffer + _front));
		_front += sizeof(wchar_t);

		return *this;
	}

	// short
	inline serialization_buffer& operator<<(short in) noexcept
	{
		if (_back + sizeof(short) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(short*)(_buffer + _back)) = in;
		_back += sizeof(short);

		return *this;
	}

	inline serialization_buffer& operator>>(short& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(short) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(short*)(_buffer + _front));
		_front += sizeof(short);

		return *this;
	}

	inline serialization_buffer& operator<<(unsigned short in) noexcept
	{
		if (_back + sizeof(unsigned short) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(unsigned short*)(_buffer + _back)) = in;
		_back += sizeof(unsigned short);

		return *this;
	}

	inline serialization_buffer& operator>>(unsigned short& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(unsigned short) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(unsigned short*)(_buffer + _front));
		_front += sizeof(unsigned short);

		return *this;
	}

	// int
	inline serialization_buffer& operator<<(int in) noexcept
	{
		if (_back + sizeof(int) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(int*)(_buffer + _back)) = in;
		_back += sizeof(int);

		return *this;
	}

	inline serialization_buffer& operator>>(int& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(int) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(int*)(_buffer + _front));
		_front += sizeof(int);

		return *this;
	}

	inline serialization_buffer& operator<<(unsigned int in) noexcept
	{
		if (_back + sizeof(unsigned int) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(unsigned int*)(_buffer + _back)) = in;
		_back += sizeof(unsigned int);

		return *this;
	}

	inline serialization_buffer& operator>>(unsigned int& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(unsigned int) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(unsigned int*)(_buffer + _front));
		_front += sizeof(unsigned int);

		return *this;
	}

	// long
	inline serialization_buffer& operator<<(long in) noexcept
	{
		if (_back + sizeof(long) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(long*)(_buffer + _back)) = in;
		_back += sizeof(long);

		return *this;
	}

	inline serialization_buffer& operator>>(long& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(long) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(long*)(_buffer + _front));
		_front += sizeof(long);

		return *this;
	}

	inline serialization_buffer& operator<<(unsigned long in) noexcept
	{
		if (_back + sizeof(unsigned long) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(unsigned long*)(_buffer + _back)) = in;
		_back += sizeof(unsigned long);

		return *this;
	}

	inline serialization_buffer& operator>>(unsigned long& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(unsigned long) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(unsigned long*)(_buffer + _front));
		_front += sizeof(unsigned long);

		return *this;
	}

	// long long
	inline serialization_buffer& operator<<(long long in) noexcept
	{
		if (_back + sizeof(long long) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(long long*)(_buffer + _back)) = in;
		_back += sizeof(long long);

		return *this;
	}

	inline serialization_buffer& operator>>(long long& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(long long) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(long long*)(_buffer + _front));
		_front += sizeof(long long);

		return *this;
	}

	inline serialization_buffer& operator<<(unsigned long long in) noexcept
	{
		if (_back + sizeof(unsigned long long) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(unsigned long long*)(_buffer + _back)) = in;
		_back += sizeof(unsigned long long);

		return *this;
	}

	inline serialization_buffer& operator>>(unsigned long long& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(unsigned long long) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(unsigned long long*)(_buffer + _front));
		_front += sizeof(unsigned long long);

		return *this;
	}

	// float
	inline serialization_buffer& operator<<(float in) noexcept
	{
		if (_back + sizeof(float) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(float*)(_buffer + _back)) = in;
		_back += sizeof(float);

		return *this;
	}

	inline serialization_buffer& operator>>(float& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(float) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(float*)(_buffer + _front));
		_front += sizeof(float);

		return *this;
	}

	// double
	inline serialization_buffer& operator<<(double in) noexcept
	{
		if (_back + sizeof(double) > _capacity)
		{
			_fail = true;
			return *this;
		}

		(*(double*)(_buffer + _back)) = in;
		_back += sizeof(double);

		return *this;
	}

	inline serialization_buffer& operator>>(double& out) noexcept
	{
#pragma warning(suppress:4018)
		if (_front + sizeof(double) > _back)
		{
			_fail = true;
			return *this;
		}

		out = (*(double*)(_buffer + _front));
		_front += sizeof(double);

		return *this;
	}

private:
	int _back;
	int _front;

	bool _fail;
	char _reserved[3];

	char _buffer[_capacity];

};
