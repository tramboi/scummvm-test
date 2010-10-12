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
	BS_SoundEngine
	-------------
	Dies ist das Soundengine Interface, dass alle Methoden enth�lt, die eine Soundengine
	implementieren muss.
	Implementationen der Soundengine m�ssen von dieser Klasse abgeleitet werden.
	Es gilt zu beachten, dass eine Soundengine eine Liste mit ALLEN geladenen Samplen enthalten
	muss, und dass diese Samples beim Aufruf des Destruktors freigegeben werden.

	Autor: Malte Thiesen
*/

#ifndef BS_SOUNDENGINE_H
#define BS_SOUNDENGINE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "kernel/common.h"
#include "kernel/resservice.h"
#include "kernel/persistable.h"

// -----------------------------------------------------------------------------
// Klassendefinition
// -----------------------------------------------------------------------------

class BS_SoundEngine : public BS_ResourceService, public BS_Persistable
{
public:
	// -----------------------------------------------------------------------------
	// Enums und Typen
	// -----------------------------------------------------------------------------
	
	enum SOUND_TYPES
	{
		MUSIC = 0,
		SPEECH = 1,
		SFX = 2
	};

	/**
		@brief Die Callbackfunktion von PlayDynamicSoundEx
		@param UserData Benutzerspezifizierter Pointer
		@param Data Pointer auf den zu beschreibenden Puffer
		@param DataLength L�nge der zu schreibenden Daten in Byte
	*/
	typedef void (*DynamicSoundReadCallback)(void * UserData, void * Data, unsigned int DataLength);

	// -----------------------------------------------------------------------------
	// Konstruktion / Destruktion
	// -----------------------------------------------------------------------------
	
	BS_SoundEngine(BS_Kernel* pKernel);
	virtual ~BS_SoundEngine() {};

	// --------------------------------------------------------------
	// DIESE METHODEN M�SSEN VON DER SOUNDENGINE IMPLEMENTIERT WERDEN
	// --------------------------------------------------------------

	/** 
		@brief Initialisiert die Sound-Engine
		@param SampleRate Gibt die zu nutzende SampleRate an
		@param Channels die maximale Anzahl der Kan�le.<br>
						Der Standardwert ist 32.
		@return Gibt bei Erfolg TRUE, ansonsten FALSE zur�ck
		@remark Aufrufe an allen anderen Methoden d�rfen erst erfolgen, wenn diese Methode erfolgreich aufgerufen wurde.
	*/
	virtual bool Init(unsigned int SampleRate, unsigned int Channels = 32) = 0;

	/**
		@brief F�hrt einen "Tick" der Sound-Engine aus

		Diese Methode sollte ein mal pro Frame aufgerufen werden. Sie dient dazu Implementationen der Sound-Engine zu erm�glichen,
		die nicht in einem eigenen Thread laufen oder zus�tzliche Verwaltungsaufgaben durchf�hren m�ssen.
	*/
	virtual void Update() = 0;

	/** 
		@brief Setzt die Standardlautst�rke f�r die verschiedenen Soundtypen
		@param Volume die Standardlautst�rke (0 = aus, 1 = volle Lautst�rke)
		@param Type der Soundtyp dessen Lautst�rke ge�ndert werden soll
	*/
	virtual void SetVolume(float Volume, SOUND_TYPES Type) = 0;

	/**
		@brief Gibt die Standardlautst�rke der verschiedenen Soundtypen
		@param Type der Soundtyp
		@return Gibt die Standardlautst�rke des �bergebenen Soundtyps zur�ck (0 = aus, 1 = volle Laust�rke)
	*/
	virtual float GetVolume(SOUND_TYPES Type) = 0;

	/**
		@brief Pausiert alle Sounds die gerade spielen
	*/
	virtual void PauseAll() = 0;

	/**
		@brief Setzt alle gestoppten Sounds fort
	*/
	virtual void ResumeAll() = 0;

