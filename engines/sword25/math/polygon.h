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

#ifndef SWORD25_POLYGON_H
#define SWORD25_POLYGON_H

// Includes
#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/math/vertex.h"

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

class BS_Vertex;

/**
	@brief Eine Polygonklasse.
*/
class BS_Polygon : public BS_Persistable
{
public:
	/**
		@brief Erzeugt ein Objekt vom Typ #BS_Polygon, das 0 Vertecies enth�lt.

		Mit der Methode Init() k�nnen dem Polygon sp�ter Vertecies hinzugef�gt werden.
	*/
	BS_Polygon();

	/**
		@brief Copy-Constructor
	*/
	BS_Polygon(const BS_Polygon& Other);

	/**
		@brief Erstellt ein Polygon anhand persistierter Daten.
	*/
	BS_Polygon(BS_InputPersistenceBlock & Reader);

	/**
		@brief Erzeugt ein Objekt vom Typ #BS_Polygon und ordnet ihm Vertecies zu.
		@param VertexCount die Anzahl der Vertecies im Vertex Array.
		@param Vertecies ein Array, das Objekte vom Typ BS_Vertex enth�lt, die die Vertecies des Polygons darstellen.
		@remark Die Vertecies m�ssen ein nicht selbst�berschneidendes Polygon definieren.
				Falls das Polygon selbst�berschneidend sein sollte wird ein leeres BS_Polygon Objekt erzeugt.
	*/
	BS_Polygon(int VertexCount, const BS_Vertex* Vertecies);

	/**
		@brief L�scht das BS_Polygon Objekt.
	*/
	virtual ~BS_Polygon();

	/**
		@brief Initialisiert das BS_Polygon mit einer Liste von Vertecies.

		Die Vertecies m�ssen ein nicht selbst�berschneidendes Polygon definieren.
		Es kann auch einem Polygon, welches bereits Vertecies enth�lt, mit einer neue Vertexliste initialisiert werden, dabei gehen die
		alten Vertecies verloren.

		@param VertexCount die Anzahl der Vertecies im Vertecies Array.
		@param Vertecies ein Array, das Objekte vom Typ BS_Vertex enth�lt, die die Vertecies des Polygons darstellen.
		@return Gibt false zur�ck, falls die Vertecies ein selbst�berschneidendes Polygon definiert haben.
				In diesem Fall wird das Objekt nicht initialisiert.
	*/
	bool Init(int VertexCount, const BS_Vertex* Vertecies);

	//@{
	/** @name Sondierende Methoden */
	
	/**
		@brief �berpr�ft, ob die Vertecies des Polygons im Uhrzeigersinn angeordnet sind.
		@return Gibt true zur�ck, wenn die Vertecies des Polygons im Uhrzeigersinn angeordnet sind oder Koplanar sind.<br>
				Gibt false zur�ck, wenn die Vertecies des Polygons entgegen dem Uhrzeigersinn angeordnet sind.
		@remark Diese Methode gibt nur ein sinnvolles Ergebnis zur�ck, wenn das Polygon mindestens 3 Vertecies hat.
	*/
	bool IsCW() const;

	/**
		@brief �berpr�ft, ob die Vertecies des Polygons entgegen dem Uhrzeigersinn angeordnet sind.
		@return Gibt true zur�ck, wenn die Vertecies des Polygons entgegen dem Uhrzeigersinn angeordnet sind.<br>
				Gibt false zur�ck, wenn die Vertecies des Polygons im Uhrzeigersinn angeordnet sind oder Koplanar sind.
		@remark Diese Methode gibt nur ein sinnvolles Ergebnis zur�ck, wenn das Polygon mindestens 3 Vertecies hat.
				
	*/
	bool IsCCW() const;

	/**
		@brief �berpr�ft, ob das Polygon konvex ist.
		@return Gibt true zur�ck, wenn das Polygon konvex ist.<br>
				Gibt false zur�ck, wenn das Polygon konkav ist.
		@remark Diese Methode gibt nur ein sinnvolles Ergebnis zur�ck, wenn das Polygon mindestens 3 Vertecies hat.
	*/
	bool IsConvex() const;

	/** 
		@brief �berpr�ft, ob das Polygon konkav ist.
		@return Gibt true zur�ck, wenn das Polygon konkav ist.<br>
				Gibt false zur�ck, wenn das Polygon konvex ist.
		@remark Diese Methode gibt nur ein sinnvolles Ergebnis zur�ck, wenn das Polygon mindestens 3 Vertecies hat.
	*/
	bool IsConcave() const;

	/**
		@brief �berpr�ft, ob sich ein Punkt innerhalb des Polygons befindet.
		@param Vertex ein Vertex, mit den Koordinaten des zu testenden Punktes.
		@param BorderBelongsToPolygon gibt an, ob der Rand des Polygons als Teil des Polygons betrachtet werden soll.<br>
									  Der Standardwert ist true.
		@return Gibt true zur�ck, wenn sich der Punkt innerhalb des Polygons befindet.<br>
				Gibt false zur�ck, wenn sich der Punkt au�erhalb des Polygons befindet.
	*/
	bool IsPointInPolygon(const BS_Vertex& Vertex, bool BorderBelongsToPolygon = true) const;

