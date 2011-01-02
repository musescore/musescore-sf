//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "bb.h"
#include "score.h"
#include "part.h"
#include "staff.h"
#include "text.h"
#include "box.h"
#include "slur.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "drumset.h"
#include "utils.h"
#include "harmony.h"
#include "layoutbreak.h"
#include "key.h"
#include "pitchspelling.h"
#include "measure.h"

//---------------------------------------------------------
//   BBTrack
//---------------------------------------------------------

BBTrack::BBTrack(BBFile* f)
      {
      bb          = f;
      _outChannel = -1;
      _drumTrack  = false;
      }

BBTrack::~BBTrack()
      {
      }

//---------------------------------------------------------
//   MNote
//	special Midi Note
//---------------------------------------------------------

struct MNote {
	Event* mc;
      QList<Tie*> ties;

      MNote(Event* _mc) : mc(_mc) {
            for (int i = 0; i < mc->notes().size(); ++i)
                  ties.append(0);
            }
      };

//---------------------------------------------------------
//   BBFile
//---------------------------------------------------------

BBFile::BBFile()
      {
      for (int i = 0; i < MAX_BARS; ++i)
            _barType[i]  = 0;
      bbDivision = 120;
      }

//---------------------------------------------------------
//   BBFile
//---------------------------------------------------------

BBFile::~BBFile()
      {
      }

//---------------------------------------------------------
//   read
//    return false on error
//---------------------------------------------------------

