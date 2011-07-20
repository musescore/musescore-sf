//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "spacer.h"
#include "score.h"
#include "painter.h"
#include "mscore.h"

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

Spacer::Spacer(Score* score)
   : Element(score)
      {
      _space = Spatium(0);
      }

Spacer::Spacer(const Spacer& s)
   : Element(s)
      {
      _space = s._space;
      path   = s.path;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Spacer::draw(Painter* painter) const
      {
      if (score()->printing() || !score()->showUnprintable())
            return;
      QPen pen;
      if (selected())
            painter->setPenColor(MScore::selectColor[0]);
      else
            painter->setPenColor(MScore::layoutBreakColor);

      painter->setLineWidth(spatium() * 0.4);
      painter->setNoBrush(true);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Spacer::layout()
      {
      qreal _spatium = spatium();

      path     = QPainterPath();
      qreal w = _spatium;
      qreal b = w * .5;
      qreal h = _space.val() * _spatium;

      if (subtype() == SPACER_DOWN) {
            path.lineTo(w, 0.0);
            path.moveTo(b, 0.0);
            path.lineTo(b, h);
            path.lineTo(0.0, h-b);
            path.moveTo(b, h);
            path.lineTo(w, h-b);
            }
      else if (subtype() == SPACER_UP) {
            path.moveTo(b, 0.0);
            path.lineTo(0.0, b);
            path.moveTo(b, 0.0);
            path.lineTo(w, b);
            path.moveTo(b, 0.0);
            path.lineTo(b, h);
            path.moveTo(0.0, h);
            path.lineTo(w, h);
            }
      qreal lw = spatium() * 0.4;
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Spacer::acceptDrop(MuseScoreView*, const QPointF&, int, int) const
      {
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Spacer::drop(const DropData& data)
      {
      return data.element;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Spacer::editDrag(const EditData& ed)
      {
      Spatium s(ed.delta.y() / spatium());
      if (subtype() == SPACER_DOWN)
            _space += s;
      else if (subtype() == SPACER_UP)
            _space -= s;
      if (_space.val() < 2.0)
            _space = Spatium(2.0);
      layout();
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Spacer::updateGrips(int* grips, QRectF* grip) const
      {
      *grips   = 1;
      qreal _spatium = spatium();
      QPointF p;
      if (subtype() == SPACER_DOWN)
            p = QPointF(_spatium * .5, _space.val() * _spatium);
      else if (subtype() == SPACER_UP)
            p = QPointF(_spatium * .5, 0.0);
      grip[0].translate(canvasPos() + p);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Spacer::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.tag("space", _space.val());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Spacer::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "space")
                  _space = Spatium(val.toDouble());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }