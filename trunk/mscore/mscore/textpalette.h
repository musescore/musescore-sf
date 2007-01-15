//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textpalette.h,v 1.2 2006/03/22 12:04:14 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __TEXTPALETTE_H__
#define __TEXTPALETTE_H__

#include "ui_textpalette.h"

class Text;

//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

class TextPalette : public QWidget, public Ui::TextPaletteBase {
      Q_OBJECT

      Text* _textElement;
      QTextCharFormat format;

   private slots:
      void symbolClicked(int);
      void sizeChanged(int value);
      void boldClicked(bool);
      void italicClicked(bool);

   public:
      TextPalette(QWidget* parent);
      void setText(Text* te) { _textElement = te; }
      void setCharFormat(QTextCharFormat);
      };

#endif