bool BBFile::read(const QString& name)
      {
      _siglist.clear();
      _siglist.add(0, Fraction(4, 4));        // debug

      _path = name;
      QFile f(name);
      if (!f.open(QIODevice::ReadOnly)) {
            return false;
            }
      ba = f.readAll();
      f.close();

      a    = (const unsigned char*) ba.data();
      size = ba.size();

      //---------------------------------------------------
      //    read version
      //---------------------------------------------------

      int idx = 0;
      _version = a[idx++];
      switch(_version) {
            case 0x43 ... 0x49:
                  break;
            default:
                  printf("BB: unknown file version %02x\n", _version);
                  return false;
            }

      printf("read <%s> version 0x%02x\n", qPrintable(name), _version);

      //---------------------------------------------------
      //    read title
      //---------------------------------------------------

      int len = a[idx++];
      _title  = new char[len+1];
      for (int i = 0; i < len; ++i)
            _title[i] = a[idx++];
      _title[len] = 0;

      //---------------------------------------------------
      //    read style(timesig), key and bpm
      //---------------------------------------------------

      ++idx;
      ++idx;
      _style = a[idx++] - 1;
      if (_style < 0 || _style >= int(sizeof(styles)/sizeof(*styles))) {
            printf("Import bb: unknown style %d\n", _style + 1);
            return false;
            }
      _key = a[idx++];

      // map D# G# A#   to Eb Ab Db
      // major   C, Db,  D, Eb,  E,  F, Gb,  G, Ab,  A, Bb,  B, C#, D#, F#, G#, A#
      // minor   C, Db,  D, Eb,  E,  F, Gb,  G, Ab,  A, Bb,  B, C#, D#, F#, G#, A#
      static int kt[] = {
           0,    0, -5,  2, -3,  4, -1, -6,  1, -4,  3, -2,  5,  7, -3,  6, -4, -2,
                -3, 4,  -1, -6,  1, -4,  3, -2,  5,  0, -5,  2,  4,  6,  3,  5, 7
           };
      if (_key >= int (sizeof(kt)/sizeof(*kt))) {
            printf("bad key %d\n", _key);
            return false;
            }
      _key = kt[_key];

      _bpm = a[idx] + (a[idx+1] << 8);
      idx += 2;

      printf("Title <%s>\n", _title);
      printf("style %d\n",   _style);
      printf("key   %d\n",   _key);
      printf("sig   %d/%d\n", timesigZ(), timesigN());
      printf("bpm   %d\n", _bpm);

      //---------------------------------------------------
      //    read bar types
      //---------------------------------------------------

      int bar = a[idx++];           // starting bar number
      while (bar < 255) {
            int val = a[idx++];
            if (val == 0)
                  bar += a[idx++];
            else {
                  printf("bar type: bar %d val %d\n", bar, val);
                  _barType[bar++] = val;
                  }
            }

      //---------------------------------------------------
      //    read chord extensions
      //---------------------------------------------------

      int beat;
      for (beat = 0; beat < MAX_BARS * 4;) {
            int val = a[idx++];
            if (val == 0)
                  beat += a[idx++];
            else {
                  BBChord c;
                  c.extension = val;
                  c.beat      = beat + timesigZ() * 4 / timesigN();
                  ++beat;
                  _chords.append(c);
                  }
            }

      //---------------------------------------------------
      //    read chord root
      //---------------------------------------------------

      int roots = 0;
      int maxbeat = 0;
      for (beat = 0; beat < MAX_BARS * 4;) {
            int val = a[idx++];
            if (val == 0)
                  beat += a[idx++];
            else {
                  int root = val % 18;
                  int bass = (root - 1 + val / 18) % 12 + 1;
                  if (root == bass)
                        bass = 0;
                  int ibeat = beat + timesigZ() * 4 / timesigN();
                  if (ibeat != _chords[roots].beat) {
                        printf("import bb: inconsistent chord type and root beat\n");
                        return false;
                        }
                  _chords[roots].root = root;
                  _chords[roots].bass = bass;
                  if (maxbeat < beat)
                        maxbeat = beat;
                  ++roots;
                  ++beat;
                  }
            }

      _measures = ((maxbeat + timesigZ() - 1) / timesigZ()) + 1;

      if (roots != _chords.size()) {
            printf("import bb: roots %d != extensions %d\n", roots, _chords.size());
            return false;
            }
      printf("Measures %d\n", _measures);

#if 0
      printf("================chords=======================\n");
      foreach(BBChord c, _chords) {
            printf("chord beat %3d bass %d root %d extension %d\n",
               c.beat, c.bass, c.root, c.extension);
            }
      printf("================chords=======================\n");
#endif

      if (a[idx] == 1) {            //??
            printf("Skip 0x%02x at 0x%04x\n", a[idx], idx);
            ++idx;
            }

      _startChorus = a[idx++];
      _endChorus   = a[idx++];
      _repeats     = a[idx++];

      printf("start chorus %d  end chorus %d repeats %d, pos now 0x%x\n",
         _startChorus, _endChorus, _repeats, idx);

      if (_startChorus >= _endChorus) {
            _startChorus = 0;
            _endChorus = 0;
            _repeats = 1;
            }

      //---------------------------------------------------
      //    read style file
      //---------------------------------------------------

      bool found = false;
      for (int i = idx; i < size; ++i) {
            if (a[i] == 0x42) {
                  if (a[i+1] < 16) {
                        for (int k = i+2; k < (i+18); ++k) {
                              if (a[k] == '.' && a[k+1] == 'S' && a[k+2] == 'T' && a[k+3] == 'Y') {
                                    found = true;
                                    break;
                                    }
                              }
                        }
                  if (found) {
                        idx = i + 1;
                        break;
                        }
                  }
            }
      if (!found) {
            printf("import bb: style file not found\n");
            return false;
            }

      printf("read styleName at 0x%x\n", idx);
      len = a[idx++];
      _styleName = new char[len+1];

      for (int i = 0; i < len; ++i)
            _styleName[i] = a[idx++];
      _styleName[len] = 0;

      printf("style name <%s>\n", _styleName);

      // read midi events
      int eventStart = a[size-4] + a[size-3] * 256;
      int eventCount = a[size-2] + a[size-1] * 256;

      int endTick = _measures * bbDivision * 4 * timesigZ() / timesigN();

      if (eventCount == 0) {
            printf("no melody\n");
            return true;
            }
      else {
            idx = eventStart;
            printf("melody found at 0x%x\n", idx);
            int i = 0;
            int lastLen = 0;
            for (i = 0; i < eventCount; ++i, idx+=12) {
                  int type = a[idx + 4] & 0xf0;
                  if (type == 0x90) {
                        int channel = a[idx + 7];
                        BBTrack* track = 0;
                        foreach (BBTrack* t, _tracks) {
                              if (t->outChannel() == channel) {
                                    track = t;
                                    break;
                                    }
                              }
                        if (track == 0) {
                              track = new BBTrack(this);
                              track->setOutChannel(channel);
                              _tracks.append(track);
                              }
                        int tick = a[idx] + (a[idx+1]<<8) + (a[idx+2]<<16) + (a[idx+3]<<24);
                        tick -= 4 * bbDivision;
                        if (tick >= endTick) {
                              printf("event tick %d > %d\n", tick, endTick);
                              continue;
                              }
                        Event* note = new Event(ME_NOTE);
                        note->setOntime((tick * AL::division) / bbDivision);
                        note->setPitch(a[idx + 5]);
                        note->setVelo(a[idx + 6]);
                        note->setChannel(channel);
                        int len = a[idx+8] + (a[idx+9]<<8) + (a[idx+10]<<16) + (a[idx+11]<<24);
                        if (len == 0) {
                              if (lastLen == 0) {
                                    printf("note event of len 0 at idx %04x\n", idx);
                                    delete note;
                                    continue;
                                    }
                              len = lastLen;
                              }
                        lastLen = len;
                        note->setDuration((len * AL::division) / bbDivision);
                        track->append(note);
                        }
                  else if (type == 0xb0) {
                        // ignore controller
                        }
                  else if (type == 0)
                        break;
                  else {
                        printf("unknown event type 0x%02x at x%04x\n", a[idx + 4], idx);
                        break;
                        }
                  }
            printf("Events found x%02x (%d)\n", i, i);
            }
      return true;
      }

