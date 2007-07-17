//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer (ws@seh.de)
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

#ifndef __DRUMSET_H__
#define __DRUMSET_H__

//---------------------------------------------------------
//   DrumInstrument
//---------------------------------------------------------

struct DrumInstrument {
      int notehead;           ///< notehead symbol set
      int line;               ///< place notehead onto this line
      int voice;
      Direction stemDirection;
      };

//---------------------------------------------------------
//   Drumset
//    defines note heads and line position for all
//    possible midi notes in a drumset
//---------------------------------------------------------

struct Drumset {
      DrumInstrument drum[128];

      bool isValid(int pitch) const      { return drum[pitch].notehead != -1; }
      int noteHead(int pitch) const      { return drum[pitch].notehead;       }
      int line(int pitch) const          { return drum[pitch].line;           }
      int voice(int pitch) const         { return drum[pitch].voice;          }
      Direction stemDirection(int pitch) const { return drum[pitch].stemDirection;  }
      };

extern Drumset* smDrumset;
extern void initDrumset();

#endif

