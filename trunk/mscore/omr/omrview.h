//=============================================================================
//  MusE Reader
//  Linux Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#ifndef __SCAN_VIEW_H__
#define __SCAN_VIEW_H__

class Omr;
class Page;
class ScoreView;

//---------------------------------------------------------
//   Tile
//---------------------------------------------------------

struct Tile {
      int no;
      QRect r;
      QPixmap pm;

      Tile();
      };

static const int TILE_H = 512;
static const int TILE_W = 512;

//---------------------------------------------------------
//   OmrView
//---------------------------------------------------------

class OmrView : public QWidget {
      Q_OBJECT
      Omr* _omr;
      ScoreView* _scoreView;
      int maxTiles;

      QList<Tile*> usedTiles;
      QStack<Tile*> freeTiles;
      QPoint startDrag;

      QTransform _matrix;
      int xoff, yoff;
      int curPage;

      bool   _fotoMode;
      QRectF _foto;

      void zoom(int step, const QPoint& pos);

      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void wheelEvent(QWheelEvent*);
      virtual void paintEvent(QPaintEvent*);
      virtual void contextMenuEvent(QContextMenuEvent*);

      qreal mag() const { return _matrix.m11(); }
      void setMag(double mag);
      void initTile(Tile* t, int pageWidth);

   public slots:
      void setScale(double);
      void setOffset(double, double);

   signals:
      void pageNumberChanged(int);
      void xPosChanged(int);
      void yPosChanged(int);

   public:
      OmrView(ScoreView*, QWidget* parent = 0);
      void setOmr(Omr*);
      Omr* omr() const           { return _omr;      }
      bool fotoMode() const      { return _fotoMode; }
      void setFotoMode(bool val) { _fotoMode = val;  }
      };


#endif

