//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "textline.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "score.h"
#include "preferences.h"
#include "sym.h"
#include "text.h"
#include "painter.h"

//---------------------------------------------------------
//   TextLineSegment
//---------------------------------------------------------

TextLineSegment::TextLineSegment(Score* s)
   : LineSegment(s)
      {
      _text = 0;
      }

TextLineSegment::TextLineSegment(const TextLineSegment& seg)
   : LineSegment(seg)
      {
      _text = 0;
      layout();    // set right _text
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextLineSegment::setSelected(bool f)
      {
      Element::setSelected(f);
      if (_text) {
            if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) {
                  if (textLine()->beginText())
                        _text->setSelected(f);
                  }
            else if (textLine()->continueText())
                  _text->setSelected(f);
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextLineSegment::draw(Painter* painter) const
      {
      TextLine* tl    = textLine();
      double _spatium = spatium();

      qreal textlineLineWidth    = tl->lineWidth().val() * _spatium;
      qreal textlineTextDistance = _spatium * .5;

      QPointF pp2(pos2());

      qreal l = 0.0;
      int sym = spannerSegmentType() == SEGMENT_MIDDLE ? tl->continueSymbol() : tl->beginSymbol();
      if (_text) {
            if (
               ((spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) && (tl->beginTextPlace() == PLACE_LEFT))
               || ((spannerSegmentType() == SEGMENT_MIDDLE || spannerSegmentType() == SEGMENT_END) && (tl->continueTextPlace() == PLACE_LEFT))
               ) {
                  QRectF bb(_text->bbox());
                  l = _text->pos().x() + bb.width() + textlineTextDistance;
                  }
            painter->save();
            painter->translate(_text->pos());
            painter->setPenColor(_text->curColor());
            _text->draw(painter);
            painter->restore();
            }
      else if (sym != -1) {
            const QRectF& bb = symbols[score()->symIdx()][sym].bbox(magS());
            qreal h = bb.height() * .5;
            QPointF o = tl->beginSymbolOffset() * _spatium;
            symbols[score()->symIdx()][sym].draw(painter, 1.0, o.x(), h + o.y());
            l = bb.width() + textlineTextDistance;
            }
      if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_END) {
            if (tl->endSymbol() != -1) {
                  int sym = tl->endSymbol();
                  const QRectF& bb = symbols[score()->symIdx()][sym].bbox(magS());
                  qreal h = bb.height() * .5;
                  QPointF o = tl->endSymbolOffset() * _spatium;
                  pp2.setX(pp2.x() - bb.width() + textlineTextDistance);
                  symbols[score()->symIdx()][sym].draw(painter, 1.0, pp2.x() + textlineTextDistance + o.x(), h + o.y());
                  }
            }

      QPointF pp1(l, 0.0);

      painter->setLineWidth(textlineLineWidth);
      painter->setLineStyle(tl->lineStyle());

      if (selected() && !(score() && score()->printing()))
            painter->setPenColor(preferences.selectColor[0]);
      else if (!visible())
            painter->setPenColor(Qt::gray);
      else
            painter->setPenColor(tl->lineColor());

      if (tl->beginHook() && tl->beginHookType() == HOOK_45)
            pp1.rx() += fabs(tl->beginHookHeight().val() * _spatium * .4);
      if (tl->endHook() && tl->endHookType() == HOOK_45)
            pp2.rx() -= fabs(tl->endHookHeight().val() * _spatium * .4);
      painter->drawLine(pp1.x(), pp1.y(), pp2.x(), pp2.y());

      if (tl->beginHook()) {
            double hh = tl->beginHookHeight().val() * _spatium;
            if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_BEGIN) {
                  if (tl->beginHookType() == HOOK_45)
                        painter->drawLine(pp1.x(), pp1.y(), pp1.x() - fabs(hh * .4), pp1.y() + hh);
                  else
                        painter->drawLine(pp1.x(), pp1.y(), pp1.x(), pp1.y() + hh);
                  }
            }
      if (tl->endHook()) {
            double hh = tl->endHookHeight().val() * _spatium;
            if (spannerSegmentType() == SEGMENT_SINGLE || spannerSegmentType() == SEGMENT_END) {
                  if (tl->endHookType() == HOOK_45)
                        painter->drawLine(pp2.x(), pp2.y(), pp2.x() + fabs(hh * .4), pp2.y() + hh);
                  else
                        painter->drawLine(pp2.x(), pp2.y(), pp2.x(), pp2.y() + hh);
                  }
            }
      }

//---------------------------------------------------------
//   bbox
//    FIXME
//---------------------------------------------------------

