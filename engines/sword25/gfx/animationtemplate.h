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

#ifndef SWORD25_ANIMATION_TEMPLATE_H
#define SWORD25_ANIMATION_TEMPLATE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "sword25/kernel/common.h"
#include "sword25/kernel/persistable.h"
#include "sword25/gfx/animationdescription.h"

#include "sword25/kernel/memlog_off.h"
#include <vector>
#include "sword25/kernel/memlog_on.h"

// -----------------------------------------------------------------------------
// Forward declarations
// -----------------------------------------------------------------------------

class BS_AnimationResource;

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_AnimationTemplate : public BS_AnimationDescription
{
public:
	static unsigned int Create(const std::string & SourceAnimation);
	static unsigned int Create(const BS_AnimationTemplate & Other);
	static unsigned int Create(BS_InputPersistenceBlock & Reader, unsigned int Handle);
	BS_AnimationTemplate * ResolveHandle(unsigned int Handle) const;

private:
	BS_AnimationTemplate(const std::string & SourceAnimation);
	BS_AnimationTemplate(const BS_AnimationTemplate & Other);
	BS_AnimationTemplate(BS_InputPersistenceBlock & Reader, unsigned int Handle);

public:
	~BS_AnimationTemplate();

	virtual const Frame &	GetFrame(unsigned int Index) const { BS_ASSERT(Index < m_Frames.size()); return m_Frames[Index]; }
	virtual unsigned int	GetFrameCount() const { return m_Frames.size(); }
	virtual void			Unlock() { delete this; }

	bool IsValid() const { return m_Valid; }

	/**
		@brief F�gt einen neuen Frame zur Animation hinzu.

		Der Frame wird an das Ende der Animation angeh�ngt.

		@param Index der Index des Frames in der Quellanimation
	*/
	void AddFrame(int Index);

	/**
		@brief �ndert einen bereits in der Animation vorhandenen Frame.
		@param DestIndex der Index des Frames der �berschrieben werden soll
		@param SrcIndex der Index des einzuf�genden Frames in der Quellanimation
	*/
	void SetFrame(int DestIndex, int SrcIndex);

	/**
		@brief Setzt den Animationstyp.
		@param Type der Typ der Animation. Muss aus den enum BS_Animation::ANIMATION_TYPES sein.
	*/
	void SetAnimationType(BS_Animation::ANIMATION_TYPES Type) { m_AnimationType = Type; }

	/**
		@brief Setzt die Abspielgeschwindigkeit.
		@param FPS die Abspielgeschwindigkeit in Frames pro Sekunde.
	*/
	void SetFPS(int FPS);

	virtual bool Persist(BS_OutputPersistenceBlock & Writer);
	virtual bool Unpersist(BS_InputPersistenceBlock & Reader);

private:
	std::vector<const Frame>	m_Frames;
	BS_AnimationResource *		m_SourceAnimationPtr;
	bool						m_Valid;

	BS_AnimationResource * RequestSourceAnimation(const std::string & SourceAnimation) const;
	bool ValidateSourceIndex(unsigned int Index) const;
	bool ValidateDestIndex(unsigned int Index) const;
};

#endif
