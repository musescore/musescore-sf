//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: ottava.cpp,v 1.3 2006/03/28 14:58:58 wschweer Exp $
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

#include "ottava.h"
#include "style.h"
#include "painter.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Ottava::setSubtype(int val)
      {
      Element::setSubtype(val);
      switch(val) {
            case 0:
                  text = "8va";
                  break;
            case 1:
                  text = "15va";
                  break;
            case 2:
                  text = "8vb";
                  break;
            case 3:
                  text = "15vb";
                  break;
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Ottava::draw1(Painter& p)
      {
      qreal ottavaLineWidth    = _spatium * .18;
      qreal ottavaTextDistance = _spatium * .5;

      for (ciLineSegment i = segments.begin(); i != segments.end(); ++i) {
            const LineSegment* s = &*i;

            ciLineSegment ii = i;
            ++ii;
            QPointF pp1(s->p1);
            QPointF pp2(s->p2);

            if (i == segments.begin())
                  pp1 += off1 * _spatium;
            if (ii == segments.end())
                  pp2 += off2 * _spatium;

            if (i == segments.begin()) {
                  QFont f(textStyles[TEXT_STYLE_DYNAMICS].font());
                  p.setFont(f);
                  QFontMetricsF fm(f);
                  QRectF bb(fm.boundingRect(text));
                  qreal h = textHeight;   // bb.height() * .5;
                  p.drawText(pp1 + 	QPointF(0.0, h), text);
                  pp1 += QPointF(bb.width() + ottavaTextDistance, 0.0);

                  QPen pen(p.pen());
                  pen.setWidthF(ottavaLineWidth);
                  pen.setStyle(Qt::DashLine);
                  p.setPen(pen);
                  p.drawLine(QLineF(pp1, pp2));
                  if (ii == segments.end())
                        p.drawLine(QLineF(pp2, QPointF(pp2.x(), h)));
                  }
            else
                  printf("===not impl.\n");
            }

      if (mode != NORMAL) {
            qreal lw = 2.0/p.matrix().m11();
            QPen pen(Qt::blue);
            pen.setWidthF(lw);
            p.setPen(pen);
            if (mode == DRAG1) {
                  p.setBrush(Qt::blue);
                  p.drawRect(r1);
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(r2);
                  }
            else {
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(r1);
                  p.setBrush(Qt::blue);
                  p.drawRect(r2);
                  }
            }
      }

//---------------------------------------------------------
//   setLen
//    used for palette operations
//---------------------------------------------------------

void Ottava::setLen(qreal l)
      {
      LineSegment hps;
      hps.p1 = QPointF(0, 0);
      hps.p2 = QPointF(l, 0);
      segments.push_back(hps);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Ottava::layout()
      {
      SLine::layout();
      qreal ottavaDistance = _spatium * 2.5;
      qreal y = 0.0;
      if (parent()) {
            Measure* measure = (Measure*)parent();
            System* system   = measure->system();
            SysStaff* sstaff = system->staff(staffIdx());
            y = sstaff->bbox().top() - ottavaDistance;
            }

      setPos(0.0, y);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Ottava::bbox() const
      {
      QFontMetricsF fm(textStyles[TEXT_STYLE_DYNAMICS].fontMetrics());
      qreal h1 = 0.0;
      int n = text.size();
      for (int i = 0; i < n; ++i) {
            qreal h = fm.boundingRect(text[i]).height();
            if (h > h1)
                  h1 = h;
            }
      h1 *= .5;
      textHeight = h1;

      QRectF r(0, 0, 0, 0);
      for (ciLineSegment i = segments.begin(); i != segments.end(); ++i) {
            LineSegment* s = (LineSegment*)(&*i);
            ciLineSegment ii = i;
            ++ii;
            QPointF pp1(s->p1);
            QPointF pp2(s->p2);

            if (i == segments.begin())
                  pp1 += off1 * _spatium;
            if (ii == segments.end())
                  pp2 += off2 * _spatium;

            s->bbox.setCoords(pp1.x(), pp1.y() - h1, pp2.x(), pp2.y() + h1);
            r |= s->bbox;
            }

      if (mode != NORMAL) {
            r |= bbr1;
            r |= bbr2;
            }
      return r;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ottava::write(Xml& xml) const
      {
      xml.stag("Ottava");
      SLine::writeProperties(xml);
      xml.etag("Ottava");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Ottava::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!SLine::readProperties(node))
                  domError(node);
            }
      }

