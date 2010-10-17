//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
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

#include "textproperties.h"
#include "text.h"
#include "score.h"

//---------------------------------------------------------
//   TextProp
//---------------------------------------------------------

TextProp::TextProp(bool os, QWidget* parent)
   : QWidget(parent)
      {
      onlyStyle = os;
      setupUi(this);
      QButtonGroup* g1 = new QButtonGroup(this);
      g1->addButton(alignLeft);
      g1->addButton(alignHCenter);
      g1->addButton(alignRight);

      QButtonGroup* g2 = new QButtonGroup(this);
      g2->addButton(alignTop);
      g2->addButton(alignVCenter);
      g2->addButton(alignBottom);

      QButtonGroup* g3 = new QButtonGroup(this);
      g3->addButton(circleButton);
      g3->addButton(boxButton);

      connect(mmUnit, SIGNAL(toggled(bool)), SLOT(mmToggled(bool)));
      connect(styledGroup, SIGNAL(toggled(bool)), SLOT(styledToggled(bool)));
      connect(unstyledGroup, SIGNAL(toggled(bool)), SLOT(unstyledToggled(bool)));
      }

//---------------------------------------------------------
//   mmToggled
//---------------------------------------------------------

void TextProp::mmToggled(bool val)
      {
      QString unit(val ? tr("mm", "millimeter unit") : tr("sp", "spatium unit"));
      xOffset->setSuffix(unit);
      yOffset->setSuffix(unit);
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void TextProp::set(TextB* tb)
      {
      styledGroup->setChecked(tb->styled());
      unstyledGroup->setChecked(!tb->styled());

      Score* score = tb->score();
      styles->clear();
      styles->addItem(tr("no style"));
      foreach(const TextStyle& st, score->style().textStyles())
            styles->addItem(st.name());
      int styleIdx = 0;
      if (tb->styled()) {
            if (tb->textStyle() != TEXT_STYLE_INVALID)
                  styleIdx = tb->textStyle() + 1;
            }
      styles->setCurrentIndex(styleIdx);

      int a = int(tb->align());
      if (a & ALIGN_HCENTER)
            alignHCenter->setChecked(true);
      else if (a & ALIGN_RIGHT)
            alignRight->setChecked(true);
      else
            alignLeft->setChecked(true);

      if (a & ALIGN_VCENTER)
            alignVCenter->setChecked(true);
      else if (a & ALIGN_BOTTOM)
            alignBottom->setChecked(true);
      else if (a & ALIGN_BASELINE)
            alignBaseline->setChecked(true);
      else
            alignTop->setChecked(true);

      systemFlag->setChecked(tb->systemFlag());

      QFont f = tb->defaultFont();
      fontBold->setChecked(f.bold());
      fontItalic->setChecked(f.italic());
      fontUnderline->setChecked(f.underline());
      color->setColor(tb->color());

      double ps = f.pointSizeF();
      fontSize->setValue(lrint(ps));
      fontSelect->setCurrentFont(f);

      frameWidth->setValue(tb->frameWidth());
      frame->setChecked(tb->textBase()->hasFrame());
      paddingWidth->setValue(tb->paddingWidth());
      frameColor->setColor(tb->frameColor());
      frameRound->setValue(tb->frameRound());
      circleButton->setChecked(tb->circle());
      boxButton->setChecked(!tb->circle());

      xOffset->setValue(tb->xoff());
      yOffset->setValue(tb->yoff());
      rxOffset->setValue(tb->reloff().x());
      ryOffset->setValue(tb->reloff().y());
      mmUnit->setChecked(tb->offsetType() == OFFSET_ABS);
      spatiumUnit->setChecked(tb->offsetType() == OFFSET_SPATIUM);

      mmToggled(tb->offsetType() == OFFSET_ABS);      // set suffix on spin boxes
      }

//---------------------------------------------------------
//   get
//---------------------------------------------------------

void TextProp::get(TextB* tb)
      {
      if (unstyledGroup->isChecked())
            tb->setTextStyle(TextStyleType(styles->currentIndex() - 1));
      tb->textBase()->setHasFrame(frame->isChecked());
      tb->setFrameWidth(frameWidth->value());
      tb->setPaddingWidth(paddingWidth->value());
      tb->setFrameColor(frameColor->color());
      tb->setFrameRound(frameRound->value());
      tb->setCircle(circleButton->isChecked());

      QFont f = fontSelect->currentFont();
      double ps = fontSize->value();
      f.setPointSizeF(ps);
      f.setBold(fontBold->isChecked());
      f.setItalic(fontItalic->isChecked());
      f.setUnderline(fontUnderline->isChecked());
      tb->setDefaultFont(f);
      tb->setColor(color->color());
      tb->setSystemFlag(systemFlag->isChecked());

      int a = 0;
      if (alignHCenter->isChecked())
            a |= ALIGN_HCENTER;
      else if (alignRight->isChecked())
            a |= ALIGN_RIGHT;
      if (alignVCenter->isChecked())
            a |= ALIGN_VCENTER;
      else if (alignBottom->isChecked())
            a |= ALIGN_BOTTOM;
      else if (alignBaseline->isChecked())
            a |= ALIGN_BASELINE;
      tb->setAlign(Align(a));
      tb->doc()->setModified(true);       // force relayout

      tb->setXoff(xOffset->value());
      tb->setYoff(yOffset->value());
      tb->setReloff(QPointF(rxOffset->value(), ryOffset->value()));
      tb->setOffsetType(mmUnit->isChecked() ? OFFSET_ABS : OFFSET_SPATIUM);
      tb->setStyled(styledGroup->isChecked());
      }

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void TextProp::set(const TextStyle& s)
      {
      styledGroup->setVisible(false);
      unstyledGroup->setCheckable(false);
      unstyledGroup->setTitle(tr("Text Style"));

      fontBold->setChecked(s.bold());
      fontItalic->setChecked(s.italic());
      fontUnderline->setChecked(s.underline());
      fontSize->setValue(s.size());
      color->setColor(s.foregroundColor());

      systemFlag->setChecked(s.systemFlag());
      int a = s.align();
      if (a & ALIGN_HCENTER)
            alignHCenter->setChecked(true);
      else if (a & ALIGN_RIGHT)
            alignRight->setChecked(true);
      else
            alignLeft->setChecked(true);

      if (a & ALIGN_VCENTER)
            alignVCenter->setChecked(true);
      else if (a & ALIGN_BOTTOM)
            alignBottom->setChecked(true);
      else if (a & ALIGN_BASELINE)
            alignBaseline->setChecked(true);
      else
            alignTop->setChecked(true);

      QString str;
      if (s.offsetType() == OFFSET_ABS) {
            xOffset->setValue(s.xoff() * INCH);
            yOffset->setValue(s.yoff() * INCH);
            mmUnit->setChecked(true);
            curUnit = 0;
            }
      else if (s.offsetType() == OFFSET_SPATIUM) {
            xOffset->setValue(s.xoff());
            yOffset->setValue(s.yoff());
            spatiumUnit->setChecked(true);
            curUnit = 1;
            }
      rxOffset->setValue(s.rxoff());
      ryOffset->setValue(s.ryoff());

      QFont f(s.family());
      f.setPixelSize(lrint(s.size() * PDPI / PPI));
      f.setItalic(s.italic());
      f.setUnderline(s.underline());
      f.setBold(s.bold());
      fontSelect->setCurrentFont(f);

      frameColor->setColor(s.frameColor());
      frameWidth->setValue(s.frameWidth());
      frame->setChecked(s.hasFrame());
      paddingWidth->setValue(s.paddingWidth());
      frameRound->setValue(s.frameRound());
      circleButton->setChecked(s.circle());
      }

//---------------------------------------------------------
//   get
//---------------------------------------------------------

TextStyle TextProp::getTextStyle() const
      {
      TextStyle s;
      if (curUnit == 0)
            s.setOffsetType(OFFSET_ABS);
      else if (curUnit == 1)
            s.setOffsetType(OFFSET_SPATIUM);
      s.setBold(fontBold->isChecked());
      s.setItalic(fontItalic->isChecked());
      s.setUnderline(fontUnderline->isChecked());
      s.setSize(fontSize->value());
      QFont f = fontSelect->currentFont();
      s.setFamily(f.family());
      s.setXoff(xOffset->value() / ((s.offsetType() == OFFSET_ABS) ? INCH : 1.0));
      s.setYoff(yOffset->value() / ((s.offsetType() == OFFSET_ABS) ? INCH : 1.0));
      s.setRxoff(rxOffset->value());
      s.setRyoff(ryOffset->value());
      s.setFrameColor(frameColor->color());
      s.setFrameWidth(frameWidth->value());
      s.setPaddingWidth(paddingWidth->value());
      s.setCircle(circleButton->isChecked());
      s.setFrameRound(frameRound->value());
      s.setHasFrame(frame->isChecked());
      s.setSystemFlag(systemFlag->isChecked());
      s.setForegroundColor(color->color());

      Align a = 0;
      if (alignHCenter->isChecked())
            a |= ALIGN_HCENTER;
      else if (alignRight->isChecked())
            a |= ALIGN_RIGHT;

      if (alignVCenter->isChecked())
            a |= ALIGN_VCENTER;
      else if (alignBottom->isChecked())
            a |= ALIGN_BOTTOM;
      else if (alignBaseline->isChecked())
            a |= ALIGN_BASELINE;
      s.setAlign(a);
      return s;
      }

//---------------------------------------------------------
//   styledToggled
//---------------------------------------------------------

void TextProp::styledToggled(bool val)
      {
      unstyledGroup->setChecked(!val);
      }

//---------------------------------------------------------
//   unstyledToggled
//---------------------------------------------------------

void TextProp::unstyledToggled(bool val)
      {
      styledGroup->setChecked(!val);
      }
