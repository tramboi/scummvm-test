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

/*
	BS_Line
	-------
	Diese Klasse enth�lt nur statische Methoden, die mit Geradensegmenten zu tun haben.
    Es gibt keine wirkliche Geradensegment-Klasse, da diese Klasse vor allem zu
	Berechnungen mit Polygonen herangezogen wird und es dabei wichtig ist, Start- und
	Endpunkte der Linien dynamisch w�hlen zu k�nnen. Dieses w�rde sich verbieten, wenn
	ein Polygon aus einer Menge von festen Geradensegmenten gebildet w�re.

	Autor: Malte Thiesen
*/

#ifndef SWORD25_LINE_H
#define SWORD25_LINE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"

// -----------------------------------------------------------------------------

class BS_Line
{
public:
	/**
	    @brief Bestimmt ob sich ein Punkt links von einer Linie befindet.
		@param a der Startpunkt der	Linie
		@param b der Endpunkt der Linie
		@param c der Testpunkt
		@return Gibt true zur�ck, wenn sich der Punkt links von der Linie befindet.<br>
				Falls sich der Punkt rechts von der Linie oder auf der Linie befindet wird false zur�ckgegeben.<br>
		@remark Ein Punkt liegt links von einer Linie, wenn er vom Startpunkt aus betrachtet links neben der Linie liegt.
	*/
	static bool IsVertexLeft(const BS_Vertex & a, const BS_Vertex & b, const BS_Vertex & c)
	{
		return _TriangleArea2(a, b, c) > 0;
	}

	static bool IsVertexLeftOn(const BS_Vertex & a, const BS_Vertex & b, const BS_Vertex & c)
	{
		return _TriangleArea2(a, b, c) >= 0;
	}

	/**
		@brief Bestimmt ob sich ein Punkt rechts von einer Linie befindet.
		@param a der Startpunkt der	Linie
		@param b der Endpunkt der Linie
		@param c der Testpunkt
		@return Gibt true zur�ck, wenn sich der Punkt recht von der Linie befindet.<br>
		Falls sich der Punkt links von der Linie oder auf der Linie befindet wird false zur�ckgegeben.<br>
		@remark Ein Punkt liegt rechts von einer Linie, wenn er vom Startpunkt aus betrachtet rechts neben der Linie liegt.
	*/
	static bool IsVertexRight(const BS_Vertex & a, const BS_Vertex & b, const BS_Vertex & c)
	{
		return _TriangleArea2(a, b, c) < 0;
	}

	static bool IsVertexRightOn(const BS_Vertex & a, const BS_Vertex & b, const BS_Vertex & c)
	{
		return _TriangleArea2(a, b, c) <= 0;
	}

	/**
		@brief Bestimmt ob sich ein Punkt auf einer Linie befindet.
		@param a der Startpunkt der	Linie
		@param b der Endpunkt der Linie
		@param c der Testpunkt
		@return Gibt true zur�ck, wenn sich der Punkt auf der Linie befindet.
	*/
	static bool IsVertexOn(const BS_Vertex & a, const BS_Vertex & b, const BS_Vertex & c)
	{
		return _TriangleArea2(a, b, c) == 0;
	}

	enum VERTEX_CLASSIFICATION
	{
		LEFT,
		RIGHT,
		ON,
	};

	/**
	    @brief Bestimmt wo sich ein Punkt relativ zu einer Linie befindet.
		@param a der Startpunkt der	Linie
		@param b der Endpunkt der Linie
		@param c der Testpunkt
		@return Gibt LEFT zur�ck, wenn sich der Punkt links von der Line befindet.<br>
				Gibt RIGHT zur�ck, wenn sich der Punkt links von der Line befindet.<br>
				Gibt ON zur�ck, wenn sich der Punkt auf der Linie befindet.
	*/
	static VERTEX_CLASSIFICATION ClassifyVertexToLine(const BS_Vertex & a, const BS_Vertex & b, const BS_Vertex & c)
	{
		int Area = _TriangleArea2(a, b, c);
		if (Area > 0) return LEFT;
		if (Area < 0) return RIGHT;
		return ON;
	}

