//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include <QtTest/QtTest>
#include "mtest/testutils.h"
#include "libmscore/score.h"

#define DIR QString("libmscore/beam/")

//---------------------------------------------------------
//   TestBeam
//---------------------------------------------------------

class TestBeam : public QObject, public MTest
      {
      Q_OBJECT

      void beam(const char* path);

   private slots:
      void initTestCase();
      void beamA() { beam("Beam-A.mscx"); }
      void beamB() { beam("Beam-B.mscx"); }
      void beamC() { beam("Beam-C.mscx"); }
      void beamD() { beam("Beam-D.mscx"); }
      void beamE() { beam("Beam-E.mscx"); }
      void beamF() { beam("Beam-F.mscx"); }
      void beamG() { beam("Beam-G.mscx"); }
      };

//---------------------------------------------------------
//   initTestCase
//---------------------------------------------------------

void TestBeam::initTestCase()
      {
      initMTest();
      }

//---------------------------------------------------------
//   beam
//---------------------------------------------------------

void TestBeam::beam(const char* path)
      {
      Score* score = readScore(DIR + path);
      QVERIFY(score);
      QVERIFY(saveCompareScore(score, path, DIR + path));
      delete score;
      }

QTEST_MAIN(TestBeam)
#include "tst_beam.moc"

