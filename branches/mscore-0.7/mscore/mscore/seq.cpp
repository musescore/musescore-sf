//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: seq.cpp,v 1.46 2006/03/02 17:08:43 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "repeat2.h"

#include "seq.h"
#include "mscore.h"
#include "fluid.h"
#include "jackaudio.h"
#include "alsa.h"
#include "slur.h"
#include "score.h"
#include "segment.h"
#include "note.h"
#include "chord.h"
#include "tempo.h"
#include "canvas.h"
#include "playpanel.h"
#include "staff.h"
#include "measure.h"
#include "layout.h"
#include "preferences.h"
#include "part.h"
#include "ottava.h"

enum {
      ME_NOTEOFF    = 0x80,
      ME_NOTEON     = 0x90,
      ME_POLYAFTER  = 0xa0,
      ME_CONTROLLER = 0xb0,
      ME_PROGRAM    = 0xc0,
      ME_AFTERTOUCH = 0xd0,
      ME_PITCHBEND  = 0xe0,
      ME_SYSEX      = 0xf0,
      ME_META       = 0xff,
      ME_SONGPOS    = 0xf2,
      ME_CLOCK      = 0xf8,
      ME_START      = 0xfa,
      ME_CONTINUE   = 0xfb,
      ME_STOP       = 0xfc,
      };


