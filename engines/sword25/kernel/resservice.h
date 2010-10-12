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

#ifndef SWORD25_RESOURCESERVICE_H
#define SWORD25_RESOURCESERVICE_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/service.h"
#include "sword25/kernel/kernel.h"
#include "sword25/kernel/resmanager.h"

class BS_Resource;

class BS_ResourceService : public BS_Service
{
public:
	BS_ResourceService(BS_Kernel* pKernel) : BS_Service(pKernel) 
	{
		BS_ResourceManager* pResource = pKernel->GetResourceManager();
		pResource->RegisterResourceService(this);
	}

	virtual ~BS_ResourceService() {}


	/**
		@brief L�dt eine Resource.
		@param FileName Dateiname
		@return gibt die Resource zur�ck, falls erfolgreich, ansonsten NULL
	*/
	virtual BS_Resource* LoadResource(const std::string& FileName) = 0;

	/**
		@brief CanLoadResource pr�ft, ob Resource vom Service geladen werden kann.
		@param FileName Dateiname
		@return true, falls Service Resource laden kann
		@remark �berpr�fung basiert auf dem Dateinamen, wenn das Dateiformat nicht passt, gibts �rger.
	*/
	virtual bool CanLoadResource(const std::string& FileName) = 0;

protected:
	// Hilfsmethoden f�r BS_ResourceService Klassen

	/**
		@brief Vergleicht zwei Strings, wobei der zweite String die Wildcards * und ? enthalten darf
		@param String der erste Vergleichsstring. Dieser darf keine Wildcards enthalten.
		@param Pattern der zweite Vergleichsstring. Dieser darf die Wildcards * und ? enthalten.
		@return Gibt true zur�ck, wenn der String auf das Pattern passt, ansonsten false.
	*/
	bool _WildCardStringMatch(const std::string& String, const std::string& Pattern)
	{
		return _WildCardStringMatchRecursion(String.c_str(), Pattern.c_str());
	}

private:
	bool _WildCardStringMatchRecursion(const char* String, const char* Pattern)
	{
		// Rekursionsabschl�sse:
		// 1. Der Pattern-String enth�lt nur noch * -> TRUE
		if (*Pattern == '*')
		{
			// Es muss mit einer Kopie von Pattern gearbeitet werden um den aktuellen Zustand nicht zu zerst�ren
			char* PatternCopy = (char*) Pattern;
			while (*PatternCopy == '*') { PatternCopy++; }
			if (!*PatternCopy) return true;
		}
		// 2. Der String ist zuende, das Pattern aber noch nicht -> FALSE
		if (!*String && *Pattern) return false;
		// 3. Der String ist zuende, also ist auch das Pattern zuende (s.o.) -> TRUE
		if (!*String) return true;
		
		// Rekursionsaufruf 1:
		// Falls die beiden aktuellen Zeichen gleich sind, oder Pattern '?' ist wird das Ergebnis von restlichen String zur�ckgegeben
		if (*String == *Pattern || *Pattern == '?') return _WildCardStringMatchRecursion(String + 1, Pattern + 1);
		
		// Falls nicht, wird untersucht ob ein '*' vorliegt
		if (*Pattern == '*')
		{
			// Rekursionsaufruf 2:
			// Zun�chst wird das Ergebnis von String und Pattern + 1 untersucht...
			if (_WildCardStringMatchRecursion(String, Pattern + 1)) return true;
			// Falls das fehlschl�gt, wird das Ergebnis von String + 1 Pattern zur�ckgegeben
			else return _WildCardStringMatchRecursion(String + 1, Pattern);
			// Die Rekursion kehrt also immer wieder an diese Stelle zur�ck, bis das Zeichen in String, 
			// dem nach dem '*' in Pattern entspricht
		}
		
		// Wenn kein '*' in Pattern vorliegt, schl�gt der Vergleich fehl
		return false;
	}
};

#endif