	/**
	    @brief Pausiert alle Sounds eines bestimmten Sound-Layers
		@param Layer ein Soundlayer
	*/
	virtual void PauseLayer(unsigned int Layer) = 0;

	/**
	    @brief Setzt alle Sounds eines Layers fort, die mit PauseLayer() zuvor gestoppt wurden
		@param Layer ein Soundlayer
	*/
	virtual void ResumeLayer(unsigned int Layer) = 0;
	

	/**
		@brief Spielt einen Sound ab
		@param FileName der Dateiname des abzuspielenden Sounds
		@param Type der Typ des Sounds
		@param Volume die Lautst�rke mit der der Soundabgespielt werden soll (0 = aus, 1 = volle Lautst�rke)
		@param Pan das Panning (-1 = ganz links, 1 = ganz rechts)
		@param Loop gibt an ob der Sound geloopt werden soll
		@param LoopStart gibt den Start-Looppoint an. Wenn ein Wert kleiner als 0 �bergeben wird, wird der Start des Sounds benutzt.
		@param LoopEnd gibt den End-Looppoint an. Wenn ein Wert kleiner als 0 �bergeben wird, wird das Ende des Sounds benutzt.
		@param Layer der Soundlayer
		@return Gibt true zur�ck, wenn das Abspielen des Sounds eingeleitet werden konnte.
		@remark Falls eine erweiterte Kontrolle �ber das Abspielen ben�tigt wird, z.B. das �ndern der Soundparameter
				Volume und Panning w�hrend des Abspielvorgangens, sollte PlaySoundEx benutzt werden.
	*/
	virtual bool PlaySound(const std::string& FileName, SOUND_TYPES Type, float Volume = 1.0f, float Pan = 0.0f, bool Loop = false, int LoopStart = -1, int LoopEnd = -1, unsigned int Layer = 0) = 0;

	/**
		@brief Spielt einen Sound ab
		@param FileName der Dateiname des abzuspielenden Sounds
		@param Type der Typ des Sounds
		@param Volume die Lautst�rke mit der der Soundabgespielt werden soll (0 = aus, 1 = volle Lautst�rke)
		@param Pan das Panning (-1 = ganz links, 1 = ganz rechts)
		@param Loop gibt an ob der Sound geloopt werden soll
		@param LoopStart gibt den Start-Looppoint an. Wenn ein Wert kleiner als 0 �bergeben wird, wird der Start des Sounds benutzt.
		@param LoopEnd gibt den End-Looppoint an. Wenn ein Wert kleiner als 0 �bergeben wird, wird das Ende des Sounds benutzt.
		@param Layer der Soundlayer
		@return Gibt ein Handle auf den Sounds zur�ck. Mit diesem Handle kann der Sound w�hrend des Abspielens manipuliert werden.
		@remark Falls eine erweiterte Kontrolle �ber das Abspielen ben�tigt wird, z.B. das �ndern der Soundparameter
				Volume und Panning w�hrend des Abspielvorgangens, sollte PlaySoundEx benutzt werden.
	*/
	virtual unsigned int PlaySoundEx(const std::string& FileName, SOUND_TYPES Type, float Volume = 1.0f, float Pan = 0.0f, bool Loop = false, int LoopStart = -1, int LoopEnd = -1, unsigned int Layer = 0) = 0;

