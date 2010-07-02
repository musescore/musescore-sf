//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __BOX_H__
#define __BOX_H__

/**
 \file
 Definition of HBox and VBox classes.
*/

#include "measurebase.h"

class BarLine;
class ScoreView;

//---------------------------------------------------------
//   Box
//---------------------------------------------------------

class Box : public MeasureBase {

      Spatium _boxWidth;       // only valid for HBox
      Spatium _boxHeight;      // only valid for VBox
      double _leftMargin, _rightMargin;   // values in metric mm
      double _topMargin, _bottomMargin;
      bool editMode;

   public:
      Box(Score*);
      virtual void draw(QPainter&, ScoreView*) const;

      virtual bool isEditable() { return true; }
      virtual void startEdit(ScoreView*, const QPointF&);
      virtual bool edit(ScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void editDrag(int, const QPointF&);
      virtual void endEditDrag();
      virtual void endEdit();
      virtual void updateGrips(int* grips, QRectF*) const;
      virtual void layout();
      virtual void write(Xml&) const;
      virtual void write(Xml& xml, int, bool) const { write(xml); }
      virtual void read(QDomElement);

      Spatium boxWidth() const         { return _boxWidth;     }
      void setBoxWidth(Spatium val)    { _boxWidth = val;      }
      Spatium boxHeight() const        { return _boxHeight;    }
      void setBoxHeight(Spatium val)   { _boxHeight = val;     }
      double leftMargin() const        { return _leftMargin;   }
      double rightMargin() const       { return _rightMargin;  }
      double topMargin() const         { return _topMargin;    }
      double bottomMargin() const      { return _bottomMargin; }
      void setLeftMargin(double val)   { _leftMargin = val;    }
      void setRightMargin(double val)  { _rightMargin = val;   }
      void setTopMargin(double val)    { _topMargin = val;     }
      void setBottomMargin(double val) { _bottomMargin = val;  }
      };

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

class HBox : public Box {
      Q_DECLARE_TR_FUNCTIONS(HBox)

   public:
      HBox(Score* score);
      ~HBox();
      virtual HBox* clone() const      { return new HBox(*this); }
      virtual ElementType type() const { return HBOX;       }

      virtual void layout();
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);
      virtual QRectF drag(const QPointF& s);
      void layout2();
      };

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

class VBox : public Box {
      Q_DECLARE_TR_FUNCTIONS(VBox)

   public:
      VBox(Score* score) : Box(score) {}
      virtual VBox* clone() const      { return new VBox(*this); }
      virtual ElementType type() const { return VBOX;       }

      virtual void layout();

      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(ScoreView*, const QString&);

      virtual void add(Element*);
      virtual void remove(Element*);
      };

#endif

