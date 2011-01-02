//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: canvas.h,v 1.35 2006/09/15 09:34:57 wschweer Exp $
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

#ifndef __SCANVAS_H__
#define __SCANVAS_H__

#include "viewer.h"

class Rest;
class Element;
class Transformation;
class Page;
class Xml;
class Note;
class Lasso;
class ShadowNote;
class Navigator;
class Cursor;
class ElementList;
class Segment;
class Measure;

//---------------------------------------------------------
//   Canvas
//---------------------------------------------------------

class Canvas : public QFrame, public Viewer {
      Q_OBJECT

   public:
      enum State {
         NORMAL, DRAG_OBJ, EDIT, DRAG_EDIT, LASSO, NOTE_ENTRY, MAG
         };

   private:
      Navigator* navigator;
      ScoreLayout* _layout;
      int level;

      State state;
      bool dragCanvasState;
      bool draggedCanvas;
      Element* dragElement;   // current moved drag&drop element
      Element* dragObject;    // current canvas element

      QPointF dragOffset;
      bool mousePressed;

      // editing mode
      int curGrip;
      QRectF grip[4];         // edit "grips"
      int grips;              // number of used grips

      QPointF startMove;

      //--input state:
      Cursor* cursor;
      QTimer* cursorTimer;    // blink timer
      ShadowNote* shadowNote;

      Lasso* lasso;           ///< temporarily drawn lasso selection
      QRectF _lassoRect;

      QColor _bgColor;
      QColor _fgColor;
      QPixmap* bgPixmap;
      QPixmap* fgPixmap;

      //============================================

      virtual void paintEvent(QPaintEvent*);
      void paint(const QRect&, QPainter&);
      virtual void updateAll(Score*) { update(); }

      void canvasPopup(const QPoint&);
      void objectPopup(const QPoint&, Element*);
      void measurePopup(const QPoint&, Measure*);

      void saveChord(Xml&);

      virtual void resizeEvent(QResizeEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void wheelEvent(QWheelEvent*);
      void mouseMoveEvent1(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void mouseDoubleClickEvent(QMouseEvent*);
      virtual bool event(QEvent*);

      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragLeaveEvent(QDragLeaveEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void dropEvent(QDropEvent*);

      virtual void keyPressEvent(QKeyEvent*);

      void contextItem(Element*);

      void lassoSelect();
      Note* searchTieNote(Note* note);

      void setShadowNote(const QPointF&);
      void drawElements(QPainter& p,const QList<const Element*>& el);
      bool dragTimeAnchorElement(const QPointF& pos);
      void dragSymbol(const QPointF& pos);
      bool dragMeasureAnchorElement(const QPointF& pos);
      bool dragAboveMeasure(const QPointF& pos);
      bool dragAboveSystem(const QPointF& pos);
      void updateGrips();
      const QList<const Element*> elementsAt(const QPointF&);

   private slots:
      void cursorBlink();

   public slots:
      void cmdCut();
      void cmdCopy();
      void cmdPaste();
      void setViewRect(const QRectF&);

   public:
      Canvas(QWidget* parent = 0);
      ~Canvas();

      void setBackground(QPixmap*);
      void setBackground(const QColor&);
      void setForeground(QPixmap*);
      void setForeground(const QColor&);

      Page* addPage();

      void modifyElement(Element* obj);

      virtual void moveCursor();
      virtual void moveCursor(Segment*);
      virtual void setCursorOn(bool);
      void clearScore();

      virtual void dataChanged(const QRectF&);
      void setState(State);
      State getState() const { return state; }
      bool startEdit(Element*, int startGrip = -1);
      void setScore(Score* s, ScoreLayout*);

      qreal mag() const;
      qreal xoffset() const;
      qreal yoffset() const;
      void setMag(qreal m);
      void setOffset(qreal x, qreal y);
      qreal xMag() const { return _matrix.m11(); }
      qreal yMag() const { return _matrix.m22(); }

      QRectF vGeometry() const {
            return imatrix.mapRect(geometry());
            }

      QSizeF fsize() const;
      void showNavigator(bool visible);
      void redraw(const QRectF& r);
      void updateNavigator(bool layoutChanged) const;
      Element* elementAt(const QPointF& pp);
      Element* elementNear(const QPointF& pp);
      QRectF lassoRect() const { return _lassoRect; }
      void setLassoRect(const QRectF& r) { _lassoRect = r; }
      void paintLasso(QPainter& p, double mag);
      bool navigatorVisible() const;
      void magCanvas();
      };

extern int searchStaff(const Element* element);

#endif
