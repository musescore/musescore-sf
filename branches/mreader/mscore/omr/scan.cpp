//=============================================================================
//  MusE Reader
//  Music Score Reader
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

#include "globals.h"
#include "scan.h"
#include "pdf.h"
#include "page.h"
// #include "xml.h"

//---------------------------------------------------------
//   Scan
//---------------------------------------------------------

Scan::Scan()
      {
      _doc = 0;
      }

//---------------------------------------------------------
//   pagesInDocument
//---------------------------------------------------------

int Scan::pagesInDocument() const
      {
      return _doc ? _doc->numPages() : 0;
      }

//---------------------------------------------------------
//   read
//    return true on success
//---------------------------------------------------------

bool Scan::read(const QString& path)
      {
      _pdfPath  = path;
      _doc = new Pdf(path);

      int n = _doc->numPages();
      if (maxPages && (maxPages < n)) {
            printf("process only %d pages\n", maxPages);
            n = maxPages;
            }
      for (int i = startPage; i < startPage+n; ++i) {
            Page* page = new Page(this);
            QImage image = _doc->page(i);
            page->setImage(image);
            pages.append(page);
            }
      double sp = 0;
      double w  = 0;
      for (int i = 0; i < n; ++i) {
            pages[i]->read();
            sp += pages[i]->spatium();
            w  += pages[i]->width();
printf("page %d: %d %d: %f\n", i, pages[i]->width(), pages[i]->height(),
              double(pages[i]->height()) / double(pages[i]->width()));
            }
      _spatium = sp / n;
      w       /= n;
      _dpmm    = w / 210.0;            // PaperSize A4

printf("*** spatium: %f mm  dpmm: %f\n", spatiumMM(), _dpmm);
      return true;
      }

//---------------------------------------------------------
//   spatiumMM
//---------------------------------------------------------

double Scan::spatiumMM() const
      {
      return _spatium / _dpmm;
      }

//---------------------------------------------------------
//   staffDistance
//---------------------------------------------------------

double Scan::staffDistance() const
      {
      return pages[0]->staffDistance();
      }

//---------------------------------------------------------
//   systemDistance
//---------------------------------------------------------

double Scan::systemDistance() const
      {
      return pages[0]->systemDistance();
      }



