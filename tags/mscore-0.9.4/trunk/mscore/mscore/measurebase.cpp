//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#include "measurebase.h"
#include "measure.h"
#include "staff.h"

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

MeasureBase::MeasureBase(Score* score)
   : Element(score)
      {
      _prev = 0;
      _next = 0;
      _lineBreak = false;
      _pageBreak = false;
      _dirty     = true;
      }

MeasureBase::MeasureBase(const MeasureBase& m)
   : Element(m)
      {
      _next      = m._next;
      _prev      = m._prev;
      _mw        = m._mw;
      _dirty     = m._dirty;
      _lineBreak = m._lineBreak;
      _pageBreak = m._pageBreak;

      foreach(Element* e, m._el) {
            add(e->clone());
            }
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void MeasureBase::collectElements(QList<const Element*>& el) const
      {
      foreach(Element* e, _el)
            e->collectElements(el);
      el.append(this);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

/**
 Add new Element \a el to MeasureBase
*/

void MeasureBase::add(Element* el)
      {
      el->setParent(this);
      _el.append(el);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

/**
 Remove Element \a el from MeasureBase.
*/

void MeasureBase::remove(Element* el)
      {
      if (!_el.remove(el))
            printf("MeasureBase(%p)::remove(%s,%p) not found\n", this, el->name(), el);
      }


//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

Measure* MeasureBase::nextMeasure()
      {
      MeasureBase* m = next();
      while (m) {
            if (m->type() == MEASURE)
                  return static_cast<Measure*>(m);
            m = m->next();
            }
      return 0;
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

Measure* MeasureBase::prevMeasure()
      {
      MeasureBase* m = prev();
      while (m) {
            if (m->type() == MEASURE)
                  return static_cast<Measure*>(m);
            m = m->prev();
            }
      return 0;
      }