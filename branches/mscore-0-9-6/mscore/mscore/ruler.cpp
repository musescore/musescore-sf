//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "ruler.h"
#include "score.h"

static const int MAP_OFFSET = 2;

QPixmap* Ruler::markIcon[3];

static const char* rmark_xpm[]={
      "18 18 2 1",
      "# c #0000ff",
      ". c None",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "........##########",
      "........#########.",
      "........########..",
      "........#######...",
      "........######....",
      "........#####.....",
      "........####......",
      "........###.......",
      "........##........",
      "........##........",
      "........##........"};
static const char* lmark_xpm[]={
      "18 18 2 1",
      "# c #0000ff",
      ". c None",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "##########........",
      ".#########........",
      "..########........",
      "...#######........",
      "....######........",
      ".....#####........",
      "......####........",
      ".......###........",
      "........##........",
      "........##........",
      "........##........"};
static const char* cmark_xpm[]={
      "18 18 2 1",
      "# c #ff0000",
      ". c None",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "..................",
      "##################",
      ".################.",
      "..##############..",
      "...############...",
      "....##########....",
      ".....########.....",
      "......######......",
      ".......####.......",
      "........##........",
      "........##........",
      "........##........"};


//---------------------------------------------------------
//   Ruler
//---------------------------------------------------------

