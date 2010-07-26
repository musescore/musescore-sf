//=============================================================================
//  MuseScore
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

#ifndef __SCORETAB_H__
#define __SCORETAB_H__

class ScoreView;
class Score;

//---------------------------------------------------------
//   ScoreTab
//---------------------------------------------------------

class ScoreTab : public QWidget {
      Q_OBJECT
      QList<Score*>* scoreList;
      QTabBar* tab;
      QTabBar* tab2;
      QStackedLayout* stack;

   private slots:
      void setCurrent(int);
      void setExcerpt(int);

   signals:
      void currentScoreViewChanged(ScoreView*);
      void tabCloseRequested(int);

   public:
      ScoreTab(QList<Score*>*, QWidget* parent = 0);
      void insertTab(int idx, const QString&);
      void setTabText(int, const QString&);
      int currentIndex() const;
      void setCurrentIndex(int);
      void removeTab(int);
      int count() const { return scoreList->size(); }
      ScoreView* view(int) const;
      QSplitter* viewSplitter(int n) const;

      ScoreView* view() const { return view(currentIndex()); }
      bool contains(ScoreView*) const;
      void initScoreView(int idx, double mag, int magIdx, double xoffset, double yoffset);
      };

#endif

