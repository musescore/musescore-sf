//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __NAVIGATE_H__
#define __NAVIGATE_H__

class ChordRest;

extern int pitch2y(int pitch, int enh, int clefOffset, int key, int& prefix, const char* tversatz);
extern ChordRest* nextChordRest(ChordRest* cr);
extern ChordRest* prevChordRest(ChordRest* cr);

#endif

