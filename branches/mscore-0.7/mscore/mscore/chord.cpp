//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: chord.cpp,v 1.6 2006/03/28 14:58:58 wschweer Exp $
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

/**
 \file
 Implementation of classes Chord, LedgerLine, NoteList and Stem.
*/

#include "chord.h"
#include "note.h"
#include "xml.h"
#include "style.h"
#include "segment.h"
#include "text.h"
#include "measure.h"
#include "system.h"
#include "tuplet.h"
#include "hook.h"
#include "layout.h"
#include "slur.h"
#include "arpeggio.h"
#include "score.h"
#include "tremolo.h"

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

Stem::Stem(Score* s)
   : Element(s)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Stem::draw(QPainter& p)
      {
      qreal lw = point(score()->style()->stemWidth);
      QPen pen(p.pen());
      pen.setWidthF(lw);
      p.setPen(pen);

      p.drawLine(QLineF(0.0, 0.0, 0.0, point(_len)));
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Stem::setLen(const Spatium& l)
      {
      _len = l;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Stem::bbox() const
      {
      double w = point(score()->style()->stemWidth);
      double l = point(_len);
      return QRectF(-w * .5, 0, w, l);
      }

//---------------------------------------------------------
//   upLine
//---------------------------------------------------------

int Chord::upLine() const
      {
      return upNote()->line();
      }

//---------------------------------------------------------
//   downLine
//---------------------------------------------------------

int Chord::downLine() const
      {
      return downNote()->line();
      }

//---------------------------------------------------------
//   Chord
//---------------------------------------------------------

Chord::Chord(Score* s)
   : ChordRest(s)
      {
      _up            = true;
      _stem          = 0;
      _hook          = 0;
      _stemDirection = AUTO;
      _grace         = false;
      _arpeggio      = 0;
      _tremolo       = 0;
      }

//---------------------------------------------------------
//   ~Chord
//---------------------------------------------------------

Chord::~Chord()
      {
      if (_arpeggio)
            delete _arpeggio;
      if (_tremolo)
            delete _tremolo;
      }

//---------------------------------------------------------
//   setHook
//---------------------------------------------------------

void Chord::setHook(Hook* f)
      {
      if (_hook)
            delete _hook;
      _hook = f;
      if (_hook)
            _hook->setParent(this);
      }

//---------------------------------------------------------
//   setStem
//---------------------------------------------------------

void Chord::setStem(Stem* s)
      {
      if (_stem)
            delete _stem;
      _stem = s;
      if (_stem)
            _stem->setParent(this);
      }

//---------------------------------------------------------
//   stemPos
//---------------------------------------------------------

QPointF Chord::stemPos(bool upFlag, bool top) const
      {
      const Note* note = (top ? !upFlag : upFlag) ? downNote() : upNote();
      return note->stemPos(upFlag);
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Chord::setSelected(bool f)
      {
      Element::setSelected(f);
      NoteList* nl = noteList();
      for (iNote in = nl->begin(); in != nl->end(); ++in)
            in->second->setSelected(f);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Chord::add(Element* e)
      {
      e->setVoice(voice());
      e->setParent(this);
      e->setStaff(staff());
      if (e->type() == NOTE)
            notes.add((Note*)e);
      else if (e->type() == ATTRIBUTE)
            attributes.push_back((NoteAttribute*)e);
      else if (e->type() == ARPEGGIO)
            _arpeggio = (Arpeggio*)e;
      else if (e->type() == TREMOLO)
            _tremolo = (Tremolo*)e;
      else
            printf("Chord::add: unknown element\n");
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Chord::remove(Element* e)
      {
      if (e->type() == NOTE) {
            iNote i = notes.begin();
            for (; i != notes.end(); ++i) {
                  if (i->second == e) {
                        notes.erase(i);
                        break;
                        }
                  }
            if (i == notes.end())
                  printf("Chord::remove() note %p not found!\n", e);
            }
      else if (e->type() == ATTRIBUTE) {
            int idx = attributes.indexOf((NoteAttribute*)e);
            if (idx == -1)
                  printf("Chord::remove(): attribute not found\n");
            else {
                  attributes.removeAt(idx);
                  }
            }
      else if (e->type() == ARPEGGIO)
            _arpeggio = 0;
      else if (e->type() == TREMOLO)
            _tremolo = 0;
      else
            printf("Chord::remove: unknown element\n");
      }

//---------------------------------------------------------
//   drawPosMark
//---------------------------------------------------------

void drawPosMark(QPainter& painter, const QPointF& p)
      {
      qreal x = p.x();
      qreal y = p.y();
      painter.setPen(Qt::blue);
      painter.drawLine(QLineF(x-10, y-10, x+10, y+10));
      painter.drawLine(QLineF(x+10, y-10, x-10, y+10));
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Chord::bbox() const
      {
      QRectF _bbox;
      for (ciNote i = notes.begin(); i != notes.end(); ++i)
            _bbox |= i->second->bbox().translated(i->second->pos());
      foreach(LedgerLine* l, _ledgerLines)
            _bbox |= l->bbox().translated(l->pos());
      for (ciAttribute i = attributes.begin(); i != attributes.end(); ++i)
            _bbox |= (*i)->bbox().translated((*i)->pos());
      if (_hook)
            _bbox |= _hook->bbox().translated(_hook->pos());
      if (_stem)
            _bbox |= _stem->bbox().translated(_stem->pos());
      if (_arpeggio)
            _bbox |= _arpeggio->bbox().translated(_arpeggio->pos());
      return _bbox;
      }

//---------------------------------------------------------
//   layoutStem
//---------------------------------------------------------

/**
 Layout chord stem and hook.
*/

void Chord::layoutStem(ScoreLayout* layout)
      {
      double _spatium = layout->spatium();
      System* s      = segment()->measure()->system();
      if (s == 0)       //DEBUG
            return;
      double sy      = s->staff(staffIdx())->bbox().y();
      Note* upnote   = upNote();
      Note* downnote = downNote();

      double uppos   = s->staff(staffIdx() + upnote->move())->bbox().y();
            uppos    = (uppos - sy)/_spatium * 2.0 + upnote->line();

      double downpos = s->staff(staffIdx() + downnote->move())->bbox().y();
            downpos  = (downpos - sy)/_spatium * 2.0 + downnote->line();

      //-----------------------------------------
      //  process stem
      //-----------------------------------------

      bool up = isUp();

      int ticks = tuplet() ? tuplet()->baseLen() : tickLen();
      int hookIdx;
      Spatium stemLen;

      bool hasStem = true;
      if (_grace) {
            stemLen = Spatium(2.5);
            hookIdx = 1;
            }
      else {
            stemLen = Spatium((up ? uppos - 3 : 3 - downpos) / 2.0);
            if (stemLen < Spatium(3.5))
                  stemLen = Spatium(3.5);
            if (ticks < division/16)
                  hookIdx = 5;
            else if (ticks < division/8)
                  hookIdx = 4;
            else if (ticks < division/4)
                  hookIdx = 3;
            else if (ticks < division/2)
                  hookIdx = 2;
            else if (ticks < division)
                  hookIdx = 1;
            else if (ticks < division*2)
                  hookIdx = 0;
            else if (ticks < division*4)  // < 1/1
                  hookIdx = 0;
            else {
                  hookIdx = 0;
                  hasStem = false;
                  }
            }

      int extraStemLen = hookIdx - 2;
      if (_tremolo) {
            int extraStemLen2 = _tremolo->subtype();
            if (hookIdx == 0)
                  extraStemLen2 -= 1;
            if (extraStemLen2 > extraStemLen)
                  extraStemLen = extraStemLen2;
            }

      if (extraStemLen > 0)
            stemLen += extraStemLen *Spatium(0.5);

      double headCorrection = 0.2;

      stemLen        += Spatium((downpos - uppos) * .5 - headCorrection);
      double pstemLen = point(stemLen);
      QPointF npos    = stemPos(up, false);
      if (up)
            npos += QPointF(0, -pstemLen);

      if (hasStem) {
            if (!_stem) {
                  _stem = new Stem(score());
                  _stem->setParent(this);
                  }
            _stem->setLen(stemLen);
            _stem->setPos(npos);
            }
      else
            setStem(0);

      //-----------------------------------------
      //    process tremolo
      //-----------------------------------------

      if (_tremolo) {
            _tremolo->layout(layout);
            qreal x  = npos.x();
            if (!hasStem) {
                  // center tremolo above note
                  x = upnote->x() + upnote->headWidth() * .5;
                  }
            qreal y  = npos.y();
            qreal h  = pstemLen;
            qreal th = _tremolo->bbox().height();
            _tremolo->setPos(x, y + h * .5 - th * .5);
            }

      //-----------------------------------------
      //  process hook
      //-----------------------------------------

      if (hookIdx) {
            if (!up)
                  hookIdx = -hookIdx;
            if (!_hook) {
                  _hook = new Hook(score());
                  _hook->setParent(this);
                  }
            _hook->setIdx(hookIdx, _grace);
            qreal lw = point(score()->style()->stemWidth) * .5;
            QPointF p = npos + QPointF(lw, up ? -lw : pstemLen);
            _hook->setPos(p);
            }
      else
            setHook(0);
      }

//---------------------------------------------------------
//   addLedgerLine
//---------------------------------------------------------

void Chord::addLedgerLine(double x, double y, int i)
      {
      LedgerLine* h = new LedgerLine(score());
      h->setParent(this);
      double ho = 0.0;
      //
      // Experimental:
      //
      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            Note* n = in->second;
            if (n->line() >= (i-1) && n->line() <= (i+1) && n->accidental()) {
                  ho = _spatium * .25;
                  h->setLen(h->len() - Spatium(.25));
                  }
            }
      h->setPos(x + ho, y + _spatium * .5 * i);
      _ledgerLines.push_back(h);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Chord::layout(ScoreLayout* layout)
      {
      if (notes.empty())
            return;
      double _spatium = layout->spatium();
      Note* upnote     = notes.back();
      double headWidth = upnote->headWidth();

      computeUp();
      bool up = isUp();

      //-----------------------------------------
      //  process notes
      //    - position
      //-----------------------------------------

      int minMove = 1;
      int maxMove = -1;

      double lx = 0.0;
      System* s = segment()->measure()->system();
      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            Note* note = in->second;
            double x = 0.0;

            int move = note->move();
            if (move < minMove)
                  minMove = move;
            if (move > maxMove)
                  maxMove = move;
            double y = s->staff(staffIdx() + move)->bbox().y();
            y        -= s->staff(staffIdx())->bbox().y();
            y        += in->second->line() * _spatium * .5;

            if (note->mirror())
                  x += up ? headWidth : - headWidth;
            note->setPos(x, y);
            Accidental* accidental = note->accidental();
            if (accidental) {
#if 0
                  double x = - point(score()->style()->prefixNoteDistance);
                  if (_grace)
                        x /= 2;
                  x -= accidental->width();
                  if (note->mirror())
                        x -= headWidth;
                  accidental->setPos(x, 0);
#else
                  double x = accidental->x();
#endif
                  if (x < lx)
                        lx = x;
                  }
            }

      //-----------------------------------------
      //  process help lines
      //-----------------------------------------

      foreach(LedgerLine* l, _ledgerLines)
            delete l;
      _ledgerLines.clear();

      //---------------------------------------------------
      //    process help lines for notes
      //    moved to upper staff
      //---------------------------------------------------

      int uppos;
      int downpos;
      if (minMove == -1) {
            uppos   = 1000;
            downpos = -1000;
            for (iNote in = notes.begin(); in != notes.end(); ++in) {
                  if (in->second->move() == -1) {
                        if (in->second->line() < uppos)
                              uppos = in->second->line();
                        if (in->second->line() > downpos)
                              downpos = in->second->line();
                        }
                  }
            if (uppos < 0 || downpos >= 10) {
                  double x = upnote->pos().x();
                  x += headWidth/2 - _spatium;

                  double y = s->staff(staffIdx() - 1)->bbox().y();
                  y        -= s->staff(staffIdx())->bbox().y();

                  for (int i = -2; i >= uppos; i -= 2)
                        addLedgerLine(x, y, i);
                  for (int i = 10; i <= downpos; i += 2)
                        addLedgerLine(x, y, i);
                  }
            }

      uppos   = 1000;
      downpos = -1000;
      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            if (in->second->move() == 0) {
                  if (in->second->line() < uppos)
                        uppos = in->second->line();
                  if (in->second->line() > downpos)
                        downpos = in->second->line();
                  }
            }
      if (uppos < 0 || downpos >= 10) {
            double x = upnote->pos().x();
            x += headWidth/2 - _spatium;

            for (int i = -2; i >= uppos; i -= 2) {
                  addLedgerLine(x, 0.0, i);
                  }
            for (int i = 10; i <= downpos; i += 2) {
                  addLedgerLine(x, 0.0, i);
                  }
            }

      //---------------------------------------------------
      //    process help lines for notes
      //    moved to lower staff
      //---------------------------------------------------

      if (maxMove == 1) {
            uppos   = 1000;
            downpos = -1000;
            for (iNote in = notes.begin(); in != notes.end(); ++in) {
                  if (in->second->move() == 1) {
                        if (in->second->line() < uppos)
                              uppos = in->second->line();
                        if (in->second->line() > downpos)
                              downpos = in->second->line();
                        }
                  }
            if (uppos < 0 || downpos >= 10) {
                  double x = upnote->pos().x();
                  x += headWidth/2 - _spatium;

                  double y = s->staff(staffIdx() + 1)->bbox().y();
                  y        -= s->staff(staffIdx())->bbox().y();

                  for (int i = -2; i >= uppos; i -= 2)
                        addLedgerLine(x, y, i);
                  for (int i = 10; i <= downpos; i += 2)
                        addLedgerLine(x, 0.0, i);
                  }
            }

      foreach(LedgerLine* l, _ledgerLines)
            l->layout(layout);

      //-----------------------------------------
      //  Note Attributes
      //-----------------------------------------

      layoutAttributes(layout);

      //-----------------------------------------
      //  Fingering
      //-----------------------------------------

      for (iNote in = notes.begin(); in != notes.end(); ++in) {
            Note* note = in->second;
            QList<Text*>& fingering = note->fingering();
            double x = _spatium * 0.8 + note->headWidth();
            foreach(Text* f, fingering) {
                  f->setPos(x, 0.0);
                  // TODO: x += _spatium;
                  // if we have two fingerings and move the first,
                  // the second will also change position because their
                  // position in this list changes
                  }
            }
      if (_arpeggio) {
            double headHeight = upnote->headHeight();
            _arpeggio->layout(layout);
            lx -= _arpeggio->width() + _spatium * .5;
            double y = upNote()->pos().y() - headHeight * .5;
            double h = downNote()->pos().y() - y;
            _arpeggio->setHeight(h);
            _arpeggio->setPos(lx, y);
            }
      }

//-----------------------------------------------------------------------------
//   computeUp
//    rules:
//      single note:
//          All notes beneath the middle line: upward stems
//          All notes on or above the middle line: downward stems
//      two notes:
//          If the interval above the middle line is greater than the interval
//             below the middle line: downward stems
//          If the interval below the middle line is greater than the interval
//             above the middle line: upward stems
//          If the two notes are the same distance from the middle line:
//             stem can go in either direction. but most engravers prefer
//             downward stems
//       > two notes:
//          If the interval of the highest note above the middle line is greater
//             than the interval of the lowest note below the middle line:
//             downward stems
//          If the interval of the lowest note below the middle line is greater
//             than the interval of the highest note above the middle line:
//             upward stem
//          If the highest and the lowest notes are the same distance from
//          the middle line:, use these rules to determine stem direction:
//             - If the majority of the notes are above the middle:
//               downward stems
//             - If the majority of the notes are below the middle:
//               upward stems
//-----------------------------------------------------------------------------

void Chord::computeUp()
      {
      if (_stemDirection != AUTO) {
            _up = _stemDirection == UP;
            return;
            }
      if (_grace) {
            _up = true;
            return;
            }
      if (tickLen() >= division*4) {
            _up = true;
            return;
            }

      Note* upnote = upNote();
      if (notes.size() == 1) {
            if (upnote->move() > 0)
                  _up = true;
            else if (upnote->move() < 0)
                  _up = false;
            else
                  _up = upnote->line() > 4;
            return;
            }
      Note* downnote = downNote();
      int ud = upnote->line() - 4;
      int dd = downnote->line() - 4;
      if (-ud == dd) {
            int up = 0;
            for (ciNote in = notes.begin(); in != notes.end(); ++in) {
                  int l = in->second->line();
                  if (l <= 4)
                        --up;
                  else
                        ++up;
                  }
            _up = up > 0;
            }
      _up = dd > -ud;
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

int Chord::move() const
      {
      int move = notes.front()->move();
      for (ciNote in = notes.begin(); in != notes.end(); ++in) {
            if (in->second->move() != move)
                  return 0;
            }
      return move;
      }

//---------------------------------------------------------
//   selectedNote
//---------------------------------------------------------

Note* Chord::selectedNote() const
      {
      Note* note = 0;
      for (ciNote in = notes.begin(); in != notes.end(); ++in) {
            if (in->second->selected()) {
                  if (note)
                        return 0;
                  note = in->second;
                  }
            }
      return note;
      }

//---------------------------------------------------------
//   Chord::write
//---------------------------------------------------------

void Chord::write(Xml& xml) const
      {
      ciNote in = notes.begin();
      Note* note = in->second;

      if (ChordRest::isSimple(xml)
         && (notes.size() == 1)
         && note->isSimple(xml)
         && (_stemDirection == AUTO)
         && !_grace
         && !_arpeggio
         && !_tremolo
         ) {
            if (tick() != xml.curTick)
                  xml.tagE(QString("Note tick=\"%1\" pitch=\"%2\" tpc=\"%3\" ticks=\"%4\"")
                     .arg(tick()).arg(note->pitch()).arg(note->tpc()).arg(tickLen()));
            else
                  xml.tagE(QString("Note pitch=\"%1\" tpc=\"%2\" ticks=\"%3\"")
                     .arg(note->pitch()).arg(note->tpc()).arg(tickLen()));
            }
      else {
            xml.stag("Chord");
            ChordRest::writeProperties(xml);
            if (_grace)
                  xml.tag("GraceNote", _grace);
            switch(_stemDirection) {
                  case UP:   xml.tag("StemDirection", QVariant("up")); break;
                  case DOWN: xml.tag("StemDirection", QVariant("down")); break;
                  case AUTO: break;
                  }
            for (; in != notes.end(); ++in)
                  in->second->write(xml);
            if (_arpeggio)
                  _arpeggio->write(xml);
            if (_tremolo)
                  _tremolo->write(xml);
            xml.etag();
            }
      xml.curTick = tick() + tickLen();
      }

//---------------------------------------------------------
//   Chord::readNote
//---------------------------------------------------------

void Chord::readNote(QDomElement e, int staffIdx)
      {
      Note* note    = new Note(score());
      int ptch      = e.attribute("pitch", "-1").toInt();
      int ticks     = e.attribute("ticks", "-1").toInt();
      int tpc       = e.attribute("tpc", "-1").toInt();

      if (ticks != -1)
            setTickLen(ticks);

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "GraceNote")
                  _grace = i;
            else if (tag == "StemDirection") {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(i);
                  }
            else if (tag == "pitch")
                  note->setPitch(i);
            else if (tag == "prefix") {
                  printf("read Note:: prefix: TODO\n");
                  }
            else if (tag == "line")
                  note->setLine(i);
            else if (tag == "Tie") {
                  Tie* _tieFor = new Tie(score());
                  _tieFor->setStaff(staff());
                  _tieFor->read(e);
                  _tieFor->setStartNote(note);
                  note->setTieFor(_tieFor);
                  }
            else if (tag == "Text") {
                  Text* f = new Text(score());
                  f->setSubtype(TEXT_FINGERING);
                  f->setStaff(staff());
                  f->read(e);
                  f->setParent(this);
                  note->add(f);
                  }
            else if (tag == "move")
                  note->setMove(i);
            else if (tag == "Slur")
                  readSlur(e, staffIdx);
            else if (!ChordRest::readProperties(e))
                  domError(e);
            }
      note->setParent(this);
      note->setGrace(_grace);
      note->setStaff(staff());
      note->setVoice(voice());
      if (ptch != -1)
            note->setPitch(ptch);
      if (tpc != -1)
            note->setTpc(tpc);
      notes.add(note);
      }

//---------------------------------------------------------
//   Chord::read
//---------------------------------------------------------

void Chord::read(QDomElement e, int staffIdx)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "Note") {
                  Note* note = new Note(score());
                  note->setParent(this);
                  note->setGrace(_grace);
                  note->setStaff(staff());
                  note->setVoice(voice());
                  note->read(e);
                  notes.add(note);
                  }
            else if (tag == "GraceNote")
                  _grace = i;
            else if (tag == "StemDirection") {
                  if (val == "up")
                        _stemDirection = UP;
                  else if (val == "down")
                        _stemDirection = DOWN;
                  else
                        _stemDirection = Direction(i);
                  }
            else if (tag == "Arpeggio") {
                  _arpeggio = new Arpeggio(score());
                  _arpeggio->setStaff(staff());
                  _arpeggio->read(e);
                  _arpeggio->setParent(this);
                  }
            else if (tag == "Tremolo") {
                  _tremolo = new Tremolo(score());
                  _tremolo->setStaff(staff());
                  _tremolo->read(e);
                  _tremolo->setParent(this);
                  }
            else if (ChordRest::readProperties(e))
                  ;
            else if (tag == "Slur") {
                  readSlur(e, staffIdx);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Chord::dump() const
      {
      printf("Chord tick %d  len %d\n", tick(), tickLen());
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

void Chord::space(double& min, double& extra) const
      {
      extra         = 0.0; // point(score()->style()->minNoteDistance);
      double mirror = 0.0;
      double hw     = 0.0;

      if (_arpeggio)
            extra = _arpeggio->width() + _spatium * .5;

      for (ciNote i = notes.begin(); i != notes.end(); ++i) {
            Note* note = i->second;
            double lhw = note->headWidth();
            if (lhw > hw)
                  hw = lhw;
            double prefixWidth  = 0.0;
            if (note->accidental()) {
//                  prefixWidth = note->accidental()->width();
                  prefixWidth = -note->accidental()->x() - 1.3;   // HACK
                  if (_grace)
                       prefixWidth += point(score()->style()->prefixNoteDistance)/2.0;
                  else
                       prefixWidth += point(score()->style()->prefixNoteDistance);
                  if (prefixWidth > extra)
                        extra += prefixWidth;
                  }
            if (note->mirror()) {
                  if (isUp()) {
                        // note head on the right side of stem
                        mirror = lhw;
                        }
                  else {
                        // note head on left side of stem
                        if ((lhw + prefixWidth) > extra)
                              extra += lhw + prefixWidth;
                        }
                  }
            }
      min = mirror + hw;
      if (isUp() && _hook)
            min += _hook->width();
      }

//---------------------------------------------------------
//   find
//---------------------------------------------------------

Note* NoteList::find(int pitch) const
      {
      ciNote i = std::multimap<const int, Note*>::find(pitch);
      if (i != end())
            return i->second;
      return 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

NoteList::iterator NoteList::add(Note* n)
      {
      return std::multimap<const int, Note*>::insert(std::pair<const int, Note*> (n->pitch(), n));
      }

//---------------------------------------------------------
//   upPos
//---------------------------------------------------------

qreal Chord::upPos() const
      {
      return upNote()->pos().y();
      }

//---------------------------------------------------------
//   downPos
//---------------------------------------------------------

qreal Chord::downPos() const
      {
      return downNote()->pos().y();
      }

//---------------------------------------------------------
//   centerX
//---------------------------------------------------------

qreal Chord::centerX() const
      {
      qreal x;
      if (_up) {
            const Note* upnote = upNote();
            x  = upnote->pos().x();
            x += upnote->headWidth() * .5;
            }
      else {
            const Note* downnote = downNote();
            x = downnote->pos().x();
            x += downnote->headWidth() * .5;
            }
      return x;
      }

//---------------------------------------------------------
//   LedgerLine
//---------------------------------------------------------

LedgerLine::LedgerLine(Score* s)
   : Line(s, false)
      {
      setLineWidth(score()->style()->ledgerLineWidth);
      setLen(Spatium(2));
      }