//---------------------------------------------------------
//   importBB
//    return true on success
//---------------------------------------------------------

bool Score::importBB(const QString& name)
      {
      BBFile bb;
      if (!bb.read(name)) {
            printf("cannot open file <%s>\n", qPrintable(name));
            return false;
            }

      QList<BBTrack*>* tracks = bb.tracks();
      int ntracks = tracks->size();
      if (ntracks == 0)             // no events?
            ntracks = 1;
      for (int i = 0; i < ntracks; ++i) {
            Part* part = new Part(this);
            Staff* s   = new Staff(this, part, 0);
            part->insertStaff(s);
            _staves.push_back(s);
            _parts.push_back(part);
            }

      //---------------------------------------------------
      //  create measures
      //---------------------------------------------------

      for (int i = 0; i < bb.measures(); ++i) {
            Measure* measure  = new Measure(this);
            int tick = sigmap()->bar2tick(i, 0, 0);
            measure->setTick(tick);
      	add(measure);
            }

      //---------------------------------------------------
      //  create notes
      //---------------------------------------------------

	foreach (BBTrack* track, *tracks)
            track->cleanup();

      if (tracks->isEmpty()) {
            for (MeasureBase* mb = first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* measure = (Measure*)mb;
                  Rest* rest = new Rest(this, measure->tick(), Duration(Duration::V_MEASURE));
                  rest->setTrack(0);
                  Segment* s = measure->getSegment(rest);
                  s->add(rest);
                  }
            }
      else {
      	int staffIdx = 0;
	      foreach (BBTrack* track, *tracks)
                  convertTrack(track, staffIdx++);
            }

      spell();

      //---------------------------------------------------
      //    create title
      //---------------------------------------------------

      Text* text = new Text(this);
      text->setSubtype(TEXT_TITLE);
      text->setTextStyle(TEXT_STYLE_TITLE);
      text->setText(bb.title());

      MeasureBase* measure = first();
      if (measure->type() != VBOX) {
            measure = new VBox(this);
            measure->setTick(0);
            measure->setNext(first());
            add(measure);
            }
      measure->add(text);

      //---------------------------------------------------
      //    create chord names
      //---------------------------------------------------

      static const int table[] = {
            14, 9, 16, 11, 18, 13, 8, 15, 10, 17, 12, 19
            };
      const QList<BBChord> cl = bb.chords();
      foreach(BBChord c, cl) {
            int tick = c.beat * AL::division;
            Measure* m = tick2measure(tick);
            if (m == 0) {
                  printf("import BB: measure for tick %d not found\n", tick);
                  continue;
                  }
            Harmony* h = new Harmony(this);
            h->setTick(tick);
            h->setTrack(0);
            h->setRootTpc(table[c.root-1]);
            if (c.bass > 0)
                  h->setBaseTpc(table[c.bass-1]);
            else
                  h->setBaseTpc(INVALID_TPC);
            h->setId(c.extension);
            h->render();
            m->add(h);
            }

      //---------------------------------------------------
      //    insert layout breaks
      //    add chorus repeat
      //---------------------------------------------------

      int startChorus = bb.startChorus() - 1;
      int endChorus   = bb.endChorus() - 1;

      int n = 0;
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* measure = (Measure*)mb;
            if (n && (n % 4) == 0) {
                  LayoutBreak* lb = new LayoutBreak(this);
                  lb->setSubtype(LAYOUT_BREAK_LINE);
                  measure->add(lb);
                  }
            if (startChorus == n)
                  measure->setRepeatFlags(RepeatStart);
            else if (endChorus == n) {
                  measure->setRepeatFlags(RepeatEnd);
                  measure->setRepeatCount(bb.repeats());
                  }
            ++n;
            }

      foreach(Staff* staff, _staves) {
            KeyList* kl = staff->keymap();
            (*kl)[0] = bb.key();
            }

      _saved   = false;
      _created = true;
      return true;
      }

