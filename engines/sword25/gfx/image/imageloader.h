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

/*
    BS_ImageLoader
    --------------

    Autor: Malte Thiesen
*/

#ifndef SWORD25_IMAGELOADER_H
#define SWORD25_IMAGELOADER_H

// Includes
#include "sword25/kernel/bs_stdint.h"
#include "sword25/kernel/common.h"
#include "sword25/gfx/graphicengine.h"

namespace Sword25 {

/**
    @brief �ber die statischen Methoden dieser Klasse werden alle unterst�tzten Bildformate geladen.

    Zum Laden von Bildern wird die #LoadImage-Methode benutzt.

    Au�erdem stellt diese Klasse das Interface da, das alle Klassen implementieren m�ssen, die Bildformate einlesen.<br>
    Zur Unterst�tzung eines neuen Bildformates muss folgenderma�en vorgegangen werden:
    - Erzeugen einer neuen von #BS_ImageLoader abgeleiteten Klasse, die die Methoden #IsCorrectImageFormat und #DecodeImage impelementiert.
    - Die Klasse muss eine statische Methode haben, die eine Instanz von ihr erzeugt und einen Pointer darauf zur�ckgibt.
    - Diese Methode muss in der Liste in der Datei imageloader_ids.h eingetragen werden.
    - Die Klasse muss JEDES Eingabebild seines Bildformates in die folgenden Farbformate konvertieren k�nnen:
        - BS_GraphicEngine::CF_ARGB32
    - Zum Konvertieren der Bilddaten k�nnen die Hilfsmethoden dieser Klasse benutzt werden, die ARGB Bilddaten in alle ben�tigten
      Farbformate konvertieren.
*/
class ImageLoader {
public:

	//@{
	/** @name Lade Methoden */

	/**
	    @brief L�dt eine Bilddatei.

	    Diese Methode kann s�mtliche unterst�tztem Bildformate lesen. Die Methode erkennt selbstst�nding um welches Dateiformat es sich
	    bei der vorliegenden Datei handelt.<br>
	    Bisher wird nur PNG unterst�tzt.

	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Gr��e der Bilddaten in Byte.
	    @param ColorFormat gibt das gew�nschte Farbformat an, in das die Bilddaten konvertiert werden sollen.<br>
	                       Folgende Farbformate werden unterst�tzt:
	                       - BS_GraphicEngine::CF_ARGB32
	    @param pUncompressedData nach erfolgreichen Laden zeigt dieser Pointer auf die enpackten und konvertierten Bilddaten.
	    @param Width gibt nach erfolgreichen Laden die Breite des geladenen Bildes an.
	    @param Height gibt nach erfolgreichen Laden die H�he des geladenen Bildes an.
	    @param Pitch gibt nach erfolgreichen Laden die L�nge einer Bildzeile in Byte an.
	    @return Gibt false zur�ck, falls das Laden fehlgeschlagen ist.
	    @remark Die Gr��e der Ausgabedaten in Bytes kann wie folgt berechnet werden: Pitch * Height.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	*/
	static bool LoadImage(const byte *pFileData, uint FileSize,
	                      GraphicEngine::COLOR_FORMATS ColorFormat,
	                      byte *&pUncompressedData,
	                      int &Width, int &Height,
	                      int &Pitch);

	/**
	    @brief Liest die Bildeigenschaften eines Bildes aus.

	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Gr��e des Bilddaten in Byte.
	    @param ColorFormat enth�lt nach einem erfolgreichem Aufruf das Farbformat des Bildes.
	    @param Width enth�lt nach einem erfolgreichem Aufruf die Breite des Bildes in Pixeln.
	    @param Height enth�lt nach einem erfolgreichem Aufruf die H�he des Bildes in Pixeln.
	    @return Gibt false zur�ck, wenn die Bildeigenschaften nicht ausgelesen werden konnten.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	*/
	static bool ExtractImageProperties(const byte *pFileData, uint FileSize,
	                                   GraphicEngine::COLOR_FORMATS &ColorFormat,
	                                   int &Width, int &Height);
	//@}

protected:

	// Protected Konstruktor, damit Instanzen dieser Klasse nur von BS_ImageLoader-Objekten erstellt werden k�nnen
	/**
	    @brief Der Standardkonstruktor.

	    Dieser Konstruktor registriert alle Instanzen von #BS_ImageLoader-Klassen in einer Liste.<br>
	    Diese Liste enth�lt jeweils eine Instanz jedes #BS_ImageLoader und wird benutzt um beliebige Bilddateien einem Loader zuzuordnen.
	    @remark Dieser Konstruktor ist protected damit nur #BS_ImageLoader-Objekte diese Klasse instanziieren k�nnen.
	*/
	ImageLoader() {
		// Klasse registrieren
		_ImageLoaderList.push_front(this);
	}

