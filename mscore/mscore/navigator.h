//=============================================================================
//  MuseScore
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

#ifndef __NAVIGATOR_H__
#define __NAVIGATOR_H__

class Score;
class ScoreView;

//---------------------------------------------------------
//   Navigator
//---------------------------------------------------------

class Navigator : public QFrame {
      Q_OBJECT

      Score* _score;
      QPointer<ScoreView> _cv;

      QRect viewRect;
      QPoint startMove;
      bool moving;
      QPixmap pm;
      bool redraw;
      QTransform matrix;

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void resizeEvent(QResizeEvent*);

   public slots:
      void updateViewRect();
      void updateLayout();

   signals:
      void viewRectMoved(const QRectF&);

   public:
      Navigator(QWidget* parent = 0);
      void setScore(ScoreView*);
      void setViewRect(const QRectF& r);
      void layoutChanged();
      };

#endif