//---------------------------------------------------------
//   processPendingNotes
//---------------------------------------------------------

int Score::processPendingNotes(QList<MNote*>* notes, int len, int track)
      {
      Staff* cstaff    = staff(track/VOICES);
      Drumset* drumset = cstaff->part()->drumset();
      bool useDrumset  = cstaff->part()->useDrumset();
      int tick         = notes->at(0)->mc->ontime();

      //
      // look for len of shortest note
      //
      foreach (const MNote* n, *notes) {
      	if (n->mc->duration() < len)
                  len = n->mc->duration();
            }

      //
      // split notes on measure boundary
      //
      Measure* measure = tick2measure(tick);
      if (measure == 0 || (tick >= (measure->tick() + measure->tickLen()))) {
            printf("no measure found for tick %d\n", tick);
            notes->clear();
            return len;
            }
      if ((tick + len) > measure->tick() + measure->tickLen())
            len = measure->tick() + measure->tickLen() - tick;

      Chord* chord = new Chord(this);
      chord->setTick(tick);
      chord->setTrack(track);
      Duration d;
      d.setVal(len);
      chord->setDuration(d);
      Segment* s = measure->getSegment(chord);
      s->add(chord);

      foreach (MNote* n, *notes) {
            QList<Event*>& nl = n->mc->notes();
            for (int i = 0; i < nl.size(); ++i) {
                  Event* mn = nl[i];
      		Note* note = new Note(this);
                  note->setPitch(mn->pitch(), mn->tpc());
      		note->setTrack(track);
            	chord->add(note);
                  note->setTick(tick);

                  if (useDrumset) {
                        if (!drumset->isValid(mn->pitch())) {
                              printf("unmapped drum note 0x%02x %d\n", mn->pitch(), mn->pitch());
                              }
                        else {
                              chord->setStemDirection(drumset->stemDirection(mn->pitch()));
                              }
                        }
                  if (n->ties[i]) {
                        n->ties[i]->setEndNote(note);
                        n->ties[i]->setTrack(note->track());
                        note->setTieBack(n->ties[i]);
                        }
                  }
            if (n->mc->duration() <= len) {
                  notes->removeAt(notes->indexOf(n));
                  continue;
                  }
            for (int i = 0; i < nl.size(); ++i) {
                  Event* mn = nl[i];
                  Note* note = chord->findNote(mn->pitch());
      		n->ties[i] = new Tie(this);
                  n->ties[i]->setStartNote(note);
      		note->setTieFor(n->ties[i]);
                  }
            n->mc->setOntime(n->mc->ontime() + len);
            n->mc->setDuration(n->mc->duration() - len);
            }
      return len;
      }

//---------------------------------------------------------
//   collectNotes
//---------------------------------------------------------

static ciEvent collectNotes(int tick, int voice, ciEvent i, const EventList* el, QList<MNote*>* notes)
      {
      for (;i != el->end(); ++i) {
            Event* e = *i;
            if (e->type() != ME_CHORD)
                  continue;
            Event* mc = e;
            if (mc->voice() != voice)
                  continue;
            if ((*i)->ontime() > tick)
                  break;
            if ((*i)->ontime() < tick)
                  continue;
            MNote* n = new MNote(mc);
            notes->append(n);
            }
      return i;
      }

//---------------------------------------------------------
//   convertTrack
//---------------------------------------------------------

