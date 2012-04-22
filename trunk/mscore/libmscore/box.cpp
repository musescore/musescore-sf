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

#include "box.h"
#include "text.h"
#include "score.h"
#include "barline.h"
#include "repeat.h"
#include "symbol.h"
#include "system.h"
#include "image.h"
#include "layoutbreak.h"
#include "fret.h"
#include "mscore.h"
#include "stafftext.h"

static const qreal BOX_MARGIN = 0.0;

//---------------------------------------------------------
//   propertyList
//---------------------------------------------------------

static qreal zeroVal = 0.0;
static qreal defaultBoxMargin = BOX_MARGIN;

Property<Box> Box::propertyList[] = {
      { P_BOX_HEIGHT,    &Box::pBoxHeight,    &zeroVal },
      { P_BOX_WIDTH,     &Box::pBoxWidth,     &zeroVal },
      { P_TOP_GAP,       &Box::pTopGap,       &zeroVal },
      { P_BOTTOM_GAP,    &Box::pBottomGap,    &zeroVal },
      { P_LEFT_MARGIN,   &Box::pLeftMargin,   &defaultBoxMargin },
      { P_RIGHT_MARGIN,  &Box::pRightMargin,  &defaultBoxMargin },
      { P_TOP_MARGIN,    &Box::pTopMargin,    &defaultBoxMargin },
      { P_BOTTOM_MARGIN, &Box::pBottomMargin, &defaultBoxMargin },
      { P_END, 0, 0 }
      };

//---------------------------------------------------------
//   Box
//---------------------------------------------------------

