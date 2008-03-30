//=============================================================================
//  MuseScore
//  Linux Music Score Editor
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

#include "textline.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "layout.h"
#include "score.h"
#include "preferences.h"

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
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void TextLineSegment::add(Element* e)
      {
      if (e->type() != TEXT) {
            printf("TextLineSegment: add illegal element\n");
            return;
            }
      _text = (TextC*)e;
      _text->setParent(this);
      TextLine* tl = (TextLine*)line();

      TextBase* tb = 0;
      if (_text->otb()) {
            tb = _text->otb();
            _text->setOtb(0);
            }
      else {
            tb = new TextBase(*tl->textBase());
            }
      tl->setTextBase(tb);
      _text->baseChanged();
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TextLineSegment::remove(Element* e)
      {
      if (e != _text) {
            printf("TextLineSegment: cannot remove %s %p %p\n", e->name(), e, _text);
            return;
            }
      _text->setOtb(_text->textBase());
      _text = 0;
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void TextLineSegment::collectElements(QList<const Element*>& el) const
      {
      if (_text)
            el.append(_text);
      el.append(this);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextLineSegment::draw(QPainter& p) const
      {
      qreal textlineLineWidth    = textLine()->lineWidth().point();
      qreal textlineTextDistance = _spatium * .5;

      QPointF pp2(pos2());

      qreal w = 0.0;
      if (_text) {
            QRectF bb(_text->bbox());
            w = bb.width();
            }

      QPointF pp1(w + textlineTextDistance, 0.0);

      QPen pen(p.pen());
      pen.setWidthF(textlineLineWidth);
      pen.setStyle(textLine()->lineStyle());

      if (selected() && !(score() && score()->printing()))
            pen.setColor(preferences.selectColor[0]);
      else
            pen.setColor(textLine()->lineColor());

      p.setPen(pen);
      p.drawLine(QLineF(pp1, pp2));

      double hh = textLine()->hookHeight().point();
      if (textLine()->hookUp())
            hh *= -1;
      if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_END) {
            p.drawLine(QLineF(pp2, QPointF(pp2.x(), hh)));
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF TextLineSegment::bbox() const
      {
      QPointF pp1;
      QPointF pp2(pos2());

      qreal h1 = _text ? _text->height() : 20.0;
      QRectF r(.0, -h1, pp2.x(), 2 * h1);
      return r;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLineSegment::layout(ScoreLayout* l)
      {
      if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_BEGIN) {
            if (_text == 0) {
                  TextLine* tl = (TextLine*)line();
                  _text = new TextC(tl->textBasePtr(), score());
                  _text->setSubtype(TEXT_TEXTLINE);
                  _text->setMovable(false);
                  _text->setParent(this);
                  }
            _text->layout(l);
            QRectF bb(_text->bbox());
            qreal textlineLineWidth = _spatium * .15;
            qreal h = bb.height() * .5 - textlineLineWidth * .5;
            _text->setPos(0.0, -h);
            }
      else if (_text) {
            delete _text;
            _text = 0;
            }
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::TextLine(Score* s)
   : SLine(s)
      {
      _text       = new TextBase;
      _hookHeight = Spatium(1.5);
      _lineWidth  = Spatium(0.15);
      _lineStyle  = Qt::SolidLine;
      _hookUp     = false;
      _lineColor  = Qt::black;
      }

TextLine::TextLine(const TextLine& e)
   : SLine(e)
      {
      _text       = e._text;
      _hookHeight = e._hookHeight;
      _lineWidth  = e._lineWidth;
      _lineStyle  = e._lineStyle;
      _hookUp     = e._hookUp;
      _lineColor  = e._lineColor;
      }

//---------------------------------------------------------
//   TextLine
//---------------------------------------------------------

TextLine::~TextLine()
      {
      // TextLine has no ownership of _text
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextLine::layout(ScoreLayout* layout)
      {
      SLine::layout(layout);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextLine::write(Xml& xml) const
      {
      xml.stag("TextLine");
      xml.tag("hookHeight", _hookHeight.val());
      xml.tag("lineWidth", _lineWidth.val());
      xml.tag("lineStyle", _lineStyle);
      xml.tag("hookUp", _hookUp);
      xml.tag("lineColor", _lineColor);

      SLine::writeProperties(xml);
      _text->writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextLine::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            const QString& text = e.text();
            if (tag == "hookHeight")
                  _hookHeight = Spatium(text.toDouble());
            else if (tag == "lineWidth")
                  _lineWidth = Spatium(text.toDouble());
            else if (tag == "lineStyle")
                  _lineStyle = Qt::PenStyle(text.toInt());
            else if (tag == "hookUp")
                  _hookUp = text.toInt();
            else if (tag == "lineColor")
                  _lineColor = readColor(e);
            else if (_text->readProperties(e))
                  ;
            else if (!SLine::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* TextLine::createLineSegment()
      {
      LineSegment* seg = new TextLineSegment(score());
      return seg;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool TextLineSegment::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void TextLineSegment::propertyAction(const QString& s)
      {
      if (s == "props") {
            LineProperties lp(textLine(), 0);
            lp.exec();
            }
      else
            Element::propertyAction(s);
      }

//---------------------------------------------------------
//   LineProperties
//---------------------------------------------------------

LineProperties::LineProperties(TextLine* l, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      tl = l;
      lineWidth->setValue(tl->lineWidth().val());
      hookHeight->setValue(tl->hookHeight().val());
      up->setChecked(tl->hookUp());
      lineStyle->setCurrentIndex(int(tl->lineStyle() - 1));
      text->setText(tl->text());
      linecolor->setColor(tl->lineColor());
      TextBase* tb = tl->textBase();
      QFont font(tb->defaultFont());
      textFont->setCurrentFont(font);
      textSize->setValue(font.pointSizeF());
      italic->setChecked(font.italic());
      bold->setChecked(font.bold());
      if (tb->frameWidth()) {
            frame->setChecked(true);
            frameWidth->setValue(tb->frameWidth());
            frameMargin->setValue(tb->paddingWidth());
            frameColor->setColor(tb->frameColor());
            frameCircled->setChecked(tb->circle());
            }
      else
            frame->setChecked(false);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void LineProperties::accept()
      {
      tl->setLineWidth(Spatium(lineWidth->value()));
      tl->setHookHeight(Spatium(hookHeight->value()));
      tl->setHookUp(up->isChecked());
      tl->setLineStyle(Qt::PenStyle(lineStyle->currentIndex() + 1));
      tl->setLineColor(linecolor->color());
      TextBase* tb = tl->textBase();
      QFont f(textFont->currentFont());
      f.setBold(bold->isChecked());
      f.setItalic(italic->isChecked());
      f.setPointSizeF(textSize->value());
      tb->setDefaultFont(f);
      if (frame->isChecked()) {
            tb->setFrameWidth(frameWidth->value());
            tb->setPaddingWidth(frameMargin->value());
            tb->setFrameColor(frameColor->color());
            tb->setCircle(frameCircled->isChecked());
            }
      else
            tb->setFrameWidth(0.0);
      tl->setText(text->text());

      QDialog::accept();
      }
