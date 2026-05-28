/* GTlab - Gas Turbine laboratory
 * Source File: gtpn_pythonnodewidget.h
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)
 *
 *  Created on: 28.05.2026
 */

#ifndef GTPN_PYTHONNODEWIDGET_H
#define GTPN_PYTHONNODEWIDGET_H

#include <memory>

#include <QPointer>
#include <QString>
#include <QWidget>

class QPlainTextEdit;
class QPushButton;
class QSvgWidget;
class QPoint;

namespace intelli
{
class Node;
}

class GtpyPythonNode;

class GtpyPythonNodeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GtpyPythonNodeWidget(GtpyPythonNode& node,
                                  QWidget* parent = nullptr);
    static std::unique_ptr<QWidget> create(intelli::Node& node);

private slots:
    void updatePlotVisibility();
    void openScriptEditor();
    void updatePlot(const QString& tmpPath);
    void updateTimePassed(int progress);
    void showCustomContextMenu(const QPoint& pos);
    void insertAssistantPrompt();

private:
    void loadPlaceholder();
    void installAssistantContextMenu(QPlainTextEdit* textEdit);

    GtpyPythonNode& m_node;
    QPushButton* m_button = nullptr;
    QSvgWidget* m_plot = nullptr;
    QPointer<QPlainTextEdit> m_textEdit;
};

#endif // GTPN_PYTHONNODEWIDGET_H