void Score::convertTrack(BBTrack* track, int staffIdx)
	{
      track->findChords();
      int voices         = track->separateVoices(2);
	const EventList el = track->events();

      for (int voice = 0; voice < voices; ++voice) {
            int track = staffIdx * VOICES + voice;
            QList<MNote*> notes;

            int ctick = 0;
            ciEvent i = collectNotes(ctick, voice, el.begin(), &el, &notes);

            for (; i != el.end();) {
                  Event* e = *i;
                  if (e->type() != ME_CHORD || e->voice() != voice) {
                        ++i;
                        continue;
                        }
                  //
                  // process pending notes
                  //
                  int restLen = e->ontime() - ctick;
// printf("ctick %d  rest %d ontick %d size %d\n", ctick, restLen, e->ontime(), notes.size());

                  if (restLen <= 0) {
                        printf("bad restlen ontime %d - ctick %d\n", e->ontime(), ctick);
                        abort();
                        }

                  while (!notes.isEmpty()) {
                        int len = processPendingNotes(&notes, restLen, track);
                        if (len == 0) {
                              printf("processPendingNotes returns zero, restlen %d, track %d\n", restLen, track);
                              ctick += restLen;
                              restLen = 0;
                              break;
                              }
                        ctick += len;
                        restLen -= len;
                        }
// printf("  1.ctick %d  rest %d\n", ctick, restLen);
                  //
                  // check for gap and fill with rest
                  //
                  if (voice == 0) {
                        while (restLen > 0) {
                              int len = restLen;
                  		Measure* measure = tick2measure(ctick);
                              if (measure == 0 || (ctick >= (measure->tick() + measure->tickLen()))) {       // at end?
                                    ctick += len;
                                    restLen -= len;
                                    break;
                                    }
                              // split rest on measure boundary
                              if ((ctick + len) > measure->tick() + measure->tickLen()) {
                                    len = measure->tick() + measure->tickLen() - ctick;
                                    if (len <= 0) {
                                          printf("bad len %d\n", len);
                                          break;
                                          }
                                    }
                              Duration d;
                              d.setVal(len);
                              Rest* rest = new Rest(this, ctick, d);
                              rest->setTrack(staffIdx * VOICES);
                              Segment* s = measure->getSegment(rest);
                              s->add(rest);
// printf("   add rest %d\n", len);

                              ctick   += len;
                              restLen -= len;
                              }
                        }
                  else
                        ctick += restLen;

// printf("  2.ctick %d  rest %d\n", ctick, restLen);
                  //
                  // collect all notes at ctick
                  //
                  i = collectNotes(ctick, voice, i, &el, &notes);
                  }

            //
      	// process pending notes
            //
            while (!notes.isEmpty())
                  processPendingNotes(&notes, 0x7fffffff, track);
            }
      }

//---------------------------------------------------------
//   quantize
//    process one segment (measure)
//---------------------------------------------------------

void BBTrack::quantize(int startTick, int endTick, EventList* dst)
      {
      int mintick = AL::division * 64;
      iEvent i = _events.begin();
      for (; i != _events.end(); ++i) {
            if ((*i)->ontime() >= startTick)
                  break;
            }
      iEvent si = i;
      for (; i != _events.end(); ++i) {
            Event* e = *i;
            if (e->ontime() >= endTick)
                  break;
            if (e->type() == ME_NOTE && (e->duration() < mintick))
                  mintick = e->duration();
            }
      if (mintick <= AL::division / 16)        // minimum duration is 1/64
            mintick = AL::division / 16;
      else if (mintick <= AL::division / 8)
            mintick = AL::division / 8;
      else if (mintick <= AL::division / 4)
            mintick = AL::division / 4;
      else if (mintick <= AL::division / 2)
            mintick = AL::division / 2;
      else if (mintick <= AL::division)
            mintick = AL::division;
      else if (mintick <= AL::division * 2)
            mintick = AL::division * 2;
      else if (mintick <= AL::division * 4)
            mintick = AL::division * 4;
      else if (mintick <= AL::division * 8)
            mintick = AL::division * 8;
      int raster;
      if (mintick > AL::division)
            raster = AL::division;
      else
            raster = mintick;

      //
      //  quantize onset
      //
      for (iEvent i = si; i != _events.end(); ++i) {
            Event* e = *i;
            if (e->ontime() >= endTick)
                  break;
            if (e->type() == ME_NOTE) {
                  Event* note = e;
                  // prefer moving note to the right
      	      int tick = ((note->ontime() + raster/2) / raster) * raster;
                  int diff = tick - note->ontime();
	            int len  = note->duration() - diff;
	            note->setOntime(tick);
      	      note->setDuration(len);
                  }
            dst->insert(e);
            }
      //
      //  quantize duration
      //
      for (iEvent i = dst->begin(); i != dst->end(); ++i) {
            Event* e = *i;
            if (e->type() != ME_NOTE)
                  continue;
            Event* note = e;
            int tick   = note->ontime();
            int len    = note->duration();
            int ntick  = tick + len;
            int nntick = -1;
            for (iEvent ii = (i+1); ii != dst->end(); ++ii) {
                  if ((*ii)->type() == ME_NOTE) {
                        Event* ee = *ii;
                        if (ee->ontime() == tick)
                              continue;
                        nntick = ee->ontime();
                        break;
                        }
                  }
            if (nntick == -1)
                  len = quantizeLen(len, raster);
            else {
                  int diff = nntick - ntick;
                  if (diff > 0) {
                        // insert rest?
                        if (diff <= raster)
                              len = nntick - tick;
                        else
                              len = quantizeLen(len, raster);
                        }
                  else {
                        if (diff > -raster)
                              len = nntick - tick;
                        else
                              len = quantizeLen(len, raster);
                        }
                  }
            note->setDuration(len);
            }
      }

