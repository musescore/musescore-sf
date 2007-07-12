//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: cmd.cpp,v 1.74 2006/04/12 14:58:10 wschweer Exp $
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

/**
 \file
 Handling of several GUI commands.
*/

#include "score.h"
#include "utils.h"
#include "key.h"
#include "clef.h"
#include "navigate.h"
#include "canvas.h"
#include "slur.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "segment.h"
#include "text.h"
#include "sig.h"
#include "padstate.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "layout.h"
#include "preferences.h"
#include "page.h"
#include "barline.h"
#include "seq.h"
#include "mscore.h"
#include "tuplet.h"
#include "padids.h"
#include "xml.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "hairpin.h"
#include "keysig.h"

//---------------------------------------------------------
//   start
//---------------------------------------------------------

/**
 Clear the area to be redrawn.
*/

void Score::start()
      {
      refresh.setRect(0.0,0.0,0.0,0.0);
      updateAll = false;
      }

//---------------------------------------------------------
//   startCmd
//---------------------------------------------------------

/**
 Start a GUI command by clearing the redraw area
 and starting a user-visble undo.
*/

void Score::startCmd()
      {
//      printf("startCmd()\n");
      start();
      startUndo();
      }

//---------------------------------------------------------
//   cmdAdd
//---------------------------------------------------------

void Score::cmdAdd(Element* e)
      {
      e->setSelected(false);
      undoAddElement(e);
      layout();
      }

//---------------------------------------------------------
//   cmdAdd
//---------------------------------------------------------

void Score::cmdAdd(Element* e, const QPointF& pos, const QPointF& dragOffset)
      {
      e->setSelected(false);
      startCmd();

      Staff* staff = 0;
      int pitch, tick;
      QPointF offset;
      Segment* segment;
      Measure* measure = pos2measure(pos, &tick, &staff, &pitch, &segment, &offset);
      if (measure == 0) {
            printf("cmdAdd: cannot put object here\n");
            delete e;
            endCmd(true);
            return;
            }
      e->setStaff(staff);
      e->setParent(measure);

      // calculate suitable endposition
      int tick2 = measure->last()->tick();
      while (tick2 <= tick) {
            measure = measure->next();
            if (measure == 0)
                  break;
            tick2 = measure->tick();
            }

      switch(e->type()) {
            case VOLTA:
                  break;
            case PEDAL:
                  {
                  Pedal* pedal = (Pedal*)e;
                  pedal->setTick(tick);
                  pedal->setTick2(tick2);
                  pedal->layout(mainLayout());
                  LineSegment* ls = pedal->lineSegments().front();
                  QPointF uo(pos - ls->canvasPos() - dragOffset);
                  ls->setUserOff(uo / _spatium);
                  }
                  break;
            case OTTAVA:
                  {
                  Ottava* ottava = (Ottava*)e;
                  ottava->setTick(tick);
                  ottava->setTick2(tick2);
                  ottava->layout(mainLayout());
                  LineSegment* ls = ottava->lineSegments().front();
                  QPointF uo(pos - ls->canvasPos() - dragOffset);
                  ls->setUserOff(uo / _spatium);
                  }
                  break;
            case TRILL:
                  {
                  Trill* trill = (Trill*)e;
                  trill->setTick(tick);
                  trill->setTick2(tick2);
                  trill->layout(mainLayout());
                  LineSegment* ls = trill->lineSegments().front();
                  QPointF uo(pos - ls->canvasPos() - dragOffset);
                  ls->setUserOff(uo / _spatium);
                  }
                  break;
            case HAIRPIN:
                  {
                  Hairpin* hairpin = (Hairpin*)e;
                  e->setTick(tick);
                  hairpin->setTick2(tick2);
                  hairpin->layout(mainLayout());
                  LineSegment* ls = hairpin->lineSegments().front();
                  QPointF uo(pos - ls->canvasPos() - dragOffset);
                  ls->setUserOff(uo / _spatium);
                  }
                  break;
            case DYNAMIC:
                  {
                  e->setTick(tick);
                  e->layout(mainLayout());
                  QPointF uo(pos - (e->ipos() + e->parent()->canvasPos()) - dragOffset);
                  e->setUserOff(uo / _spatium);
                  }
                  break;
            default:
                  break;
            }
      cmdAdd(e);
      select(e, 0, 0);
      endCmd(true);
      }

//---------------------------------------------------------
//   cmdRemove
//---------------------------------------------------------

