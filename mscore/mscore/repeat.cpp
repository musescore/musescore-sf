//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "repeat.h"
#include "sym.h"
#include "score.h"
#include "jumpproperties.h"
#include "markerproperties.h"
#include "system.h"
#include "measure.h"
#include "globals.h"

//---------------------------------------------------------
//   JumpProperties
//---------------------------------------------------------

JumpProperties::JumpProperties(Jump* jp, QWidget* parent)
   : QDialog(parent)
      {
      jump = jp;
      setupUi(this);
      jumpTo->setText(jump->jumpTo());
      playUntil->setText(jump->playUntil());
      continueAt->setText(jump->continueAt());
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void JumpProperties::saveValues()
      {
      jump->setJumpTo(jumpTo->text());
      jump->setPlayUntil(playUntil->text());
      jump->setContinueAt(continueAt->text());
      }

//---------------------------------------------------------
//   MarkerProperties
//---------------------------------------------------------

MarkerProperties::MarkerProperties(Marker* mk, QWidget* parent)
   : QDialog(parent)
      {
      marker = mk;
      setupUi(this);
      label->setText(marker->label());
      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void MarkerProperties::saveValues()
      {
      marker->setLabel(label->text());
      }

//---------------------------------------------------------
//   RepeatMeasure
//---------------------------------------------------------

RepeatMeasure::RepeatMeasure(Score* score)
   : Rest(score)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void RepeatMeasure::draw(QPainter& p, ScoreView*) const
      {
      p.setBrush(p.pen().color());
      p.drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RepeatMeasure::layout()
      {
      double sp  = spatium();

      double y   = sp;
      double w   = sp * 2.0;
      double h   = sp * 2.0;
      double lw  = sp * .30;  // line width
      double r   = sp * .15;  // dot radius

      path       = QPainterPath();

      path.moveTo(w - lw, y);
      path.lineTo(w,  y);
      path.lineTo(lw,  h+y);
      path.lineTo(0.0, h+y);
      path.closeSubpath();
      path.addEllipse(QRectF(w * .25 - r, y+h * .25 - r, r * 2.0, r * 2.0 ));
      path.addEllipse(QRectF(w * .75 - r, y+h * .75 - r, r * 2.0, r * 2.0 ));

      setbbox(path.boundingRect());
      }

//---------------------------------------------------------
//   Marker
//---------------------------------------------------------

Marker::Marker(Score* s)
   : Text(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setSubtype(TEXT_REPEAT);
      setTextStyle(TEXT_STYLE_REPEAT);
      }

//---------------------------------------------------------
//   setMarkerType
//---------------------------------------------------------

void Marker::setMarkerType(MarkerType t)
      {
      _markerType = t;
      switch(t) {
            case MARKER_SEGNO:
                  setHtml(symToHtml(symbols[score()->symIdx()][segnoSym], 8));
                  setLabel("segno");
                  break;

            case MARKER_CODA:
                  setHtml(symToHtml(symbols[score()->symIdx()][codaSym], 8));
                  setLabel("codab");
                  break;

            case MARKER_VARCODA:
                  setHtml(symToHtml(symbols[score()->symIdx()][varcodaSym], 8));
                  setLabel("varcoda");
                  break;

            case MARKER_CODETTA:
                  setHtml(symToHtml(symbols[score()->symIdx()][codaSym], symbols[score()->symIdx()][codaSym], 8));
                  setLabel("codetta");
                  break;

            case MARKER_FINE:
                  setText("Fine");
                  setLabel("fine");
                  break;

            case MARKER_TOCODA:
                  setText("To Coda");
                  setLabel("coda");
                  break;

            case MARKER_USER:
                  break;

            default:
                  printf("unknown marker type %d\n", t);
                  break;
            }
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Marker::styleChanged()
      {
      setMarkerType(_markerType);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Marker::layout()
      {
      Text::layout();
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF Marker::canvasPos() const
      {
      if (parent())
            return measure()->canvasPos() + pos();
      return pos();
      }

//---------------------------------------------------------
//   markerType
//---------------------------------------------------------

MarkerType Marker::markerType(const QString& s) const
      {
      if (s == "segno")
            return MARKER_SEGNO;
      else if (s == "codab")
            return MARKER_CODA;
      else if (s == "varcoda")
            return MARKER_VARCODA;
      else if (s == "codetta")
            return MARKER_CODETTA;
      else if (s == "fine")
            return MARKER_FINE;
      else if (s == "coda")
            return MARKER_TOCODA;
      else
            return MARKER_USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Marker::read(QDomElement e)
      {
      MarkerType mt;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "label") {
                  setLabel(e.text());
                  mt = markerType(e.text());
                  }
            else if (!Text::readProperties(e))
                  domError(e);
            }
      switch(mt) {
            case MARKER_SEGNO:
            case MARKER_CODA:
            case MARKER_VARCODA:
            case MARKER_CODETTA:
                  setTextStyle(TEXT_STYLE_REPEAT_LEFT);
                  break;

            case MARKER_FINE:
            case MARKER_TOCODA:
                  setTextStyle(TEXT_STYLE_REPEAT_RIGHT);
                  break;

            case MARKER_USER:
                  setTextStyle(TEXT_STYLE_REPEAT);
                  break;
            }
      setMarkerType(mt);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Marker::write(Xml& xml) const
      {
      xml.stag(name());
      Text::writeProperties(xml);
      xml.tag("label", _label);
      xml.etag();
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Marker::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Marker Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Marker::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            MarkerProperties rp(this);
            rp.exec();
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Marker::dragAnchor() const
      {
      return QLineF(measure()->canvasPos(), canvasPos());
      }

//---------------------------------------------------------
//   Jump
//---------------------------------------------------------

Jump::Jump(Score* s)
   : Text(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setSubtype(TEXT_REPEAT);
      setTextStyle(TEXT_STYLE_REPEAT);
      }

//---------------------------------------------------------
//   setJumpType
//---------------------------------------------------------

void Jump::setJumpType(int t)
      {
      switch(t) {
            case JUMP_DC:
                  setText("D.C.");
                  setJumpTo("start");
                  setPlayUntil("end");
                  break;

            case JUMP_DC_AL_FINE:
                  setText("D.C. al Fine");
                  setJumpTo("start");
                  setPlayUntil("fine");
                  break;

            case JUMP_DC_AL_CODA:
                  setText("D.C. al Coda");
                  setJumpTo("start");
                  setPlayUntil("coda");
                  setContinueAt("codab");
                  break;

            case JUMP_DS_AL_CODA:
                  setText("D.S. al Coda");
                  setJumpTo("segno");
                  setPlayUntil("coda");
                  setContinueAt("codab");
                  break;

            case JUMP_DS_AL_FINE:
                  setText("D.S. al Fine");
                  setJumpTo("segno");
                  setPlayUntil("fine");
                  break;

            case JUMP_DS:
                  setText("D.S.");
                  setJumpTo("segno");
                  setPlayUntil("end");
                  break;

            case JUMP_USER:
                  break;

            default:
                  printf("unknown jump type\n");
                  break;
            }
      }

//---------------------------------------------------------
//   jumpType
//---------------------------------------------------------

int Jump::jumpType() const
      {
      if (_jumpTo == "start" && _playUntil == "end" && _continueAt == "")
            return JUMP_DC;
      else if (_jumpTo == "start" && _playUntil == "fine" && _continueAt == "")
            return JUMP_DC_AL_FINE;
      else if (_jumpTo == "start" && _playUntil == "coda" && _continueAt == "codab")
            return JUMP_DC_AL_CODA;
      else if (_jumpTo == "segno" && _playUntil == "coda" && _continueAt == "codab")
            return JUMP_DS_AL_CODA;
      else if (_jumpTo == "segno" && _playUntil == "fine" && _continueAt == "")
            return JUMP_DS_AL_FINE;
      else if (_jumpTo == "segno" && _playUntil == "end" && _continueAt == "")
            return JUMP_DS;
      return JUMP_USER;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Jump::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "jumpTo")
                  _jumpTo = e.text();
            else if (tag == "playUntil")
                  _playUntil = e.text();
            else if (tag == "continueAt")
                  _continueAt = e.text();
            else if (!Text::readProperties(e))
                  domError(e);
            }
      setTextStyle(TEXT_STYLE_REPEAT_RIGHT);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Jump::write(Xml& xml) const
      {
      xml.stag(name());
      Text::writeProperties(xml);
      xml.tag("jumpTo", _jumpTo);
      xml.tag("playUntil", _playUntil);
      xml.tag("continueAt", _continueAt);
      xml.etag();
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Jump::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Jump Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Jump::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            JumpProperties rp(this);
            rp.exec();
            }
      else
            Element::propertyAction(viewer, s);
      }
