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
    BS_Image
    --------

    Autor: Malte Thiesen
*/

#ifndef SWORD25_IMAGE_H
#define SWORD25_IMAGE_H

// Includes
#include "sword25/kernel/common.h"
#include "common/rect.h"
#include "sword25/gfx/graphicengine.h"

namespace Sword25 {

class Image {
public:
	virtual ~Image() {}

	// Enums
	/**
	    @brief Die m�glichen Flippingparameter f�r die Blit-Methode.
	*/
	enum FLIP_FLAGS {
		/// Das Bild wird nicht gespiegelt.
		FLIP_NONE = 0,
		/// Das Bild wird an der horizontalen Achse gespiegelt.
		FLIP_H = 1,
		/// Das Bild wird an der vertikalen Achse gespiegelt.
		FLIP_V = 2,
		/// Das Bild wird an der horizontalen und vertikalen Achse gespiegelt.
		FLIP_HV = FLIP_H | FLIP_V,
		/// Das Bild wird an der horizontalen und vertikalen Achse gespiegelt.
		FLIP_VH = FLIP_H | FLIP_V
	};

	//@{
	/** @name Accessor-Methoden */

	/**
	    @brief Gibt die Breite des Bildes in Pixeln zur�ck
	*/
	virtual int getWidth() const = 0;

	/**
	    @brief Gibt die H�he des Bildes in Pixeln zur�ck
	*/
	virtual int getHeight() const = 0;

	/**
	    @brief Gibt das Farbformat des Bildes zur�ck
	*/
	virtual GraphicEngine::COLOR_FORMATS getColorFormat() const = 0;

	//@}

	//@{
	/** @name Render-Methoden */

	/**
	    @brief Rendert das Bild in den Framebuffer.
	    @param pDest ein Pointer auf das Zielbild. In den meisten F�llen ist dies der Framebuffer.
	    @param PosX die Position auf der X-Achse im Zielbild in Pixeln, an der das Bild gerendert werden soll.<br>
	                Der Standardwert ist 0.
	    @param PosY die Position auf der Y-Achse im Zielbild in Pixeln, an der das Bild gerendert werden soll.<br>
	                Der Standardwert ist 0.
	    @param Flipping gibt an, wie das Bild gespiegelt werden soll.<br>
	                    Der Standardwert ist BS_Image::FLIP_NONE (keine Spiegelung)
	    @param pSrcPartRect Pointer auf ein Common::Rect, welches den Ausschnitt des Quellbildes spezifiziert, der gerendert
	                        werden soll oder NULL, falls das gesamte Bild gerendert werden soll.<br>
	                        Dieser Ausschnitt bezieht sich auf das ungespiegelte und unskalierte Bild.<br>
	                        Der Standardwert ist NULL.
	    @param Color ein ARGB Farbwert, der die Parameter f�r die Farbmodulation und f�rs Alphablending festlegt.<br>
	                 Die Alpha-Komponente der Farbe bestimmt den Alphablending Parameter (0 = keine Deckung, 255 = volle Deckung).<br>
	                 Die Farbkomponenten geben die Farbe f�r die Farbmodulation an.<br>
	                 Der Standardwert is BS_ARGB(255, 255, 255, 255) (volle Deckung, keine Farbmodulation).
	                 Zum Erzeugen des Farbwertes k�nnen die Makros BS_RGB und BS_ARGB benutzt werden.
	    @param Width gibt die Ausgabebreite des Bildausschnittes an.
	                 Falls diese von der Breite des Bildausschnittes abweicht wird
	                 das Bild entsprechend Skaliert.<br>
	                 Der Wert -1 gibt an, dass das Bild nicht Skaliert werden soll.<br>
	                 Der Standardwert ist -1.
	    @param Width gibt die Ausgabeh�he des Bildausschnittes an.
	                 Falls diese von der H�he des Bildauschnittes abweicht, wird
	                 das Bild entsprechend Skaliert.<br>
	                 Der Wert -1 gibt an, dass das Bild nicht Skaliert werden soll.<br>
	                 Der Standardwert ist -1.
	    @return Gibt false zur�ck, falls das Rendern fehlgeschlagen ist.
	    @remark Er werden nicht alle Blitting-Operationen von allen BS_Image-Klassen unterst�tzt.<br>
	            Mehr Informationen gibt es in der Klassenbeschreibung von BS_Image und durch folgende Methoden:
	            - IsBlitTarget()
	            - IsScalingAllowed()
	            - IsFillingAllowed()
	            - IsAlphaAllowed()
	            - IsColorModulationAllowed()
	            - IsSetContentAllowed()
	*/
	virtual bool blit(int posX = 0, int posY = 0,
	                  int flipping = FLIP_NONE,
	                  Common::Rect *pPartRect = NULL,
	                  uint color = BS_ARGB(255, 255, 255, 255),
	                  int width = -1, int height = -1) = 0;

