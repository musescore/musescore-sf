//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: slur.cpp,v 1.53 2006/03/28 14:58:58 wschweer Exp $
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

#include "note.h"
#include "chord.h"
#include "xml.h"
#include "slur.h"
#include "measure.h"
#include "utils.h"
#include "score.h"
#include "preferences.h"
#include "system.h"
#include "segment.h"
#include "staff.h"
#include "layout.h"
#include "viewer.h"
#include "navigate.h"
#include "canvas.h"
#include "articulation.h"

//---------------------------------------------------------
//   SlurSegment
//---------------------------------------------------------

SlurSegment::SlurSegment(Score* score)
   : Element(score)
      {
      _segmentType = SEGMENT_SINGLE;
      _system      = 0;
      }

SlurSegment::SlurSegment(const SlurSegment& b)
   : Element(b)
      {
      for (int i = 0; i < 4; ++i)
            ups[i] = b.ups[i];
      path         = b.path;
      bow          = b.bow;
      _segmentType = b._segmentType;
      _system      = b._system;
      }

//---------------------------------------------------------
//   updatePath
//---------------------------------------------------------

void SlurSegment::updatePath()
      {
      QPointF pp[4];
      for (int i = 0; i < 4; ++i)
            pp[i] = ups[i].pos();
      path = QPainterPath();
      QPointF t(0.0, _spatium * .08);    // thickness of slur
      path.moveTo(pp[0]);
      path.cubicTo(pp[1]-t, pp[2]-t, pp[3]);
      path.cubicTo(pp[2]+t, pp[1]+t, pp[0]);
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void SlurSegment::move(const QPointF& s)
      {
      _pos += s;
      for (int k = 0; k < 4; ++k)
            ups[k].p += s;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SlurSegment::draw(QPainter& p) const
      {
      p.setBrush(curColor());
      p.drawPath(path);
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void SlurSegment::updateGrips(int* n, QRectF* r) const
      {
      *n = 4;
      QPointF p(canvasPos());
      for (int i = 0; i < 4; ++i)
            r[i].translate(ups[i].pos() + p);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool SlurSegment::startEdit(Viewer*, const QPointF&)
      {
      return true;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool SlurSegment::edit(Viewer*, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString& s)
      {
      if (slurTie()->type() != SLUR)
            return false;
      Slur* sl = static_cast<Slur*>(slurTie());

      if (key == Qt::Key_X) {
            sl->setSlurDirection(sl->isUp() ? DOWN : UP);
            sl->layout(score()->layout());
            return true;
            }
      if (!((modifiers & Qt::ShiftModifier)
         && ((_segmentType == SEGMENT_SINGLE)
              || (_segmentType == SEGMENT_BEGIN && curGrip == 0)
              || (_segmentType == SEGMENT_END && curGrip == 3)
            )))
            return false;

      int segments  = sl->slurSegments()->size();
      ChordRest* cr = 0;
      Element* e    = curGrip == 0 ? sl->startElement() : sl->endElement();
      Element* e1   = curGrip == 0 ? sl->endElement() : sl->startElement();

      if (key == Qt::Key_Left)
            cr = prevChordRest((ChordRest*)e);
      else if (key == Qt::Key_Right)
            cr = nextChordRest((ChordRest*)e);

      if (cr == 0 || cr == (ChordRest*)e1)
            return true;
      if (curGrip == 0) {
            ((ChordRest*)sl->startElement())->removeSlurFor(sl);
            sl->setStartElement(cr);
            cr->addSlurFor(sl);
            }
      else {
            ((ChordRest*)sl->endElement())->removeSlurBack(sl);
            sl->setEndElement(cr);
            cr->addSlurBack(sl);
            }

      ups[curGrip].off = QPointF();
      sl->layout(score()->layout());
      if (sl->slurSegments()->size() != segments) {
            QList<SlurSegment*>* ss = sl->slurSegments();
            SlurSegment* newSegment = curGrip == 3 ? ss->back() : ss->front();
            score()->canvas()->setState(Canvas::NORMAL);
            score()->endCmd();
            score()->startCmd();
            score()->canvas()->startEdit(newSegment, curGrip);
            score()->setLayoutAll(true);
            }
      return true;
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF SlurSegment::gripAnchor(int grip) const
      {
      Slur* sl = (Slur*) slurTie();

      QPointF sp(_system->canvasPos());
      System* s;  // dummy
      switch(_segmentType) {
            case SEGMENT_SINGLE:
                  if (grip == 0)
                        return sl->slurPos(sl->startElement(), s);
                  else if (grip == 3)
                        return sl->slurPos(sl->endElement(), s);
                  return QPointF();
            case SEGMENT_BEGIN:
                  if (grip == 0)
                        return sl->slurPos(sl->startElement(), s);
                  else if (grip == 3)
                        return _system->abbox().topRight();
                  break;
            case SEGMENT_MIDDLE:
                  if (grip == 0)
                        return sp;
                  else if (grip == 3)
                        return _system->abbox().topRight();
                  break;
            case SEGMENT_END:
                  if (grip == 0)
                        return sp;
                  else if (grip == 3)
                        return sl->slurPos(sl->endElement(), s);
                  break;
            }
      return QPointF();
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void SlurSegment::editDrag(int curGrip, const QPointF& delta)
      {
      ups[curGrip].off += (delta / _spatium);

      if (curGrip == 0 || curGrip == 3) {
            //
            //  compute bezier help points
            //
            QPointF p0 = ups[0].pos();
            QPointF p3 = ups[3].pos();

            qreal xdelta = p3.x() - p0.x();
            if (xdelta == 0.0) {
                  printf("bad slur slope\n");
                  return;
                  }

            qreal d    = xdelta / 4.0;
            qreal x1   = p0.x() + d;
            qreal x2   = p3.x() - d;

            qreal slope = (p3.y() - p0.y()) / xdelta;
            qreal y1    = p0.y() + (x1-p0.x()) * slope + bow;
            qreal y2    = p0.y() + (x2-p0.x()) * slope + bow;
            ups[1].p    = QPointF(x1, y1);
            ups[2].p    = QPointF(x2, y2);
            }
      updatePath();
      }

//---------------------------------------------------------
//    bbox
//---------------------------------------------------------

QRectF SlurSegment::bbox() const
      {
      QRectF r(path.boundingRect());
      return r;
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath SlurSegment::shape() const
      {
      return path;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurSegment::write(Xml& xml, int no) const
      {
      bool empty = true;
      for (int i = 0; i < 4; ++i) {
            if (!(ups[i].off.isNull())) {
                  empty = false;
                  break;
                  }
            }
      if (!_userOff.isNull())
            empty = false;
      if (empty)
            return;

      xml.stag(QString("SlurSegment no=\"%1\"").arg(no));
      if (!_userOff.isNull())
            xml.tag("offset", _userOff);
      if (!(ups[0].off.isNull()))
            xml.tag("o1", ups[0].off);
      if (!(ups[1].off.isNull()))
            xml.tag("o2", ups[1].off);
      if (!(ups[2].off.isNull()))
            xml.tag("o3", ups[2].off);
      if (!(ups[3].off.isNull()))
            xml.tag("o4", ups[3].off);
      xml.etag();
      }

//---------------------------------------------------------
//   readSegment
//---------------------------------------------------------

void SlurSegment::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "o1")
                  ups[0].off = readPoint(e);
            else if (tag == "o2")
                  ups[1].off = readPoint(e);
            else if (tag == "o3")
                  ups[2].off = readPoint(e);
            else if (tag == "o4")
                  ups[3].off = readPoint(e);
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SlurSegment::layout(ScoreLayout*, const QPointF& p1, const QPointF& p2, qreal b)
      {
// printf("SlurSegment %p %p layout\n", slur, this);
      bow      = b;
      ups[0].p = p1;
      ups[3].p = p2;

      //
      //  compute bezier help points
      //
      qreal x0 = ups[0].pos().x();
      qreal x3 = ups[3].pos().x();
      qreal y0 = ups[0].pos().y();
      qreal y3 = ups[3].pos().y();

      qreal xdelta = x3 - x0;
      if (xdelta == 0.0) {
            printf("warning: slur has zero width at %d-%d\n",
               slurTie()->startElement()->tick(), slurTie()->endElement()->tick());
            return;
            }

      qreal d  = xdelta / 4.0;
      qreal x1 = x0 + d;
      qreal x2 = x3 - d;

      double maxBow = xdelta * .4;
      if (fabs(bow) > maxBow) {          // limit bow for small slurs
            if (bow > 0.0)
                  bow = maxBow;
            else
                  bow = -maxBow;
            }
      qreal slope = (y3 - y0) / xdelta;

      qreal y1 = y0 + (x1-x0) * slope + bow;
      qreal y2 = y0 + (x2-x0) * slope + bow;

      ups[1].p = QPointF(x1, y1);
      ups[2].p = QPointF(x2, y2);
      updatePath();
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void SlurSegment::dump() const
      {
      printf("SlurSegment %f/%f %f/%f %f/%f %f/%f\n",
            ups[0].off.x(), ups[0].off.y(),
            ups[1].off.x(), ups[1].off.y(),
            ups[2].off.x(), ups[2].off.y(),
            ups[3].off.x(), ups[3].off.y());
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool SlurSegment::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Edit Mode"));
      a->setData("edit");
      return true;
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::SlurTie(Score* s)
   : Element(s)
      {
      _slurDirection = AUTO;
      up             = true;
      _startElement  = 0;
      _endElement    = 0;
      }

SlurTie::SlurTie(const SlurTie& t)
   : Element(t)
      {
      up             = t.up;
      _slurDirection = t._slurDirection;
      _startElement  = t._startElement;
      _endElement    = t._endElement;
      delSegments    = t.delSegments;

      //
      // duplicate segments
      //
      foreach(const Element* s, t.segments) {
            SlurSegment* ss = new SlurSegment(*(const SlurSegment*)s);
            ss->setParent(this);
            segments.append(ss);
            }
      }

//---------------------------------------------------------
//   SlurTie
//---------------------------------------------------------

SlurTie::~SlurTie()
      {
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void SlurTie::add(Element* s)
      {
//      printf("SlurTie: %d vor add %p\n", segments.size(), s);
      s->setParent(this);
      segments.push_back((SlurSegment*)s);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SlurTie::remove(Element* s)
      {
      int idx = segments.indexOf((SlurSegment*)s);
      if (idx == -1)
            printf("SlurTie: cannot remove %p\n", s);
      else
            segments.removeAt(idx);
//      printf("SlurTie: %d nach remove %p\n", segments.size(), s);
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void SlurTie::change(Element* o, Element* n)
      {
      int idx = segments.indexOf((SlurSegment*)o);
      if (idx == -1) {
            printf("SlurTie: cannot change %p\n", o);
            return;
            }
      n->setParent(this);
      segments[idx] = (SlurSegment*)n;
      }

//---------------------------------------------------------
//   slurPos
//---------------------------------------------------------

QPointF SlurTie::slurPos(Element* e, System*& s)
      {
      ChordRest* cr;
      if (e->type() == NOTE)
            cr = static_cast<Note*>(e)->chord();
      else
            cr = static_cast<ChordRest*>(e);
      s = cr->measure()->system();

      //-----------------------------------------
      //    off
      //-----------------------------------------

      qreal xo = cr->width() * .5;
      qreal yo = 0.0;
      if (cr->type() == CHORD) {
            Chord* c = static_cast<Chord*>(cr);
            Stem* stem = c->stem();
            Beam* beam = c->beam();

            Chord* sc;
            if (startElement()->type() == NOTE)
                  sc = ((Note*)startElement())->chord();
            else
                  sc = (Chord*)startElement();

            bool startIsGrace = false;
            if (sc->type() == CHORD && sc->noteType() != NOTE_NORMAL)
                  startIsGrace = true;
            bool mainNoteOfGraceSlur = false;
            if (startIsGrace && c == endElement() && c->noteType() == NOTE_NORMAL)
                  mainNoteOfGraceSlur = true;

            if (up) {
                  yo = c->upNote()->pos().y() - c->upNote()->headHeight();
                  //
                  // handle special case of tenuto and staccato;
                  // should be generalized
                  //
                  QList<Articulation*>* al = c->getArticulations();
                  if (al->size() == 1) {
                        Articulation* a = al->at(0);
                        if (a->subtype() == TenutoSym || a->subtype() == StaccatoSym) {
                              yo = a->y() - _spatium * .5;
                              }
                        }
                  if (c->isUp() && stem) {
                        if (beam && !mainNoteOfGraceSlur)
                              yo = c->downNote()->pos().y() - stem->height() - _spatium;
                        else if (!startIsGrace) {
                              // slurs on the stem side of stemmed notes start half
                              // of a staff space from the end of the stem
                              yo = c->downNote()->pos().y() - stem->height() + _spatium * .5;
                              }
                        }
                  }
            else {
                  yo = c->downNote()->pos().y() + c->downNote()->headHeight();
                  //
                  // handle special case of tenuto and staccato;
                  // should be generalized
                  //
                  QList<Articulation*>* al = c->getArticulations();
                  if (al->size() == 1) {
                        Articulation* a = al->at(0);
                        if (a->subtype() == TenutoSym || a->subtype() == StaccatoSym) {
                              yo = a->y() + a->height() + _spatium * .5;
                              }
                        }
                  if (!c->isUp() && stem) {
                        if (beam && !mainNoteOfGraceSlur)
                              yo = c->upNote()->pos().y() + stem->height() + _spatium;
                        else if (!startIsGrace) {
                              // slurs on the stem side of stemmed notes start half
                              // of a staff space from the end of the stem
                              yo = c->upNote()->pos().y() + stem->height() - _spatium * .5;
                              }
                        }
                  }
            if (up == c->isUp() && stem) {
                  if (beam && !mainNoteOfGraceSlur) {
                        // for beamed notes and slurs on stem side, slurs are aligned
                        // to the stem rather than the middle of the notehead
                        xo = stem->pos().x();
                        if (cr == startElement())
                              xo += stem->width() * .5;
                        else
                              xo -= stem->width() * .5;
                        }
                  else {
                        if (cr == startElement()) {
                              // don't collide with start stem
                              xo = stem->pos().x() + _spatium * .5;
                              }
                        if (cr == endElement()) {
                              // don't collide with end stem
                              xo = stem->pos().x() - _spatium * .5;
                              }
                        }
                  }
            }
      QPointF off(xo, yo);

      //-----------------------------------------
      //    p
      //-----------------------------------------

      return cr->canvasPos() + off;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SlurTie::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      int idx = 0;
      foreach(SlurSegment* ss, segments)
            ss->write(xml, idx++);
      if (_slurDirection)
            xml.tag("up", _slurDirection);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void SlurTie::setSelected(bool f)
      {
      foreach(SlurSegment* ss, segments)
            ss->setSelected(f);
      Element::setSelected(f);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SlurTie::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());

      if (tag == "SlurSegment") {
            SlurSegment* segment = new SlurSegment(score());
            segment->read(e);
            add(segment);
            }
      else if (tag == "up")
            _slurDirection = Direction(val.toInt());
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void SlurTie::collectElements(QList<const Element*>& el) const
      {
      foreach(const SlurSegment* seg, segments)
            el.append(seg);
      }

//---------------------------------------------------------
//   resetUserOffsets
//---------------------------------------------------------

void SlurTie::resetUserOffsets()
      {
      setUserOff(QPointF());
      }

void SlurSegment::resetUserOffsets()
      {
      setUserOff(QPointF());
      for (int i = 0; i < 4; ++i)
            ups[i].off = QPointF();
      parent()->resetUserOffsets();
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::Slur(Score* s)
   : SlurTie(s)
      {
      setTick(0);
      _tick2  = 0;
      _track2 = 0;
      }

//---------------------------------------------------------
//   Slur
//---------------------------------------------------------

Slur::~Slur()
      {
      foreach(SlurSegment* ss, segments)
            delete ss;
      }

//---------------------------------------------------------
//   setTick2
//---------------------------------------------------------

void Slur::setTick2(int val)
      {
      if (val != -1)
            _tick2 = val;
      }

//---------------------------------------------------------
//   setStart
//---------------------------------------------------------

void Slur::setStart(int t, int track)
      {
      setTick(t);
      setTrack(track);
      }

//---------------------------------------------------------
//   setEnd
//---------------------------------------------------------

void Slur::setEnd(int t, int track)
      {
      _tick2  = t;
      _track2 = track;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Slur::write(Xml& xml) const
      {
      xml.stag(QString("Slur id=\"%1\"").arg(_id + 1));
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Slur::read(QDomElement e)
      {
      setTrack(0);      // set staff
      _id = e.attribute("id").toInt() - 1;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "tick2")
                  _tick2 = score()->fileDivision(i);
            else if (tag == "track2")
                  _track2 = i;
            else if (tag == "startTick")        // obsolete
                  setTick(i);
            else if (tag == "endTick")          // obsolete
                  setTick2(i);
            else if (tag == "startTrack")       // obsolete
                  setTrack(i);
            else if (tag == "endTrack")         // obsolete
                  setTrack2(i);
            else if (!SlurTie::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   chordsHaveTie
//---------------------------------------------------------
static bool chordsHaveTie (Chord* c1, Chord* c2)
      {
      NoteList* nl1 = c1->noteList();
      NoteList* nl2 = c2->noteList();
      for (iNote in1 = nl1->begin(); in1 != nl1->end(); ++in1) {
            for (iNote in2 = nl2->begin(); in2 != nl2->end(); ++in2) {
                  if (in1->second->tieFor() && in1->second->tieFor() == in2->second->tieBack())
                        return true;
                  }
            }
      return false;
      }

//---------------------------------------------------------
//   directionMixture
//---------------------------------------------------------

static bool isDirectionMixture (Chord* c1, Chord* c2)
      {
      bool up = c1->isUp();
      for (Segment* seg = c1->segment(); seg; seg = seg->next()) {
            if (seg->subtype() == Segment::SegChordRest) {
                  Element* e = seg->element(c1->track());
                  if (!e)
                        continue;
                  Chord* c = 0;
                  if (e->type() == NOTE)
                        c = static_cast<Note*>(e)->chord();
                  else if (e->type() == CHORD)
                        c = static_cast<Chord*>(e);
                  else
                        continue;
                  if (c && c->isUp() != up)
                        return true;
                  }
            if (seg == c2->segment())
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Slur::layout(ScoreLayout* layout)
      {
      double _spatium = layout->spatium();
      switch (_slurDirection) {
            case UP:    up = true; break;
            case DOWN:  up = false; break;
            case AUTO:
                  {
                  ChordRest* cr1 = (ChordRest*)startElement();
                  ChordRest* cr2 = (ChordRest*)endElement();
                  Measure* m1    = cr1->measure();

                  Chord* c1;
                  if (cr1->type() == NOTE)
                        c1 = ((Note*)cr1)->chord();
                  else
                        c1 = (Chord*)cr1;
                  Chord* c2;
                  if (cr2->type() == NOTE)
                        c2 = ((Note*)cr2)->chord();
                  else
                        c2 = (Chord*)cr2;

                  up = !(cr1->isUp());

                  if ((cr2->tick() - cr1->tick()) > m1->tickLen()) {
                        // long slurs are always above
                        up = true;
                        }
                  if (cr1->type() == CHORD && cr2->type() == CHORD) {
                        if (isDirectionMixture(c1, c2) && c1->noteType() == NOTE_NORMAL) {
                              // slurs go above if start and end note have different stem directions,
                              // but grace notes are exceptions
                              up = true;
                              }
                        else if (m1->mstaff(cr1->staffIdx())->hasVoices && c1->noteType() == NOTE_NORMAL) {
                              // in polyphonic passage, slurs go on the stem side
                              up = cr1->isUp();
                              }
                        else if (chordsHaveTie(c1, c2)) {
                              // could confuse slur with tie, put slur on stem side
                              up = cr1->isUp();
                              }
                        }
                  }
                  break;
            }

      System *s1, *s2;
      QPointF p1 = slurPos(startElement(), s1);
      QPointF p2 = slurPos(endElement(), s2);

      QList<System*>* sl = layout->systems();
      iSystem is = sl->begin();
      while (is != sl->end()) {
            if (*is == s1)
                  break;
            ++is;
            }
      setPos(0, 0);

      //---------------------------------------------------------
      //   count number of segments, if no change, all
      //    user offsets (drags) are retained
      //---------------------------------------------------------

      unsigned nsegs = 1;
      for (iSystem iis = is; iis != sl->end(); ++iis) {
            if (*iis == s2)
                  break;
            ++nsegs;
            }

      unsigned onsegs = segments.size();
      if (nsegs > onsegs) {
            for (unsigned i = onsegs; i < nsegs; ++i) {
                  SlurSegment* s;
                  if (!delSegments.isEmpty()) {
                        s = delSegments.dequeue();
                        }
                  else
                        s = new SlurSegment(score());
                  s->setTrack(track());
                  add(s);
                  }
            }
      else if (nsegs < onsegs) {
            for (unsigned i = nsegs; i < onsegs; ++i) {
                  SlurSegment* s = (SlurSegment*)(segments.takeLast());
                  delSegments.enqueue(s);  // cannot delete: used in SlurSegment->edit()
                  }
            }

      qreal bow = up ? 2.0 * -_spatium : 2.0 * _spatium;
      for (int i = 0; is != sl->end(); ++i, ++is) {
            System* system  = *is;
            SlurSegment* segment = segments[i];
            segment->setSystem(system);
            QPointF sp(system->canvasPos());

            // case 1: one segment
            if (s1 == s2) {
                  segment->layout(layout, p1, p2, bow);
                  segment->setLineSegmentType(SEGMENT_SINGLE);
                  }
            // case 2: start segment
            else if (i == 0) {
                  qreal x = sp.x() + system->bbox().width();
                  segment->layout(layout, p1, QPointF(x, p1.y()), bow);
                  segment->setLineSegmentType(SEGMENT_BEGIN);
                  }
            // case 3: middle segment
            else if (i != 0 && system != s2) {
                  qreal x1 = firstNoteRestSegmentX(system) - _spatium;
                  qreal x2 = sp.x() + system->bbox().width();
                  segment->layout(layout, QPointF(x1, sp.y()), QPointF(x2, sp.y()), bow);
                  segment->setLineSegmentType(SEGMENT_MIDDLE);
                  }
            // case 4: end segment
            else {
                  //qreal x = sp.x();
                  qreal x = firstNoteRestSegmentX(system) - _spatium;
                  segment->layout(layout, QPointF(x, p2.y()), p2, bow);
                  segment->setLineSegmentType(SEGMENT_END);
                  }
            if (system == s2)
                  break;
            }
      }

//---------------------------------------------------------
//   firstNoteRestSegmentX
//---------------------------------------------------------

double SlurTie::firstNoteRestSegmentX(System* system)
      {
      foreach(const MeasureBase* mb, system->measures()) {
            if (mb->type() == MEASURE) {
                  Measure* measure = (Measure*)mb;
                  for (Segment* seg = measure->first(); seg; seg = seg->next()) {
                        if (seg->subtype() == Segment::SegChordRest) {
                              return seg->canvasPos().x();
                              }
                        }
                  }
            }
      printf("firstNoteRestSegmentX: did not find segment\n");
      return 0.0;
      }

//---------------------------------------------------------
//   bbox
//    DEBUG: needed?
//---------------------------------------------------------

QRectF Slur::bbox() const
      {
      QRectF r;
      foreach(SlurSegment* ss, segments)
            r |= ss->abbox().translated(canvasPos());
      return r;
      }

//---------------------------------------------------------
//   setTrack
//---------------------------------------------------------

void Slur::setTrack(int n)
      {
      Element::setTrack(n);
      foreach(SlurSegment* ss, segments)
            ss->setTrack(n);
      }

//---------------------------------------------------------
//   Tie
//---------------------------------------------------------

Tie::Tie(Score* s)
   : SlurTie(s)
      {
      }

//---------------------------------------------------------
//   setStartNote
//---------------------------------------------------------

void Tie::setStartNote(Note* note)
      {
      setStartElement(note);
      setParent(note);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tie::write(Xml& xml) const
      {
      xml.stag("Tie");
      SlurTie::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tie::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (Element::readProperties(e))
                  ;
            else if (SlurTie::readProperties(e))
                  ;
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Tie::layout(ScoreLayout* layout)
      {
      //
      // TODO: if there is a startNote but no endNote
      //    show short bow
      if (startElement() == 0 || endElement() == 0)
            return;

      double _spatium = layout->spatium();

      Chord* c1   = startNote()->chord();
      Measure* m1 = c1->measure();
      System* s1  = m1->system();
      Chord* c2   = endNote()->chord();
      Measure* m2 = c2->measure();
      System* s2  = m2->system();

      if (_slurDirection == AUTO)
            if (m1->mstaff(c1->staffIdx())->hasVoices) {
                  // in polyphonic passage, ties go on the stem side
                  up = c1->isUp();
                  }
            else
                  up = !(c1->isUp());
      else
            up = _slurDirection == UP ? true : false;
      qreal w   = startNote()->headWidth();
      qreal xo1 = w * 1.12;
      qreal h   = w * 0.3;
      qreal yo  = up ? -h : h;

      QPointF off1(xo1, yo);
      QPointF off2(0.0, yo);

      QPointF ppos(canvasPos());
      QPointF p1 = startNote()->canvasPos() + off1;
      QPointF p2 = endNote()->canvasPos()   + off2;

      QList<System*>* systems = layout->systems();
      setPos(0, 0);

      //---------------------------------------------------------
      //   count number of segments, if no change, all
      //    user offsets (drags) are retained
      //---------------------------------------------------------

      int sysIdx1      = systems->indexOf(s1);
      int sysIdx2      = systems->indexOf(s2, sysIdx1);
      unsigned nsegs   = sysIdx2 - sysIdx1 + 1;
      unsigned onsegs  = segments.size();


      if (nsegs != onsegs) {
            if (nsegs > onsegs) {
                  int n = nsegs - onsegs;
                  for (int i = 0; i < n; ++i) {
                        SlurSegment* s = new SlurSegment(score());
                        s->setParent(this);
                        segments.append(s);
                        }
                  }
            else {
                  int n = onsegs - nsegs;
                  for (int i = 0; i < n; ++i) {
                        /* LineSegment* seg = */ segments.takeLast();
                        // delete seg;   // DEBUG: will be used later
                        }
                  }
            }

      qreal bow = up ? -_spatium : _spatium;

      p1 -= canvasPos();
      p2 -= canvasPos();
      for (unsigned int i = 0; i < nsegs; ++i) {
            System* system       = (*systems)[sysIdx1++];
            SlurSegment* segment = segments[i];
            segment->setSystem(system);
            QPointF sp(system->canvasPos() - canvasPos());

            // case 1: one segment
            if (s1 == s2) {
                  segment->layout(layout, p1, p2, bow);
                  segment->setLineSegmentType(SEGMENT_SINGLE);
                  }
            // case 2: start segment
            else if (i == 0) {
                  qreal x = sp.x() + system->bbox().width();
                  segment->layout(layout, p1, QPointF(x, p1.y()), bow);
                  segment->setLineSegmentType(SEGMENT_BEGIN);
                  }
            // case 3: middle segment
            else if (i != 0 && system != s2) {
                  // cannot happen
                  printf("sysIdx %d - %d\n", sysIdx1, sysIdx2);
                  Measure* m1 = c1->measure();
                  Measure* m2 = c2->measure();
                  printf("Measure %d - %d, %d %d\n", m1->no(), m2->no(), m1->tick(), m2->tick());
                  // abort();
                  }
            // case 4: end segment
            else {
                  // qreal x = sp.x();
                  qreal x = firstNoteRestSegmentX(system) - 2 * _spatium - canvasPos().x();

                  segment->layout(layout, QPointF(x, p2.y()), p2, bow);
                  segment->setLineSegmentType(SEGMENT_END);
                  }
            }
      }
