//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: line.h,v 1.2 2006/03/02 17:08:34 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __LINE_H__
#define __LINE_H__

#include "element.h"
#include "globals.h"

class SLine;
class System;
class Viewer;

//---------------------------------------------------------
//   LineSegment
//    Virtual base class for OttavaSegment, PedalSegment
//    HairpinSegment and TrillSegment.
//    This class describes part of a line object. Line objects
//    can span multiple staves. For every staff an Segment
//    is created.
//---------------------------------------------------------

class LineSegment : public Element {
   protected:
      QPointF _p2;
      QPointF _userOff2;
      QRectF r1, r2;
      LineSegmentType _segmentType;
      System* _system;

      virtual bool isMovable() const { return true; }
      virtual bool startEdit(Viewer*, const QPointF&);
      virtual void editDrag(int, const QPointF&);
      virtual bool edit(Viewer*, int grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int) const;
      virtual QPointF pos2anchor(const QPointF& pos, int* tick) const;
      virtual void layout(ScoreLayout*) {}

   public:
      LineSegment(Score* s);
      virtual void draw(QPainter& p) const = 0;
      SLine* line() const                 { return (SLine*)parent(); }
      const QPointF& userOff2() const     { return _userOff2;  }
      void setUserOff2(const QPointF& o)  { _userOff2 = o;     }
      void setUserXoffset2(qreal x)       { _userOff2.setX(x); }
      void setPos2(const QPointF& p)      { _p2 = p;     }
      void setXpos2(qreal x)              { _p2.setX(x); }
      QPointF pos2() const                { return _p2 + _userOff2 * _spatium; }
      void setLineSegmentType(LineSegmentType s)  { _segmentType = s;  }
      void setSystem(System* s)                   { _system = s;       }
      virtual void resetUserOffsets();
      friend class SLine;
      };

//---------------------------------------------------------
//   SLine
//    virtual base class for Ottava, Pedal, Hairpin,
//    Trill and TextLine
//---------------------------------------------------------

class SLine : public Element {
   protected:
      QList<LineSegment*> segments;
      int _tick2;
      bool _diagonal;

   public:
      SLine(Score* s);
      virtual void draw(QPainter& p) const;
      void setTick2(int t);
      int tick2() const    { return _tick2; }
      virtual void layout(ScoreLayout*);
      bool readProperties(QDomElement node);
      void writeProperties(Xml& xml) const;
      virtual LineSegment* createLineSegment() = 0;
      void setLen(double l);
      virtual void collectElements(QList<const Element*>& el) const;
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void change(Element* o, Element* n);
      virtual QRectF bbox() const;
      QList<LineSegment*> lineSegments() { return segments; }
      virtual QPointF tick2pos(int grip, int tick, int staff, System** system);
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      bool diagonal() const    { return _diagonal; }
      void setDiagonal(bool v) { _diagonal = v; }
      };

typedef QList<LineSegment*>::iterator iLineSegment;
typedef QList<LineSegment*>::const_iterator ciLineSegment;

#endif