//---------------------------------------------------------
//   cleanup
//    - quantize
//    - remove overlaps
//---------------------------------------------------------

void BBTrack::cleanup()
	{
      EventList dl;

      //
      //	quantize
      //
      int lastTick = 0;
      foreach (Event* e, _events) {
            if (e->type() != ME_NOTE)
                  continue;
            int offtime  = e->offtime();
            if (offtime > lastTick)
                  lastTick = offtime;
            }
      int startTick = 0;
      for (int i = 1;; ++i) {
            int endTick = bb->siglist().bar2tick(i, 0, 0);
            quantize(startTick, endTick, &dl);
            if (endTick > lastTick)
                  break;
            startTick = endTick;
            }

      //
      //
      //
      _events.clear();

      for(iEvent i = dl.begin(); i != dl.end(); ++i) {
            Event* e = *i;
            if (e->type() == ME_NOTE) {
                  iEvent ii = i;
                  ++ii;
                  for (; ii != dl.end(); ++ii) {
                        Event* ee = *ii;
                        if (ee->type() != ME_NOTE || ee->pitch() != e->pitch())
                              continue;
                        if (ee->ontime() >= e->ontime() + e->duration())
                              break;
                        e->setDuration(ee->ontime() - e->ontime());
                        break;
                        }
                  if (e->duration() <= 0)
                        continue;
                  }
		_events.insert(e);
            }
      }

//---------------------------------------------------------
//   findChords
//---------------------------------------------------------

void BBTrack::findChords()
      {
      EventList dl;
      int n = _events.size();

      Drumset* drumset;
      if (_drumTrack)
            drumset = smDrumset;
      else
            drumset = 0;
      int jitter = 3;   // tick tolerance for note on/off

      for (int i = 0; i < n; ++i) {
            Event* e = _events[i];
            if (e == 0)
                  continue;
            if (e->type() != ME_NOTE) {
                  dl.append(e);
                  continue;
                  }

            Event* note      = e;
            int ontime       = note->ontime();
            int offtime      = note->offtime();
            Event* chord = new Event(ME_CHORD);
            chord->setOntime(ontime);
            chord->setDuration(note->duration());
            chord->notes().append(note);
            int voice = 0;
            chord->setVoice(voice);
            dl.append(chord);
            _events[i] = 0;

            bool useDrumset = false;
            if (drumset) {
                  int pitch = note->pitch();
                  if (drumset->isValid(pitch)) {
                        useDrumset = true;
                        voice = drumset->voice(pitch);
                        chord->setVoice(voice);
                        }
                  }
            for (int k = i + 1; k < n; ++k) {
                  if (_events[k] == 0 || _events[k]->type() != ME_NOTE)
                        continue;
                  Event* nn = _events[k];
                  if (nn->ontime() - jitter > ontime)
                        break;
                  if (qAbs(nn->ontime() - ontime) > jitter || qAbs(nn->offtime() - offtime) > jitter)
                        continue;
                  int pitch = nn->pitch();
                  if (useDrumset) {
                        if (drumset->isValid(pitch) && drumset->voice(pitch) == voice) {
                              chord->notes().append(nn);
                              _events[k] = 0;
                              }
                        }
                  else {
                        chord->notes().append(nn);
                        _events[k] = 0;
                        }
                  }
            }
      _events = dl;
      }


//---------------------------------------------------------
//   separateVoices
//---------------------------------------------------------

int BBTrack::separateVoices(int /*maxVoices*/)
      {
      return 1;
      }
