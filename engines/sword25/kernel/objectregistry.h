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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* 
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#ifndef SWORD25_OBJECTREGISTRY_H
#define SWORD25_OBJECTREGISTRY_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common/func.h"
#include "common/hashmap.h"
#include "sword25/kernel/bs_stdint.h"
#include "sword25/kernel/common.h"

namespace Sword25 {

// -----------------------------------------------------------------------------
// Klassendeklaration
// -----------------------------------------------------------------------------

template<typename T>
class BS_ObjectRegistry {
public:
	BS_ObjectRegistry() : m_NextHandle(1) {}
	virtual ~BS_ObjectRegistry() {}

	// -------------------------------------------------------------------------

	unsigned int RegisterObject(T *ObjectPtr) {
		// Null-Pointer k�nnen nicht registriert werden.
		if (ObjectPtr == 0) {
			LogErrorLn("Cannot register a null pointer.");
			return 0;
		}

		// Falls das Objekt bereits registriert wurde, wird eine Warnung ausgeben und das Handle zur�ckgeben.
		unsigned int Handle = FindHandleByPtr(ObjectPtr);
		if (Handle != 0) {
			LogWarningLn("Tried to register a object that was already registered.");
			return Handle;
		}
		// Ansonsten wird das Objekt in beide Maps eingetragen und das neue Handle zur�ckgeben.
		else {
			m_Handle2PtrMap[m_NextHandle] = ObjectPtr;
			m_Ptr2HandleMap[ObjectPtr] = m_NextHandle;

			return m_NextHandle++;
		}
	}

	// -----------------------------------------------------------------------------

	unsigned int RegisterObject(T *ObjectPtr, unsigned int Handle) {
		// Null-Pointer und Null-Handle k�nnen nicht registriert werden.
		if (ObjectPtr == 0 || Handle == 0) {
			LogErrorLn("Cannot register a null pointer or a null handle.");
			return 0;
		}

		// Falls das Objekt bereits registriert wurde, wird ein Fehler ausgegeben und 0 zur�ckgeben.
		unsigned int HandleTest = FindHandleByPtr(ObjectPtr);
		if (HandleTest != 0) {
			LogErrorLn("Tried to register a object that was already registered.");
			return 0;
		}
		// Falls das Handle bereits vergeben ist, wird ein Fehler ausgegeben und 0 zur�ckgegeben.
		else if (FindPtrByHandle(Handle) != 0) {
			LogErrorLn("Tried to register a handle that is already taken.");
			return 0;
		}
		// Ansonsten wird das Objekt in beide Maps eingetragen und das gew�nschte Handle zur�ckgeben.
		else {
			m_Handle2PtrMap[Handle] = ObjectPtr;
			m_Ptr2HandleMap[ObjectPtr] = Handle;

			// Falls das vergebene Handle gr��er oder gleich dem n�chsten automatische vergebenen Handle ist, wird das n�chste automatisch
			// vergebene Handle erh�ht.
			if (Handle >= m_NextHandle) m_NextHandle = Handle + 1;

			return Handle;
		}
	}

	// -----------------------------------------------------------------------------

	void DeregisterObject(T *ObjectPtr) {
		unsigned int Handle = FindHandleByPtr(ObjectPtr);

		if (Handle != 0) {
			// Registriertes Objekt aus beiden Maps entfernen.
			m_Handle2PtrMap.erase(FindHandleByPtr(ObjectPtr));
			m_Ptr2HandleMap.erase(ObjectPtr);
		} else {
			LogWarningLn("Tried to remove a object that was not registered.");
		}
	}

	// -----------------------------------------------------------------------------

	T *ResolveHandle(unsigned int Handle) {
		// Zum Handle geh�riges Objekt in der Hash-Map finden.
		T *ObjectPtr = FindPtrByHandle(Handle);

		// Pointer zur�ckgeben. Im Fehlerfall ist dieser 0.
		return ObjectPtr;
	}

	// -----------------------------------------------------------------------------

	unsigned int ResolvePtr(T *ObjectPtr) {
		// Zum Pointer geh�riges Handle in der Hash-Map finden.
		unsigned int Handle = FindHandleByPtr(ObjectPtr);

		// Handle zur�ckgeben. Im Fehlerfall ist dieses 0.
		return Handle;
	}

protected:
	// FIXME: I'm not entirely sure my current hash function is legitimate
	struct ClassPointer_EqualTo {
		bool operator()(const T *x, const T *y) const { return x == y; }
	};
	struct ClassPointer_Hash {
		uint operator()(const T *x) const { 
			return static_cast<uint>((int64)x % ((int64)1 << sizeof(uint)));
		}
	};

	typedef Common::HashMap<unsigned int, T *>	HANDLE2PTR_MAP;
	typedef Common::HashMap<T *, unsigned int, ClassPointer_Hash, ClassPointer_EqualTo> PTR2HANDLE_MAP;

	HANDLE2PTR_MAP	m_Handle2PtrMap;
	PTR2HANDLE_MAP	m_Ptr2HandleMap;
	unsigned int	m_NextHandle;

	// -----------------------------------------------------------------------------

	T *FindPtrByHandle(unsigned int Handle) {
		// Zum Handle geh�rigen Pointer finden.
		HANDLE2PTR_MAP::const_iterator it = m_Handle2PtrMap.find(Handle);

		// Pointer zur�ckgeben, oder, falls keiner gefunden wurde, 0 zur�ckgeben.
		return (it != m_Handle2PtrMap.end()) ? it->_value : 0;
	}

	// -----------------------------------------------------------------------------

	unsigned int FindHandleByPtr(T *ObjectPtr) {
		// Zum Pointer geh�riges Handle finden.
		PTR2HANDLE_MAP::const_iterator it = m_Ptr2HandleMap.find(ObjectPtr);

		// Handle zur�ckgeben, oder, falls keines gefunden wurde, 0 zur�ckgeben.
		return (it != m_Ptr2HandleMap.end()) ? it->_value : 0;
	}

	// -----------------------------------------------------------------------------

	virtual void LogErrorLn(const char *Message) const = 0;
	virtual void LogWarningLn(const char *Message) const = 0;
};

} // End of namespace Sword25

#endif
