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

#ifndef BS_REGION_H
#define BS_REGION_H

#include "kernel/memlog_off.h"
#include <vector>
#include "kernel/memlog_on.h"

#include "kernel/common.h"
#include "kernel/persistable.h"
#include "vertex.h"
#include "polygon.h"
#include "rect.h"

/**
	@brief Diese Klasse ist die Basisklasse aller Regionen.

	Mit der Methode IsValid() l�sst sich abfragen, ob sich das Objekt in einem g�ltigen Zustand befindet.<br>
	Sollte dies nicht der Fall sein, ist die Methode Init() die einzige Methode die aufgerufen werden darf.
	Diese Klasse garantiert, dass die Vertecies der die Umriss- und die Lochpolygone im Uhrzeigersinn angeordnet sind, so dass auf den Polygonen
	arbeitende Algorithmen nur f�r diese Anordnung implementiert werden m�ssen.
*/
class BS_Region : public BS_Persistable
{
protected:
	/**
		@brief Erzeugt ein uninitialisiertes #BS_Region Objekt.

		Nach dem Erzeugen ist das Objekt noch ung�ltig (IsValid() gibt false zur�ck), allerdings kann das Objekt nachtr�glich �ber
		einen Aufruf von Init() in einen g�ltigen Zustand versetzt werden.
	*/
	BS_Region();

	BS_Region(BS_InputPersistenceBlock & Reader, unsigned int Handle);

public:
	enum REGION_TYPE
	{
		RT_REGION,
		RT_WALKREGION,
	};

	static unsigned int Create(REGION_TYPE Type);
	static unsigned int Create(BS_InputPersistenceBlock & Reader, unsigned int Handle = 0);

	virtual ~BS_Region();

	/**
		@brief Initialisiert ein BS_Region Objekt.
		@param Contour ein Polygon das den Umriss der Region angibt.
		@param pHoles ein Pointer auf einen Vector von Polygonen, die L�cher in der Region angeben.<br>
					  Falls die Region keine L�cher hat, muss NULL �bergeben werden.<br>
					  Der Standardwert ist NULL.
		@return Gibt true zur�ck, wenn die Initialisierung erfolgreich war.<br>
				Gibt false zur�ck, wenn die Intialisierung fehlgeschlagen ist.
		@remark Falls die Region bereits initialisiert war, wird der alte Zustand gel�scht.
	*/
	virtual bool Init(const BS_Polygon& Contour, const std::vector<BS_Polygon>* pHoles = NULL);
	
	//@{
	/** @name Sondierende Methoden */

	/**
		@brief Gibt an, ob das Objekt in einem g�ltigen Zustand ist.
		@return Gibt true zur�ck, wenn sich das Objekt in einem g�ltigen Zustand befindet.
				Gibt false zur�ck, wenn sich das Objekt in einem ung�ltigen Zustand befindet.
		@remark Ung�ltige Objekte k�nnen durch einen Aufruf von Init() in einen g�ltigen Zustand versetzt werden.
	*/
	bool IsValid() const { return m_Valid; }

	/**
		@brief Gibt die Position der Region zur�ck.
	*/
	const BS_Vertex& GetPosition() const { return m_Position; }

	/**
		@brief Gibt die Position des Region auf der X-Achse zur�ck.
	*/
	int GetPosX() const { return m_Position.X; }

	/**
		@brief Gibt die Position des Region auf der Y-Achse zur�ck.
	*/
	int GetPosY() const { return m_Position.Y; }

	/**
		@brief Gibt an, ob sich ein Punkt innerhalb der Region befindet.
		@param Vertex ein Vertex, mit den Koordinaten des zu testenden Punktes.
		@return Gibt true zur�ck, wenn sich der Punkt innerhalb der Region befindet.<br>
				Gibt false zur�ck, wenn sich der Punkt au�erhalb der Region befindet.
	*/
	bool IsPointInRegion(const BS_Vertex& Vertex) const;

	/**
		@brief Gibt an, ob sich ein Punkt innerhalb der Region befindet.
		@param X die Position des Punktes auf der X-Achse.
		@param Y die Position des Punktes auf der Y-Achse.
		@return Gibt true zur�ck, wenn sich der Punkt innerhalb der Region befindet.<br>
				Gibt false zur�ck, wenn sich der Punkt au�erhalb der Region befindet.
	*/
	bool IsPointInRegion(int X, int Y) const;