	/**
	    @brief F�llt einen Rechteckigen Bereich des Bildes mit einer Farbe.
	    @param pFillRect Pointer auf ein Common::Rect, welches den Ausschnitt des Bildes spezifiziert, der gef�llt
	                      werden soll oder NULL, falls das gesamte Bild gef�llt werden soll.<br>
	                      Der Standardwert ist NULL.
	    @param Color der 32 Bit Farbwert mit dem der Bildbereich gef�llt werden soll.
	    @remark Es ist m�glich �ber die Methode transparente Rechtecke darzustellen, indem man eine Farbe mit einem Alphawert ungleich
	            255 angibt.
	    @remark Unabh�ngig vom Farbformat des Bildes muss ein 32 Bit Farbwert angegeben werden. Zur Erzeugung, k�nnen die Makros
	            BS_RGB und BS_ARGB benutzt werden.
	    @remark Falls das Rechteck nicht v�llig innerhalb des Bildschirms ist, wird es automatisch zurechtgestutzt.
	*/
	virtual bool fill(const Common::Rect *pFillRect = 0, uint color = BS_RGB(0, 0, 0)) = 0;

	/**
	    @brief F�llt den Inhalt des Bildes mit Pixeldaten.
	    @param Pixeldata ein Vector der die Pixeldaten enth�lt. Sie m�ssen in dem Farbformat des Bildes vorliegen und es m�ssen gen�gend Daten
	           vorhanden sein, um das ganze Bild zu f�llen.
	    @param Offset der Offset in Byte im Pixeldata-Vector an dem sich der erste zu schreibende Pixel befindet.<br>
	           Der Standardwert ist 0.
	    @param Stride der Abstand in Byte zwischen dem Zeilenende und dem Beginn einer neuen Zeile im Pixeldata-Vector.<br>
	           Der Standardwert ist 0.
	    @return Gibt false zur�ck, falls der Aufruf fehlgeschlagen ist.
	    @remark Ein Aufruf dieser Methode ist nur erlaubt, wenn IsSetContentAllowed() true zur�ckgibt.
	*/
	virtual bool setContent(const byte *pixeldata, uint size, uint offset, uint stride) = 0;

	/**
	    @brief Liest einen Pixel des Bildes.
	    @param X die X-Koordinate des Pixels.
	    @param Y die Y-Koordinate des Pixels
	    @return Gibt den 32-Bit Farbwert des Pixels an der �bergebenen Koordinate zur�ck.
	    @remark Diese Methode sollte auf keine Fall benutzt werden um gr��ere Teile des Bildes zu lesen, da sie sehr langsam ist. Sie ist
	            eher daf�r gedacht einzelne Pixel des Bildes auszulesen.
	*/
	virtual uint getPixel(int x, int y) = 0;

	//@{
	/** @name Auskunfts-Methoden */

	/**
	    @brief �berpr�ft, ob an dem BS_Image Blit() aufgerufen werden darf.
	    @return Gibt false zur�ck, falls ein Blit()-Aufruf an diesem Objekt nicht gestattet ist.
	*/
	virtual bool isBlitSource() const = 0;

	/**
	    @brief �berpr�ft, ob das BS_Image ein Zielbild f�r einen Blit-Aufruf sein kann.
	    @return Gibt false zur�ck, falls ein Blit-Aufruf mit diesem Objekt als Ziel nicht gestattet ist.
	*/
	virtual bool isBlitTarget() const = 0;

	/**
	    @brief Gibt true zur�ck, falls das BS_Image bei einem Aufruf von Blit() skaliert dargestellt werden kann.
	*/
	virtual bool isScalingAllowed() const = 0;

	/**
	    @brief Gibt true zur�ck, wenn das BS_Image mit einem Aufruf von Fill() gef�llt werden kann.
	*/
	virtual bool isFillingAllowed() const = 0;

	/**
	    @brief Gibt true zur�ck, wenn das BS_Image bei einem Aufruf von Blit() mit einem Alphawert dargestellt werden kann.
	*/
	virtual bool isAlphaAllowed() const = 0;

	/**
	    @brief Gibt true zur�ck, wenn das BS_Image bei einem Aufruf von Blit() mit Farbmodulation dargestellt werden kann.
	*/
	virtual bool isColorModulationAllowed() const = 0;

	/**
	    @brief Gibt true zur�ck, wenn der Inhalt des BS_Image durch eine Aufruf von SetContent() ausgetauscht werden kann.
	*/
	virtual bool isSetContentAllowed() const = 0;

	//@}
};

} // End of namespace Sword25

#endif