	/**
	    @brief Bestimmt ob sich zwei Linien schneiden.
		@param a der Startpunkt der ersten Linie
		@param b der Endpunkt der ersten Linie
		@param c der Startpunkt der zweiten Linie
		@param d der Endpunkt der zweiten Linie
		@remark In den F�llen in denen eine Linie die andere nur ber�hrt, wird false zur�ckgegeben (improper intersection).
	*/
	static bool DoesIntersectProperly(const BS_Vertex & a, const BS_Vertex & b, const BS_Vertex & c, const BS_Vertex & d)
	{
		VERTEX_CLASSIFICATION Class1 = ClassifyVertexToLine(a, b, c);
		VERTEX_CLASSIFICATION Class2 = ClassifyVertexToLine(a, b, d);
		VERTEX_CLASSIFICATION Class3 = ClassifyVertexToLine(c, d, a);
		VERTEX_CLASSIFICATION Class4 = ClassifyVertexToLine(c, d, b);

		if (Class1 == ON || Class2 == ON || Class3 == ON || Class4 == ON) return false;

		return ((Class1 == LEFT) ^ (Class2 == LEFT)) && ((Class3 == LEFT) ^ (Class4 == LEFT));
	}

	/**
	    @brief Bestimmt ob sich ein Punkt auf einem Liniensegment befindet
		@param a der Startpunkt der	Liniensegmentes
		@param b der Endpunkt der Liniensegmentes
		@param c der Testpunkt
	*/
	static bool IsOnLine(const BS_Vertex & a, const BS_Vertex & b, const BS_Vertex & c)
	{
		// Die Punkte m�ssen alle Kollinear sein, sonst liegt der Testpunkt nicht auf dem Liniensegment
		if (_TriangleArea2(a, b, c) != 0) return false;

		// Falls das Liniensegment nicht vertikal ist pr�fe auf der X-Achse, ansonsten auf der Y-Achse
		if (a.X != b.X)
		{
			return ((a.X <= c.X) &&
					(c.X <= b.X)) ||
				   ((a.X >= c.X) &&
					(c.X >= b.X));
		}
		else
		{
			return ((a.Y <= c.Y) &&
					(c.Y <= b.Y)) ||
				   ((a.Y >= c.Y) &&
					(c.Y >= b.Y));
		}
	}

	static bool IsOnLineStrict(const BS_Vertex & a, const BS_Vertex & b, const BS_Vertex & c)
	{
		// Die Punkte m�ssen alle Kollinear sein, sonst liegt der Testpunkt nicht auf dem Liniensegment
		if (_TriangleArea2(a, b, c) != 0) return false;

		// Falls das Liniensegment nicht vertikal ist pr�fe auf der X-Achse, ansonsten auf der Y-Achse
		if (a.X != b.X)
		{
			return ((a.X < c.X) &&
				(c.X < b.X)) ||
				((a.X > c.X) &&
				(c.X > b.X));
		}
		else
		{
			return ((a.Y < c.Y) &&
				(c.Y < b.Y)) ||
				((a.Y > c.Y) &&
				(c.Y > b.Y));
		}
	}

private:

	/**
	    @brief Gibt die doppelte Gr��e des durch a, b und c definierten Dreiecks zur�ck.

		Das Ergebnis ist positiv wenn die Punkte entgegen dem Uhrzeigersinn angeordnet sind und negativ wenn sie mit dem
		Uhrzeigersinn angeordnet sind.
	*/
	static int _TriangleArea2(const BS_Vertex & a, const BS_Vertex & b, const BS_Vertex & c)
	{
		return a.X * b.Y - a.Y * b.X +
			   a.Y * c.X - a.X * c.Y +
			   b.X * c.Y - c.X * b.Y;
	}
};

#endif