	/**
		@brief Gibt das Umrisspolygon der Region zur�ck.
	*/
	const BS_Polygon& GetContour() const { return m_Polygons[0]; }

	/**
		@brief Gibt die Anzahl der Lochpolygone in der Region zur�ck.
	*/
	int GetHoleCount() const { return static_cast<int>(m_Polygons.size() - 1); }

	/**
		@brief Gibt ein bestimmtes Lochpolygon in der Region zur�ck.
		@param i die Nummer des zur�ckzugebenen Loches.<br>
				 Dieser Wert muss zwischen 0 und GetHoleCount() - 1 liegen.
		@return Gibt das gew�nschte Lochpolygon zur�ck.
	*/
	inline const BS_Polygon& GetHole(unsigned int i) const;

	/**
		@brief Findet f�r einen Punkt ausserhalb der Region den n�chsten Punkt, der sich innerhalb der Region befindet.
		@param Point der Punkt, der sich ausserhalb der Region befindet
		@return Gibt den Punkt innerhalb der Region zur�ck, der den geringsten Abstand zum �bergebenen Punkt hat.
		@remark Diese Methode arbeitet nicht immer Pixelgenau. Man sollte sich also nicht darauf verlassen, dass es wirklich keine Punkt innerhalb der
				Region gibt, der dichter am �bergebenen Punkt liegt.
	*/
	BS_Vertex FindClosestRegionPoint(const BS_Vertex& Point) const;

	/**
		@brief Gibt den Schwerpunkt des Umrisspolygons zur�ck.
	*/
	BS_Vertex GetCentroid() const;

	bool IsLineOfSight(const BS_Vertex & a, const BS_Vertex & b) const;

	//@}

	//@{
	/** @name Manipulierende Methoden */

	/**
		@brief Setzt die Position der Region.
		@param X die neue Position der Region auf der X-Achse.
		@param Y die neue Position der Region auf der Y-Achse.
	*/
	virtual void SetPos(int X, int Y);

	/**
		@brief Setzt die Position der Region auf der X-Achse.
		@param X die neue Position der Region auf der X-Achse.
	*/
	void SetPosX(int X);

	/**
		@brief Setzt die Position der Region auf der Y-Achse.
		@param Y die neue Position der Region auf der Y-Achse.
	*/
	void SetPosY(int Y);

	//@}

	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

protected:
	/// Diese Variable gibt den Typ des Objektes an.
	REGION_TYPE m_Type;
	/// Diese Variable gibt an, ob der aktuelle Objektzustand g�ltig ist.
	bool m_Valid;
	/// Dieses Vertex gibt die Position der Region an.
	BS_Vertex m_Position;
	/// Dieser Vector enth�lt alle Polygone die die Region definieren. Das erste Element des Vectors ist die Kontur, alle weiteren sind die L�cher.
	std::vector<BS_Polygon> m_Polygons;
	/// Die Bounding-Box der Region.
	BS_Rect m_BoundingBox;

	/**
		@brief Aktualisiert die Bounding-Box der Region.
	*/
	void UpdateBoundingBox();

	/**
		@brief Findet den Punkt auf einer Linie, der einem anderen Punkt am n�chsten ist.
		@param LineStart der Startpunkt der Linie
		@param LineEnd der Endpunkt der Linie
		@param Point der Punkt, zu dem der n�chste Punkt auf der Linie konstruiert werden soll.
		@return Gibt den Punkt auf der Linie zur�ck, der dem �bergebenen Punkt am n�chsten ist.
	*/
	BS_Vertex FindClosestPointOnLine(const BS_Vertex & LineStart, const BS_Vertex & LineEnd, const BS_Vertex Point) const;
};


// -----------------------------------------------------------------------------
// Inlines
// -----------------------------------------------------------------------------

inline const BS_Polygon& BS_Region::GetHole(unsigned int i) const
{
	BS_ASSERT(i < m_Polygons.size() - 1);
	return m_Polygons[i + 1];
}

#endif
