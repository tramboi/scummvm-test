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
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_PARSER_H
#define HUGO_PARSER_H
namespace Hugo {

enum seqTextParser {
	kTBExit  = 0, kTBMaze,    kTBNoPoint, kTBNoun,       kTBVerb,
	kTBEh,        kTBUnusual, kTBHave,    kTBNoUse,      kTBDontHave,
	kTBNeed,      kTBOk,      kCmtAny1,   kCmtAny2,      kCmtAny3,
	kCmtClose,    kTBIntro,   kTBOutro,   kTBUnusual_1d, kCmtAny4,
	kCmtAny5,     kTBExit_1d, kTBEh_1d,   kTBEh_2d,      kTBNoUse_2d
};

class Parser {
public:
	Parser(HugoEngine &vm);
	virtual ~Parser();

	bool  isWordPresent(char **wordArr);

	void  charHandler();
	void  command(const char *format, ...);
	void  keyHandler(uint16 nChar, uint16 nFlags);
	virtual void lineHandler() = 0;

protected:
	HugoEngine &_vm;

protected:
	bool  isCarrying(uint16 wordIndex);

	char *findNoun();
	char *findVerb();

	void  dropObject(object_t *obj);
	void  takeObject(object_t *obj);
	void  showTakeables();

private:
	char   _ringBuffer[32];                         // Ring buffer
	uint16 _putIndex;
	uint16 _getIndex;                               // Index into ring buffer
	bool   _checkDoubleF1Fl;                        // Flag used to display user help or instructions

	void  showDosInventory();
};

class Parser_v1w : public Parser {
public:
	Parser_v1w(HugoEngine &vm);
	~Parser_v1w();

	virtual void  lineHandler();

private:
	bool  isBackgroundWord(objectList_t obj);
	bool  isCatchallVerb(objectList_t obj);
	bool  isGenericVerb(object_t *obj, char *comment);
	bool  isNear(object_t *obj, char *verb, char *comment);
	bool  isObjectVerb(object_t *obj, char *comment);
};

class Parser_v1d : public Parser {
public:
	Parser_v1d(HugoEngine &vm);
	~Parser_v1d();

	void lineHandler();

protected:
	char *findNextNoun(char *noun);
	bool isNear(char *verb, char *noun, object_t *obj, char *comment);
	bool isGenericVerb(char *word, object_t *obj);
	bool isObjectVerb(char *word, object_t *obj);
	bool isBackgroundWord(char *noun, char *verb, objectList_t obj);
	bool isCatchallVerb(bool test_noun, char *noun, char *verb, objectList_t obj);

};

class Parser_v2d : public Parser_v1d {
public:
	Parser_v2d(HugoEngine &vm);
	~Parser_v2d();

	void lineHandler();
};
	
} // End of namespace Hugo

#endif //HUGO_PARSER_H