Ruler::Ruler(QWidget* parent)
   : QWidget(parent)
      {
      if (markIcon[0] == 0) {
            markIcon[0] = new QPixmap(cmark_xpm);
            markIcon[1] = new QPixmap(lmark_xpm);
            markIcon[2] = new QPixmap(rmark_xpm);
            }
      setMouseTracking(true);
      magStep = 0;
      _xpos   = 0;
      _xmag   = 0.1;
      _timeType = AL::TICKS;
      _font2.setPixelSize(14);
      _font2.setBold(true);
      _font1.setPixelSize(10);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Ruler::setScore(Score* s, AL::Pos* lc)
      {
      _score = s;
      _locator = lc;
      _cursor.setContext(s->tempomap(), s->sigmap());
      }

//---------------------------------------------------------
//   setXmag
//---------------------------------------------------------

void Ruler::setMag(double x, double /*y*/)
      {
      if (_xmag != x) {
            _xmag = x;

            int tpix  = (480 * 4) * _xmag;
            magStep = 0;
            if (tpix < 64)
                  magStep = 1;
            if (tpix < 32)
                  magStep = 2;
            if (tpix <= 16)
                  magStep = 3;
            if (tpix < 8)
                  magStep = 4;
            if (tpix <= 4)
                  magStep = 5;
            if (tpix <= 2)
                  magStep = 6;
            update();
            }
      }

//---------------------------------------------------------
//   setXpos
//---------------------------------------------------------

void Ruler::setXpos(int val)
      {
      _xpos = val;
      update();
      }

//---------------------------------------------------------
//   pix2pos
//---------------------------------------------------------

AL::Pos Ruler::pix2pos(int x) const
      {
      int val = lrint((x + _xpos - MAP_OFFSET)/_xmag - 480);
      if (val < 0)
            val = 0;
      return AL::Pos(_score->tempomap(), _score->sigmap(), val, _timeType);
      }

//---------------------------------------------------------
//   pos2pix
//---------------------------------------------------------

int Ruler::pos2pix(const AL::Pos& p) const
      {
      return lrint((p.time(_timeType)+480) * _xmag) + MAP_OFFSET - _xpos;
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void Ruler::paintEvent(QPaintEvent* e)
      {
      QPainter p(this);
      const QRect& r = e->rect();

      static const int mag[7] = {
            1, 1, 2, 5, 10, 20, 50
            };

      int x  = r.x();
      int w  = r.width();
      int y  = rulerHeight - 16;
      int h  = 14;
      int y1 = r.y();
      int rh = r.height();
      if (y1 < rulerHeight) {
            rh -= rulerHeight - y1;
            y1 = rulerHeight;
            }
      int y2 = y1 + rh;

      if (x < (MAP_OFFSET - _xpos))
            x = MAP_OFFSET - _xpos;

      AL::Pos pos1 = pix2pos(x);
      AL::Pos pos2 = pix2pos(x+w);

      //---------------------------------------------------
      //    draw raster
      //---------------------------------------------------

      int bar1, bar2, beat, tick;

      pos1.mbt(&bar1, &beat, &tick);
      pos2.mbt(&bar2, &beat, &tick);

      int n = mag[magStep];

      bar1 = (bar1 / n) * n;        // round down
      if (bar1 && n >= 2)
            bar1 -= 1;
      bar2 = ((bar2 + n - 1) / n) * n; // round up

      for (int bar = bar1; bar <= bar2;) {
            AL::Pos stick(_score->tempomap(), _score->sigmap(), bar, 0, 0);
            if (magStep) {
                  p.setFont(_font2);
                  int x = pos2pix(stick);
                  QString s;
                  s.setNum(bar + 1);

                  p.setPen(Qt::black);
                  p.drawLine(x, y, x, y + h);
                  QRect r = QRect(x+2, y, 1000, h);
                  p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, s);
                  p.setPen(Qt::lightGray);
                  if (x > 0)
                        p.drawLine(x, y1, x, y2);
                  }
            else {
                  AL::SigEvent sig = stick.timesig();
                  int z = sig.fraction().numerator();
                  for (int beat = 0; beat < z; beat++) {
                        AL::Pos xx(_score->tempomap(), _score->sigmap(), bar, beat, 0);
                        int xp = pos2pix(xx);
                        if (xp < 0)
                              continue;
                        QString s;
                        QRect r(xp+2, y + 1, 1000, h);
                        int y3;
                        int num;
                        if (beat == 0) {
                              num = bar + 1;
                              y3  = y + 2;
                              p.setFont(_font2);
                              }
                        else {
                              num = beat + 1;
                              y3  = y + 8;
                              p.setFont(_font1);
                              r.moveTop(r.top() + 1);
                              }
                        s.setNum(num);
                        p.setPen(Qt::black);
                        p.drawLine(xp, y3, xp, y+h);
                        p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, s);
                        p.setPen(beat == 0 ? Qt::lightGray : Qt::gray);
                        if (xp > 0)
                              p.drawLine(xp, y1, xp, y2);
                        }
                  }
            if (bar == 0 && n >= 2)
                  bar += (n-1);
            else
                  bar += n;
            }
      //
      //  draw mouse cursor marker
      //
      p.setPen(Qt::black);
      if (_cursor.valid()) {
            int xp = pos2pix(_cursor);
            if (xp >= x && xp < x+w)
                  p.drawLine(xp, 0, xp, rulerHeight-1);
            }
      static const QColor lcColors[3] = { Qt::red, Qt::blue, Qt::blue };
      for (int i = 0; i < 3; ++i) {
            if (!_locator[i].valid())
                  continue;
            p.setPen(lcColors[i]);
            int xp      = pos2pix(_locator[i]);
            QPixmap* pm = markIcon[i];
            int pw = (pm->width() + 1) / 2;
            int x1 = x - pw;
            int x2 = x + w + pw;
            if (xp >= x1 && xp < x2)
                  p.drawPixmap(xp - pw, y-2, *pm);
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Ruler::mousePressEvent(QMouseEvent* e)
      {
      if (e->buttons() & Qt::LeftButton) {
            _locator[0] = pix2pos(e->pos().x());
            emit locatorMoved(0);
            update();
            }
      else if (e->buttons() & Qt::MidButton) {
            _locator[1] = pix2pos(e->pos().x());
            emit locatorMoved(1);
            update();
            }
      else if (e->buttons() & Qt::RightButton) {
            _locator[2] = pix2pos(e->pos().x());
            emit locatorMoved(2);
            update();
            }
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Ruler::mouseReleaseEvent(QMouseEvent*)
      {
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Ruler::mouseMoveEvent(QMouseEvent* event)
      {
      if (event->buttons() == 0) {
            _cursor = pix2pos(event->pos().x());
            emit posChanged(_cursor);
            }
      else if (event->buttons() & Qt::LeftButton) {
            _locator[0] = pix2pos(event->pos().x());
            emit locatorMoved(0);
            }
      else if (event->buttons() & Qt::MidButton) {
            _locator[1] = pix2pos(event->pos().x());
            emit locatorMoved(1);
            }
      else if (event->buttons() & Qt::RightButton) {
            _locator[2] = pix2pos(event->pos().x());
            emit locatorMoved(2);
            }
      update();
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void Ruler::leaveEvent(QEvent*)
      {
      _cursor.setInvalid();
      emit posChanged(_cursor);
      update();
      }

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void Ruler::setPos(const AL::Pos& pos)
      {
      if (_cursor != pos) {
            int x1 = pos2pix(_cursor);
            int x2 = pos2pix(pos);
            if (x1 > x2) {
                  int tmp = x2;
                  x2 = x1;
                  x1 = tmp;
                  }
            update(QRect(x1-1, 0, x2-x1+2, height()));
            _cursor = pos;
            }
      }
