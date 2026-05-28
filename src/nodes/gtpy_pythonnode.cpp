/* GTlab - Gas Turbine laboratory
 * Source File: gtpy_pythonnode.cpp
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)
 *
 *  Created on: 22.03.2024
 *  Author: Jens Schmeink (AT-TWK)
 */

#include "gtpy_pythonnode.h"

#include "gtpy_gilscope.h"

#include "gtpy_contextmanager.h"
#include "gtpy_tempdir.h"
#include "../gui/gtpn_pythonnodewidget.h"

#include <intelli/nodedata.h>
#include <intelli/data/object.h>
#include <intelli/data/string.h>
#include <intelli/data/double.h>
#include <intelli/data/bool.h>
#include <intelli/data/int.h>
#include <intelli/data/stringlist.h>

// This raw strings is used for the internal serialization and deserialization
// of python classes
const QString p_deser_script = R"(
import pickle
ser_data_hex = __serialized_data_tmp
ser_data_bytes = bytes.fromhex(ser_data_hex)
$$var_name$$ = pickle.loads(ser_data_bytes)
)";

const QString p_ser_script = R"(
import pickle
#gtError() << "Serialization Scipt execution"
#gtError() << $$var_name$$
__deserialized_data_tmp = pickle.dumps($$var_name$$)
#gtError() << "Serialization Scipt execution finished"
)";

void
gt::nodes::python::deserializeToContext(int context,
                                        const intelli::ByteArrayData* pvd,
                                        const QString& caption)
{
    if (!pvd) return;

    QByteArray in = pvd->data();

    QString p_deser_script_tmp = p_deser_script;
    p_deser_script_tmp.replace("$$var_name$$", caption);

    QString ser_data_hex = in.toHex();
    GtpyContextManager::instance()->addVariable(context,
                                                "__serialized_data_tmp",
                                                ser_data_hex);

    GtpyContextManager::instance()->evalScript(context, p_deser_script_tmp);
}

QVariant
gt::nodes::python::serializeFromContextToVariant(
        int context, const intelli::Node::PortInfo& pd)
{
    QString p_ser_script_tmp = p_ser_script;
    p_ser_script_tmp.replace("$$var_name$$", pd.caption);
    gtTrace() << p_ser_script_tmp;
    GtpyContextManager::instance()->evalScript(context, p_ser_script_tmp);

    QVariant var = GtpyContextManager::instance()->getVariable(
                context,  "__deserialized_data_tmp");

    return var;
}


GtpyPythonNode::GtpyPythonNode() :
    intelli::DynamicNode("Python Scripting Node",
            {},  /// whitelist for inputs is empty: means everything is ok
            /// outputlist is restricted to compatible node data classes
            {GT_CLASSNAME(intelli::ByteArrayData),
             GT_CLASSNAME(intelli::ObjectData),
             GT_CLASSNAME(intelli::DoubleData),
             GT_CLASSNAME(intelli::IntData),
             GT_CLASSNAME(intelli::StringData),
             GT_CLASSNAME(intelli::BoolData),
             GT_CLASSNAME(intelli::StringListData)}
             )
{
    setFlag(GtObject::UserRenamable);

    setNodeFlag(NodeFlag::Resizable);
    setNodeEvalMode(intelli::NodeEvalMode::Exclusive);

    m_script.hide(true);

    registerProperty(m_script);
    registerProperty(m_plot_active);

    connect(GtpyContextManager::instance(),
            SIGNAL(errorMessage(QString,int,QString)), this,
            SLOT(appendErrorMessage(QString,int,QString)));

    // registering the widget factory
    registerWidgetFactory(GtpyPythonNodeWidget::create);
}

QString
GtpyPythonNode::script() const
{
    return m_script;
}

void
GtpyPythonNode::setScript(const QString& scr)
{
    m_script = scr;
}

bool
GtpyPythonNode::plotActive() const
{
    return m_plot_active;
}

GtBoolProperty&
GtpyPythonNode::plotActiveProperty()
{
    return m_plot_active;
}

const GtBoolProperty&
GtpyPythonNode::plotActiveProperty() const
{
    return m_plot_active;
}

void
GtpyPythonNode::prepareScriptEditorContext(int context)
{
    deserializePythonData(context);
}

void
GtpyPythonNode::syncPlotState(bool active)
{
    setNodeFlag(NodeFlag::Resizable, active);
    nodeChanged();
}

void
GtpyPythonNode::addInputPort(const QString& className,
                               const QString& caption)
{
    PortId pid = addInPort(PortInfo{className, caption}, Optional);
}

void
GtpyPythonNode::addOutputPort(const QString& className,
                                const QString& caption)
{
    PortId pid = addOutPort(PortInfo{className, caption});
}

void
GtpyPythonNode::eval()
{
    emit timePassed(0);
    // init python context
    int context = GtpyContextManager::instance()->createNewContext(
        GtpyContextManager::CalculatorRunContext, true);

    GtpyContextManager::instance()->setLoggingPrefix(context, objectName());

    /// handle python input variables and add also rest of the input ports
    /// to context
    deserializePythonData(context);

    if (!GtpyContextManager::instance()->evalScript(context, script()))
    {
        gtError() << tr("Script evaluation failed in python node");
        gtError() << script();

        for (auto&& p : ports(PortType::Out))
        {
            setNodeData(p.id(), nullptr);
        }

        return;
    }

    /// handle python output variables
    serealizePythonData(context);

    GtpyContextManager::instance()->removeAllAddedObjects(context);

    emit timePassed(100.);

    GtpyContextManager::instance()->deleteContext(context);

    if (m_plot_active)
    {
        auto tmp_path = gtpy::tempdir::currentThreadTempPath();

        emit onComputeChange(tmp_path);
    }
}

