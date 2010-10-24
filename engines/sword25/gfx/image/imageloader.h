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
    ImageLoader
    --------------

    Autor: Malte Thiesen
*/

#ifndef SWORD25_IMAGELOADER_H
#define SWORD25_IMAGELOADER_H

#include "common/singleton.h"
#include "common/list.h"

#include "sword25/kernel/common.h"
#include "sword25/gfx/graphicengine.h"

namespace Sword25 {

/**
 * This interface is implemented by classes for loading various image file formats.
 * Adding support for new formats requires subclassing this, and implementing all
 * three pure virtual methods. In addition, ImageLoaderManager needs to be taught
 * about the new loader class.
 *
 * Note that an implementation must be able to convert any input to format GraphicEngine::CF_ARGB32
 */
class ImageLoader {
protected:
	friend class ImageLoaderManager;

	// Make the default constructor protected to ensure that only the
	// ImageLoaderManager can create instances of ImageLoader subclasses.
	ImageLoader() {}

	virtual ~ImageLoader() {}

	/**
	    @brief Gibt an, ob der #ImageLoader ein Bild lesen kann.
	    @param pFileData ein Pointer auf die kompletten Daten des Bildes.
	    @param FileSize die Gr��e der Daten in Byte.
	    @return Gibt true zur�ck, wenn der #ImageLoader das Bild lesen kann, ansonsten false.
	    @remark Diese Methode muss von allen ImageLoader Klassen implementiert werden.
	*/
	virtual bool isCorrectImageFormat(const byte *pFileData, uint fileSize) = 0;

	/**
	    @brief L�dt eine Bilddatei.
	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Gr��e der Bilddaten in Byte.
	    @param ColorFormat gibt das gew�nschte Farbformat an, in das die Bilddaten konvertiert werden sollen.<br>
	                       Folgende Farbformate werden unterst�tzt:
	                       - GraphicEngine::CF_ARGB32
	    @param pUncompressedData nach erfolgreichen Laden zeigt dieser Pointer auf die enpackten und konvertierten Bilddaten.
	    @param Width gibt nach erfolgreichen Laden die Breite des geladenen Bildes an.
	    @param Height gibt nach erfolgreichen Laden die H�he des geladenen Bildes an.
	    @param Pitch gibt nach erfolgreichen Laden die L�nge einer Bildzeile in Byte an.
	    @return Gibt false zur�ck, falls das Laden fehlgeschlagen ist.
	    @remark Die Gr��e der Ausgabedaten in Bytes kann wie folgt berechnet werden: Pitch * Height.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	    @remark Diese Methode muss von allen ImageLoader Klassen implementiert werden.
	*/
	virtual bool decodeImage(const byte *pFileData, uint fileSize,
	                         GraphicEngine::COLOR_FORMATS colorFormat,
	                         byte *&pUncompressedData,
	                         int &width, int &height,
	                         int &pitch) = 0;

	/**
	    @brief Liest die Bildeigenschaften aus.
	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Gr��e des Bilddaten in Byte.
	    @param ColorFormat enth�lt nach einem erfolgreichem Aufruf das Farbformat des Bildes.
	    @param Width enth�lt nach einem erfolgreichem Aufruf die Breite des Bildes in Pixeln.
	    @param Height enth�lt nach einem erfolgreichem Aufruf die H�he des Bildes in Pixeln.
	    @return Gibt false zur�ck, wenn die Bildeigenschaften nicht ausgelesen werden konnten.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	    @remark Diese Methode muss von allen ImageLoader Klassen implementiert werden.
	*/
	virtual bool imageProperties(const byte *pFileData, uint fileSize,
	                             GraphicEngine::COLOR_FORMATS &colorFormat,
	                             int &width, int &height) = 0;

};


class ImageLoaderManager : public Common::Singleton<ImageLoaderManager> {
public:

	/**
	 * Create instances of various ImageLoader subclasses and keep them
	 * in an internal list. These are then later used when loading images
	 * via this manager.
	 *
	 * @note To add support for additional ImageManagers, you need to modify
	 *       this method.
	 */
	ImageLoaderManager();

	/**
	 * Destroy any ImageLoader subclasses instances kept by this manager.
	 */
	~ImageLoaderManager();

	//@{
	/** @name Methods for loading */

	/**
	    @brief L�dt eine Bilddatei.

	    Diese Methode kann s�mtliche unterst�tztem Bildformate lesen. Die Methode erkennt selbstst�nding um welches Dateiformat es sich
	    bei der vorliegenden Datei handelt.<br>
	    Bisher wird nur PNG unterst�tzt.

	    @param pFileData ein Pointer auf die Bilddaten.
	    @param FileSize die Gr��e der Bilddaten in Byte.
	    @param ColorFormat gibt das gew�nschte Farbformat an, in das die Bilddaten konvertiert werden sollen.<br>
	                       Folgende Farbformate werden unterst�tzt:
	                       - GraphicEngine::CF_ARGB32
	    @param pUncompressedData nach erfolgreichen Laden zeigt dieser Pointer auf die enpackten und konvertierten Bilddaten.
	    @param Width gibt nach erfolgreichen Laden die Breite des geladenen Bildes an.
	    @param Height gibt nach erfolgreichen Laden die H�he des geladenen Bildes an.
	    @param Pitch gibt nach erfolgreichen Laden die L�nge einer Bildzeile in Byte an.
	    @return Gibt false zur�ck, falls das Laden fehlgeschlagen ist.
	    @remark Die Gr��e der Ausgabedaten in Bytes kann wie folgt berechnet werden: Pitch * Height.
	    @remark Es darf nicht vergessen werden, die Ausgabedaten nach erfolgter Benutzung mit delete freizugeben.
	*/
	bool loadImage(const byte *pFileData, uint fileSize,
	                      GraphicEngine::COLOR_FORMATS colorFormat,
	                      byte *&pUncompressedData,
	                      int &width, int &height,
	                      int &pitch);

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
	bool extractImageProperties(const byte *pFileData, uint fileSize,
	                                   GraphicEngine::COLOR_FORMATS &colorFormat,
	                                   int &width, int &height);
	//@}

private:

	/**
	 * Finds an ImageLoader instances capable of decoding the given
	 * image data.
	 * @return pointer to a suitable ImageLoader, or NULL if none has been be found
	 */
	ImageLoader *findSuitableImageLoader(const byte *pFileData, uint fileSize);

	Common::List<ImageLoader *> _imageLoaderList;	///< List of ImageLoader instances
};

/** Shortcut for accessing the image loader manager. */
#define ImageMan		ImageLoaderManager::instance()

} // End of namespace Sword25

#endif
