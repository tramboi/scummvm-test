/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 */

#ifndef COMMON_ARRAY_H
#define COMMON_ARRAY_H

#include "common/scummsys.h"
#include "common/algorithm.h"

namespace Common {

// TODO: Improve the storage management of this class.
// In particular, don't use new[] and delete[], but rather
// construct/destruct objects manually. This way, we can
// ensure that storage which is not currently used does not
// correspond to a live active object.
// (This is only of interest for array of non-POD objects).
template<class T>
class Array {
protected:
	uint _capacity;
	uint _size;
	T *_storage;

public:
	typedef T *iterator;
	typedef const T *const_iterator;

	typedef T value_type;

public:
	Array() : _capacity(0), _size(0), _storage(0) {}
	Array(const Array<T> &array) : _capacity(0), _size(0), _storage(0) {
		_capacity = _size = array._size;
		_storage = new T[_capacity];
		copy(array._storage, array._storage + _size, _storage);
	}

	/**
	 * Construct an array by copying data from a regular array.
	 */
	template<class T2>
	Array(const T2 *data, int n) {
		_capacity = _size = n;
		_storage = new T[_capacity];
		copy(data, data + _size, _storage);
	}

	~Array() {
		delete[] _storage;
	}

	/** Appends element to the end of the array. */
	void push_back(const T &element) {
		ensureCapacity(_size + 1);
		_storage[_size++] = element;
	}

	void push_back(const Array<T> &array) {
		ensureCapacity(_size + array._size);
		copy(array._storage, array._storage + array._size, _storage + _size);
		_size += array._size;
	}

	/** Removes the last element of the array. */
	void pop_back() {
		assert(_size > 0);
		_size--;
	}

	/** Returns a reference to the first element of the array. */
	T &front() {
		assert(_size > 0);
		return _storage[0];
	}

	/** Returns a reference to the first element of the array. */
	const T &front() const {
		assert(_size > 0);
		return _storage[0];
	}

	/** Returns a reference to the last element of the array. */
	T &back() {
		assert(_size > 0);
		return _storage[_size-1];
	}

	/** Returns a reference to the last element of the array. */
	const T &back() const {
		assert(_size > 0);
		return _storage[_size-1];
	}

	void insert_at(int idx, const T &element) {
		assert(idx >= 0 && (uint)idx <= _size);
		ensureCapacity(_size + 1);
		copy_backward(_storage + idx, _storage + _size, _storage + _size + 1);
		_storage[idx] = element;
		_size++;
	}

	T remove_at(int idx) {
		assert(idx >= 0 && (uint)idx < _size);
		T tmp = _storage[idx];
		copy(_storage + idx + 1, _storage + _size, _storage + idx);
		_size--;
		return tmp;
	}

	// TODO: insert, remove, ...

	T& operator[](int idx) {
		assert(idx >= 0 && (uint)idx < _size);
		return _storage[idx];
	}

	const T& operator[](int idx) const {
		assert(idx >= 0 && (uint)idx < _size);
		return _storage[idx];
	}

	Array<T>& operator=(const Array<T> &array) {
		if (this == &array)
			return *this;

		delete[] _storage;
		_size = array._size;
		_capacity = _size + 32;
		_storage = new T[_capacity];
		copy(array._storage, array._storage + _size, _storage);

		return *this;
	}

	uint size() const {
		return _size;
	}

	void clear() {
		delete[] _storage;
		_storage = 0;
		_size = 0;
		_capacity = 0;
	}

	bool empty() const {
		return (_size == 0);
	}


	iterator		begin() {
		return _storage;
	}

	iterator		end() {
		return _storage + _size;
	}

	const_iterator	begin() const {
		return _storage;
	}

	const_iterator	end() const {
		return _storage + _size;
	}

	void reserve(uint newCapacity) {
		if (newCapacity <= _capacity)
			return;

		T *old_storage = _storage;
		_capacity = newCapacity;
		_storage = new T[newCapacity];

		if (old_storage) {
			// Copy old data
			copy(old_storage, old_storage + _size, _storage);
			delete[] old_storage;
		}
	}

	void resize(uint newSize) {
		reserve(newSize);
		_size = newSize;
	}

protected:
	void ensureCapacity(uint len) {
		if (len >= _capacity)
			reserve(len + 32);
	}
};

} // End of namespace Common

#endif
