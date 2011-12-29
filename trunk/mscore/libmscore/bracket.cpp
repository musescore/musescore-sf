//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "bracket.h"
#include "xml.h"
#include "style.h"
#include "utils.h"
#include "staff.h"
#include "score.h"
#include "system.h"
#include "sym.h"
#include "mscore.h"

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket(Score* s)
   : Element(s)
      {
      h2       = 3.5 * spatium();
      _span    = 1;
      _column   = 0;
      yoff     = 0.0;
      setGenerated(true);     // brackets are not saved
      }

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Bracket::setHeight(qreal h)
      {
      h2 = h * .5;
      }

//---------------------------------------------------------
//   width
//---------------------------------------------------------

qreal Bracket::width() const
      {
      qreal w;
      if (subtype() == BRACKET_AKKOLADE)
            w = point(score()->styleS(ST_akkoladeWidth) + score()->styleS(ST_akkoladeBarDistance));
      else
            w = point(score()->styleS(ST_bracketWidth) + score()->styleS(ST_bracketDistance));
      return w;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bracket::layout()
      {
      qreal _spatium = spatium();
      path = QPainterPath();
      if (h2 == 0.0)
            return;

      qreal h = h2 + yoff * .5;
//      qreal d = 0.0;

      if (subtype() == BRACKET_AKKOLADE) {
            qreal w = point(score()->styleS(ST_akkoladeWidth));
#if 0
            const qreal X1 =  2.0 * w;
            const qreal X2 = -0.7096 * w;
            const qreal X3 = -1.234 * w;
            const qreal X4 =  1.734 * w;
            const qreal Y1 =  .3359 * h;
            const qreal Y2 =  .5089 * h;
            const qreal Y3 =  .5025 * h;
            const qreal Y4 =  .2413 * h;

            path.moveTo(0, h);
            path.cubicTo(X1, h + Y1,   X2, h + Y2,    w, 2 * h);
            path.cubicTo(X3, h + Y3,   X4, h + Y4,    0, h);

            path.cubicTo(X1, h - Y1,   X2, h - Y2,    w, 0);
            path.cubicTo(X3, h - Y3,   X4, h - Y4,    0, h);
#endif

#define XM(a) (a+700)*w/700
#define YM(a) (a+7100)*h2/7100

path.moveTo( XM(   -8), YM(-2048));
path.cubicTo(XM(   -8), YM(-3192), XM(-360), YM(-4304), XM( -360), YM(-5400)); // c 0
path.cubicTo(XM( -360), YM(-5952), XM(-264), YM(-6488), XM(   32), YM(-6968)); // c 1
path.cubicTo(XM(   40), YM(-6976), XM(  40), YM(-6976), XM(   40), YM(-6984)); // c 0
path.cubicTo(XM(   40), YM(-7000), XM(  16), YM(-7024), XM(    0), YM(-7024)); // c 0
path.cubicTo(XM(   -8), YM(-7024), XM( -24), YM(-7024), XM(  -32), YM(-7008)); // c 1
path.cubicTo(XM( -416), YM(-6392), XM(-544), YM(-5680), XM( -544), YM(-4960)); // c 0
path.cubicTo(XM( -544), YM(-3800), XM(-168), YM(-2680), XM( -168), YM(-1568)); // c 0
path.cubicTo(XM( -168), YM(-1016), XM(-264), YM( -496), XM( -560), YM(  -16)); // c 1
path.lineTo( XM( -560), YM(    0));  //  l 1
path.lineTo( XM( -560), YM(   16));  //  l 1
path.cubicTo(XM( -264), YM(  496), XM(-168), YM( 1016), XM( -168), YM( 1568)); // c 0
path.cubicTo(XM( -168), YM( 2680), XM(-544), YM( 3800), XM( -544), YM( 4960)); // c 0
path.cubicTo(XM( -544), YM( 5680), XM(-416), YM( 6392), XM(  -32), YM( 7008)); // c 1
path.cubicTo(XM(  -24), YM( 7024), XM(  -8), YM( 7024), XM(    0), YM( 7024)); // c 0
path.cubicTo(XM(   16), YM( 7024), XM(  40), YM( 7000), XM(   40), YM( 6984)); // c 0
path.cubicTo(XM(   40), YM( 6976), XM(  40), YM( 6976), XM(   32), YM( 6968)); // c 1
path.cubicTo(XM( -264), YM( 6488), XM(-360), YM( 5952), XM( -360), YM( 5400)); // c 0
path.cubicTo(XM( -360), YM( 4304), XM(  -8), YM( 3192), XM(   -8), YM( 2048)); // c 0
path.cubicTo(XM( -  8), YM( 1320), XM(-136), YM(  624), XM( -512), YM(    0)); // c 1
path.cubicTo(XM( -136), YM( -624), XM(  -8), YM(-1320), XM(   -8), YM(-2048)); // c 0

            }
      else if (subtype() == BRACKET_NORMAL) {
            qreal w = point(score()->styleS(ST_bracketWidth));

            QChar up   = symbols[score()->symIdx()][brackettipsRightUp].code();
            QChar down = symbols[score()->symIdx()][brackettipsRightDown].code();

            QFont f(fontId2font(0));
            f.setPointSizeF(2.0 * _spatium);

            qreal o   = _spatium * .17;
            qreal slw = point(score()->styleS(ST_staffLineWidth));

            path.setFillRule(Qt::WindingFill);

            path.addText(QPointF(0.0, -o), f,          QString(up));
            path.addText(QPointF(0.0, h * 2.0 + o), f, QString(down));
            path.addRect(0.0, -slw * .5, w, h * 2.0 + slw);
            }
      QRectF r(path.boundingRect());
      setbbox(path.boundingRect());
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bracket::draw(QPainter* painter) const
      {
      painter->setPen(Qt::NoPen);
      painter->setBrush(QBrush(curColor()));
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   Bracket::write
//---------------------------------------------------------

void Bracket::write(Xml& xml) const
      {
      switch(subtype()) {
            case BRACKET_AKKOLADE:
                  xml.stag("Bracket type=\"Akkolade\"");
                  break;
            case BRACKET_NORMAL:
                  xml.stag("Bracket");
                  break;
            case NO_BRACKET:
                  break;
            }
      if (_column)
            xml.tag("level", _column);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Bracket::read
//---------------------------------------------------------

void Bracket::read(const QDomElement& de)
      {
      QString t(de.attribute("type", "Normal"));

      if (t == "Normal")
            setSubtype(BRACKET_NORMAL);
      else if (t == "Akkolade")
            setSubtype(BRACKET_AKKOLADE);
      else
            qDebug("unknown brace type <%s>\n", t.toLatin1().data());

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "level")
                  _column = e.text().toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Bracket::startEdit(MuseScoreView*, const QPointF&)
      {
      yoff = 0.0;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Bracket::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 1;
      grip[0].translate(QPointF(0.0, h2 * 2) + QPointF(0.0, yoff) + pagePos());
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF Bracket::gripAnchor(int) const
      {
      return QPointF();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Bracket::endEdit()
      {
      endEditDrag();
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Bracket::editDrag(const EditData& ed)
      {
      yoff += ed.delta.y();
      layout();
      }

//---------------------------------------------------------
//   endEditDrag
//    snap to nearest staff
//---------------------------------------------------------

void Bracket::endEditDrag()
      {
      h2 += yoff * .5;
      yoff = 0.0;

      qreal ay1 = pagePos().y();
      qreal ay2 = ay1 + h2 * 2;

      int staffIdx1 = staffIdx();
      int staffIdx2;
      int n = system()->staves()->size();
      if (staffIdx1 + 1 >= n)
            staffIdx2 = staffIdx1;
      else {
            qreal ay  = parent()->pagePos().y();
            System* s = system();
            qreal y   = s->staff(staffIdx1)->y() + ay;
            qreal h1  = staff()->height();

            for (staffIdx2 = staffIdx1 + 1; staffIdx2 < n; ++staffIdx2) {
                  qreal h = s->staff(staffIdx2)->y() + ay - y;
                  if (ay2 < (y + (h + h1) * .5))
                        break;
                  y += h;
                  }
            staffIdx2 -= 1;
            }

      qreal sy = system()->staff(staffIdx1)->y();
      qreal ey = system()->staff(staffIdx2)->y() + score()->staff(staffIdx2)->height();
      h2 = (ey - sy) * .5;

      int span = staffIdx2 - staffIdx1 + 1;
      staff()->setBracketSpan(_column, span);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Bracket::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      return e->type() == BRACKET;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Bracket::drop(const DropData& data)
      {
      Element* e = data.element;
      if (e->type() == BRACKET) {
            Bracket* b = (Bracket*)e;
            b->setParent(parent());
            b->setTrack(track());
            b->setSpan(span());
            b->setLevel(level());
            score()->undoRemoveElement(this);
            score()->undoAddElement(b);
            return b;
            }
      delete e;
      return 0;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Bracket::edit(MuseScoreView*, int, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if (modifiers & Qt::ShiftModifier) {
            if (key == Qt::Key_Left) {
                  BracketType bt = staff()->bracket(_column);
                  // search empty level
                  int oldColumn = _column;
                  staff()->setBracket(_column, NO_BRACKET);
                  for (;;) {
                        ++_column;
                        if (staff()->bracket(_column) == NO_BRACKET)
                              break;
                        }
                  staff()->setBracket(_column, bt);
                  staff()->setBracketSpan(_column, _span);
                  score()->moveBracket(staffIdx(), oldColumn, _column);
                  score()->setLayoutAll(true);
                  return true;
                  }
            else if (key == Qt::Key_Right) {
                  if (_column) {
                        int l = _column - 1;
                        for (; l >= 0; --l) {
                              if (staff()->bracket(l) == NO_BRACKET) {
                                    BracketType bt = staff()->bracket(_column);
                                    staff()->setBracket(_column, NO_BRACKET);
                                    staff()->setBracket(l, bt);
                                    staff()->setBracketSpan(l, _span);
                                    score()->moveBracket(staffIdx(), _column, l);
                                    _column = l;
                                    score()->setLayoutAll(true);
                                    break;
                                    }
                              }
                        }
                  return true;
                  }
            }
      return false;
      }