	/**
		@brief �berpr�ft, ob sich ein Punkt innerhalb des Polygons befindet.
		@param X die Position des Punktes auf der X-Achse.
		@param Y die Position des Punktes auf der Y-Achse.
		@param BorderBelongsToPolygon gibt an, ob der Rand des Polygons als Teil des Polygons betrachtet werden soll.<br>
									  Der Standardwert ist true.
		@return Gibt true zur�ck, wenn sich der Punkt innerhalb des Polygons befindet.<br>
				Gibt false zur�ck, wenn sich der Punkt au�erhalb des Polygons befindet.
	*/
	bool IsPointInPolygon(int X, int Y, bool BorderBelongsToPolygon = true) const;

	/**
		@brief Gibt den Schwerpunkt des Polygons zur�ck.
	*/
	BS_Vertex GetCentroid() const;

	// Rand geh�rt zum Polygon
	// Polygon muss CW sein
	bool IsLineInterior(const BS_Vertex & a, const BS_Vertex & b) const;
	// Rand geh�rt nicht zum Polygon
	// Polygon muss CW sein
	bool IsLineExterior(const BS_Vertex & a, const BS_Vertex & b) const;

	//@}

	//@{
	/** @name Manipulierende Methoden */

	/**
		@brief Stellt sicher, dass die Vertecies des Polygons im Uhrzeigersinn angeordnet sind.
	*/
	void EnsureCWOrder();

	/**
		@brief Stellt sicher, dass die Vertecies des Polygons entgegen dem Uhrzeigersinn angeordnet sind.
	*/
	void EnsureCCWOrder();

	/**
		@brief Kehrt die Reihenfolge der Vertecies um.
	*/
	void ReverseVertexOrder();

	/**
		@brief Verschiebt das Polygon.
		@param Delta das Vertex um das das Polygon verschoben werden soll.
	*/
	void operator+=(const BS_Vertex& Delta);

	//@}

	/// Gibt die Anzahl an Vertecies im Vertecies-Array an.
	int VertexCount;
	/// Enth�lt die Vertecies des Polygons.
	BS_Vertex* Vertecies;

	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

private:
	bool m_IsCW;
	bool m_IsConvex;
	BS_Vertex m_Centroid;
	
	/**
		@brief Berechnet den Schwerpunkt des Polygons.
	*/
	BS_Vertex ComputeCentroid() const;

	/**
		@brief Bestimmt wie die Vertecies des Polygon angeordnet sind.
		@return Gibt true zur�ck, wenn die Vertecies im Uhrzeigersinn angeordnet sind, ansonsten false.
	*/
	bool ComputeIsCW() const;

	/**
		@brief Bestimmt ob das Polygon Konvex oder Konkav ist.
		@return Gibt true zur�ck, wenn das Polygon Konvex ist, ansonsten false.
	*/
	bool ComputeIsConvex() const;

	/**
		@brief Berechnet das Kreuzprodukt dreier Vertecies.
		@param V1 das erste Vertex
		@param V2 des zweite Vertex
		@param V3 das dritte Vertex
		@return Gibt das Kreuzprodukt der drei Vertecies zur�ck.
		@todo Diese Methode sollte an geeigneter Stelle in die BS_Vertex Klasse integriert werden.
	*/
	int CrossProduct(const BS_Vertex& V1, const BS_Vertex& V2, const BS_Vertex& V3) const;

	/**
		@brief Berechnet des Skalarprodukt der beiden von drei Vertecies aufgespannten Vektoren.

		Die Vektoren werden von V2 -> V1 und V2 -> V3 aufgespannt.
		
		@param V1 das erste Vertex
		@param V2 des zweite Vertex
		@param V3 das dritte Vertex
		@return Gibt das Skalarprodukt der drei Vertecies zur�ck.
		@todo Diese Methode sollte an geeigneter Stelle in die BS_Vertex Klasse integriert werden.
	*/
	int DotProduct(const BS_Vertex& V1, const BS_Vertex& V2, const BS_Vertex& V3) const;

	/**
		@brief �berpr�ft ob das Polygon selbst�berschneidend ist.
		@return Gibt true zur�ck, wenn das Polygon selbst�berschneidend ist.<br>
				Gibt false zur�ck, wenn das Polygon nicht selbst�berschneidend ist.
	*/
	bool CheckForSelfIntersection() const;

	/**
		@brief Findet das Vertex des Polygons das am weitesten rechts unten liegt und gibt dessen Index im Vertex-Array zur�ck.
		@return Gibt den Index des Vertex zur�ck das am weiteesten rechts unten liegt.<br>
				Gibt -1 zur�ck, wenn die Vertexliste leer ist.
	*/
	int FindLRVertexIndex() const;

	bool IsLineInCone(int StartVertexIndex, const BS_Vertex & EndVertex, bool IncludeEdges) const;
};

#endif
