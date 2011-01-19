//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: textstyle.cpp,v 1.8 2006/03/02 17:08:43 wschweer Exp $
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

#include "style.h"
#include "textstyle.h"
#include "globals.h"
#include "score.h"

static const int INTERNAL_STYLES = 2;     // do not present first two styles to user

//---------------------------------------------------------
//   TextStyleDialog
//---------------------------------------------------------

TextStyleDialog::TextStyleDialog(QWidget* parent, Score* score)
   : QDialog(parent)
      {
      setupUi(this);
      cs = score;
      foreach(TextStyle* s, score->textStyles())
            styles.append(new TextStyle(*s));

      QFontDatabase fdb;
      QStringList families = fdb.families();
      fonts = 0;
      for (QStringList::Iterator f = families.begin(); f != families.end(); ++f) {
            QString family = *f;
            if (fdb.isSmoothlyScalable(family)) {
                  fontName->addItem(family);
                  ++fonts;
                  }
            }
      textNames->setSelectionMode(QListWidget::SingleSelection);
      textNames->clear();
      for (int i = INTERNAL_STYLES; i < styles.size(); ++i) {
            TextStyle* s = styles.at(i);
            textNames->addItem(qApp->translate("MuseScore", s->name.toAscii().data()));
            }

      connect(textNames,     SIGNAL(currentRowChanged(int)), SLOT(nameSelected(int)));
      connect(buttonOk,      SIGNAL(clicked()), SLOT(ok()));
      connect(buttonApply,   SIGNAL(clicked()), SLOT(apply()));
      connect(fontBold,      SIGNAL(clicked()), SLOT(fontChanged()));
      connect(fontUnderline, SIGNAL(clicked()), SLOT(fontChanged()));
      connect(fontItalic,    SIGNAL(clicked()), SLOT(fontChanged()));
      connect(fontSize,      SIGNAL(valueChanged(int)), SLOT(fontSizeChanged(int)));
      connect(fontName,      SIGNAL(activated(int)), SLOT(fontNameChanged(int)));
      connect(leftH,         SIGNAL(clicked()), SLOT(alignLeftH()));
      connect(rightH,        SIGNAL(clicked()), SLOT(alignRightH()));
      connect(centerH,       SIGNAL(clicked()), SLOT(alignCenterH()));
      connect(topV,          SIGNAL(clicked()), SLOT(alignTopV()));
      connect(bottomV,       SIGNAL(clicked()), SLOT(alignBottomV()));
      connect(centerV,       SIGNAL(clicked()), SLOT(alignCenterV()));

      QButtonGroup* bg = new QButtonGroup(this);
      bg->setExclusive(true);
      bg->addButton(unitMM, 0);
      bg->addButton(unitSpace, 1);
      connect(bg, SIGNAL(buttonClicked(int)), SLOT(unitChanged(int)));

      current = -1;
      textNames->setCurrentItem(textNames->item(0));
      }

//---------------------------------------------------------
//   ~TextStyleDialog
//---------------------------------------------------------

TextStyleDialog::~TextStyleDialog()
      {
      foreach(TextStyle* ts, styles)
            delete ts;
      styles.clear();
      }

void TextStyleDialog::alignLeftH()
      {
      TextStyle* s = styles[current];
      s->align &= ~(ALIGN_LEFT | ALIGN_RIGHT | ALIGN_HCENTER);
      s->align |= ALIGN_LEFT;
      }

void TextStyleDialog::alignRightH()
      {
      TextStyle* s = styles[current];
      s->align &= ~(ALIGN_LEFT | ALIGN_RIGHT | ALIGN_HCENTER);
      s->align |= ALIGN_RIGHT;
      }

void TextStyleDialog::alignCenterH()
      {
      TextStyle* s = styles[current];
      s->align &= ~(ALIGN_LEFT | ALIGN_RIGHT | ALIGN_HCENTER);
      s->align |= ALIGN_HCENTER;
      }

void TextStyleDialog::alignTopV()
      {
      TextStyle* s = styles[current];
      s->align &= ~(ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER);
      s->align |= ALIGN_TOP;
      }

void TextStyleDialog::alignBottomV()
      {
      TextStyle* s = styles[current];
      s->align &= ~(ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER);
      s->align |= ALIGN_BOTTOM;
      }

void TextStyleDialog::alignCenterV()
      {
      TextStyle* s = styles[current];
      s->align &= ~(ALIGN_TOP | ALIGN_BOTTOM | ALIGN_VCENTER);
      s->align |= ALIGN_VCENTER;
      }

//---------------------------------------------------------
//   unitChanged
//---------------------------------------------------------

void TextStyleDialog::unitChanged(int)
      {
      int unit = 0;
      if (unitSpace->isChecked())
            unit = 1;
      if (unit == curUnit)
            return;
      curUnit = unit;

      if (curUnit) {
            xOffset->setValue(xOffset->value() * DPMM / _spatium);
            yOffset->setValue(yOffset->value() * DPMM / _spatium);
            }
      else {
            xOffset->setValue(xOffset->value() * _spatium / DPMM);
            yOffset->setValue(yOffset->value() * _spatium / DPMM);
            }
      }