	virtual ~ImageLoader() {}

	//@{
	/** @name Abstrakte Methoden */

	/**
	    @brief Gibt an, ob der #BS_ImageLoader ein Bild lesen kann.
	    @param pFileData ein Pointer auf die kompletten Daten des Bildes.
	    @param FileSize die Gr��e der Daten in Byte.
	    @return Gibt true zur�ck, wenn der #BS_ImageLoader das Bild lesen kann, ansonsten false.
	    @remark Diese Methode muss von allen BS_ImageLoader Klassen implementiert werden.
	*/
	virtual bool IsCorrectImageFormat(const byte *pFileData, uint FileSize) = 0;

	/**
	    @brief L�dt eine Bilddatei.
	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Gr��e der Bilddaten in Byte.
	    @param ColorFormat gibt das gew�nschte Farbformat an, in das die Bilddaten konvertiert werden sollen.<br>
	                       Folgende Farbformate werden unterst�tzt:
	                       - BS_GraphicEngine::CF_ARGB32
	    @param pUncompressedData nach erfolgreichen Laden zeigt dieser Pointer auf die enpackten und konvertierten Bilddaten.
	    @param Width gibt nach erfolgreichen Laden die Breite des geladenen Bildes an.
	    @param Height gibt nach erfolgreichen Laden die H�he des geladenen Bildes an.
	    @param Pitch gibt nach erfolgreichen Laden die L�nge einer Bildzeile in Byte an.
	    @return Gibt false zur�ck, falls das Laden fehlgeschlagen ist.
	    @remark Die Gr��e der Ausgabedaten in Bytes kann wie folgt berechnet werden: Pitch * Height.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	    @remark Diese Methode muss von allen BS_ImageLoader Klassen implementiert werden.
	*/
	virtual bool DecodeImage(const byte *pFileData, uint FileSize,
	                         GraphicEngine::COLOR_FORMATS ColorFormat,
	                         byte *&pUncompressedData,
	                         int &Width, int &Height,
	                         int &Pitch) = 0;

	/**
	    @brief Liest die Bildeigenschaften aus.
	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Gr��e des Bilddaten in Byte.
	    @param ColorFormat enth�lt nach einem erfolgreichem Aufruf das Farbformat des Bildes.
	    @param Width enth�lt nach einem erfolgreichem Aufruf die Breite des Bildes in Pixeln.
	    @param Height enth�lt nach einem erfolgreichem Aufruf die H�he des Bildes in Pixeln.
	    @return Gibt false zur�ck, wenn die Bildeigenschaften nicht ausgelesen werden konnten.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	    @remark Diese Methode muss von allen BS_ImageLoader Klassen implementiert werden.
	*/
	virtual bool ImageProperties(const byte *pFileData, uint FileSize,
	                             GraphicEngine::COLOR_FORMATS &ColorFormat,
	                             int &Width, int &Height) = 0;

	//@}

private:

	/**
	    @brief Erzeugt je eine Instanz aller BS_ImageLoader Klassen und f�gt diese in eine interne Liste ein. Diese werden dann beim
	           Laden von Bildern benutzt.
	    @remark Die Klassen m�ssen in der Datei imageloader_ids.h eingetragen sein, damit sie an dieser Stelle ber�cksichtigt werden.
	*/
	static void _InitializeLoaderList();

	/**
	    @brief Zerst�rt alle Instanzen von BS_ImageLoader Klassen, die in dieser Klasse registriert sind.
	*/
	static void _DeinitializeLoaderList();

	/**
	    @brief Sucht zu Bilddaten ein BS_ImageLoader Objekt, dass die Bilddaten dekodieren kann.
	    @return Gibt einen Pointer auf ein passendes BS_ImageLoader Objekt zur�ck, oder NULL, wenn kein passendes Objekt gefunden wurde.
	*/
	static ImageLoader *_FindSuitableImageLoader(const byte *pFileData, uint FileSize);

	static Common::List<ImageLoader *>   _ImageLoaderList;              // Die Liste aller BS_ImageLoader-Objekte
	static bool                         _ImageLoaderListInitialized;    // Gibt an, ob die Liste schon intialisiert wurde
};

} // End of namespace Sword25

#endif