Seq* seq;

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::Seq()
      {
      running    = false;
      pauseState = false;

      playlistChanged = false;
      cs = 0;

#ifndef __MINGW32__
      endTick  = 0;
      state    = STOP;
      synti    = new ISynth();
      audio    = 0;
      _volume  = 1.0;
      playPos  = events.begin();

      //---------------------------------------------------
      //  pipe for asynchronous gui to sequencer
      //  messages
      //---------------------------------------------------

      int filedes[2];         // 0 - reading   1 - writing
      if (pipe(filedes) == -1) {
            perror("creating pipe0");
            ::exit(-1);
            }
      // pipe for gui -> sequencer
      fromThreadFdr = filedes[0];
      fromThreadFdw = filedes[1];
      int rv = fcntl(fromThreadFdw, F_SETFL, O_NONBLOCK);
      if (rv == -1)
            perror("set pipe write O_NONBLOCK");
      rv = fcntl(fromThreadFdr, F_SETFL, O_NONBLOCK);
      if (rv == -1)
            perror("set pipe read O_NONBLOCK");

      //
      // pipe for asynchronous sequencer to gui messages
      //
      if (pipe(filedes) == -1) {
            perror("creating pipe1");
            ::exit(-1);
            }
      sigFd = filedes[1];
      QSocketNotifier* ss = new QSocketNotifier(filedes[0], QSocketNotifier::Read);
      connect(ss, SIGNAL(activated(int)), this, SLOT(seqMessage(int)));
      heartBeatTimer = new QTimer(this);
      connect(heartBeatTimer, SIGNAL(timeout()), this, SLOT(heartBeat()));
      heartBeatTimer->stop();
#endif
      }

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::~Seq()
      {
//      delete synti;
//      delete audio;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Seq::setScore(Score* s)
      {
#ifndef __MINGW32__
      if (cs) {
            disconnect(cs, SIGNAL(selectionChanged(int)), this, SLOT(selectionChanged(int)));
            stop();
            while (state != STOP)
                  usleep(100000);
            }
      cs = s;
      playlistChanged = true;
      connect(cs, SIGNAL(selectionChanged(int)), SLOT(selectionChanged(int)));
      if (audio)
            seek(0);
#endif
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void Seq::selectionChanged(int mode)
      {
      if (mode != SEL_SINGLE || state != STOP || cs == 0 || audio == 0)
            return;
      int tick = cs->pos();
      if (tick != -1)
            seek(tick);
      }

//---------------------------------------------------------
//   isRealtime
//---------------------------------------------------------

bool Seq::isRealtime() const
      {
      return audio->isRealtime();
      }

//---------------------------------------------------------
//   init
//    return true on error
//---------------------------------------------------------

bool Seq::init()
      {
      audio = 0;
      if (preferences.useJackAudio) {
            audio = new JackAudio;
            if (audio->init()) {
                  printf("no JACK server found\n");
                  delete audio;
                  audio = 0;
                  }
            }
      if (audio == 0 && preferences.useAlsaAudio) {
            audio = new AlsaAudio;
            if (audio->init()) {
                  printf("no ALSA audio found\n");
                  delete audio;
                  audio = 0;
                  }
            }
      if (audio == 0)
            return true;
      if (synti->init(audio->sampleRate())) {
            printf("Synti init failed\n");
            return true;
            }
      audio->start();
      running = true;
      return false;
      }

//---------------------------------------------------------
//   exit
//---------------------------------------------------------

void Seq::exit()
      {
      if (audio)
            audio->stop();
      }

//---------------------------------------------------------
//   sampleRate
//---------------------------------------------------------

int Seq::sampleRate() const
      {
      return audio->sampleRate();
      }

//---------------------------------------------------------
//   frame2tick
//---------------------------------------------------------

int Seq::frame2tick(int frame) const
      {
      return cs->tempomap->time2tick(double(frame) / sampleRate());
      }

//---------------------------------------------------------
//   tick2frame
//---------------------------------------------------------

int Seq::tick2frame(int tick) const
      {
      return int(cs->tempomap->tick2time(tick) * sampleRate());
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

std::list<QString> Seq::inputPorts()
      {
      return audio->inputPorts();
      }

//---------------------------------------------------------
//   loadSoundFont
//---------------------------------------------------------

bool Seq::loadSoundFont(const QString& s)
      {
      return synti->loadSoundFont(s);
      }

//---------------------------------------------------------
//   rewindStart
//---------------------------------------------------------

void Seq::rewindStart()
      {
      seek(0);
      }

//---------------------------------------------------------
//   start
//    called from gui thread
//---------------------------------------------------------

void Seq::start()
      {
      if (!audio)
            return;
      QAction* a = getAction("play");
      if (!a->isChecked()) {
            if (pauseState) {
                  guiStop();
                  QAction* a = getAction("pause");
                  a->setChecked(false);
                  pauseState = false;
                  state = STOP;
                  }
            else {
                  audio->stopTransport();
                  }
            }
      else {
            if (events.empty() || cs->playlistDirty() || playlistChanged)
                  collectEvents();
            seek(cs->playPos());
            heartBeatTimer->start(100);
            if (!pauseState)
                  audio->startTransport();
            else
                  emit started();
            }
      }

//---------------------------------------------------------
//   stop
//    called from gui thread
//---------------------------------------------------------

void Seq::stop()
      {
      if (!audio)
            return;
      audio->stopTransport();
      }

//---------------------------------------------------------
//   pause
//    called from gui thread
//---------------------------------------------------------

void Seq::pause()
      {
      if (!audio)
            return;
      QAction* a = getAction("pause");
      int pstate = a->isChecked();
      a = getAction("play");
      int playState = a->isChecked();
      if (state == PLAY && pstate)  {
            pauseState = pstate;
            audio->stopTransport();
            }
      else if (state == STOP && pauseState && playState) {
            audio->startTransport();
            }
      pauseState = pstate;
      }

//---------------------------------------------------------
//   seqStarted
//---------------------------------------------------------

void MuseScore::seqStarted()
      {
      setState(STATE_PLAY);
      foreach(Viewer* v, cs->getViewer())
            v->setCursorOn(true);
      }

//---------------------------------------------------------
//   seqStopped
//    JACK has stopped
//    executed in gui environment
//---------------------------------------------------------

void MuseScore::seqStopped()
      {
      setState(STATE_NORMAL);
      foreach(Viewer* v, cs->getViewer())
            v->setCursorOn(false);
      cs->start();
      cs->setLayoutAll(false);
      cs->setUpdateAll();
      cs->end();
      }

//---------------------------------------------------------
//   guiStop
//---------------------------------------------------------

void Seq::guiStop()
      {
      if (!pauseState) {
            heartBeatTimer->stop();
            QAction* a = getAction("play");
            a->setChecked(false);
            }

      // code from heart beat:
      Note* note = 0;
      for (ciEvent i = guiPos; i != playPos; ++i) {
            if (i->second.type == ME_NOTEON) {
                  i->second.note->setSelected(i->second.val2 != 0);
                  cs->addRefresh(i->second.note->abbox());
                  if (i->second.val2)
                        note = i->second.note;
                  }
            }
      if (note == 0) {
            for (ciEvent i = playPos; i != events.end(); ++i) {
                  if (i->second.type == ME_NOTEON) {
                        note = i->second.note;
                        break;
                        }
                  }
            }
      if (note) {
            cs->select(note, 0, 0);
            cs->setPlayPos(note->chord()->tick());
            }
      emit stopped();
      }

//---------------------------------------------------------
//   seqSignal
//    sequencer message to GUI
//    execution environment: gui thread
//---------------------------------------------------------

void Seq::seqMessage(int fd)
      {
      char buffer[16];

      int n = ::read(fd, buffer, 16);
      if (n < 0) {
            printf("MScore::Seq: seqMessage(): READ PIPE failed: %s\n",
               strerror(errno));
            return;
            }
      for (int i = 0; i < n; ++i) {
            switch(buffer[i]) {
                  case '0':         // STOP
                        guiStop();
                        break;

                  case '1':         // PLAY
                        emit started();
                        break;

                  default:
                        printf("MScore::Seq:: unknown seq msg %d\n", buffer[i]);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   stopTransport
//    JACK has stopped
//    executed in realtime environment
//---------------------------------------------------------

void Seq::stopTransport()
      {
      // send note off events
      foreach(const Event& e, _activeNotes) {
            synti->playNote(e.channel, e.val1, 0);
            e.note->setSelected(false);
            }
      _activeNotes.clear();
      if (!pauseState)
            toGui('0');
      state = STOP;
      }

//---------------------------------------------------------
//   startTranspor
//    JACK has started
//    executed in realtime environment
//---------------------------------------------------------

void Seq::startTransport()
      {
      // dont start transport, if we have nothing to play
      //
      if (endTick == 0)
            return;
      if (!pauseState)
            toGui('1');
      state = PLAY;
      }

//---------------------------------------------------------
//   playEvent
//    send one event to the synthesizer
//---------------------------------------------------------

void Seq::playEvent(const Event& event)
      {
      int channel = event.channel;
      int type    = event.type;
      if (type == ME_NOTEON) {
            if (event.val2) {         // note on:
                  _activeNotes.append(event);
                  synti->playNote(channel, event.val1, event.val2);
                  }
            else {
                  for (QList<Event>::iterator k = _activeNotes.begin(); k != _activeNotes.end(); ++k) {
                        if (k->channel == channel && k->val1 == event.val1) {
                              _activeNotes.erase(k);
                              synti->playNote(channel, event.val1, 0);
                              break;
                              }
                        }
                  }
            }
      else if (type == 0xb0)
            synti->setController(channel, event.val1, event.val2);
      else {
            printf("bad event type %x\n", type);
            abort();
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Seq::process(unsigned n, float* lbuffer, float* rbuffer)
      {
      int frames = n;
      int jackState = audio->getState();

      if (state == START_PLAY && jackState == PLAY)
            startTransport();
      else if (state == PLAY && jackState == STOP)
            stopTransport();
      else if (state == START_PLAY && jackState == STOP)
            stopTransport();
      else if (state == STOP && jackState == PLAY)
            startTransport();
      else if (state != jackState)
            printf("JACK: state transition %d -> %d ?\n",
               state, jackState);

      float* l = lbuffer;
      float* r = rbuffer;

      //
      // read messages from gui
      //
      SeqMsg msg;
      while (read(fromThreadFdr, &msg, sizeof(SeqMsg)) == sizeof(SeqMsg)) {
            switch(msg.id) {
                  case SEQ_TEMPO_CHANGE:
                        {
                        int tick = frame2tick(playFrame);
                        cs->tempomap->setRelTempo(msg.data1);
                        playFrame = tick2frame(tick);
                        }
                        break;

                  case SEQ_PLAY:
                        {
                        int channel = msg.data1 & 0xf;
                        int type    = msg.data1 & 0xf0;
                        if (type == ME_NOTEON)
                              synti->playNote(channel, msg.data2, msg.data3);
                        else if (type == ME_CONTROLLER)
                              synti->setController(channel, msg.data2, msg.data3);
                        }
                        break;
                  case SEQ_SEEK:
                        setPos(msg.data1);
                        break;

                  default:
                        printf("unknown seq msg %d\n", msg.id);
                        break;
                  }
            }

      if (state == PLAY) {

            //
            // collect events for one segment
            //
            int endFrame = playFrame + frames;
            for (; playPos != events.end(); ++playPos) {
                  int f = tick2frame(playPos->first);
                  if (f >= endFrame)
                        break;

                  int n = f - playFrame;
                  if (n < 0 || n > int(frames)) {
                        printf("Seq: at %d bad n %d(>%d) = %d - %d\n",
                           playPos->first, n, frames, f, playFrame);
                        break;
                        }
                  synti->process(n, l, r);
                  l         += n;
                  r         += n;
                  playFrame += n;
                  frames    -= n;
                  playEvent(playPos->second);
                  }
            if (frames) {
                  synti->process(frames, l, r);
                  playFrame += frames;
                  }
            if (playPos == events.end()) {
                  audio->stopTransport();
                  }
            }
      else
            synti->process(frames, l, r);

      // apply volume:
      for (unsigned i = 0; i < n; ++i) {
            lbuffer[i] *= _volume;
            rbuffer[i] *= _volume;
            }
      }

//---------------------------------------------------------
//   collectEvents
//---------------------------------------------------------

void Seq::collectEvents()
      {
      events.clear();

      int staffIdx = 0;
      int gateTime = 80;  // 100 - legato (100%)
      int jump = 0;


      foreach(Part* part, *cs->parts()) {
            int channel = part->midiChannel();
            setController(channel, CTRL_PROGRAM, part->midiProgram());
            setController(channel, CTRL_VOLUME, part->volume());
            setController(channel, CTRL_REVERB_SEND, part->reverb());
            setController(channel, CTRL_CHORUS_SEND, part->chorus());
            setController(channel, CTRL_PAN, part->pan());


            for (int i = 0; i < part->staves()->size(); ++i) {

                  // create stack for repeats and jumps, added by DK. 23.08.07
                  RepeatStack* rs = new RepeatStack();

                  QList<OttavaE> ol;
                  for (Measure* m = cs->mainLayout()->first(); m; m = m->next())
                        foreach(Element* e, *m->el()) {
                              if (e->type() == OTTAVA) {
                                    Ottava* ottava = (Ottava*)e;
                                    OttavaE oe;
                                    oe.offset = ottava->pitchShift();
                                    oe.start  = ottava->tick();
                                    oe.end    = ottava->tick2();
                                    ol.append(oe);
                                    }
                              }
                  // Loop changed, "m = m->next()" moved to the end of loop,
                  // by DK. 23.08.07
                  for (Measure* m = cs->mainLayout()->first(); m;) {

                        // push each measure for checking of any of repeat or jumps,
                        // added by DK.23.08.07
                        jump = rs->push(m);

                        if (jump != 0) { // if set, jump "n"-measure for-/backward
                              if (jump < 0) {
                                    for (; jump < 0; jump++)
                                          m = m->prev();
                                    }
                              else {
                                    for (; jump > 0; jump--)
                                          m = m->next();
                                    }
                              continue;
                              }

                        for (int voice = 0; voice < VOICES; ++voice) {
                              for (Segment* seg = m->first(); seg; seg = seg->next()) {
                                    Element* el = seg->element(staffIdx * VOICES + voice);
                                    if (!el || el->type() != CHORD)
                                          continue;
                                    Chord* chord = (Chord*)el;
                                    NoteList* nl = chord->noteList();

                                    for (iNote in = nl->begin(); in != nl->end(); ++in) {
                                          Note* note = in->second;
                                          if (note->tieBack())
                                                continue;
                                          unsigned len = 0;
                                          while (note->tieFor()) {
                                                if (note->tieFor()->endNote() == 0)
                                                      break;
                                                len += note->chord()->tickLen();
                                                note = note->tieFor()->endNote();
                                                }
                                          len += (note->chord()->tickLen() * gateTime / 100);

                                          unsigned tick = chord->tick();

                                          Event ev;
                                          ev.type       = ME_NOTEON;
                                          ev.val1       = note->pitch() + part->pitchOffset();
                                          if (ev.val1 > 127)
                                                ev.val1 = 127;
                                          foreach(OttavaE o, ol) {
                                                if (tick >= o.start && tick <= o.end) {
                                                      ev.val1 += o.offset;
                                                      break;
                                                      }
                                                }
                                          ev.val2       = 60;
                                          ev.note       = note;
                                          ev.channel    = channel;
                                          // added rtickOffSet , by DK. 23.08.07
                                          events.insert(std::pair<const unsigned, Event> (tick+rtickOffSet, ev));

                                          ev.val2 = 0;
                                          events.insert(std::pair<const unsigned, Event> (tick+rtickOffSet+len, ev));
                                          }
                                    }
                              }
                        // Don't forget to save measure, because pop may change it,
                        // returned m may differ from the original, new start measure
                        // of "repeat", 0 means nothing to repeat continue with next measure
                        // functions push and pop are in repeat2.h/cpp files, by DK. 23.08.07
                        Measure* ms = m;
                        m = rs->pop(m);
                        if ( m != 0 )
                              continue;
                        else
                              m = ms;

                        m = m->next();

                        }
                  ++staffIdx;
                  // delete repeat Stackelemente, added by DK. 23.08.07
                  if (rs)
                        rs->delStackElement(rs);
                  }
            }

      PlayPanel* pp = mscore->getPlayPanel();
      if (pp) {
            if (events.empty())
                  pp->setEndpos(0);
            else {
                  iEvent e = events.end();
                  --e;
                  endTick = e->first;
                  pp->setEndpos(endTick);
                  }
            }
      }

//---------------------------------------------------------
//   heartBeat
//    paint currently sounding notes
//---------------------------------------------------------

void Seq::heartBeat()
      {
      if (guiPos == playPos)
            return;
      cs->start();
      Note* note = 0;
      if (guiPos == events.end()) {
            // special case seek:
            guiPos = playPos;
            if (guiPos->second.type == ME_NOTEON) {
                  note = guiPos->second.note;
                  foreach(Viewer* v, cs->getViewer())
                        v->moveCursor(note->chord()->segment());
                  }
            }
      else {
            for (ciEvent i = guiPos; i != playPos; ++i) {
                  if (i->second.type == ME_NOTEON) {
                        i->second.note->setSelected(i->second.val2 != 0);
                        cs->addRefresh(i->second.note->abbox());
                        if (i->second.val2)
                              note = i->second.note;
                        }
                  }
            }
      if (note) {
            foreach(Viewer* v, cs->getViewer())
                  v->moveCursor(note->chord()->segment());
            PlayPanel* pp = mscore->getPlayPanel();
            if (pp)
                  pp->heartBeat(note->chord()->tick(), guiPos->first);
            }
      cs->setLayoutAll(false);      // DEBUG
      cs->end();
      guiPos = playPos;
      }

//---------------------------------------------------------
//   setVolume
//---------------------------------------------------------

void Seq::setVolume(float val)
      {
      _volume = val;
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void Seq::setRelTempo(int relTempo)
      {
      SeqMsg msg;
      msg.id    = SEQ_TEMPO_CHANGE;
      msg.data1 = relTempo;
      sendMessage(msg);

      double tempo = cs->tempomap->tempo(playPos->first) * relTempo * 0.01;

      PlayPanel* pp = mscore->getPlayPanel();
      if (pp) {
            pp->setTempo(tempo);
            pp->setRelTempo(relTempo);
            }
      }

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void Seq::setPos(int tick)
      {
      // send note off events
      foreach(const Event& e, _activeNotes) {
            synti->playNote(e.channel, e.val1, 0);
            e.note->setSelected(false);
            }
      _activeNotes.clear();
      playFrame = tick2frame(tick);
      playPos   = events.lower_bound(tick);
      guiPos    = events.end();     // special case so signal heartBeat a seek
      }

//---------------------------------------------------------
//   seek
//---------------------------------------------------------

void Seq::seek(int tick)
      {
/*      PlayPanel* pp = mscore->getPlayPanel();
      if (pp)
            pp->heartBeat(tick);
 */
      cs->setPlayPos(tick);
      SeqMsg msg;
      msg.id    = SEQ_SEEK;
      msg.data1 = tick;
      sendMessage(msg);
      }

//---------------------------------------------------------
//   sendMessage
//    send Message to sequencer
//---------------------------------------------------------

void Seq::sendMessage(SeqMsg& msg) const
      {
      if (!running)
            return;
      if (write(fromThreadFdw, &msg, sizeof(SeqMsg)) != sizeof(SeqMsg)) {
            fprintf(stderr, "sendMessage to sequencer failed: %s\n",
               strerror(errno));
            }
      }

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

void Seq::startNote(int channel, int pitch, int velo)
      {
      if (state != STOP)
            return;
      SeqMsg msg;
      msg.id    = SEQ_PLAY;
      msg.data1 = ME_NOTEON | channel;
      msg.data2 = pitch;
      msg.data3 = velo;
      sendMessage(msg);
      Event event;
      event.type = ME_NOTEON;
      event.channel = channel;
      event.val1 = pitch;
      event.val2 = velo;
      eventList.append(event);
      }

//---------------------------------------------------------
//   stopNotes
//---------------------------------------------------------

void Seq::stopNotes()
      {
      foreach(const Event& event, eventList) {
            if (event.type == ME_NOTEON) {
                  SeqMsg msg;
                  msg.id    = SEQ_PLAY;
                  msg.data1 = event.type | event.channel;
                  msg.data2 = event.val1;
                  msg.data3 = 0;
                  sendMessage(msg);
                  }
            }
      eventList.clear();
      }

//---------------------------------------------------------
//   setController
//---------------------------------------------------------

void Seq::setController(int channel, int ctrl, int data) const
      {
      SeqMsg msg;
      msg.id    = SEQ_PLAY;
      msg.data1 = 0xb0 | channel;
      msg.data2 = ctrl;
      msg.data3 = data;
      sendMessage(msg);
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

void Seq::nextMeasure()
      {
      ciEvent i = playPos;
      Note* note = 0;
      for (;;) {
            if (i->second.type == ME_NOTEON) {
                  note = i->second.note;
                  break;
                  }
            if (i == events.begin())
                  break;
            --i;
            }
      if (!note)
            return;
      Measure* m = note->chord()->segment()->measure();
      m = m->next();
      if (m) {
            int rtick = m->tick() - note->chord()->tick();
            seek(playPos->first + rtick);
            }
      }

//---------------------------------------------------------
//   nextChord
//---------------------------------------------------------

void Seq::nextChord()
      {
      int tick = playPos->first;
      for (ciEvent i = playPos; i != events.end(); ++i) {
            if (i->second.type == ME_NOTEON && i->first > tick && i->second.val2) {
                  seek(i->first);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

void Seq::prevMeasure()
      {
      ciEvent i = playPos;
      Note* note = 0;
      for (;;) {
            if (i->second.type == ME_NOTEON) {
                  note = i->second.note;
                  break;
                  }
            if (i == events.begin())
                  break;
            --i;
            }
      if (!note)
            return;
      Measure* m = note->chord()->segment()->measure();
      m = m->prev();
      if (m) {
            int rtick = note->chord()->tick() - m->tick();
            seek(playPos->first - rtick);
            }
      else
            seek(0);
      }

//---------------------------------------------------------
//   prevChord
//---------------------------------------------------------

void Seq::prevChord()
      {
      int tick  = playPos->first;
      ciEvent i = playPos;
      for (;;) {
            if (i->second.type == ME_NOTEON && i->first < tick && i->second.val2) {
                  seek(i->first);
                  break;
                  }
            if (i == events.begin())
                  break;
            --i;
            }
      }

//---------------------------------------------------------
//   seekEnd
//---------------------------------------------------------

void Seq::seekEnd()
      {
      printf("seek to end\n");
      }
