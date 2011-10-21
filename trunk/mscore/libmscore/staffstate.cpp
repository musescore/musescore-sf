//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "staffstate.h"
#include "score.h"
#include "instrtemplate.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "painter.h"
#include "mscore.h"

//---------------------------------------------------------
//   StaffState
//---------------------------------------------------------

StaffState::StaffState(Score* score)
   : Element(score)
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffState::write(Xml& xml) const
      {
      xml.stag(name());
      if (subtype() == STAFF_STATE_INSTRUMENT)
            _instrument.write(xml);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffState::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "Instrument")
                  _instrument.read(e);
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffState::draw(Painter* painter) const
      {
      if (score()->printing())
            return;
      QPen pen;
      if (selected())
            painter->setPenColor(MScore::selectColor[0]);
      else
            painter->setPenColor(MScore::layoutBreakColor);

      painter->setLineWidth(lw);
      painter->setCapStyle(Qt::RoundCap);
      painter->setJoinStyle(Qt::RoundJoin);
      painter->setNoBrush(true);
      painter->drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffState::layout()
      {
      qreal _spatium = spatium();
      path      = QPainterPath();
      lw        = _spatium * 0.3;
      qreal h  = _spatium * 4;
      qreal w  = _spatium * 2.5;
//      qreal w1 = w * .6;

      switch(subtype()) {
            case STAFF_STATE_INSTRUMENT:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  path.moveTo(w * .5, h - _spatium * .5);
                  path.lineTo(w * .5, _spatium * 2);
                  path.moveTo(w * .5, _spatium * .8);
                  path.lineTo(w * .5, _spatium * 1.0);
                  break;

            case STAFF_STATE_TYPE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case STAFF_STATE_VISIBLE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case STAFF_STATE_INVISIBLE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            default:
                  qDebug("unknown layout break symbol\n");
                  break;
            }
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      setPos(0.0, _spatium * -6.0);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void StaffState::setSubtype(const QString& s)
      {
      if (s == "instrument")
            setSubtype(STAFF_STATE_INSTRUMENT);
      else if (s == "type")
            setSubtype(STAFF_STATE_TYPE);
      else if (s == "visible")
            setSubtype(STAFF_STATE_VISIBLE);
      else if (s == "invisible")
            setSubtype(STAFF_STATE_INVISIBLE);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString StaffState::subtypeName() const
      {
      switch(subtype()) {
            case STAFF_STATE_INSTRUMENT:
                  return "instrument";
            case STAFF_STATE_TYPE:
                  return "type";
            case STAFF_STATE_VISIBLE:
                  return "visible";
            case STAFF_STATE_INVISIBLE:
                  return "invisible";
            default:
                  return "??";
            }
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool StaffState::acceptDrop(MuseScoreView*, const QPointF&, int /*type*/, int /*st*/) const
      {
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* StaffState::drop(const DropData& data)
      {
      Element* e = data.element;
      score()->undoChangeElement(this, e);
      return e;
      }