void Score::cmdRemove(Element* e)
      {
      switch(e->type()) {
            case CLEF:
                  {
                  Clef* clef    = (Clef*)e;
                  Staff* staff  = clef->staff();
                  ClefList* cl  = staff->clef();
                  int tick      = clef->tick();
                  iClefEvent ki = cl->find(tick);
                  if (ki == cl->end()) {
                        printf("cmdRemove(KeySig): cannot find keysig at %d\n", tick);
                        return;
                        }
                  int oval = (*cl)[tick];
                  iClefEvent nki = ki;
                  ++nki;
                  cl->erase(ki);
                  undoOp(UndoOp::ChangeClef, staff, tick, oval, -1000);

                  undoOp(UndoOp::RemoveElement, clef);
                  removeElement(clef);
                  Measure* measure = tick2measure(tick);
                  measure->cmdRemoveEmptySegment((Segment*)(clef->parent()));

                  oval = cl->clef(tick);
                  if (nki->second != oval)
                        break;

                  undoOp(UndoOp::ChangeClef, staff, nki->first, oval, -1000);

                  tick = nki->first;
                  for (Measure* m = measure; m; m = m->next()) {
                        bool found = false;
                        for (Segment* segment = m->first(); segment; segment = segment->next()) {
                              if (segment->subtype() != Segment::SegClef)
                                    continue;
                              //
                              // we assume keySigs are only in first track (voice 0)
                              //
                              int track = staff->idx() * VOICES;
                              Clef* e = (Clef*)segment->element(track);
                              int etick = segment->tick();
                              if (!e || (etick != tick))
                                    continue;
                              if (etick > tick)
                                    break;
                              undoRemoveElement(e);
                              m->cmdRemoveEmptySegment(segment);
                              found = true;
                              break;
                              }
                        if (found)
                              break;
                        }
                  }
                  break;

            case KEYSIG:
                  {
                  KeySig* ks   = (KeySig*)e;
                  Staff* staff = ks->staff();
                  KeyList* kl  = staff->keymap();
                  int tick     = ks->tick();
                  iKeyEvent ki = kl->find(tick);
                  if (ki == kl->end()) {
                        printf("cmdRemove(KeySig): cannot find keysig at %d\n", tick);
                        return;
                        }
                  int oval = (*kl)[tick];
                  iKeyEvent nki = ki;
                  ++nki;
                  kl->erase(ki);
                  undoOp(UndoOp::ChangeKeySig, staff, tick, oval, -1000);

                  undoOp(UndoOp::RemoveElement, ks);
                  removeElement(ks);
                  Measure* measure = tick2measure(tick);
                  measure->cmdRemoveEmptySegment((Segment*)(ks->parent()));

                  oval = kl->key(tick);
                  if (nki->second != oval)
                        break;

                  undoOp(UndoOp::ChangeKeySig, staff, nki->first, oval, -1000);

                  tick = nki->first;
                  for (Measure* m = measure; m; m = m->next()) {
                        bool found = false;
                        for (Segment* segment = m->first(); segment; segment = segment->next()) {
                              if (segment->subtype() != Segment::SegKeySig)
                                    continue;
                              //
                              // we assume keySigs are only in first track (voice 0)
                              //
                              int track = staff->idx() * VOICES;
                              KeySig* e = (KeySig*)segment->element(track);
                              int etick = segment->tick();
                              if (!e || (etick != tick))
                                    continue;
                              if (etick > tick)
                                    break;
                              undoRemoveElement(e);
                              m->cmdRemoveEmptySegment(segment);
                              found = true;
                              break;
                              }
                        if (found)
                              break;
                        }
                  }
                  break;

            default:
                  undoRemoveElement(e);
                  break;
            }
      layout();
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void Score::update(const QRectF& r)
      {
      foreach(Viewer* v, viewer)
            v->dataChanged(r);
      updateAll = false;
      }

//---------------------------------------------------------
//   end
//---------------------------------------------------------

/**
 Update the redraw area.
*/
void Score::end()
      {
      layout();   // DEBUG
      if (updateAll) {
            foreach(Viewer* v, viewer)
                  v->updateAll(this);
            updateAll = false;
            }
      else
            update(refresh);
      }

void Score::end1()
      {
      if (updateAll) {
            foreach(Viewer* v, viewer)
                  v->updateAll(this);
            updateAll = false;
            }
      else
            update(refresh);
      }

//---------------------------------------------------------
//   endCmd
//---------------------------------------------------------

/**
 End a GUI command by (if \a undo) ending a user-visble undo
 and (always) updating the redraw area.
*/

void Score::endCmd(bool undo)
      {
      if (undo)
            endUndo();
      end();
      setPadState();
      }

//---------------------------------------------------------
//   cmdAddPitch
//---------------------------------------------------------

void Score::cmdAddPitch(int note, bool addFlag)
      {
      // c d e f g a b entered: insert note or add note to chord
      int octave = padState.pitch / 12;
      static int ptab[15][7] = {
        //    c  d  e  f  g  a  b
            { -1, 1, 3, 5, 6,  8, 10 },     // Bes
            { -1, 2, 3, 5, 6,  8, 10 },     // Ges
            {  0, 2, 3, 5, 6,  8, 10 },     // Des
            {  0, 2, 3, 5, 7,  8, 10 },     // As
            {  0, 2, 3, 5, 7,  8, 10 },     // Es
            {  0, 2, 3, 5, 7,  9, 10 },     // B
            {  0, 2, 4, 5, 7,  9, 11 },     // F
            {  0, 2, 4, 5, 7,  9, 11 },     // C
            {  0, 2, 4, 6, 7,  9, 11 },     // G
            {  1, 2, 4, 6, 7,  9, 11 },     // D
            {  1, 2, 4, 6, 8,  9, 11 },     // A
            {  1, 3, 4, 6, 8,  9, 11 },     // E
            {  1, 3, 4, 6, 8, 10, 11 },     // H
            {  1, 3, 5, 6, 8, 10, 11 },     // Fis
            {  1, 3, 5, 6, 8, 10, 12 },     // Cis
            };

      ChordRest* cr = 0;
      if (cis->pos == -1) {
            //
            // start note entry mode at current selected
            // note/rest
            //
            cr = setNoteEntry(true, false);
            if (cr == 0) {
                  // cannot enter notes
                  // no note/rest selected?
                  return;
                  }
            }
      else {
            //
            // look for next note position
            //
            cr = (ChordRest*)searchNote(cis->pos, cis->staff, cis->voice);
            if (!cr || !cr->isChordRest()) {
                  cis->pos = -1;
                  return;
                  }
            }

      int key   = cr->staff()->keymap()->key(cis->pos) + 7;
      int pitch = ptab[key][note];

      int delta = padState.pitch - (octave*12 + pitch);
      if (delta > 6)
            padState.pitch = (octave+1)*12 + pitch;
      else if (delta < -6)
            padState.pitch = (octave-1)*12 + pitch;
      else
            padState.pitch = octave*12 + pitch;
      if (padState.pitch < 0)
            padState.pitch = 0;
      if (padState.pitch > 127)
            padState.pitch = 127;


      if (addFlag) {
            // add note to chord
            Note* on = getSelectedNote();
            if (on) {
                  Note* n = addNote(on->chord(), padState.pitch);
                  select(n, 0, 0);
                  }
            }
      else {
            // insert note
            int len = padState.tickLen;
            if (cr->tuplet())
                  len = cr->tuplet()->noteLen();
            setNote(cis->pos, staff(cis->staff), cis->voice, padState.pitch, len);
            cis->pos += len;
            }
      }

//---------------------------------------------------------
//   cmdAddInterval
//---------------------------------------------------------

void Score::cmdAddInterval(int val)
      {
      Note* on = getSelectedNote();
      if (on == 0)
            return;

      setNoteEntry(true, true);
      int pitch = on->pitch();
      switch(val) {
            case 1:   break;
            case 2:   pitch += 2;  break;
            case 3:   pitch += 4;  break;
            case 4:   pitch += 5;  break;
            case 5:   pitch += 7;  break;
            case 6:   pitch += 9;  break;
            case 7:   pitch += 11; break;
            case 8:   pitch += 12; break;
            case 9:   pitch += 14; break;
            case -2:  pitch -= 2;  break;
            case -3:  pitch -= 4;  break;
            case -4:  pitch -= 5;  break;
            case -5:  pitch -= 7;  break;
            case -6:  pitch -= 9;  break;
            case -7:  pitch -= 11; break;
            case -8:  pitch -= 12; break;
            case -9:  pitch -= 14; break;
            default:
                  printf("cmdAddInterval: unknown idx %d\n", val);
                  abort();
            }
      if (pitch > 127)
            pitch = 127;
      if (pitch < 0)
            pitch = 0;
      Note* n = addNote(on->chord(), pitch);
      select(n, 0, 0);
      padState.pitch = n->pitch();
      }

//---------------------------------------------------------
//   setNote
//---------------------------------------------------------

/**
 Set note (\a pitch, \a len) at position \a tick / \a staff / \a voice.
*/

void Score::setNote(int tick, Staff* staff, int voice, int pitch, int len)
      {
      bool addTie  = padState.tie;
      int staffIdx = staff->idx();
      int track    = staffIdx * VOICES + voice;
      Tie* tie     = 0;
      Note* note   = 0;
      Tuplet* tuplet = 0;

      while (len) {
            int stick = tick;
            Measure* measure = tick2measure(stick);
            if (measure == 0) {
                  printf("setNote:  ...measure not found\n");
                  return;
                  }
            Segment* segment = measure->first();
            int noteLen      = 0;
            while (segment) {
                  for (; segment; segment = segment->next()) {
                        if (segment->tick() >= stick)
                              break;
                        }
                  if (segment == 0 || segment->tick() != stick) {
                        Segment* newSegment = new Segment(measure, stick);
                        newSegment->setSubtype(Segment::SegChordRest);
                        measure->insert(newSegment, segment);
                        segment = newSegment;
                        }
                  Element* element = segment->element(track);
                  int l = 0;
                  if (element && element->isChordRest()) {
                        ChordRest* cr = (ChordRest*)element;
                        tuplet        = cr->tuplet();
                        l             = cr->tickLen();
                        if (tuplet)
                              tuplet->remove(cr);
                        segment->setElement(track, 0);
                        undoOp(UndoOp::RemoveElement, element);
                        }
                  segment = segment->next();
                  if (l == 0) {
                        if (segment == 0)
                              l = measure->tick() + measure->tickLen() - stick;
                        else
                              l = segment->tick() - stick;
                        }
                  len     -= l;
                  noteLen += l;
                  stick   += l;
                  if (len <= 0)
                        break;
                  }
            if (len < 0)
                  noteLen += len;

            note = new Note(this);
            note->setPitch(pitch);
            note->setStaff(staff);

            if (seq && mscore->playEnabled()) {
                  Staff* staff = note->staff();
                  seq->startNote(staff->midiChannel(), note->pitch(), 64);
                  }

            if (tie) {
                  tie->setEndNote(note);
                  note->setTieBack(tie);
                  }
            Chord* chord = new Chord(this);
            chord->setTick(tick);
            chord->setVoice(voice);
            chord->setStaff(staff);
            chord->add(note);
            chord->setTickLen(noteLen);
            chord->setStemDirection(preferences.stemDir[voice]);
            if (tuplet) {
                  chord->setTuplet(tuplet);
                  tuplet->add(chord);
                  }
            Segment::SegmentType st = Segment::segmentType(chord->type());
            Segment* seg = measure->findSegment(st, tick);
            if (seg == 0) {
                  seg = measure->createSegment(st, tick);
                  undoAddElement(seg);
                  }
            chord->setParent(seg);
            undoAddElement(chord);
//            layout();
//            measure->layoutNoteHeads(staffIdx);
            select(note, 0, 0);

            tick += noteLen;

            if (len < 0) {
                  if (voice == 0) {
                        // only for first voice: fill with rest
                        setRest(tick, -len, staff, voice, measure);
                        }
                  }
            if (len <= 0)
                  break;
            //
            //  Note does not fit on current measure, create Tie to
            //  next part of note
            tie = new Tie(this);
            tie->setStartNote(note);
            tie->setStaff(note->staff());
            note->setTieFor(tie);
            }
      if (note && addTie) {
            tie = new Tie(this);
            tie->setStartNote(note);
            tie->setStaff(note->staff());
            note->setTieFor(tie);
            }
      _layout->connectTies();
      }

//---------------------------------------------------------
//   setRest
//---------------------------------------------------------

/**
 Set rest(\a len) at position \a tick / \a staff / \a voice.
*/

void Score::setRest(int tick, Staff* st, int voice, int len)
      {
      int staffIdx = st->idx();
      int track    = staffIdx * VOICES + voice;
      int stick    = tick;
      Measure* measure = tick2measure(stick);
      if (measure == 0) {
            printf("setRest:  ...measure not found\n");
            return;
            }
      Segment* segment = measure->first();
      int noteLen      = 0;
      Tuplet* tuplet   = 0;
      while (segment) {
            for (; segment; segment = segment->next()) {
                  if (segment->tick() >= stick)
                        break;
                  }
            if (segment == 0 || segment->tick() != stick) {
                  printf("setRest: ..no segment at tick %d found\n", stick);
                  return;
                  }
            Element* element = segment->element(track);
            int l = 0;
            if (element && element->isChordRest()) {
                  ChordRest* cr = (ChordRest*) element;
                  tuplet = cr->tuplet();
                  if (tuplet)
                        tuplet->remove(cr);
                  l = cr->tickLen();
                  segment->setElement(track, 0);
                  undoOp(UndoOp::RemoveElement, element);
                  }
            segment = segment->next();
            if (l == 0) {
                  if (segment == 0)
                        l = measure->tick() + measure->tickLen() - stick;
                  else
                        l = segment->tick() - stick;
                  }
            noteLen += l;
            stick   += l;
            if (noteLen >= len)     // collected enough time?
                  break;
            }
      if (noteLen < len)
            printf("setRest: cannot find segment! rest: %d\n", len - noteLen);

      Rest* rest = setRest(tick, len, st, voice, measure);
      if (tuplet) {
            rest->setTuplet(tuplet);
            tuplet->add(rest);
            }
      select(rest, 0, 0);
      if (noteLen - len > 0) {
            setRest(tick + len, noteLen - len, st, voice, measure);
            }
      layout();
      }

//---------------------------------------------------------
//   cmdAddText
//---------------------------------------------------------

void Score::cmdAddText(int subtype)
      {
      if (editObject) {
            endEdit();
            endCmd(true);
            }
      Page* page = _layout->pages()->front();
      QList<System*>* sl = page->systems();
      if (sl == 0 || sl->empty()) {
            printf("first create measure, then repeat operation\n");
            return;
            }
      const QList<Measure*>& ml = sl->front()->measures();
      if (ml.empty()) {
            printf("first create measure, then repeat operation\n");
            return;
            }
      Measure* measure = ml.front();

      Text* s = new Text(this);
      s->setSubtype(subtype);
      s->setText(s->subtypeName());

      startCmd();
      s->setAnchorMeasure(measure);
      s->setParent(page);
      undoAddElement(s);
      layout();

      select(s, 0, 0);
      canvas()->startEdit(s);
      }

//---------------------------------------------------------
//   cmdAddTitle
//---------------------------------------------------------

void Score::cmdAddTitle()
      {
      cmdAddText(TEXT_TITLE);
      }

//---------------------------------------------------------
//   cmdAddSubtitle
//---------------------------------------------------------

void Score::cmdAddSubTitle()
      {
      cmdAddText(TEXT_SUBTITLE);
      }

//---------------------------------------------------------
//   cmdAddComposer
//---------------------------------------------------------

void Score::cmdAddComposer()
      {
      cmdAddText(TEXT_COMPOSER);
      }

//---------------------------------------------------------
//   cmdAddPoet
//---------------------------------------------------------

void Score::cmdAddPoet()
      {
      cmdAddText(TEXT_POET);
      }

//---------------------------------------------------------
//   upDown
//---------------------------------------------------------

/**
 Increment/decrement pitch of note by one or by an octave.
*/

void Score::upDown(bool up, bool octave)
      {
      ElementList el;
      foreach(Element* e, *sel->elements()) {
            if (e->type() != NOTE)
                  continue;
            Note* note = (Note*)e;
            while (note->tieBack())
                  note = note->tieBack()->startNote();
            for (; note; note = note->tieFor() ? note->tieFor()->endNote() : 0) {
                  iElement ii;
                  for (ii = el.begin(); ii != el.end(); ++ii) {
                        if (*ii == note)
                              break;
                        }
                  if (ii == el.end())
                        el.push_back(note);
                  }
            }
      if (el.empty())
            return;

      int newPitch = padState.pitch;
      for (iElement i = el.begin(); i != el.end(); ++i) {
            Note* oNote = (Note*)(*i);
            int pitch   = oNote->pitch();
            if (octave)
                  newPitch = pitch + (up ? 12 : -12);
            else
                  newPitch = up ? pitch+1 : pitch-1;
            if (newPitch < 0)
                  newPitch = 0;
            if (newPitch > 127)
                  newPitch = 127;

            UndoOp i;
            i.type  = UndoOp::ChangePitch;
            i.obj   = oNote;
            i.val1  = oNote->pitch();
            undoList.back()->push_back(i);

            oNote->changePitch(newPitch);
            }
      padState.pitch = newPitch;
      sel->updateState();     // accidentals may have changed
      layout();
      }

//---------------------------------------------------------
//   cmdAppendMeasure
//---------------------------------------------------------

/**
 Append one measure.

 Keyboard callback, called from pulldown menu.
*/

void Score::cmdAppendMeasure()
      {
      cmdAppendMeasures(1);
      }

//---------------------------------------------------------
//   cmdAppendMeasures
//    - keyboard callback
//    - called from pulldown menu
//---------------------------------------------------------

/**
 Append \a n measures.

 Keyboard callback, called from pulldown menu.
*/

void Score::cmdAppendMeasures(int n)
      {
      startCmd();
      appendMeasures(n);
      endCmd(true);
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void Score::appendMeasures(int n)
      {
      if (noStaves()) {
            QMessageBox::warning(0, "MuseScore",
               tr("No staves found:\n"
                  "please use the instruments dialog to\n"
                  "first create some staves"));
            return;
            }
      for (int i = 0; i < n; ++i) {
            Measure* last = _layout->last();
            int tick = last ? last->tick() + last->tickLen() : 0;
            Measure* measure  = new Measure(this);
            measure->setTick(tick);

            for (int idx = 0; idx < nstaves(); ++idx) {
                  Rest* rest = new Rest(this, tick, sigmap->ticksMeasure(tick));
                  Staff* staffp = staff(idx);
                  rest->setStaff(staffp);
                  Segment* s = measure->getSegment(rest);
                  s->add(rest);
                  BarLine* barLine = 0;
                  if (staffp->isTop()) {
                        barLine = new BarLine(this);
                        barLine->setStaff(staffp);
                        measure->setEndBarLine(barLine);
                        }
                  }
            undoOp(UndoOp::InsertMeasure, measure);
            _layout->push_back(measure);
            }
      layout();
      }

//---------------------------------------------------------
//   addAttribute
//---------------------------------------------------------

/**
 Add attribute \a attr to all selected notes/rests.

 Called from padToggle() to add note prefix/accent.
*/

void Score::addAttribute(int attr)
      {
      foreach(Element* el, *sel->elements()) {
            if (el->type() != NOTE && el->type() != REST)
                  continue;
            addAttribute(el, new NoteAttribute(this, attr));
            }
      }

//---------------------------------------------------------
//   addAccidental
//---------------------------------------------------------

/**
 Add accidental of subtype \a idx to all selected notes.
*/

void Score::addAccidental(int idx)
      {
      foreach(Element* el, *sel->elements()) {
            if (el->type() == NOTE)
                  addAccidental((Note*)el, idx);
            }
      }

//---------------------------------------------------------
//   addAccidental
//---------------------------------------------------------

/**
 Add accidental of subtype \a idx to note \a oNote.
*/

void Score::addAccidental(Note* oNote, int accidental)
      {
      UndoOp i;
      i.type  = UndoOp::ChangeAccidental;
      i.obj   = oNote;
      i.val1  = oNote->pitch();
      i.val2  = oNote->tpc();
      i.val3  = oNote->accidentalIdx();
      undoList.back()->push_back(i);
      oNote->changeAccidental(accidental);
      layout();
      }

//---------------------------------------------------------
//   addAttribute
//---------------------------------------------------------

void Score::addAttribute(Element* el, NoteAttribute* atr)
      {
      ChordRest* cr;
      if (el->type() == NOTE)
            cr = ((Note*)el)->chord();
      else if (el->type() == REST)
            cr = (Rest*)el;
      else
            return;
      atr->setParent(cr);
      NoteAttribute* oa = cr->hasAttribute(atr);
      if (oa) {
            delete atr;
            atr = 0;
            cmdRemove(oa);
            }
      else
            cmdAdd(atr);
      }

//---------------------------------------------------------
//   toggleInvisible
//---------------------------------------------------------

void Score::toggleInvisible(Element* obj)
      {
      obj->setVisible(!obj->visible());
      undoOp(UndoOp::ToggleInvisible, obj);
      refresh |= obj->abbox();
      }

//---------------------------------------------------------
//   resetUserOffsets
//---------------------------------------------------------

/**
 Reset user offset for all selected notes.

 Called from pulldown menu.
*/

void Score::resetUserOffsets()
      {
      startCmd();
      QList<Element*> el = *sel->elements();
      for (iElement i = el.begin(); i != el.end(); ++i)
            (*i)->setUserOff(QPointF(0.0, 0.0));
      layout();
      endCmd(true);
      }

//---------------------------------------------------------
//   resetUserStretch
//---------------------------------------------------------

void Score::resetUserStretch()
      {
      for (Measure* m = _layout->first(); m; m = m->next())
            m->setUserStretch(1.0);
      setDirty();
      layout();
      }

//---------------------------------------------------------
//   moveUp
//---------------------------------------------------------

void Score::moveUp(Note* note)
      {
      int rstaff   = note->staff()->rstaff();

      if (note->move() == -1) {
            return;
            }
      if (rstaff + note->move() <= 0) {
            return;
            }

      note->setMove(note->move() - 1);
//      note->chord()->segment()->measure()->layoutNoteHeads(staff(note->staff()));
      layout();
      }

//---------------------------------------------------------
//   moveDown
//---------------------------------------------------------

void Score::moveDown(Note* note)
      {
//      int staffIdx = staff(note->staff());
      Staff* staff = note->staff();
      Part* part   = staff->part();
      int rstaff   = staff->rstaff();
      int rstaves  = part->nstaves();

      if (note->move() == 1) {
            return;
            }
      if (rstaff + note->move() >= rstaves-1) {
            return;
            }
      note->setMove(note->move() + 1);
//      note->chord()->segment()->measure()->layoutNoteHeads(staffIdx);
      layout();
      }

//---------------------------------------------------------
//   cmdAddStretch
//    TODO: cmdAddStretch: undo
//---------------------------------------------------------

void Score::cmdAddStretch(double val)
      {
      if (sel->state != SEL_SYSTEM && sel->state != SEL_STAFF)
            return;
      int startTick = sel->tickStart;
      int endTick   = sel->tickEnd;
      for (Measure* im = _layout->first(); im; im = im->next()) {
            if (im->tick() < startTick)
                  continue;
            if (im->tick() >= endTick)
                  break;
            double stretch = im->userStretch();
            stretch += val;
            im->setUserStretch(stretch);
            }
      layout();
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void Score::cmd(const QString& cmd)
      {
      if (debugMode)
            printf("cmd <%s>\n", cmd.toLatin1().data());
      if (editObject) {                          // in edit mode?
            endUndo();
            canvas()->setState(Canvas::NORMAL);  //calls endEdit()
            }
      if (cmd == "print")
            printFile();
      else if (cmd == "undo")
            doUndo();
      else if (cmd == "redo")
            doRedo();
      else if (cmd == "note-input") {
            setNoteEntry(true, false);
            padState.rest = false;
            endCmd(false);
            }
      else {
            startCmd();
            if (cmd == "append-measure")
                  appendMeasures(1);
            else if (cmd == "page-prev")
                  pagePrev();
            else if (cmd == "page-next")
                  pageNext();
            else if (cmd == "page-top")
                  pageTop();
            else if (cmd == "page-end")
                  pageEnd();
            else if (cmd == "add-tie")
                  cmdAddTie();
            else if (cmd == "add-slur")
                  cmdAddSlur();
            else if (cmd == "add-hairpin")
                  cmdAddHairpin(false);
            else if (cmd == "add-hairpin-reverse")
                  cmdAddHairpin(true);
            else if (cmd == "escape") {
                  if (cis->pos != -1)
                        setNoteEntry(false, false);
                  select(0, 0, 0);
                  }
            else if (cmd == "delete")
                  cmdDeleteSelection();
            else if (cmd == "rest") {
                  if (cis->pos == -1) {
                        setNoteEntry(true, true);
                        Element* el = sel->element();
                        if (el) {
                              if (el->type() == NOTE)
                                    el = el->parent();
                              if (el->isChordRest())
                                    cis->pos = ((ChordRest*)el)->tick();
                              }
                        }
                  if (cis->pos != -1) {
                        int len = padState.tickLen;
                        setRest(cis->pos, staff(cis->staff), cis->voice, len);
                        cis->pos += len;
                        }
                  padState.rest = false;  // continue with normal note entry
//                  moveCursor();
                  }
            else if (cmd == "pitch-up")
                  upDown(true, false);
            else if (cmd == "pitch-down")
                  upDown(false, false);
            else if (cmd == "pitch-up-octave")
                  upDown(true, true);
            else if (cmd == "pitch-down-octave")
                  upDown(false, true);
            else if (cmd == "move-up") {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE)
                        moveUp((Note*)el);
                  }
            else if (cmd == "move-down") {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE) {
                        moveDown((Note*)el);
                        }
                  }
            else if (cmd == "up-chord") {
                  Element* el = sel->element(); // single selection
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        Element* e = upAlt(el);
                        if (e) {
                              if (e->type() == NOTE)
                                    padState.pitch = ((Note*)e)->pitch();
                              select(e, 0, 0);
                              }
                        }
                  }
            else if (cmd == "down-chord") {
                  Element* el = sel->element(); // single selection
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        Element* e = downAlt(el);
                        if (e) {
                              if (e->type() == NOTE)
                                    padState.pitch = ((Note*)e)->pitch();
                              select(e, 0, 0);
                              }
                        }
                  }
            else if (cmd == "top-chord" ) {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE) {
                        Element* e = upAltCtrl((Note*)el);
                        if (e) {
                              if (e->type() == NOTE)
                                    padState.pitch = ((Note*)e)->pitch();
                              select(e, 0, 0);
                              }
                        }
                  }
            else if (cmd == "bottom-chord") {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE) {
                        Element* e = downAltCtrl((Note*)el);
                        if (e) {
                              if (e->type() == NOTE)
                                    padState.pitch = ((Note*)e)->pitch();
                              select(e, 0, 0);
                              }
                        }
                  }
            else if (cmd == "next-chord"
               || cmd == "prev-chord"
               || cmd == "next-measure"
               || cmd == "prev-measure")
                  move(cmd);

            else if (cmd == "note-c")
                  cmdAddPitch(0, false);
            else if (cmd == "note-d")
                  cmdAddPitch(1, false);
            else if (cmd == "note-e")
                  cmdAddPitch(2, false);
            else if (cmd == "note-f")
                  cmdAddPitch(3, false);
            else if (cmd == "note-g")
                  cmdAddPitch(4, false);
            else if (cmd == "note-a")
                  cmdAddPitch(5, false);
            else if (cmd == "note-b")
                  cmdAddPitch(6, false);
            else if (cmd == "chord-c")
                  cmdAddPitch(0, true);
            else if (cmd == "chord-d")
                  cmdAddPitch(1, true);
            else if (cmd == "chord-e")
                  cmdAddPitch(2, true);
            else if (cmd == "chord-f")
                  cmdAddPitch(3, true);
            else if (cmd == "chord-g")
                  cmdAddPitch(4, true);
            else if (cmd == "chord-a")
                  cmdAddPitch(5, true);
            else if (cmd == "chord-b")
                  cmdAddPitch(6, true);
            else if (cmd == "pad-note-1")
                  padToggle(PAD_NOTE1);
            else if (cmd == "pad-note-2")
                  padToggle(PAD_NOTE2);
            else if (cmd == "pad-note-4")
                  padToggle(PAD_NOTE4);
            else if (cmd == "pad-note-8")
                  padToggle(PAD_NOTE8);
            else if (cmd == "pad-note-16")
                  padToggle(PAD_NOTE16);
            else if (cmd == "pad-note-32")
                  padToggle(PAD_NOTE32);
            else if (cmd == "pad-note-64")
                  padToggle(PAD_NOTE64);
            else if (cmd == "pad-rest")
                  padToggle(PAD_REST);
            else if (cmd == "pad-dot")
                  padToggle(PAD_DOT);
            else if (cmd == "beam-start")
                  padToggle(PAD_BEAM_START);
            else if (cmd == "beam-mid")
                  padToggle(PAD_BEAM_MID);
            else if (cmd == "no-beam")
                  padToggle(PAD_BEAM_NO);
            else if (cmd == "beam-32")
                  padToggle(PAD_BEAM32);
            else if (cmd == "pad-tie") {
                  padState.tie = !padState.tie;
                  if (cis->pos == -1 && sel->state == SEL_SINGLE) {
                        Element* el = sel->element();
                        if (el->type() == NOTE) {
                  		Tie* tie = new Tie(this);
                              tie->setParent(el);
            	      	cmdAdd(tie);
                              _layout->connectTies();
                              }
                        }
                  }
            else if (cmd == "pad-sharp2") {
                  padState.prefix = padState.prefix != 3 ? 3 : 0;
                  addAccidental(padState.prefix);
                  }
            else if (cmd == "pad-sharp") {
                  padState.prefix = padState.prefix != 1 ? 1 : 0;
                  addAccidental(padState.prefix);
                  }
            else if (cmd == "pad-nat") {
                  padState.prefix = padState.prefix != 5 ? 5 : 0;
                  addAccidental(padState.prefix);
                  }
            else if (cmd == "pad-flat") {
                  padState.prefix = padState.prefix != 2 ? 2 : 0;
                  addAccidental(padState.prefix);
                  }
            else if (cmd == "pad-flat2") {
                  padState.prefix = padState.prefix != 4 ? 4 : 0;
                  addAccidental(padState.prefix);
                  }
            else if (cmd == "flip")
                  cmdFlipStemDirection();
            else if (cmd == "voice-1")
                  changeVoice(0);
            else if (cmd == "voice-2")
                  changeVoice(1);
            else if (cmd == "voice-3")
                  changeVoice(2);
            else if (cmd == "voice-4")
                  changeVoice(3);
            else if (cmd.startsWith("interval")) {
                  int n = cmd.mid(8).toInt();
                  cmdAddInterval(n);
                  }
            else if (cmd == "duplet")
                  cmdTuplet(2);
            else if (cmd == "triplet")
                  cmdTuplet(3);
            else if (cmd == "quintuplet")
                  cmdTuplet(5);
            else if (cmd == "stretch+")
                  cmdAddStretch(0.1);
            else if (cmd == "stretch-")
                  cmdAddStretch(-0.1);
            else if (cmd == "cut") {
                  if (sel->state == SEL_SINGLE) {
                        QMimeData* mimeData = new QMimeData;
                        Element* el = sel->element();
                        mimeData->setData("application/mscore/symbol", el->mimeData(QPointF()));
                        QApplication::clipboard()->setMimeData(mimeData);
                        deleteItem(el);
                        }
                  }
            else if (cmd == "copy") {
                  const char* mimeType = sel->mimeType();
                  if (mimeType) {
                        QMimeData* mimeData = new QMimeData;
                        mimeData->setData(mimeType, sel->mimeData());
                        QApplication::clipboard()->setMimeData(mimeData);
                        }
                  }
            else if (cmd == "paste") {
                  const QMimeData* ms = QApplication::clipboard()->mimeData();
                  if (sel->state == SEL_SINGLE && ms && ms->hasFormat("application/mscore/symbol")) {
                        QByteArray data(ms->data("application/mscore/symbol"));
                        QDomDocument doc;
                        int line, column;
                        QString err;
                        if (!doc.setContent(data, &err, &line, &column)) {
                              printf("error reading paste data\n");
                              return;
                              }
                        QDomElement e = doc.documentElement();
                        QPointF dragOffset;
                        int type      = Element::readType(e, &dragOffset);
                        addRefresh(sel->element()->abbox());   // layout() ?!
                        sel->element()->drop(QPointF(), QPointF(), type, e);
                        addRefresh(sel->element()->abbox());
                        }
                  else if (sel->state == SEL_STAFF && ms && ms->hasFormat("application/mscore/staff"))
                        pasteStaff(ms);
                  else if (sel->state == SEL_SYSTEM && ms && ms->hasFormat("application/mscore/system")) {
                        printf("paste system\n");
                        }
                  else {
                        printf("paste not supported: sel state %d ms %p\n", sel->state, ms);
                        if (ms) {
                              QStringList formats = ms->formats();
                              printf("Formate:\n");
                              foreach(QString s, formats)
                                    printf("format <%s>\n", s.toLatin1().data());
                              }
                        }
                  }
            else if (cmd == "lyrics")
                  addLyrics();
            else if (cmd == "expression")
                  addExpression();
            else if (cmd == "technik")
                  addTechnik();
            else if (cmd == "tempo")
                  addTempo();
            else if (cmd == "metronome")
                  addMetronome();
            else if (cmd == "pitch-spell")
                  spell();
            endCmd(true);
            }
      }

