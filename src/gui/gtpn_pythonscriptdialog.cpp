/* GTlab - Gas Turbine laboratory
 * Source File: gtpn_pythonscriptdialog.cpp
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)
 *
 *  Created on: 22.03.2024
 *  Author: Jens Schmeink (AT-TWK)
 */

#include <QVBoxLayout>
#include <QTextOption>

#include "gtpy_contextmanager.h"
#include "gtpy_scripteditorwidget.h"

#include "gtpn_pythonscriptdialog.h"

GtpnPythonScriptDialog::GtpnPythonScriptDialog(QWidget* parent)
{
    setWindowTitle(tr("Config Python Node"));

    auto* layout = new QVBoxLayout;

    auto* contextManager = GtpyContextManager::instance();
    m_context = contextManager->createNewContext(
                GtpyContextManager::ScriptEditorContext);

    m_editor = new GtpyScriptEditorWidget(m_context, this);

    layout->addWidget(m_editor);

    setLayout(layout);
}

GtpnPythonScriptDialog::~GtpnPythonScriptDialog()
{
    GtpyContextManager::instance()->deleteContext(m_context);
}

QString
GtpnPythonScriptDialog::pythonScript()
{
    if (!m_editor) return {};

    return m_editor->script();
}

void
GtpnPythonScriptDialog::setScript(const QString& str)
{
    if (!m_editor) return;

    m_editor->setScript(str);
}

int
GtpnPythonScriptDialog::context() const
{
    return m_context;
}
