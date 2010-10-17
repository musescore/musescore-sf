/****************************************************************************
**
** Copyright (C) 2007-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Script Debug project on Trolltech Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSCRIPTEMBEDDEDDEBUGGER_H
#define QSCRIPTEMBEDDEDDEBUGGER_H

#include <QtCore/qobject.h>

class QScriptEngine;
class QMainWindow;
class QScriptDebugger;
class QScriptEngineDebuggerFrontend;

class QScriptEmbeddedDebugger : public QObject
{
    Q_OBJECT
public:
    QScriptEmbeddedDebugger(QObject *parent = 0);
    ~QScriptEmbeddedDebugger();

    void attachTo(QScriptEngine *engine);
    void detach();

    void breakAtFirstStatement();

signals:
    void executionHalted();

private slots:
    void _q_onExecutionHalted();

private:
    QScriptEngineDebuggerFrontend *m_frontend;
    QScriptDebugger *m_debugger;
    QMainWindow *m_window;
};

#endif