/* GTlab - Gas Turbine laboratory
 * Source File: gtpn_pythonnodewidget.cpp
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)
 *
 *  Created on: 28.05.2026
 */

#include "gtpn_pythonnodewidget.h"

#include "gtpn_pythonscriptdialog.h"
#include "nodes/gtpy_pythonnode.h"

#include "gt_icons.h"
#include "gt_sharedfunction.h"
#include "gtpy_contextmanager.h"
#include "gtpy_gilscope.h"
#include "gtpy_tempdir.h"

#include "qsvgwidget.h"

#include <QAction>
#include <QDir>
#include <QFile>
#include <QMenu>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSettings>
#include <QSize>
#include <QTextCursor>
#include <QVBoxLayout>

std::unique_ptr<QWidget>
GtpyPythonNodeWidget::create(intelli::Node& node)
{
    auto* pythonNode = qobject_cast<GtpyPythonNode*>(&node);
    if (!pythonNode)
    {
        return {};
    }

    return std::make_unique<GtpyPythonNodeWidget>(*pythonNode);
}

GtpyPythonNodeWidget::GtpyPythonNodeWidget(GtpyPythonNode& node, QWidget* parent) :
    QWidget(parent),
    m_node(node)
{
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    // the optional plot widget based on svg
    m_plot = new QSvgWidget(this);
    m_plot->setVisible(m_node.plotActive());
    loadPlaceholder();
    layout->addWidget(m_plot);

    // the button to open the script editor
    m_button = new QPushButton(this);
    m_button->setIcon(gt::gui::icon::python());
    layout->addWidget(m_button);

    connect(&m_node, &GtpyPythonNode::onComputeChange,
            this, &GtpyPythonNodeWidget::updatePlot);
    connect(&m_node, &GtpyPythonNode::timePassed,
            this, &GtpyPythonNodeWidget::updateTimePassed);
    connect(&m_node.plotActiveProperty(), &GtBoolProperty::changed,
            this, &GtpyPythonNodeWidget::updatePlotVisibility);
    connect(m_button, &QPushButton::clicked,
            this, &GtpyPythonNodeWidget::openScriptEditor);
}

void
GtpyPythonNodeWidget::updatePlotVisibility()
{
    m_plot->setVisible(m_node.plotActive());

    if (m_node.plotActive())
    {
        resize(m_node.size());
        m_node.triggerNodeEvaluation();
    }
    else
    {
        resize(minimumSize());
    }

    m_node.syncPlotState(m_node.plotActive());
}

void
GtpyPythonNodeWidget::openScriptEditor()
{
    GTPY_GIL_SCOPE

    GtpnPythonScriptDialog dialog;

    dialog.setScript(m_node.script());
    m_node.prepareScriptEditorContext(dialog.context());

    QSettings settings;

    dialog.resize(settings.value("pythonNode/python_editor/size",
                                 QSize(400, 300)).toSize());

    if (gt::interface::getSharedFunction("Assistant", "prompt"))
    {
        gtInfo() << tr("GTlab Assistant found! installing filter...");

        QList<QWidget*> children = dialog.findChildren<QWidget*>();
        for (QWidget* child : qAsConst(children))
        {
            if (QString(child->metaObject()->className()) ==
                QStringLiteral("GtpyScriptView"))
            {
                if (auto* textEdit = qobject_cast<QPlainTextEdit*>(child))
                {
                    installAssistantContextMenu(textEdit);
                }
            }
        }
    }

    dialog.exec();

    const QString oldScript = m_node.script();
    m_node.setScript(dialog.pythonScript());

    if (oldScript != m_node.script())
    {
        gtTrace() << tr("Script changed");
        m_node.triggerNodeEvaluation();
    }

    settings.setValue("pythonNode/python_editor/size", dialog.size());
}

void
GtpyPythonNodeWidget::updatePlot(const QString& tmpPath)
{
    QDir tempDir(tmpPath);
    QFile figure(tempDir.absoluteFilePath("figure_0.svg"));

    if (figure.exists())
    {
        m_plot->load(tempDir.absoluteFilePath("figure_0.svg"));
        figure.remove();
    }
    else
    {
        loadPlaceholder();
    }
}

void
GtpyPythonNodeWidget::updateTimePassed(int progress)
{
    if (progress != 100)
    {
        m_button->setIcon(gt::gui::icon::processRunningIcon(progress));
    }
    else
    {
        m_button->setIcon(gt::gui::icon::python());
    }
}

void
GtpyPythonNodeWidget::showCustomContextMenu(const QPoint& pos)
{
    if (!m_textEdit)
    {
        return;
    }

    QMenu* menu = m_textEdit->createStandardContextMenu();
    QAction* customAction = new QAction(QStringLiteral("Assistant"), menu);

    connect(customAction, &QAction::triggered,
            this, &GtpyPythonNodeWidget::insertAssistantPrompt);

    menu->addSeparator();
    menu->addAction(customAction);
    menu->exec(m_textEdit->mapToGlobal(pos));
    delete menu;
}

void
GtpyPythonNodeWidget::insertAssistantPrompt()
{
    if (!m_textEdit)
    {
        return;
    }

    auto function = gt::interface::getSharedFunction("Assistant",
                                                     "open_prompt_python");

    if (function.isNull())
    {
        return;
    }

    QTextCursor cursor = m_textEdit->textCursor();
    const QString selectedText = cursor.selectedText();
    const auto retVals = function({selectedText});

    if (!retVals.isEmpty())
    {
        cursor.insertText(retVals[0].toString());
    }
}

void
GtpyPythonNodeWidget::loadPlaceholder()
{
    m_plot->load(QStringLiteral(":/pyNodes/figure_empty.svg"));
}

void
GtpyPythonNodeWidget::installAssistantContextMenu(QPlainTextEdit* textEdit)
{
    m_textEdit = textEdit;
    textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(textEdit, &QPlainTextEdit::customContextMenuRequested,
            this, &GtpyPythonNodeWidget::showCustomContextMenu);
}
