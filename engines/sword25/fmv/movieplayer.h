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

#ifndef SWORD25_MOVIEPLAYER_H
#define SWORD25_MOVIEPLAYER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/service.h"

#include "sword25/kernel/memlog_off.h"
#include <string>
#include "sword25/kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_MoviePlayer : public BS_Service
{
public:
	// -----------------------------------------------------------------------------
	// Konstruktion / Destruktion
	// -----------------------------------------------------------------------------

	BS_MoviePlayer(BS_Kernel * pKernel);
	virtual ~BS_MoviePlayer() {};

	// -----------------------------------------------------------------------------
	// Abstraktes Interface, muss von jedem MoviePlayer implementiert werden
	// -----------------------------------------------------------------------------

	/**
		@brief L�dt eine Filmdatei

		Diese Methode l�dt eine Filmdatei und bereitet sie zur Wiedergabe vor.
		Es kann immer nur eine Filmdatei zur Zeit geladen sein. Falls bereits eine Filmdatei geladen
		ist, wird diese entladen und n�tigenfalls die Wiedergabe gestoppt.

		@param Filename der Dateiname der zu ladenden Filmdatei
		@param Z gibt die Z Position des Films auf dem Graphik-Hauptlayer an
		@return Gibt false zur�ck, wenn beim Laden ein Fehler aufgetreten ist, ansonsten true.
	*/
	virtual bool LoadMovie(const std::string & Filename, unsigned int Z) = 0;

	/**
		@brief Entl�dt die gerade geladene Filmdatei

		@return Gibt false zur�ck, wenn beim Entladen ein Fehler aufgetreten ist, ansonsten true.
		@remark Diese Methode darf nur aufgerufen werden, wenn IsMovieLoaded() true zur�ckgibt.
	*/
	virtual bool UnloadMovie() = 0;

	/**
		@brief Spielt den Film ab.

		Der Film wird unter Beibehaltung der Seitenverh�ltnisse auf Bildschirmgr��e skaliert.<br>
		Falls der Film mit einem Aufruf von Pause() pausiert wurde, f�hrt der Film an dieser Stelle fort.

		@return Gibt false zur�ck, wenn ein Fehler aufgetreten ist, ansonsten true.
		@remark Diese Methode darf nur aufgerufen werden, wenn IsMovieLoaded() true zur�ckgibt.
	*/
	virtual bool Play() = 0;

	/**
		@brief Pausiert die Filmwiedergabe.

		Bei einem sp�teren Aufruf von Play() f�hrt die Wiedergabe an der Stelle fort an der der Film Pausiert wurde.

		@return Gibt false zur�ck, wenn ein Fehler aufgetreten ist, ansonsten true.
		@remark Diese Methode darf nur aufgerufen werden, wenn IsMovieLoaded() true zur�ckgibt.
	*/
	virtual bool Pause() = 0;

	/**
		@brief Diese Funktion muss ein mal pro Frame aufgerufen werden.
	*/
	virtual void Update() = 0;

	/**
		@brief Gibt zur�ck, ob ein Film zur Wiedergabe geladen wurde.
	*/
	virtual bool IsMovieLoaded() = 0;

	/**
		@brief Gibt zur�ck, ob die Filmwiedergabe pausiert wurde.
		@remark Diese Methode darf nur aufgerufen werden, wenn IsMovieLoaded() true zur�ckgibt.
	*/
	virtual bool IsPaused() = 0;

	/**
		@brief Gibt den Faktor zur�ck um den der geladene Film skaliert wird.

		Beim Laden wird der Skalierungsfaktor automatisch so gew�hlt, dass der Film die maximal m�gliche Bildschirmfl�che einnimmt, ohne dass der
		Film verzerrt wird.

		@return Gibt den Skalierungsfaktor des Filmes zur�ck.
		@remark Diese Methode darf nur aufgerufen werden, wenn IsMovieLoaded() true zur�ckgibt.
	*/
	virtual float GetScaleFactor() = 0;

	/**
		@brief Legt den Faktor fest um den der geladene Film skaliert werden soll.
		@param ScaleFactor der gew�nschte Skalierungsfaktor.
		@remark Diese Methode darf nur aufgerufen werden, wenn IsMovieLoaded() true zur�ckgibt.
	*/
	virtual void SetScaleFactor(float ScaleFactor) = 0;

	/**
		@brief Gibt die aktuelle Abspielposition in Sekunden zur�ck.
		@remark Diese Methode darf nur aufgerufen werden, wenn IsMovieLoaded() true zur�ckgibt.
	*/
	virtual double GetTime() = 0;

private:
	bool _RegisterScriptBindings();
};

#endif
