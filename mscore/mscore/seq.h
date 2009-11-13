//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __SEQ_H__
#define __SEQ_H__

#include "event.h"
#include "driver.h"
#include "fifo.h"
#include "al/tempo.h"

class Note;
class QTimer;
class Score;
class Painter;
class Measure;
class Driver;
class Part;
struct Channel;

//---------------------------------------------------------
//   SeqMsg
//    message format for gui -> sequencer messages
//---------------------------------------------------------

enum { SEQ_NO_MESSAGE, SEQ_TEMPO_CHANGE, SEQ_PLAY, SEQ_SEEK };

struct SeqMsg {
      int id;
      int data;
      Event event;
      };

//---------------------------------------------------------
//   SeqMsgFifo
//---------------------------------------------------------

static const int SEQ_MSG_FIFO_SIZE = 256;

class SeqMsgFifo : public FifoBase {
      SeqMsg messages[SEQ_MSG_FIFO_SIZE];

   public:
      SeqMsgFifo();
      virtual ~SeqMsgFifo()     {}
      void enqueue(const SeqMsg&);        // put object on fifo
      SeqMsg dequeue();                   // remove object from fifo
      };

//---------------------------------------------------------
//   Seq
//    sequencer
//---------------------------------------------------------

class Seq : public QObject {
      Q_OBJECT

      Score* cs;
      bool running;                       // true if sequencer is available
      int state;                          // STOP, PLAY, START_PLAY
      bool playlistChanged;

      SeqMsgFifo toSeq;

      Driver* driver;

      double meterValue[2];
      double meterPeakValue[2];
      int peakTimer[2];

      EventMap events;                    // playlist

      QList<const Event*> activeNotes;    // notes sounding
      double playTime;
      double startTime;

      EventMap::const_iterator playPos;   // moved in real time thread
      EventMap::const_iterator guiPos;    // moved in gui thread
      QList<Note*> markedNotes;           // notes marked as sounding

      int endTick;
      int curTick;
      int curUtick;

      QTimer* heartBeatTimer;
      QTimer* noteTimer;

      QList<Event*> eventList;


      void collectMeasureEvents(Measure*, int staffIdx);

      void stopTransport();
      void startTransport();
      void setPos(int);
      void playEvent(const Event*, unsigned framePos);
      void guiToSeq(const SeqMsg& msg);
      void startNote(Channel*, int, int, double nt);

   private slots:
      void seqMessage(int msg);
      void heartBeat();
      void selectionChanged(int);
      void midiInputReady();

   public slots:
      void setRelTempo(double);
      void setMasterVolume(float);
      void seek(int);
      void stopNotes();

   signals:
      void started();
      void stopped();
      int toGui(int);
      void masterVolumeChanged(float);

   public:
      enum { STOP, PLAY, START_PLAY };

      Seq();
      ~Seq();
      void start();
      void stop();
      void rewindStart();
      void seekEnd();
      void nextMeasure();
      void nextChord();
      void prevMeasure();
      void prevChord();

      void collectEvents();
      void guiStop();

      bool init();
      void exit();
      bool isRunning() const    { return running; }
      bool isPlaying() const    { return state == PLAY; }
      bool isStopped() const    { return state == STOP; }

      void processMessages();
      void process(unsigned, float*, float*, int stride);
      QList<QString> inputPorts();
      int getEndTick() const    { return endTick;  }
      bool isRealtime() const   { return true;     }
      void sendMessage(SeqMsg&) const;
      void startNote(Channel*, int, int, int, double nt);
      void setController(int, int, int);
      void sendEvent(const Event&);
      void setScore(Score* s);
      Score* score() const { return cs; }
      void initInstruments();

      const QList<MidiPatch*>& getPatchInfo() const;
      Driver* getDriver()  { return driver; }
      int getCurTime();
      int getCurTick();
      void getCurTick(int*, int*);
      float masterVolume() const;
      };

extern Seq* seq;
extern void initSequencer();
extern bool initMidi();
#endif

