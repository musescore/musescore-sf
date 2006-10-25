//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: staff.cpp,v 1.11 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include "staff.h"
#include "part.h"
#include "clef.h"
#include "xml.h"
#include "score.h"
#include "bracket.h"

//---------------------------------------------------------
//   isTopSplit
//---------------------------------------------------------

bool Staff::isTopSplit() const
      {
      return _part->nstaves() > 1 && isTop();
      }

//---------------------------------------------------------
//   trackName
//---------------------------------------------------------

QString Staff::trackName() const
      {
      return _part->trackName();
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

QString Staff::longName() const
      {
      return _part->longName();
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

QString Staff::shortName() const
      {
      return _part->shortName();
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int Staff::midiChannel() const
      {
      return _part->midiChannel();
      }

//---------------------------------------------------------
//   midiProgram
//---------------------------------------------------------

int Staff::midiProgram() const
      {
      return _part->midiProgram();
      }

//---------------------------------------------------------
//   volume
//---------------------------------------------------------

int Staff::volume() const
      {
      return _part->volume();
      }

//---------------------------------------------------------
//   reverb
//---------------------------------------------------------

int Staff::reverb() const
      {
      return _part->reverb();
      }

//---------------------------------------------------------
//   chorus
//---------------------------------------------------------

int Staff::chorus() const
      {
      return _part->chorus();
      }

//---------------------------------------------------------
//   Staff
//---------------------------------------------------------

Staff::Staff(Score* s, Part* p, int rs)
      {
      _score  = s;
      _rstaff = rs;
      _part   = p;
      _clef   = new ClefList;
      _bracket = NO_BRACKET;
      _bracketSpan = 0;
      }

//---------------------------------------------------------
//   ~Staff
//---------------------------------------------------------

Staff::~Staff()
      {
      delete _clef;
      }

//---------------------------------------------------------
//   Staff::key
//---------------------------------------------------------

int Staff::key(int tick) const
      {
      return _clef->clef(tick);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Staff::write(Xml& xml) const
      {
      xml.stag("Staff");
      _clef->write(xml, "cleflist");
      if (_bracket != NO_BRACKET) {
            xml.tagE("bracket type=\"%d\" span=\"%d\"", _bracket, _bracketSpan);
            }
      xml.etag("Staff");
      }

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int Staff::idx() const
      {
      return _score->staves()->idx(this);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Staff::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            if (tag == "cleflist") {
                  _clef->read(node);
                  }
            else if (tag == "bracket") {
                  _bracket = e.attribute("type", "-1").toInt();
                  _bracketSpan = e.attribute("span", "0").toInt();
                  }
            else
                  printf("Mscore:Staff: unknown tag %s\n", tag.toLatin1().data());
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void StaffList::remove(Staff* p)
      {
      for (iStaff i = begin(); i != end(); ++i) {
            if (*i == p) {
                  erase(i);
                  return;
                  }
            }
      printf("StaffList::remove(%p): not found\n", p);
      }

//---------------------------------------------------------
//   idx
//---------------------------------------------------------

int StaffList::idx(const Staff* p) const
      {
      int idx = 0;
      for (ciStaff i = begin(); i != end(); ++i, ++idx) {
            if (*i == p)
                  return idx;
            }
      printf("StaffList::idx(%p): not found\n", p);
      return -1;
      }