Box::Box(Score* score)
   : MeasureBase(score)
      {
      editMode      = false;
      _boxWidth     = Spatium(0);
      _boxHeight    = Spatium(0);
      _leftMargin   = BOX_MARGIN;
      _rightMargin  = BOX_MARGIN;
      _topMargin    = BOX_MARGIN;
      _bottomMargin = BOX_MARGIN;
      _topGap       = 0.0;
      _bottomGap    = 0.0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Box::layout()
      {
      MeasureBase::layout();
      foreach (Element* el, _el) {
            if (el->type() != LAYOUT_BREAK)
                  el->layout();
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Box::draw(QPainter* painter) const
      {
      if (score() && score()->printing())
            return;
      if (selected() || editMode || dropTarget() || score()->showFrames()) {
            qreal w = 2.0 / painter->transform().m11();
            painter->setPen(QPen(
               (selected() || editMode || dropTarget()) ? Qt::blue : Qt::gray, w));
            painter->setBrush(Qt::NoBrush);
            w *= .5;
            painter->drawRect(bbox().adjusted(w, w, -w, -w));
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Box::startEdit(MuseScoreView*, const QPointF&)
      {
      editMode = true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Box::edit(MuseScoreView*, int /*grip*/, int /*key*/, Qt::KeyboardModifiers, const QString&)
      {
      return false;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Box::editDrag(const EditData& ed)
      {
      if (type() == VBOX) {
            _boxHeight = Spatium((ed.pos.y() - abbox().y()) / spatium());
            if (ed.vRaster) {
                  qreal vRaster = 1.0 / MScore::vRaster();
                  int n = lrint(_boxHeight.val() / vRaster);
                  _boxHeight = Spatium(vRaster * n);
                  }
            setbbox(QRectF(0.0, 0.0, system()->width(), point(boxHeight())));
            system()->setHeight(height());
            score()->doLayoutPages();
            }
      else {
            _boxWidth += Spatium(ed.delta.x() / spatium());
            if (ed.hRaster) {
                  qreal hRaster = 1.0 / MScore::hRaster();
                  int n = lrint(_boxWidth.val() / hRaster);
                  _boxWidth = Spatium(hRaster * n);
                  }

            foreach(Element* e, _el) {
                  if (e->type() == TEXT) {
                        static_cast<Text*>(e)->setModified(true);  // force relayout
                        }
                  }
            score()->setLayoutAll(true);
            }
      layout();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Box::endEdit()
      {
      editMode = false;
      layout();
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Box::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 1;
      QRectF r(abbox());
      if (type() == HBOX)
            grip[0].translate(QPointF(r.right(), r.top() + r.height() * .5));
      else if (type() == VBOX)
            grip[0].translate(QPointF(r.x() + r.width() * .5, r.bottom()));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Box::write(Xml& xml) const
      {
      xml.stag(name());

      for (int i = 0;; ++i) {
            const Property<Box>& p = propertyList[i];
            P_ID id = p.id;
            if (id == P_END)
                  break;
            QVariant val        = getVariant(id, ((*(Box*)this).*(p.data))());
            QVariant defaultVal = getVariant(id, p.defaultVal);
            if (val != defaultVal)
                  xml.tag(propertyName(id), val);
            }

      Element::writeProperties(xml);
      foreach (const Element* el, _el)
            el->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Box::read(const QDomElement& de)
      {
      _leftMargin = _rightMargin = _topMargin = _bottomMargin = 0.0;
      bool keepMargins = false;        // whether original margins have to be kept when reading old file

      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            if (setProperty(tag, e))
                  ;
            else if (tag == "Text") {
                  Text* t;
                  if (type() == TBOX) {
                        t = static_cast<TBox*>(this)->getText();
                        t->read(e);
                        }
                  else {
                        t = new Text(score());
                        t->read(e);
                        add(t);
                        }
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->read(e);
                  add(s);
                  }
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  QDomElement ee = e.firstChildElement("path");
                  if (!ee.isNull())
                        path = ee.text();
                  Image* image = 0;
                  QString s(path.toLower());
                  if (s.endsWith(".svg"))
                        image = new SvgImage(score());
                  else
                        if (s.endsWith(".jpg")
                     || s.endsWith(".png")
                     || s.endsWith(".gif")
                     || s.endsWith(".xpm")
                        ) {
                        image = new RasterImage(score());
                        }
                  else {
                        qDebug("unknown image format <%s>\n", path.toLatin1().data());
                        }
                  if (image) {
                        image->setTrack(score()->curTrack);
                        image->read(e);
                        add(image);
                        }
                  }
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(e);
                  add(lb);
                  }
            else if (tag == "HBox") {
                  HBox* hb = new HBox(score());
                  hb->read(e);
                  add(hb);
                  keepMargins = true;     // in old file, box nesting used outer box margins
                  }
            else if (tag == "VBox") {
                  VBox* vb = new VBox(score());
                  vb->read(e);
                  add(vb);
                  keepMargins = true;     // in old file, box nesting used outer box margins
                  }
            else
                  domError(e);
            }

      // with .msc versions prior to 1.17, box margins were only used when nesting another box inside this box:
      // for backward compatibility set them to 0 in all other cases

      if (score()->mscVersion() < 117 && (type() == HBOX || type() == VBOX) && !keepMargins)  {
            _leftMargin = _rightMargin = _topMargin = _bottomMargin = 0.0;
            }
      }

//---------------------------------------------------------
//   add
///   Add new Element \a el to Box
//---------------------------------------------------------

void Box::add(Element* e)
      {
      if (e->type() == TEXT)
            static_cast<Text*>(e)->setLayoutToParentWidth(true);
      MeasureBase::add(e);
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

Property<Box>* Box::property(P_ID id) const
      {
      for (int i = 0;; ++i) {
            if (propertyList[i].id == P_END)
                  break;
            if (propertyList[i].id == id)
                  return &propertyList[i];
            }
      qFatal("Box: property %d not found\n", id);
      return 0;
      }

//---------------------------------------------------------
//   getProperty
//---------------------------------------------------------

QVariant Box::getProperty(P_ID propertyId) const
      {
      Property<Box>* p = property(propertyId);
      if (p)
            return getVariant(propertyId, ((*(Box*)this).*(p->data))());
      return Element::getProperty(propertyId);
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

bool Box::setProperty(P_ID propertyId, const QVariant& v)
      {
      score()->addRefresh(canvasBoundingRect());
      Property<Box>* p = property(propertyId);
      bool rv = true;
      if (p) {
            setVariant(propertyId, ((*this).*(p->data))(), v);
            setGenerated(false);
            }
      else
            rv = Element::setProperty(propertyId, v);
      score()->setLayoutAll(true);
      return rv;
      }

bool Box::setProperty(const QString& name, const QDomElement& e)
      {
      for (int i = 0;; ++i) {
            P_ID id = propertyList[i].id;
            if (id == P_END)
                  break;
            if (propertyName(id) == name) {
                  QVariant v = ::getProperty(id, e);
                  setVariant(id, ((*this).*(propertyList[i].data))(), v);
                  setGenerated(false);
                  return true;
                  }
            }
      return Element::setProperty(name, e);
      }

//---------------------------------------------------------
//   propertyDefault
//---------------------------------------------------------

QVariant Box::propertyDefault(P_ID id) const
      {
      return getVariant(id, property(id)->defaultVal);
      }

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

HBox::HBox(Score* score)
   : Box(score)
      {
      setBoxWidth(Spatium(5.0));
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HBox::layout()
      {
      if (parent() && parent()->type() == VBOX) {
            VBox* vb = static_cast<VBox*>(parent());
            qreal x = vb->leftMargin() * MScore::DPMM;
            qreal y = vb->topMargin() * MScore::DPMM;
            qreal w = point(boxWidth());
            qreal h = vb->height() - (vb->topMargin() + vb->bottomMargin()) * MScore::DPMM;
            setPos(x, y);
            setbbox(QRectF(0.0, 0.0, w, h));
            }
      else {
            setbbox(QRectF(0.0, 0.0, point(boxWidth()), system()->height()));
            }
      Box::layout();
      adjustReadPos();
      }

//---------------------------------------------------------
//   layout2
//    height (bbox) is defined now
//---------------------------------------------------------

void HBox::layout2()
      {
      Box::layout();
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Box::acceptDrop(MuseScoreView*, const QPointF&, Element* e) const
      {
      int type = e->type();
      if (type == LAYOUT_BREAK || type == TEXT || type == STAFF_TEXT)
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Box::drop(const DropData& data)
      {
      Element* e = data.element;
      switch(e->type()) {
            case LAYOUT_BREAK:
                  {
                  LayoutBreak* lb = static_cast<LayoutBreak*>(e);
                  if (_pageBreak || _lineBreak) {
                        if (
                           (lb->subtype() == LAYOUT_BREAK_PAGE && _pageBreak)
                           || (lb->subtype() == LAYOUT_BREAK_LINE && _lineBreak)
                           || (lb->subtype() == LAYOUT_BREAK_SECTION && _sectionBreak)
                           ) {
                              //
                              // if break already set
                              //
                              delete lb;
                              break;
                              }
                        foreach(Element* elem, _el) {
                              if (elem->type() == LAYOUT_BREAK) {
                                    score()->undoChangeElement(elem, e);
                                    break;
                                    }
                              }
                        break;
                        }
                  lb->setTrack(-1);       // this are system elements
                  lb->setParent(this);
                  score()->undoAddElement(lb);
                  return lb;
                  }
            case STAFF_TEXT:
                  {
                  Text* text = new Text(score());
                  text->setTextStyle(score()->textStyle(TEXT_STYLE_FRAME));
                  text->setParent(this);
                  text->setHtml(static_cast<StaffText*>(e)->getHtml());
                  score()->undoAddElement(text);
                  delete e;
                  return text;
                  }

            default:
                  e->setParent(this);
                  score()->undoAddElement(e);
                  return e;
            }
      return 0;
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF HBox::drag(const EditData& data)
      {
      QRectF r(canvasBoundingRect());
      qreal diff = data.delta.x();
      qreal x1   = userOff().x() + diff;
      if (parent()->type() == VBOX) {
            VBox* vb = static_cast<VBox*>(parent());
            qreal x2 = parent()->width() - width() - (vb->leftMargin() + vb->rightMargin()) * MScore::DPMM;
            if (x1 < 0.0)
                  x1 = 0.0;
            else if (x1 > x2)
                  x1 = x2;
            }
      setUserOff(QPointF(x1, 0.0));
      setStartDragPosition(data.pos);
      return canvasBoundingRect() | r;
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void HBox::endEditDrag()
      {
      score()->setLayoutAll(true);
      score()->end2();
      }

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool HBox::isMovable() const
      {
      return parent() && (parent()->type() == HBOX || parent()->type() == VBOX);
      }

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

VBox::VBox(Score* score)
   : Box(score)
      {
      setBoxHeight(Spatium(10.0));
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VBox::layout()
      {
      setPos(QPointF());      // !?
      if (system())
            setbbox(QRectF(0.0, 0.0, system()->width(), point(boxHeight())));
      else
            setbbox(QRectF(0.0, 0.0, 50, 50));
      Box::layout();
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

QPointF VBox::getGrip(int) const
      {
      return QPointF(0.0, boxHeight().val());
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void VBox::setGrip(int, const QPointF& pt)
      {
      setBoxHeight(Spatium(pt.y()));
      layout();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FBox::layout()
      {
      setPos(QPointF());      // !?
      setbbox(QRectF(0.0, 0.0, system()->width(), point(boxHeight())));
      Box::layout();
      }

//---------------------------------------------------------
//   add
///   Add new Element \a e to fret diagram box
//---------------------------------------------------------

void FBox::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == FRET_DIAGRAM) {
            FretDiagram* fd = static_cast<FretDiagram*>(e);
            fd->setFlag(ELEMENT_MOVABLE, false);
            }
      else {
            qDebug("FBox::add: element not allowed\n");
            return;
            }
      _el.append(e);
      }

