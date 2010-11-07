//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include "globals.h"
#include "event.h"
#include "interval.h"

class InstrumentTemplate;
class Xml;
class Drumset;
class Tablature;

//---------------------------------------------------------
//   NamedEventList
//---------------------------------------------------------

struct NamedEventList {
      QString name;
      QString descr;
      EventList events;

      void write(Xml&, const QString& name) const;
      void read(QDomElement);
      bool operator==(const NamedEventList& i) const { return i.name == name && i.events == events; }
      };

//---------------------------------------------------------
//   MidiArticulation
//---------------------------------------------------------

struct MidiArticulation {
      QString name;
      QString descr;
      int velocity;           // velocity change: -100% - +100%
      int gateTime;           // gate time change: -100% - +100%
      void write(Xml&) const;
      void read(QDomElement);

      MidiArticulation() {}
      bool operator==(const MidiArticulation& i) const;
      };

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

// this are the indexes of controllers which are always present in
// Channel init EventList (maybe zero)

enum {
      A_HBANK, A_LBANK, A_PROGRAM, A_VOLUME, A_PAN, A_CHORUS, A_REVERB,
      A_INIT_COUNT
      };

struct Channel {
      QString name;
      QString descr;
      int channel;      // mscore channel number, mapped to midi port/channel
      mutable EventList init;

      int synti;
      int program;     // current values as shown in mixer
      int bank;        // initialized from "init"
      char volume;
      char pan;
      char chorus;
      char reverb;

      bool mute;
      bool solo;
      bool soloMute;

      QList<NamedEventList> midiActions;
      QList<MidiArticulation> articulation;

      Channel();
      void write(Xml&) const;
      void read(QDomElement);
      void updateInitList() const;
      bool operator==(const Channel& c) { return (name == c.name) && (channel == c.channel); }
      };

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

class InstrumentData;

class Instrument {
      QSharedDataPointer<InstrumentData> d;

   public:
      Instrument();
      Instrument(const Instrument&);
      ~Instrument();
      Instrument& operator=(const Instrument&);
      bool operator==(const Instrument&) const;

      void read(QDomElement);
      void write(Xml& xml) const;
      NamedEventList* midiAction(const QString& s, int channel) const;
      int channelIdx(const QString& s) const;
      void updateVelocity(int* velocity, int channel, const QString& name);

      int minPitchP() const;
      int maxPitchP() const;
      int minPitchA() const;
      int maxPitchA() const;
      void setMinPitchP(int v);
      void setMaxPitchP(int v);
      void setMinPitchA(int v);
      void setMaxPitchA(int v);
      Interval transpose() const;
      void setTranspose(const Interval& v);

      void setDrumset(Drumset* ds);       // drumset is now owned by Instrument
      Drumset* drumset() const;
      bool useDrumset() const;
      void setUseDrumset(bool val);
      void setAmateurPitchRange(int a, int b);
      void setProfessionalPitchRange(int a, int b);
      Channel& channel(int idx);
      const Channel& channel(int idx) const;

      const QList<NamedEventList>& midiActions() const;
      const QList<MidiArticulation>& articulation() const;
      const QList<Channel>& channel() const;

      void setMidiActions(const QList<NamedEventList>& l);
      void setArticulation(const QList<MidiArticulation>& l);
      void setChannel(const QList<Channel>& l);
      void setChannel(int i, const Channel& c);
      Tablature* tablature() const;
      void setTablature(Tablature* t);
      static Instrument fromTemplate(const InstrumentTemplate*);

      const QTextDocumentFragment& longName() const;
      const QTextDocumentFragment& shortName() const;
      QTextDocumentFragment& longName();
      QTextDocumentFragment& shortName();
      QString trackName() const;
      void setTrackName(const QString&);
      };

//---------------------------------------------------------
//   InstrumentList
//---------------------------------------------------------

typedef std::map<const int, Instrument>::iterator iInstrument;
typedef std::map<const int, Instrument>::const_iterator ciInstrument;

class InstrumentList : public std::map<const int, Instrument> {
      static Instrument defaultInstrument;
   public:
      InstrumentList() {}
      const Instrument& instrument(int tick) const;
      Instrument& instrument(int tick);
      void setInstrument(const Instrument&, int tick);
      };

#endif
