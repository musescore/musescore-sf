//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: beam.cpp,v 1.41 2006/09/15 09:34:57 wschweer Exp $
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

#include "beam.h"
#include "segment.h"
#include "score.h"
#include "chord.h"
#include "preferences.h"
#include "sig.h"
#include "style.h"
#include "note.h"
#include "tuplet.h"
#include "layout.h"

//---------------------------------------------------------
//   Beam
//---------------------------------------------------------

Beam::~Beam()
      {
      //
      // delete all references from chords
      //
      foreach(ChordRest* cr, elements)
            cr->setBeam(0);
      for (iBeamSegment i = beamSegments.begin();
         i != beamSegments.end(); ++i) {
            delete *i;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Beam::remove(ChordRest* a)
      {
      int idx = elements.indexOf(a);
      if (idx != -1)
            elements.removeAt(idx);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Beam::draw(QPainter& p) const
      {
      p.setPen(QPen(Qt::NoPen));
      p.setBrush(selected() ? preferences.selectColor[0] : Qt::black);

      for (ciBeamSegment ibs = beamSegments.begin();
         ibs != beamSegments.end(); ++ibs) {
            BeamSegment* bs = *ibs;

            QPointF ip1 = bs->p1;
            QPointF ip2 = bs->p2;
            qreal lw2   = point(score()->style()->beamWidth) * .5;

            QPolygonF a(4);
            a[0] = QPointF(ip1.x(), ip1.y()-lw2);
            a[1] = QPointF(ip2.x(), ip2.y()-lw2);
            a[2] = QPointF(ip2.x(), ip2.y()+lw2);
            a[3] = QPointF(ip1.x(), ip1.y()+lw2);
            p.drawPolygon(a);
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Beam::move(double x, double y)
      {
      Element::move(x, y);
      for (ciBeamSegment ibs = beamSegments.begin();
         ibs != beamSegments.end(); ++ibs) {
            BeamSegment* bs = *ibs;
            bs->move(x, y);
            }
      }

//---------------------------------------------------------
//   layoutBeams1
//    auto - beamer
//    called before layout spacing of notes
//---------------------------------------------------------

void Measure::layoutBeams1(ScoreLayout* layout)
      {
      foreach(Beam* beam, _beamList)
            delete beam;
      _beamList.clear();

      int tracks = _score->nstaves() * VOICES;
      for (int track = 0; track < tracks; ++track) {
            ChordRest* a1 = 0;      // start of (potential) beam
            Beam* beam    = 0;
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);
                  if (e == 0)
                        continue;
                  if (e->type() == CLEF) {
                        if (beam) {
                              beam->layout1(layout);
                              beam = 0;
                              }
                        if (a1) {
                              a1->layoutStem1(layout);
                              a1 = 0;
                              }
                        continue;
                        }
                  if (!e->isChordRest())
                        continue;
                  ChordRest* cr = (ChordRest*) e;
                  BeamMode bm   = cr->beamMode();

                  //---------------------------------------
                  //   check for beam end
                  //---------------------------------------

                  bool tooLong = cr->tickLen() >= division;
                  if (beam) {
                        // end beam if there are chords/rests missing
                        // in voice:
                        ChordRest* le = beam->getElements().back();
                        if (le->tick() + le->tickLen() != cr->tick()) {
                              tooLong = true;
                              }
                        int group = division;
                        int z, n;
                        _score->sigmap->timesig(cr->tick(), z, n);
                        if (z == 3 && n == 8)   // hack!
                              group = division * 3 / 2;
                        else if (z == 9 && n == 8)
                              group = division * 3 / 2;
                        else if (z == 5 && n == 8)
                              group = division * 5 / 2;
                        else if (z == 6 && n == 8)
                              group = division * 6 / 2;

                        bool styleBreak = ((cr->tick() - tick()) % group) == 0;
                        if (styleBreak) {
                              // some experimental optimization
                              const QList<ChordRest*> l = beam->getElements();
                              if (l.size() == 2 && cr->tickLen() == l.back()->tickLen())
                                    styleBreak = false;
                              }
                        if (styleBreak || tooLong || bm == BEAM_BEGIN || bm == BEAM_NO) {
                              beam->layout1(layout);
                              beam = 0;
                              a1   = 0;
                              goto newBeam;
                              }
                        else {
                              cr->setBeam(beam);
                              beam->add(cr);

                              // is this the last beam element?
                              if (bm == BEAM_END) {
                                    beam->layout1(layout);
                                    beam = 0;
                                    a1   = 0;
                                    }
                              }
                        }
                  else {
newBeam:
                        bool hint = !tooLong;	  // start new beam
                        if (bm == BEAM_NO)
                              hint = false;
                        else if (bm == BEAM_BEGIN)
                              hint = true;
                        else if (bm == BEAM_AUTO) {
                              if (a1 == 0) {
                                    //
                                    // start a new beam?
                                    //
                        		int group = division;
                        		int z, n;
                        		_score->sigmap->timesig(cr->tick(), z, n);

                                    // handle special time signatures:
                        		if (z == 9 && n == 8) {
                              		group = division * 3 / 2;
                                          }
                                    if ((cr->tick() - tick()) % group)
                                          hint = false;
                                    }
                              }
                        if (hint) {
                              if (a1 == 0) {
                                    a1 = cr;
                                    }
                              else {
                                    if (bm == BEAM_BEGIN) {
                                          a1->layoutStem1(layout);
                                          a1 = cr;
                                          }
                                    else {
                                          beam = new Beam(score());
                                          beam->setStaff(a1->staff());
                                          beam->setParent(this);
                                          _beamList.push_back(beam);
                                          a1->setBeam(beam);
                                          beam->add(a1);
                                          cr->setBeam(beam);
                                          beam->add(cr);
                                          a1 = 0;
                                          }
                                    }
                              }
                        else {
                              cr->layoutStem1(layout);
                              if (a1)
                                    a1->layoutStem1(layout);
                              a1 = 0;
                              }
                        }
                  }
            if (beam)
                  beam->layout1(layout);
            else if (a1)
                  a1->layoutStem1(layout);
            }
      }

//---------------------------------------------------------
//   layoutBeams
//    auto - beamer
//    called after layout spacing of notes
//---------------------------------------------------------

void Measure::layoutBeams(ScoreLayout* layout)
      {
      int nstaves = _score->nstaves();
      int tracks = nstaves * VOICES;

      // fix for staffOffset
      for (int track = 0; track < tracks; ++track) {
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);
                  if (e && e->isChordRest()) {
                        ChordRest* cr = (ChordRest*) e;
                        cr->layout(layout);
                        }
                  }
            }

      foreach(Beam* beam, _beamList)
            beam->layout(layout);

      for (int track = 0; track < tracks; ++track) {
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);
                  if (e && e->isChordRest()) {
                        ChordRest* cr = (ChordRest*) e;
                        if (cr->beam())
                              continue;
                        cr->layoutStem(layout);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   xmlType
//---------------------------------------------------------

QString Beam::xmlType(ChordRest* cr) const
      {
      if (cr == elements.front())
            return QString("begin");
      if (cr == elements.back())
            return QString("end");
      int idx = elements.indexOf(cr);
      if (idx == -1)
            printf("Beam::xmlType(): cannot find ChordRest\n");
      return QString("continue");
      }

//---------------------------------------------------------
//   layout1
//---------------------------------------------------------

void Beam::layout1(ScoreLayout* /*layout*/)
      {
      //delete old segments
      for (iBeamSegment i = beamSegments.begin(); i != beamSegments.end(); ++i)
            delete *i;
      beamSegments.clear();

      //
      // delete stem & hook for all chords
      //
      foreach(ChordRest* cr, elements) {
            if (cr->type() == CHORD) {
                  Chord* chord = (Chord*)cr;
                  chord->setStem(0);
                  chord->setHook(0);
                  }
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Beam::layout(ScoreLayout* layout)
      {
      double _spatium = layout->spatium();

      //---------------------------------------------------
      //   calculate direction of beam
      //    - majority
      //          number count of up or down notes
      //    - mean
      //          mean centre distance of all notes
      //    - median
      //          mean centre distance weighted per note
      //
      //   currently we use the "majority" method
      //---------------------------------------------------

      int upCount    = 0;
      int maxTickLen = 0;
      const ChordRest* a1  = elements.front();
      const ChordRest* a2  = elements.back();
      Chord* c1      = 0;
      Chord* c2      = 0;
      int move       = 0;
      int firstMove  = elements.front()->staffMove();

      foreach(ChordRest* cr, elements) {
            if (cr->type() == CHORD) {
                  c2 = (Chord*)(cr);
                  if (c1 == 0)
                        c1 = c2;
                  Chord* chord = c2;
                  //
                  // if only one stem direction is manually set it
                  // determines if beams are up or down
                  //
                  if (chord->stemDirection() != AUTO)
                        upCount += chord->stemDirection() == UP ? 1000 : -1000;
                  else
                        upCount += chord->isUp() ? 1 : -1;
                  if (chord->staffMove()) {
                        if (firstMove == 0)
                              move = chord->staffMove() * -1;
                        else
                              move = chord->staffMove() * -1;
                        }
                  }
            int tl = cr->tickLen();
            Tuplet* tuplet = cr->tuplet();
            if (tuplet)
                  tl = tuplet->baseLen();
            if (tl > maxTickLen)
                  maxTickLen = tl;
            }

// printf("   move %d  firstMove %d\n", move, firstMove);

      bool upFlag = upCount >= 0;

      //
      //  move = 0     no cross beaming
      //       = -1    staff 1 - 2
      //       = 1     staff 2 - 1

      foreach(ChordRest* cr, elements) {
            if (move == 1)
                  cr->setUp(cr->staffMove() == 0);
            else if (move == -1)
                  cr->setUp(cr->staffMove() != 0);
            else
                  cr->setUp(upFlag);
            }

      if (move == 1)
            upFlag = true;
      else if (move == -1)
            upFlag = false;

      //------------------------------------------------------------
      //   calculate slope of beam
      //    - the slope is set to zero on "concave" chord sequences
      //------------------------------------------------------------

      bool concave = false;
      for (int i = 0; i < elements.size() - 2; ++i) {
            int l1 = elements[i]->line(upFlag);
            int l  = elements[i+1]->line(upFlag);
            int l2 = elements[i+2]->line(upFlag);

            concave = ((l1 < l2) && ((l < l1) || (l > l2)))
                    || ((l1 > l2) && ((l > l1) || (l < l2)));
            if (concave)
                  break;
            }

      int l1 = elements.front()->line(upFlag);
      int l2 = elements.back()->line(upFlag);

      int cut     = 0;
      qreal slope = 0.0;

      if (!concave) {
            double dx = (a2->pos().x() + a2->segment()->pos().x())
                          - (a1->pos().x() + a1->segment()->pos().x());
            if (dx) {
                  slope = (l2 - l1) * _spatium * .5 / dx;
                  if (fabs(slope) < score()->style()->beamMinSlope) {
                        cut = slope > 0.0 ? 0 : -1;
                        slope = 0;
                        }
                  else if (slope > score()->style()->beamMaxSlope) {
                        slope = score()->style()->beamMaxSlope;
                        cut = 1;
                        }
                  else if (-slope > score()->style()->beamMaxSlope) {
                        slope = -score()->style()->beamMaxSlope;
                        cut = -1;
                        }
                  }
            }

      cut *= (upFlag ? 1 : -1);

      //---------------------------------------------------
      //    create beam segments
      //---------------------------------------------------

            //---------------------------------------------
            //   create top beam segment
            //---------------------------------------------

      double xoffLeft  = point(score()->style()->stemWidth)/2;
      double xoffRight = xoffLeft;

      QPointF p1s(a1->stemPos(a1->isUp(), false) + a1->pos() + a1->segment()->pos());
      QPointF p2s(a2->stemPos(a2->isUp(), false) + a2->pos() + a2->segment()->pos());
      double x1 = p1s.x() - xoffLeft;
      double x2 = p2s.x() + xoffRight;

      QPointF p1, p2;
      double ys = (x2 - x1) * slope;
      if (cut >= 0) {
            // linker Punkt ist Referenz
            p1 = QPointF(x1, p1s.y());
            p2 = QPointF(x2, p1.y() + ys);
            }
      else {
            // rechter Punkt ist Referenz
            p2 = QPointF(x2, p2s.y());
            p1 = QPointF(x1, p2.y() - ys);
            }

      //---------------------------------------------------
      // calculate min stem len
      //    adjust beam position if necessary
      //
      double beamDist = point(score()->style()->beamDistance * score()->style()->beamWidth
                        + score()->style()->beamWidth) * (upFlag ? 1.0 : -1.0);
      double min = 1000;
      double max = -1000;
      int lmove = elements.front()->staffMove();
      foreach(ChordRest* cr, elements) {
            if (cr->type() != CHORD)
                  continue;
            Chord* chord  = (Chord*)(cr);
            if (chord->staffMove() != lmove)
                  break;
            QPointF npos(chord->stemPos(chord->isUp(), true) + chord->pos() + chord->segment()->pos());
            double bd = (chord->beams() - 1) * beamDist * (chord->isUp() ? 1.0 : -1.0);
            double y1 = npos.y();
            double y2  = p1.y() + (npos.x() - x1) * slope;
            double stemLen = chord->isUp() ? (y1 - y2) : (y2 - y1);
            stemLen -= bd;
            if (stemLen < min)
                  min = stemLen;
            if (stemLen > max)
                  max = stemLen;
            }

      // adjst beam position
      double n = 3.0;   // minimum stem len (should be a style parameter)
      if (fabs(max-min) > _spatium * 2)
            n = 2.0;    // reduce minimum stem len (heuristic)

      {
      double diff = _spatium * n - min;
      if (upFlag)
            diff = -diff;
      p1.ry() += diff;
      p2.ry() += diff;
      }

      //---------------------------------------------------

      BeamSegment* bs = new BeamSegment;
      beamSegments.push_back(bs);
      bs->p1  = p1;
      bs->p2  = p2;

      if (maxTickLen <= division/16) {       // 1/64     24
            bs = new BeamSegment(p1, p2);
            bs->move(0, beamDist * 3);
            beamSegments.push_back(bs);
            }
      if (maxTickLen <= division/8) {        // 1/32   48
            bs = new BeamSegment(p1, p2);
            bs->move(0, beamDist * 2);
            beamSegments.push_back(bs);
            }
      if (maxTickLen <= division/4+division/8) {   //       144
            bs = new BeamSegment(p1, p2);
            bs->move(0, beamDist);
            beamSegments.push_back(bs);
            }

      //---------------------------------------------
      //   create broken/short beam segments
      //---------------------------------------------

      int l = maxTickLen;
      if (l > division/4)
            l = division/4;
      else if (l > division/8)
            l = division/8;
      else if (l > division/16)
            l = division/16;
      else
            l /= 2;

      for (;l >= division/16; l = l/2) {
            int n = 1;
            if (l == division/8)
                  n = 2;
            else if (l == division/16)
                  n = 3;

            double y1 = p1.y() + beamDist * n;

            Note* nn1 = 0;
            Note* nn2 = 0;
            bool nn1r = false;
            foreach(ChordRest* cr, elements) {
                  if (cr->type() != CHORD)
                        continue;
                  Chord* chord = (Chord*)(cr);
                  int tl = chord->tickLen();
                  if (tl > l) {
                        if (nn2) {
                              // create short segment
                              bs = new BeamSegment;
                              beamSegments.push_back(bs);
                              double x2 = nn1->stemPos(nn1->chord()->isUp()).x() + nn1->chord()->pos().x() + nn1->chord()->segment()->pos().x();
                              double x3 = nn2->stemPos(nn2->chord()->isUp()).x() + nn2->chord()->pos().x() + nn2->chord()->segment()->pos().x();
                              bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                              bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                              }
                        else if (nn1) {
                              // create broken segment
                              bs = new BeamSegment;
                              beamSegments.push_back(bs);
                              double x2 = nn1->stemPos(nn1->chord()->isUp()).x() + nn1->chord()->pos().x() + nn1->chord()->segment()->pos().x();
                              double x3 = x2 + point(score()->style()->beamMinLen);

                              if (!nn1r) {
                                    double tmp = x3;
                                    x3 = x2;
                                    x2 = tmp;
                                    }

                              bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                              bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                              }
                        nn1r = false;
                        nn1 = nn2 = 0;
                        continue;
                        }
                  Note* n = chord->isUp() ? chord->noteList()->back()
                                      : chord->noteList()->front();
                  nn1r = false;
                  if (nn1)
                        nn2 = n;
                  else {
                        nn1 = n;
                        nn1r = cr == elements.front();
                        }
                  }
            if (nn2) {
                  // create short segment
                  bs = new BeamSegment;
                  beamSegments.push_back(bs);
                  double x2 = nn1->stemPos(nn1->chord()->isUp()).x() + nn1->chord()->pos().x() + nn1->chord()->segment()->pos().x();
                  double x3 = nn2->stemPos(nn2->chord()->isUp()).x() + nn2->chord()->pos().x() + nn2->chord()->segment()->pos().x();
                  bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                  bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                  }
           else if (nn1) {
                  // create broken segment
                  bs = new BeamSegment;
                  beamSegments.push_back(bs);
                  double x3 = nn1->stemPos(nn1->chord()->isUp()).x() + nn1->chord()->pos().x() + nn1->chord()->segment()->pos().x();
                  double x2 = x3 - point(score()->style()->beamMinLen);
                  bs->p1 = QPointF(x2, (x2 - x1) * slope + y1);
                  bs->p2 = QPointF(x3, (x3 - x1) * slope + y1);
                  }
            }

      //---------------------------------------------------
      //    create stem's
      //    stem Pos() is relative to Chord
      //---------------------------------------------------

      foreach(ChordRest* cr, elements) {
            if (cr->type() != CHORD)
                  continue;
            Chord* chord = (Chord*)(cr);

            Stem* stem = chord->stem();
            if (!stem) {
                  stem = new Stem(score());
                  chord->setStem(stem);
                  }

            QPointF npos(chord->stemPos(chord->isUp(), false) + chord->pos() + chord->segment()->pos());
            double x2 = npos.x();
            double y1 = npos.y();
            double y2 = p1.y() + (x2 - x1) * slope;

            double stemLen = chord->isUp() ? (y1 - y2) : (y2 - y1);
            stem->setLen(spatium(stemLen));

            if (chord->isUp())
                  npos += QPointF(0, -stemLen);
            stem->setPos(npos - chord->pos() - chord->segment()->pos());
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Beam::bbox() const
      {
      QRectF r;
      for (ciBeamSegment ibs = beamSegments.begin();
         ibs != beamSegments.end(); ++ibs) {
            BeamSegment* bs = *ibs;
            r |= QRectF(bs->p1, QSizeF(1.0, 1.0));
            r |= QRectF(bs->p2, QSizeF(1.0, 1.0));
            }
      return r;
      }


