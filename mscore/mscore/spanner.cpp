//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "spanner.h"

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

Spanner::Spanner(Score* s)
   : Element(s)
      {
      _startElement = 0;
      _endElement   = 0;
      _anchor       = ANCHOR_SEGMENT;
//      _tick         = 0;
//      _tick2        = 0;
      }

Spanner::Spanner(const Spanner& s)
   : Element(s)
      {
      _startElement = s._startElement;
      _endElement   = s._endElement;
//      _tick         = s._tick;
//      _tick2        = s._tick2;
      _anchor       = s._anchor;
      }