//---------------------------------------------------------
//   pasteStaff
//---------------------------------------------------------

void Score::pasteStaff(const QMimeData* ms)
      {
      if (sel->state != SEL_STAFF) {
            printf("  cannot paste to selection\n");
            return;
            }
      int tickStart  = sel->tickStart;
      int staffStart = sel->staffStart;

      Measure* measure;
      for (measure = _layout->first(); measure; measure = measure->next()) {
            if (measure->tick() == tickStart)
                  break;
            }
      if (measure->tick() != tickStart) {
            printf("  cannot find measure\n");
            return;
            }

      QByteArray data(ms->data("application/mscore/staff"));
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(data, &err, &line, &column)) {
            printf("error reading paste data\n");
            return;
            }
      int srcStaffStart = -1;
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "Staff") {
                  Measure* m = measure;
                  int staffIdx = e.attribute("id", "0").toInt();
                  if (srcStaffStart == -1)
                        srcStaffStart = staffIdx;
                  staffIdx = staffStart - srcStaffStart + staffIdx;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "Measure") {
                              Measure* sm = new Measure(this);
                              sm->read(ee, staffIdx);
                              cmdReplaceElements(sm, m, staffIdx);
                              delete sm;
                              m = m->next();
                              if (m == 0)
                                    break;
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   cmdReplaceElements
//---------------------------------------------------------

void Score::cmdReplaceElements(Measure* sm, Measure* dm, int staffIdx)
      {
      //
      // TODO: handle special cases: sm->tickLen() != ds->tickLen()
      //

      select(0, 0, 0);
      // clear staff in destination Measure
      for (Segment* s = dm->first(); s;) {
            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int t = startTrack; t < endTrack; ++t) {
                  Element* e = s->element(t);
                  if (e)
                        undoRemoveElement(e);
                  }
            Segment* ns = s->next();
            dm->cmdRemoveEmptySegment(s);
            s = ns;
            }
      // add src elements to destination
      int srcTickOffset = sm->tick();
      int dstTickOffset = dm->tick();

      for (Segment* s = sm->first(); s; s = s->next()) {
            int startTrack = staffIdx * VOICES;
            int endTrack   = startTrack + VOICES;
            int tick       = s->tick() - srcTickOffset + dstTickOffset;
            Segment* ns    = dm->findSegment((Segment::SegmentType)s->subtype(), tick);
            if (ns == 0) {
                  ns = dm->createSegment((Segment::SegmentType)s->subtype(), tick);
                  undoAddElement(ns);
                  }
            for (int t = startTrack; t < endTrack; ++t) {
                  Element* e = s->element(t);
                  if (!e || !e->isChordRest())
                        continue;
                  e->setParent(ns);
                  e->setTick(tick);
                  undoAddElement(e);
                  e->setSelected(false);
                  if (e->type() == REST)
                        select(e, Qt::ShiftModifier, 0);
                  else {
                        Chord* c = (Chord*)e;
                        NoteList* nl = c->noteList();
                        for (iNote in = nl->begin(); in != nl->end(); ++in) {
                              select(in->second, Qt::ShiftModifier, 0);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Score::move(const QString& cmd)
      {
      ChordRest* cr = sel->lastChordRest();
      if (cr) {
            Element* el = 0;
            if (cmd == "next-chord")
                  el = nextChordRest(cr);
            else if (cmd == "prev-chord")
                  el = prevChordRest(cr);
            else if (cmd == "next-measure")
                  el = nextMeasure(cr);
            else if (cmd == "prev-measure")
                  el = prevMeasure(cr);
            if (el) {
                  int tick = el->tick();
                  if (el->type() == CHORD)
                        el = ((Chord*)el)->upNote();
                  select(el, 0, 0);
                  adjustCanvasPosition(el);
                  if (canvas()->getState() == Canvas::NOTE_ENTRY) {
                        cis->pos = tick;
//                        moveCursor();
                        }
                  }
            }
      }
