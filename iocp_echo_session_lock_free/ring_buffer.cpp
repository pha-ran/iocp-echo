#include "ring_buffer.h"
#include <memory.h>

int ring_buffer::enqueue(const char* in, int size) noexcept
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

    if (free_size < size) return -1;

    if (size <= direct_enqueue_size)
    {
        memcpy(_data + b, in, size);
        _back = (b + size) % _capacity;
    }
    else
    {
        memcpy(_data + b, in, direct_enqueue_size);
        b = size - direct_enqueue_size;
        memcpy(_data, in + direct_enqueue_size, b);
        _back = b;
    }

    return size;
}

int ring_buffer::dequeue(char* out, int size) noexcept
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

    if (use_size < size) return -1;

    if (size <= direct_dequeue_size)
    {
        memcpy(out, _data + f, size);
        _front = (f + size) % _capacity;
    }
    else
    {
        memcpy(out, _data + f, direct_dequeue_size);
        f = size - direct_dequeue_size;
        memcpy(out + direct_dequeue_size, _data, f);
        _front = f;
    }

    return size;
}

int ring_buffer::peek(char* out, int size) noexcept
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

    if (use_size < size) return -1;

    if (size <= direct_dequeue_size)
        memcpy(out, _data + f, size);
    else
    {
        memcpy(out, _data + f, direct_dequeue_size);
        memcpy(out + direct_dequeue_size, _data, size - direct_dequeue_size);
    }

    return size;
}
