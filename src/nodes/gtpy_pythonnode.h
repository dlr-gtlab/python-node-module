/* GTlab - Gas Turbine laboratory
 * Source File: gtpy_pythonnode.h
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)
 *
 *  Created on: 22.03.2024
 *  Author: Jens Schmeink (AT-TWK)
 */

#ifndef GTPYPYTHONNODE_H
#define GTPYPYTHONNODE_H

#include <intelli/dynamicnode.h>
#include <intelli/data/bytearray.h>

#include "gt_stringproperty.h"
#include "gt_boolproperty.h"

class QPlainTextEdit;
class QSvgWidget;

namespace gt
{
namespace nodes
{
constexpr const char* script_init_content = R"(
################################################################################
# ...
################################################################################
#import pandas as pd
#import numpy as np
#import matplotlib.pyplot as plt

)";
} // nodes
} // gt

namespace gt {
namespace nodes {
namespace python {

/**
 * @brief deserializeToContext
 * @param context
 * @param data
 * @param varName
 */
    void deserializeToContext(int context, const QByteArray& data,
                              QString const& varName);

/**
 * @brief deserializeObjectToContext
 * @param context
 * @param obj
 * @param varName
 */
    bool deserializeObjectToContext(int context, QObject* obj,
                                    QString const& varName);

/**
 * @brief serializeFromContextToVariant
 * @param context
 * @param varName
 * @param pickle - flag if the variable should be pickled
 * @return
 */
    QVariant serializeFromContextToVariant(int context,
                                           const QString& varName,
                                           bool pickle = false);

    bool evalScript(int context, QString const& script);

    int initCalculatorRunContext();

    void clearContext(int context);

} /// python
} /// nodes
} /// gt


/**
 * @brief The GtpyPythonNode class
 * Node for the flexible python scriot based evaluation of data
 * The input and output information can be added via ports
 */
class GtpyPythonNode : public intelli::DynamicNode
{
    Q_OBJECT
public:
    Q_INVOKABLE GtpyPythonNode();

    /**
     * @brief script
     * @return the script used for aevaluation
     */
    QString script() const;

    void setScript(const QString& scr);

    void addInputPort(const QString& className, const QString& caption);

    void addOutputPort(const QString& className, const QString& caption);

    void eval() override;
private:
    GtStringProperty m_script{"script", "script", {},
                              gt::nodes::script_init_content};

    GtBoolProperty m_plot_active{"plot", "Plot enabled", "Plot enabled", false};

    void deserializePythonData(int context);

    void serealizePythonData(int context);

    void showCustomContextMenu(const QPoint& pos, QPlainTextEdit* textEdit);

    void loadPlaceHolder(QSvgWidget* w);

private slots:
    void appendErrorMessage(QString string, int i);

signals:
    void timePassed(int progress);

    void onComputeChange(const QString& tmpPath);
};

#endif // GTPYPYTHONNODE_H
