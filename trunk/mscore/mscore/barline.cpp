//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: barline.cpp,v 1.3 2006/04/12 14:58:10 wschweer Exp $
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

#include "xml.h"
#include "barline.h"
#include "preferences.h"
#include "style.h"
#include "utils.h"
#include "score.h"
#include "sym.h"
#include "viewer.h"

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

BarLine::BarLine(Score* s)
   : Element(s)
      {
      setSubtype(NORMAL_BAR);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void BarLine::draw(QPainter& p)
      {
      qreal lw    = point(score()->style()->barWidth);
      qreal h     = height();

      bool split = height() > (_spatium * 4.01);

      QPen pen(color());
      pen.setWidthF(lw);
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);

      switch(subtype()) {
            case BROKEN_BAR:
            case NORMAL_BAR:
                  p.drawLine(QLineF(lw*.5, 0.0, lw*.5, h));
                  break;

            case END_BAR:
                  {
                  qreal lw2 = point(score()->style()->endBarWidth);

                  p.drawLine(QLineF(lw*.5, 0.0, lw*.5, h));
                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  qreal x = point(score()->style()->endBarDistance) + lw2*.5 + lw;
                  p.drawLine(QLineF(x, 0.0, x, h));
                  }
                  break;

            case DOUBLE_BAR:
                  {
                  qreal lw2 = point(score()->style()->doubleBarWidth);
                  qreal d   = point(score()->style()->doubleBarDistance);

                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  qreal x = lw2/2;
                  p.drawLine(QLineF(x, 0.0, x, h));
                  x += d + lw2;
                  p.drawLine(QLineF(x, 0.0, x, h));
                  }
                  break;

            case START_REPEAT:
                  {
                  qreal lw2  = point(score()->style()->endBarWidth);
                  qreal lw22 = point(score()->style()->endBarWidth) / 2.0;
                  qreal d1   = point(score()->style()->endBarDistance);
                  qreal dotw = symbols[dotSym].width();
                  qreal x2   =  dotw/2;
                  qreal x1   =  dotw + d1 + lw/2;
                  qreal x0   =  dotw + d1 + lw + d1 + lw22;

                  symbols[dotSym].draw(p, x0, 1.5 * _spatium);
                  symbols[dotSym].draw(p, x0, 2.5 * _spatium);
                  if (split) {
                        symbols[dotSym].draw(p, x0, h - 1.5 * _spatium);
                        symbols[dotSym].draw(p, x0, h - 2.5 * _spatium);
                        }

                  p.drawLine(QLineF(x1, 0.0, x1, h));
                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  p.drawLine(QLineF(x2, 0.0, x2, h));
                  }
                  break;

            case END_REPEAT:
                  {
                  qreal lw2  = point(score()->style()->endBarWidth);
                  qreal lw22 = point(score()->style()->endBarWidth) / 2.0;
                  qreal d1   = point(score()->style()->endBarDistance);

                  qreal dotw = symbols[dotSym].width();
                  qreal x0   =  dotw/2;
                  qreal x1   =  dotw + d1 + lw/2;
                  qreal x2   =  dotw + d1 + lw + d1 + lw22;

                  symbols[dotSym].draw(p, x0, 1.5 * _spatium);
                  symbols[dotSym].draw(p, x0, 2.5 * _spatium);
                  if (split) {
                        symbols[dotSym].draw(p, x0, h - 1.5 * _spatium);
                        symbols[dotSym].draw(p, x0, h - 2.5 * _spatium);
                        }

                  p.drawLine(QLineF(x1, 0.0, x1, h));

                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  p.drawLine(QLineF(x2, 0.0, x2, h));
                  }
                  break;

            case INVISIBLE_BAR:
                  break;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void BarLine::write(Xml& xml) const
      {
      if (subtype() == NORMAL_BAR)
            xml.tagE("BarLine");
      else {
            xml.stag("BarLine");
            Element::writeProperties(xml);
            xml.etag();
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BarLine::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  domError(e);
            }
      setSubtype(subtype());
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void BarLine::dump() const
      {
      Element::dump();
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void BarLine::setSubtype(int t)
      {
      Element::setSubtype(t);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF BarLine::bbox() const
      {
      Spatium w = score()->style()->barWidth;
      qreal dw = 0.0;

      switch(subtype()) {
            case DOUBLE_BAR:
                  w  = score()->style()->doubleBarWidth * 2 + score()->style()->doubleBarDistance;
                  dw = point(w);
                  break;
            case START_REPEAT:
            case END_REPEAT:
                  w  += score()->style()->endBarWidth + 2 * score()->style()->endBarDistance;
                  dw = point(w) + symbols[dotSym].width();
                  break;
            case END_BAR:
                  w += score()->style()->endBarWidth + score()->style()->endBarDistance;
                  dw = point(w);
                  break;
            case BROKEN_BAR:
            case NORMAL_BAR:
            case INVISIBLE_BAR:
                  dw = point(w);
                  break;
            default:
                  printf("illegal bar line type\n");
                  break;
            }
      return QRectF(0.0, 0.0, dw, _height);
      }

//---------------------------------------------------------
//   space
//---------------------------------------------------------

void BarLine::space(double& min, double& extra) const
      {
      min   = width();
      extra = 0.0;
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool BarLine::acceptDrop(Viewer* viewer, const QPointF&, int type,
   const QDomElement&) const
      {
      if (type == BAR_LINE) {
            viewer->setDropTarget(this);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BarLine::drop(const QPointF&, const QPointF&, int type, const QDomElement& e)
      {
      if (type != BAR_LINE)
            return 0;
      score()->cmdRemove(this);

      BarLine* bl = new BarLine(score());
      bl->read(e);
      bl->setParent(parent());
      bl->setStaff(staff());
      if (subtype() == bl->subtype()) {
            delete bl;
            return 0;
            }
      score()->cmdAdd(bl);
      return bl;
      }