QRectF TextLineSegment::bbox() const
      {
      QPointF pp1;
      QPointF pp2(pos2());

      if (!_text && pp2.y() != 0)
            return QRectF(pp1, pp2).normalized();
      double y1 = point(-textLine()->lineWidth());
      double y2 = -y1;

      int sym = textLine()->beginSymbol();
      if (_text) {
            double h = _text->height();
            if (textLine()->beginTextPlace() == PLACE_ABOVE)
                  y1 = -h;
            else if (textLine()->beginTextPlace() == PLACE_BELOW)
                  y2 = h;
            else {
                  y1 = -h * .5;
                  y2 = h * .5;
                  }
            }
      else if (sym != -1) {
            double hh = symbols[score()->symIdx()][sym].height(magS()) * .5;
            y1 = -hh;
            y2 = hh;
            }
      if (textLine()->endHook()) {
            double h = point(textLine()->endHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }
      if (textLine()->beginHook()) {
            double h = point(textLine()->beginHookHeight());
            if (h > y2)
                  y2 = h;
            else if (h < y1)
                  y1 = h;
            }
      return QRectF(.0, y1, pp2.x(), y2 - y1);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLineSegment::layout()
      {
      TextLine* tl = (TextLine*)line();
      if (!tl->diagonal())
            _userOff2.setY(0);
      switch (spannerSegmentType()) {
            case SEGMENT_SINGLE:
            case SEGMENT_BEGIN:
                  if (tl->beginText()) {
                        if (_text == 0) {
                              _text = new Text(*tl->beginText());
                              _text->setFlag(ELEMENT_MOVABLE, false);
                              _text->setParent(this);
                              }
                        }
                  else {
                        delete _text;
                        _text = 0;
                        }
                  break;
            case SEGMENT_MIDDLE:
            case SEGMENT_END:
                  if (tl->continueText()) {
                        if (_text == 0) {
                              _text = new Text(*tl->continueText());
                              _text->setFlag(ELEMENT_MOVABLE, false);
                              _text->setParent(this);
                              }
                        }
                  else {
                        delete _text;
                        _text = 0;
                        }
                  break;
            }
      if (_text)
            _text->layout();
      }

//---------------------------------------------------------
//   clearText
//---------------------------------------------------------

void TextLineSegment::clearText()
      {
      delete _text;
      _text = 0;
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::TextLine(Score* s)
   : SLine(s)
      {
      _beginText         = 0;
      _continueText      = 0;

      _beginHookHeight   = Spatium(1.5);
      _endHookHeight     = Spatium(1.5);
      _beginHook         = false;
      _endHook           = false;
      _beginHookType     = HOOK_90;
      _endHookType       = HOOK_90;

      _lineWidth         = Spatium(0.15);
      _lineStyle         = Qt::SolidLine;
      _beginTextPlace    = PLACE_LEFT;
      _continueTextPlace = PLACE_LEFT;
      _lineColor         = Qt::black;
      _mxmlOff2          = 0;

      _beginSymbol       = -1;
      _continueSymbol    = -1;
      _endSymbol         = -1;

      setLen(spatium() * 7);   // for use in palettes
      _sp  = 0;
      }

TextLine::TextLine(const TextLine& e)
   : SLine(e)
      {
      _lineWidth            = e._lineWidth;
      _lineColor            = e._lineColor;
      _lineStyle            = e._lineStyle;
      _beginTextPlace       = e._beginTextPlace;
      _continueTextPlace    = e._continueTextPlace;

      _beginHook            = e._beginHook;
      _endHook              = e._endHook;
      _beginHookHeight      = e._beginHookHeight;
      _endHookHeight        = e._endHookHeight;
      _beginHookType        = e._beginHookType;
      _endHookType          = e._endHookType;

      _beginSymbol          = e._beginSymbol;
      _continueSymbol       = e._continueSymbol;
      _endSymbol            = e._endSymbol;
      _beginSymbolOffset    = e._beginSymbolOffset;
      _continueSymbolOffset = e._continueSymbolOffset;
      _endSymbolOffset      = e._endSymbolOffset;
      _mxmlOff2             = e._mxmlOff2;
      _beginText            = 0;
      _continueText         = 0;
      if (e._beginText)
            _beginText = e._beginText->clone(); // deep copy
      if (e._continueText)
            _continueText = e._continueText->clone();
      _sp = 0;
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLine::setBeginText(const QString& s, TextStyleType textStyle)
      {
      if (!_beginText) {
            _beginText = new Text(score());
            _beginText->setParent(this);
            _beginText->setSubtype(TEXT_TEXTLINE);
            _beginText->setTextStyle(textStyle);
            }
      _beginText->setText(s);
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLine::setContinueText(const QString& s, TextStyleType textStyle)
      {
      if (!_continueText) {
            _continueText = new Text(score());
            _continueText->setParent(this);
            _continueText->setSubtype(TEXT_TEXTLINE);
            _continueText->setTextStyle(textStyle);
            }
      _continueText->setText(s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLine::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextLine::read(QDomElement e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      setId(e.attribute("id", "-1").toInt());
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   writeProperties
//    write properties different from prototype
//---------------------------------------------------------

void TextLine::writeProperties(Xml& xml, const TextLine* proto) const
      {
// printf("  TextLine: begin text <%s> styled %d\n", qPrintable(_beginText->getText()), _beginText->styled());
      if (_beginHook) {
            if (proto == 0 || proto->beginHookHeight() != _beginHookHeight)
                  xml.tag("beginHookHeight", _beginHookHeight.val());
            if (proto == 0 || proto->beginHookType() != _beginHookType)
                  xml.tag("beginHookType", int(_beginHookType));
            }
      if (_endHook) {
            if (proto == 0 || proto->endHookHeight() != _endHookHeight)
                  xml.tag("endHookHeight", _endHookHeight.val());
            if (proto == 0 || proto->endHookType() != _endHookType)
                  xml.tag("endHookType", int(_endHookType));
            }

      if (proto == 0 || proto->lineWidth() != _lineWidth)
            xml.tag("lineWidth", _lineWidth.val());
      if (proto == 0 || proto->lineStyle() != _lineStyle)
            xml.tag("lineStyle", _lineStyle);
      if (proto == 0 || proto->lineColor() != _lineColor)
            xml.tag("lineColor", _lineColor);
      if (proto == 0 || proto->beginTextPlace() != _beginTextPlace)
            xml.pTag("beginTextPlace", _beginTextPlace);
      if (proto == 0 || proto->continueTextPlace() != _continueTextPlace)
            xml.pTag("continueTextPlace", _continueTextPlace);

      SLine::writeProperties(xml);
      if (_beginText) {
            xml.stag("beginText");
            _beginText->writeProperties(xml);
            xml.etag();
            }
      if (_continueText) {
            xml.stag("continueText");
            _continueText->writeProperties(xml);
            xml.etag();
            }
      if (_beginSymbol != -1) {
            xml.tag("beginSymbol", _beginSymbol);   // symbols[_symbol].name();
            xml.tag("beginSymbolOffset", _beginSymbolOffset);
            }
      if (_continueSymbol != -1) {
            xml.tag("continueSymbol", _continueSymbol);
            xml.tag("continueSymbolOffset", _continueSymbolOffset);
            }
      if (_endSymbol != -1) {
            xml.tag("endSymbol", _endSymbol);
            xml.tag("endSymbolOffset", _endSymbolOffset);
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextLine::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      const QString& text = e.text();
      if (tag == "beginHookHeight") {
            _beginHookHeight = Spatium(text.toDouble());
            _beginHook = true;
            }
      else if (tag == "beginHookType")
            _beginHookType = HookType(text.toInt());
      else if (tag == "endHookHeight" || tag == "hookHeight") { // hookHeight is obsolete
            _endHookHeight = Spatium(text.toDouble());
            _endHook = true;
            }
      else if (tag == "endHookType")
            _endHookType = HookType(text.toInt());
      else if (tag == "hookUp")           // obsolete
            _endHookHeight *= -1.0;
      else if (tag == "beginSymbol" || tag == "symbol")     // "symbol" is obsolete
            _beginSymbol = text.toInt();
      else if (tag == "continueSymbol")
            _continueSymbol = text.toInt();
      else if (tag == "endSymbol")
            _endSymbol = text.toInt();
      else if (tag == "beginSymbolOffset")
            _beginSymbolOffset = readPoint(e);
      else if (tag == "continueSymbolOffset")
            _continueSymbolOffset = readPoint(e);
      else if (tag == "endSymbolOffset")
            _endSymbolOffset = readPoint(e);
      else if (tag == "lineWidth")
            _lineWidth = Spatium(text.toDouble());
      else if (tag == "lineStyle")
            _lineStyle = Qt::PenStyle(text.toInt());
      else if (tag == "beginTextPlace")
            _beginTextPlace = readPlacement(e);
      else if (tag == "continueTextPlace")
            _continueTextPlace = readPlacement(e);
      else if (tag == "lineColor")
            _lineColor = readColor(e);
      else if (tag == "beginText") {
            _beginText = new Text(score());
            _beginText->setSubtype(TEXT_TEXTLINE);
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (!_beginText->readProperties(ee))
                        domError(e);
                  }
            }
      else if (tag == "continueText") {
            _continueText = new Text(score());
            _continueText->setSubtype(TEXT_TEXTLINE);
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (!_continueText->readProperties(ee))
                        domError(e);
                  }
            }
      else if (!SLine::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* TextLine::createLineSegment()
      {
      return new TextLineSegment(score());
      }

//---------------------------------------------------------
//   setBeginText
//---------------------------------------------------------

void TextLine::setBeginText(Text* v)
      {
      delete _beginText;
      _beginText = v;
      }

//---------------------------------------------------------
//   setContinueText
//---------------------------------------------------------

void TextLine::setContinueText(Text* v)
      {
      delete _continueText;
      _continueText = v;
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick() to _tick2
//---------------------------------------------------------

void TextLine::layout()
      {
      setPos(0.0, 0.0);
      SLine::layout();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TextLineSegment::spatiumChanged(double ov, double nv)
      {
      parent()->spatiumChanged(ov, nv);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TextLine::spatiumChanged(double ov, double nv)
      {
      if (_beginText)
            _beginText->spatiumChanged(ov, nv);
      if (_continueText)
            _continueText->spatiumChanged(ov, nv);
      }
