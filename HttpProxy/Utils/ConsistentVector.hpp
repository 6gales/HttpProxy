#pragma once
#include <sys/types.h>
#include <cstddef>

template <class T>
class ConsistentVector
{
	T *_data;
	size_t _size;
	size_t _capacity;

	T *copy(size_t capacity) const;

public:
	ConsistentVector();
	ConsistentVector(size_t capacity);
	ConsistentVector(const ConsistentVector &another);

	~ConsistentVector()
	{
		delete[] _data;
		_data = NULL;
	}

	void push_back(T elem);

	void clear()
	{
		_size = 0;
	}

	T *data()
	{
		return _data;
	}

	size_t size() const
	{
		return _size;
	}
	size_t capacity() const
	{
		return _capacity;
	}

	T &operator[](size_t i)
	{
		return _data[i];
	}
	const T &operator[](size_t i) const
	{
		return _data[i];
	}
};

template<class T>
inline ConsistentVector<T>::ConsistentVector()
{
	_capacity = 1024;
	_size = 0;
	_data = new T[_capacity];
}

template<class T>
inline ConsistentVector<T>::ConsistentVector(size_t capacity)
{
	_capacity = capacity;
	_size = 0;
	_data = new T[_capacity];
}

template<class T>
inline ConsistentVector<T>::ConsistentVector(const ConsistentVector &another)
{
	_capacity = another._capacity;
	_size = another._size;
	_data = another.copy(_capacity);
}

template<class T>
void ConsistentVector<T>::push_back(T elem)
{
	if (_size == _capacity)
	{
		_capacity *= 2;
		T *newData = copy(_capacity);

		delete[] _data;
		_data = newData;
	}

	_data[_size++] = elem;
}

template<class T>
T *ConsistentVector<T>::copy(size_t capacity) const
{
	T *data = new T[capacity];
	for (size_t i = 0; i < _size; i++)
	{
		data[i] = _data[i];
	}

	return data;
}