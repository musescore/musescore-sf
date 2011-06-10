//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
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

#ifndef __INTERVAL_H__
#define __INTERVAL_H__

//---------------------------------------------------------
//   Interval
//---------------------------------------------------------

struct Interval {
      char diatonic;
      char chromatic;

      Interval();
      Interval(int a, int b);
      Interval(int _chromatic);
      void flip();
      bool isZero() const;
      };

#endif
