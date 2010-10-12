// -----------------------------------------------------------------------------
// This file is part of Broken Sword 2.5
// Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsd�rfer
//
// Broken Sword 2.5 is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Broken Sword 2.5 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Broken Sword 2.5; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
// -----------------------------------------------------------------------------

#ifndef BS_CPUINFO_H
#define BS_CPUINFO_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "common.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

/**
    @brief Diese Singleton-Klasse stellt Informationen �ber die CPU zur verf�gung.
*/

class BS_CPUInfo
{
public:
	/**
	    @brief Definiert die Vendor-IDs
	*/
	enum VENDORID
	{
		V_UNKNOWN,
		V_INTEL,
		V_AMD,
		V_CYRIX,
		V_CENTAUR,
		V_NEXGEN,
		V_TRANSMETA,
		V_RISE,
		V_UMC,
		V_SIS,
		V_NSC,
	};

	/**
	    @brief Gibt eine Referenz auf die einzige Instanz dieser Klasse zur�ck.
	*/
	static const BS_CPUInfo & GetInstance()
	{
		static BS_CPUInfo Instance;
		return Instance;
	}

	/**
	    @brief Gibt die Vendor-ID des CPU-Herstellers zur�ck.
		@remark Gibt BS_CPUInfo::V_UNKNOWN zur�ck, wenn die Vendor-ID nicht bestimmt werden konnte.
	*/
	VENDORID GetVendorID() const { return _VendorID; }

	/**
	    @brief Gibt den Vendor-String zur�ck.
		@remark Gibt "unknown" zur�ck, wenn der Vendor-String nicht bestimmt werden konnte.
	*/
	const std::string & GetVendorString() const { return _VendorString; }

	/**
	    @brief Gibt den CPU-Namen zur�ck.
		@remark Gibt "unknown" zur�ck, wenn der CPU-Name nicht bestimmt werden konnte.
	*/
	const std::string & GetCPUName() const { return _CPUName; }

	/**
	    @brief Gibt zur�ck, ob der Prozessor MMX unters�tzt.
	*/
	bool IsMMXSupported() const { return _MMXSupported; }

	/**
	    @brief Gibt zur�ck, ob der Prozessor SSE unterst�tzt.
	*/
	bool IsSSESupported() const { return _SSESupported; }
	
	/**
		@brief Gibt zur�ck, ob der Prozessor SSE2 unterst�tzt.
	*/
	bool IsSSE2Supported() const { return _SSE2Supported; }

	/**
		@brief Gibt zur�ck, ob der Prozessor 3DNow! unterst�tzt.
	*/
	bool Is3DNowSupported() const { return _3DNowSupported; }

	/**
		@brief Gibt zur�ck, ob der Prozessor 3DNow!-Ext. unterst�tzt.
	*/
	bool Is3DNowExtSupported() const { return _3DNowExtSupported; }

private:
	BS_CPUInfo();

	VENDORID	_VendorID;
	std::string	_VendorString;
	std::string	_CPUName;
	bool		_MMXSupported;
	bool		_SSESupported;
	bool		_SSE2Supported;
	bool		_3DNowSupported;
	bool		_3DNowExtSupported;

	bool _ReadVendor();
	bool _ReadCPUFeatures();
	bool _ReadCPUName();
	bool _IsCPUIDSupported() const;
};

#endif
