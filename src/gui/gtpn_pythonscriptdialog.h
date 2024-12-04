/* GTlab - Gas Turbine laboratory
 * Source File: gtpn_pythonscriptdialog.h
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)
 *
 *  Created on: 22.03.2024
 *  Author: Jens Schmeink (AT-TWK)
 */
#ifndef GTPN_PYTHONSCRIPTDIALOG_H
#define GTPN_PYTHONSCRIPTDIALOG_H

#include <QDialog>

class GtpyScriptEditorWidget;

class GtpnPythonScriptDialog : public QDialog
{
public:
    /**
     * @brief Constructor.
     * @param parent Parent widget
     */
    explicit GtpnPythonScriptDialog(QWidget* parent = nullptr);

    ~GtpnPythonScriptDialog();

    /**
     * @brief Returns user defined script.
     * @return User defined script.
     */
    QString pythonScript();

    /**
     * @brief Sets given script.
     * @param str Initialization script.
     */
    void setScript(const QString& str);

    int context() const;

private:
    /// Python Editor
    GtpyScriptEditorWidget* m_editor;

    int m_context;

};

#endif // GTPN_PYTHONSCRIPTDIALOG_H