void
GtpyPythonNode::deserializePythonData(int context)
{
    // register dynamic input port variables
    auto& pinlist = ports(intelli::PortType::In);

    GTPY_GIL_SCOPE

    for (int i = 0; i < pinlist.size(); ++i)
    {
        auto& data_tmp = pinlist[i];

        if (auto* pd = qobject_cast<intelli::ByteArrayData const*>(
                    nodeData(data_tmp.id()).get()))
        {
            gt::nodes::python::deserializeToContext(context, pd,
                                                    data_tmp.caption);
        }
        else if (auto pd2 = nodeData(data_tmp.id()))
        {
            auto* helper = const_cast<intelli::NodeData*>(pd2.get());

            if (!GtpyContextManager::instance()->addGtObject(context,
                                                             data_tmp.caption,
                                                             helper))
            {
                gtError() << tr("Cannot add '%1' to python context").arg(
                                 data_tmp.caption);
                continue;
            }

            gtTrace() << tr("Added '%1' to context as input").arg(
                             data_tmp.caption);
        }
    }

    GtpyContextManager::instance()->evalScript(context, "");
}

void
GtpyPythonNode::serealizePythonData(int context)
{
    gtTrace() << __FUNCTION__;

    std::vector<Node::PortInfo> const poutlist = ports(intelli::PortType::Out);

    GTPY_GIL_SCOPE

    for (int i = 0; i < poutlist.size(); ++i)
    {
        auto& data_tmp = poutlist[i];

        gtDebug() << tr("extracting dynamic out data... ") << data_tmp.caption;

        /// check each outport for typeId which was used
        if (data_tmp.typeId == GT_CLASSNAME(intelli::ByteArrayData))
        {
            QVariant var = gt::nodes::python::serializeFromContextToVariant(
                        context, data_tmp);

            if (var.isValid())
            {
                gtTrace() << __FUNCTION__ << tr("Do serealization");
                QByteArray d_tmp = var.toByteArray();
                setNodeData(data_tmp.id(),
                            std::make_shared<intelli::ByteArrayData>(d_tmp));
            }
            else
            {
                gtWarning() << tr("Cannot set port ") << data_tmp.caption;
            }
        }
        else /// not python type
        {
            ///     Currently the object type is able to be used here
            ///     aswell as string, double and bool
            gtTrace() << tr("Serialize datatype:") << data_tmp.typeId;

            QString p_ser_script_tmp =
                    R"(__deserialized_Objdata_tmp = $$var_name$$)";

            p_ser_script_tmp.replace("$$var_name$$", data_tmp.caption);
            gtTrace() << tr("Script execution") << p_ser_script_tmp;
            GtpyContextManager::instance()->evalScript(context,
                                                       p_ser_script_tmp);

            QVariant var = GtpyContextManager::instance()->getVariable(
                        context,  "__deserialized_Objdata_tmp");

            if (!var.isValid())
            {
                gtWarning() << tr("Cannot read value for output port "
                                  "'%1'").arg(data_tmp.caption);

                setNodeData(data_tmp.id(), nullptr);

                continue;
            }

            if (data_tmp.typeId == GT_CLASSNAME(intelli::ObjectData))
            {
                auto* obj = qvariant_cast<QObject *>(var);
                if (auto* objFromPython = qobject_cast<GtObject*>(obj))
                {
                    setNodeData(data_tmp.id(),
                                std::make_shared<intelli::ObjectData>(objFromPython));
                }
            }
            else if (data_tmp.typeId == GT_CLASSNAME(intelli::StringData))
            {
                QString string = var.toString();

                setNodeData(data_tmp.id(),
                            std::make_shared<intelli::StringData>(string));
            }
            else if (data_tmp.typeId == GT_CLASSNAME(intelli::StringListData))
            {
                QStringList string = var.toStringList();

                setNodeData(data_tmp.id(),
                            std::make_shared<intelli::StringListData>(string));
            }
            else if (data_tmp.typeId == GT_CLASSNAME(intelli::DoubleData))
            {
                double val = var.toDouble();

                setNodeData(data_tmp.id(),
                            std::make_shared<intelli::DoubleData>(val));
            }
            else if (data_tmp.typeId == GT_CLASSNAME(intelli::IntData))
            {
                int val = var.toInt();

                setNodeData(data_tmp.id(),
                            std::make_shared<intelli::IntData>(val));
            }
            else if (data_tmp.typeId == GT_CLASSNAME(intelli::BoolData))
            {
                bool val = var.toBool();

                setNodeData(data_tmp.id(),
                            std::make_shared<intelli::BoolData>(val));
            }
        }
    }
}

void
GtpyPythonNode::appendErrorMessage(QString string, int i, QString prefix)
{
    if (prefix.isEmpty()) prefix = "Python Node";

    gtLogOnce(Error) << tr("[%1] %2").arg(prefix, string);
}