//---------------------------------------------------------
//   nameSelected
//---------------------------------------------------------

void TextStyleDialog::nameSelected(int n)
      {
      if (current != -1)
            saveStyle(current);
      TextStyle* s = styles[n + INTERNAL_STYLES];

      fontBold->setChecked(s->bold);
      fontItalic->setChecked(s->italic);
      fontUnderline->setChecked(s->underline);
      fontSize->setValue(s->size);

      if (s->align & ALIGN_RIGHT)
            rightH->setChecked(true);
      else if (s->align & ALIGN_HCENTER)
            centerH->setChecked(true);
      else
            leftH->setChecked(true);

      if (s->align & ALIGN_BOTTOM)
            bottomV->setChecked(true);
      else if (s->align & ALIGN_VCENTER)
            centerV->setChecked(true);
      else if (s->align & ALIGN_BASELINE)
            baselineV->setChecked(true);
      else
            topV->setChecked(true);

      QString str;
      if (s->offsetType == OFFSET_ABS) {
            xOffset->setValue(s->xoff * INCH);
            yOffset->setValue(s->yoff * INCH);
            unitMM->setChecked(true);
            curUnit = 0;
            }
      else if (s->offsetType == OFFSET_SPATIUM) {
            xOffset->setValue(s->xoff);
            yOffset->setValue(s->yoff);
            unitSpace->setChecked(true);
            curUnit = 1;
            }
      rxOffset->setValue(s->rxoff);
      ryOffset->setValue(s->ryoff);

      QFont f(s->family);
      f.setPixelSize(lrint(s->size * PDPI / PPI));
      f.setItalic(s->italic);
      f.setUnderline(s->underline);
      f.setBold(s->bold);
      fontSample->clear();
      fontSample->setFont(f);
      fontSample->setText(tr("Ich und du, Muellers Kuh..."));
      int i;
      for (i = 0; i < fonts; ++i) {
            QString ls = fontName->itemText(i);
            if (ls.toLower() == s->family.toLower()) {
                  fontName->setCurrentIndex(i);
                  break;
                  }
            }
      if (i == fonts) {
            printf("font not in list: <%s>\n", s->family.toLower().toLatin1().data());
            }
      borderColor->setColor(s->frameColor);
      borderWidth->setValue(s->frameWidth);
      paddingWidth->setValue(s->paddingWidth);
      frameRound->setValue(s->frameRound);
      circleButton->setChecked(s->circle);
      current = n + INTERNAL_STYLES;
      }

//---------------------------------------------------------
//   fontChanged
//---------------------------------------------------------

void TextStyleDialog::fontChanged()
      {
      QFont f(fontSample->font());
      f.setItalic(fontItalic->isChecked());
      f.setUnderline(fontUnderline->isChecked());
      f.setBold(fontBold->isChecked());
      fontSample->setFont(f);
      }

//---------------------------------------------------------
//   fontSizeChanged
//---------------------------------------------------------

void TextStyleDialog::fontSizeChanged(int n)
      {
      QFont f(fontSample->font());
      f.setPixelSize(lrint(n * PDPI / PPI));
      fontSample->setFont(f);
      }

//---------------------------------------------------------
//   fontNameChanged
//---------------------------------------------------------

void TextStyleDialog::fontNameChanged(int)
      {
      QFont f(fontSample->font());
      f.setFamily(fontName->currentText());
      fontSample->setFont(f);
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

void TextStyleDialog::saveStyle(int n)
      {
      TextStyle* s    = styles[n];
      if (curUnit == 0)
            s->offsetType = OFFSET_ABS;
      else if (curUnit == 1)
            s->offsetType = OFFSET_SPATIUM;
      s->bold         = fontBold->isChecked();
      s->italic       = fontItalic->isChecked();
      s->underline    = fontUnderline->isChecked();
      s->size         = fontSize->value();
      s->family       = strdup(fontName->currentText().toLatin1().data());  // memory leak
      s->xoff         = xOffset->value() / ((s->offsetType == OFFSET_ABS) ? INCH : 1.0);
      s->yoff         = yOffset->value() / ((s->offsetType == OFFSET_ABS) ? INCH : 1.0);
      s->rxoff        = rxOffset->value();
      s->ryoff        = ryOffset->value();
      s->frameColor   = borderColor->color();
      s->frameWidth   = borderWidth->value();
      s->paddingWidth = paddingWidth->value();
      s->circle       = circleButton->isChecked();
      s->frameRound   = frameRound->value();
      }

//---------------------------------------------------------
//   ok
//---------------------------------------------------------

void TextStyleDialog::ok()
      {
      apply();
      done(0);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void TextStyleDialog::apply()
      {
      cs->startCmd();
      saveStyle(current);
      cs->setTextStyles(styles);
      cs->setLayoutAll(true);
      cs->textStyleChanged();
      cs->endCmd();
      cs->setDirty(true);
      }
