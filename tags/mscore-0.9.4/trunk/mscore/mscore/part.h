//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: part.h,v 1.8 2006/03/13 21:35:59 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __PART_H__
#define __PART_H__

#include "globals.h"
#include "instrument.h"

class Xml;
class Staff;
class Score;
class Drumset;
class InstrumentTemplate;
class TextBase;

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

class Part {
      Score* _score;
      QString _trackName;           ///< used in tracklist

      TextBase* _longNameBase;
      TextBase* _shortNameBase;

      Instrument _instrument;
      QList<Staff*> _staves;
      QString _id;                  ///< used for MusicXml import
      bool _show;                   ///< show part in partitur if true

   public:
      Part(Score*);
      ~Part();
      void initFromInstrTemplate(const InstrumentTemplate*);

      void read(QDomElement);
      void write(Xml& xml) const;
      int nstaves() const                      { return _staves.size(); }
      QList<Staff*>* staves()                  { return &_staves; }
      const QList<Staff*>* staves() const      { return &_staves; }
      Staff* staff(int idx) const;
      void setId(const QString& s)             { _id = s; }
      QString id() const                       { return _id; }
      QString trackName() const                { return _trackName;  }
      void setTrackName(const QString& s)      { _trackName = s; }

      QString shortName() const;
      QString longName()  const;
      QString shortNameHtml() const;
      QString longNameHtml()  const;

      TextBase** longNameBase()                { return &_longNameBase; }
      TextBase** shortNameBase()               { return &_shortNameBase; }
      void setLongName(const QString& s);
      void setShortName(const QString& s);
      void setLongNameHtml(const QString& s);
      void setShortNameHtml(const QString& s);
      void setLongName(const QTextDocument& s);
      void setShortName(const QTextDocument& s);

      void setStaves(int);

      void setMinPitch(int val)                { _instrument.minPitch = val;     }
      void setMaxPitch(int val)                { _instrument.maxPitch = val;     }

      void setDrumset(Drumset* ds)             { _instrument.drumset = ds;       }
      Drumset* drumset() const                 { return _instrument.drumset;     }
      bool useDrumset() const                  { return _instrument.useDrumset;  }
      void setUseDrumset(bool val);

      int minPitch() const                     { return _instrument.minPitch;    }
      int maxPitch() const                     { return _instrument.maxPitch;    }

      int volume() const;
      int reverb() const;
      int chorus() const;
      int pan() const;
      int midiProgram() const;
      void setMidiProgram(int);

      int midiChannel() const;
      void setMidiChannel(int);

      void setPitchOffset(int val)             { _instrument.pitchOffset = val;  }
      int pitchOffset() const                  { return _instrument.pitchOffset; }
      void insertStaff(Staff*);
      void removeStaff(Staff*);
      const Instrument* instrument() const     { return &_instrument; }
      Instrument* instrument()                 { return &_instrument; }
      void setInstrument(const Instrument& i)  { _instrument = i;     }
      bool show() const                        { return _show;        }
      void setShow(bool val);
      Score* score() const                     { return _score; }
      };

#endif