	/**
		@brief Spielt einen zur Laufzeit generierten Sound ab
		@param ReadCallback ein Pointer auf eine Callbackfunktion, die aufgerufen wird, wenn Sounddaten ben�tigt werden.<br>
							N�here Details zu dieser Funktion gibt es bei der Dokumentation von DynamicSoundReadCallback.
		@param UserData ein Pointer auf benutzerdefinierte Daten. Diese werden der Callback-Funktion bei jedem Aufruf �bergeben.<br>
						Falls keine solche Daten ben�tigt werden, kann dieser Parameter auf 0 gesetzt werden.
		@param Type der Typ des Sounds
		@param SampleRate die Sample-Rate des Sounds
		@param BitsPerSample die Gr��e eines Samples in Bits. Hiermit sind tats�chlich nur einzelne Sample gemeint, diese Angabe ist unabh�ngig von der Anzahl der Kan�le.<br>
							 Erlaubte Werte sind 8, 16, 24 und 32.
		@param Channels die Anzahl der Kan�le.<br>
						Erlaubte Werte sind 1 und 2.
		@param Volume die Lautst�rke mit der der Soundabgespielt werden soll (0 = aus, 1 = volle Lautst�rke)
		@param Pan das Panning (-1 = ganz links, 1 = ganz rechts)
		@param Layer der Soundlayer
		@return Gibt ein Handle auf den Sounds zur�ck. Mit diesem Handle kann der Sound w�hrend des Abspielens manipuliert werden.
		@remark Dynamische Sounds k�nnen nicht persistiert werden.
	*/
	virtual unsigned int PlayDynamicSoundEx(DynamicSoundReadCallback ReadCallback, void * UserData, SOUND_TYPES Type, unsigned int SampleRate, unsigned int BitsPerSample, unsigned int Channels, float Volume = 1.0f, float Pan = 0.0f, unsigned int Layer = 0) = 0;

	/**
		@brief Setzt die Lautst�rke eines spielenden Sounds
		@param Handle das Handle des Sounds
		@param Volume die Lautst�rke mit der der Soundabgespielt werden soll (0 = aus, 1 = volle Lautst�rke)
	*/
	virtual void SetSoundVolume(unsigned int Handle, float Volume) = 0;

	/**
		@brief Setzt das Panning eines spielenden Sounds
		@param Handle das Handle des Sounds
		@param Pan das Panning (-1 = ganz links, 1 = ganz rechts)
	*/
	virtual void SetSoundPanning(unsigned int Handle, float Pan) = 0;

	/**
		@brief Pausiert einen Sound
		@param Handle das Handle des Sounds
	*/
	virtual void PauseSound(unsigned int Handle) = 0;

	/**
		@brief Setzt einen Sound fort
		@param Handle das Handle des Sounds
	*/
	virtual void ResumeSound(unsigned int Handle) = 0;

	/**
		@brief Stoppt einen Sound
		@param Handle das Handle des Sounds
		@remark Nach einem Aufruf dieser Methode ist das Handle ung�ltig und darf nicht mehr benutzt werden.
	*/
	virtual void StopSound(unsigned int Handle) = 0;

	/**
	    @brief Gibt zur�ck, ob ein Sound pausiert ist
		@param Handle das Handle des Sounds
		@return Gibt true zur�ck, wenn der Sound pausiert ist, ansonsten false.
	*/
	virtual bool IsSoundPaused(unsigned int Handle) = 0;
	
	/**
		@brief Gibt zur�ck, ob ein Sound noch spielt
		@param Handle das Handle des Sounds
		@return Gibt true zur�ck, wenn der Sound noch spielt, ansonsten false.
	*/
	virtual bool IsSoundPlaying(unsigned int Handle) = 0;

	/**
		@brief Gibt die Lautst�rke eines spielenden Sounds zur�ck (0 = aus, 1 = volle Lautst�rke)
	*/
	virtual float GetSoundVolume(unsigned int Handle) = 0;

	/**
		@brief Gibt das Panning eines spielenden Sounds zur�ck (-1 = ganz links, 1 = ganz rechts)
	*/
	virtual float GetSoundPanning(unsigned int Handle) = 0;

	/**
		@brief Gibt die Position innerhalb des abgespielten Sounds in Sekunden zur�ck
	*/
	virtual float GetSoundTime(unsigned int Handle) = 0;

private:
	bool _RegisterScriptBindings();
};

#endif
