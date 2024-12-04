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

#include "gtpy_contextmanager.h"
#include "gtpy_tempdir.h"

#include "gt_icons.h"
#include "gt_sharedfunction.h"

#include <intelli/nodedata.h>
#include <intelli/data/object.h>
#include <intelli/data/string.h>
#include <intelli/data/double.h>
#include <intelli/data/bool.h>
#include <intelli/data/int.h>

#include "../gui/gtpn_pythonscriptdialog.h"
#include "qsvgwidget.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QPlainTextEdit>
#include <QAction>
#include <QMenu>
#include <QDir>

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
             GT_CLASSNAME(intelli::BoolData)}
             )
{
    setFlag(GtObject::UserRenamable);

    setNodeFlag(NodeFlag::Resizable);
    setNodeEvalMode(intelli::NodeEvalMode::Exclusive);

    m_script.hide(true);

    registerProperty(m_script);
    registerProperty(m_plot_active);

    connect(GtpyContextManager::instance(),
            SIGNAL(errorMessage(QString, int)), this,
            SLOT(appendErrorMessage(QString, int)));

    // registering the widget factory
    registerWidgetFactory([=](intelli::Node& /*this*/){
        auto w = std::make_unique<QWidget>();
        auto l = new QVBoxLayout();
        auto p = new QPushButton();

        // the optional plot widget based on svg
        auto plot = new QSvgWidget;
        plot->setVisible(m_plot_active.get());
        loadPlaceHolder(plot);
        l->addWidget(plot);

        // the button to open the script editor
        p->setIcon(gt::gui::icon::python());
        l->addWidget(p);
        w->setLayout(l);
        l->setContentsMargins(0, 0, 0, 0);

        auto togglePlot  = [w_ = w.get(), plot, this]()
        {
            plot->setVisible(m_plot_active.get());

            if (m_plot_active.get())
            {
                w_->resize(size());
                this->triggerNodeEvaluation();
            }
            else
            {
                w_->resize(w_->minimumSize());
            }

            this->setNodeFlag(NodeFlag::Resizable, m_plot_active.get());
            nodeChanged();
        };

        connect(&m_plot_active, &GtBoolProperty::changed, plot, togglePlot);

        auto openEditor = [w_ = w.get(), this](){
            auto state = PyGILState_Ensure();
            GtpnPythonScriptDialog dialog;

            dialog.setScript(m_script);
            /// try to add to context to have auto completion
            deserializePythonData(dialog.context());

            QSettings settings;

            dialog.resize(settings.value("pythonNode/python_editor/size",
                                         QSize(400, 300)).toSize());

            if (gt::interface::getSharedFunction("Assistant", "prompt"))
            {
                gtInfo() << tr("GTlab Assistant found! installing filter...");

                QList<QWidget *> children = dialog.findChildren<QWidget *>();
                for (QWidget* child : qAsConst(children))
                {
                    if (QString(child->metaObject()->className()) == QStringLiteral("GtpyScriptView"))
                    {
                        if (auto* textEdit = qobject_cast<QPlainTextEdit*>(child))
                        {
                            // Set custom context menu policy
                            textEdit->setContextMenuPolicy(Qt::CustomContextMenu);
                            connect(textEdit, &QPlainTextEdit::customContextMenuRequested,
                                    [textEdit, this](const QPoint &pos)
                            {
                                showCustomContextMenu(pos, textEdit);
                            });
                        }
                    }
                }
            }

            dialog.exec();

            QString oldScript = m_script;
            m_script = dialog.pythonScript();

            if (oldScript != m_script)
            {
                gtTrace() << tr("Script changed");
                emit triggerNodeEvaluation();
            }

            settings.setValue("pythonNode/python_editor/size", dialog.size());

            PyGILState_Release(state);
        };

        connect(this, &GtpyPythonNode::onComputeChange, plot,
                        [=, w_ = plot](const QString& tmpPath)
        {
            QDir tempDir(tmpPath);

            QFile figure(tempDir.absoluteFilePath("figure_0.svg"));

            if (figure.exists())
            {
                w_->load(tempDir.absoluteFilePath("figure_0.svg"));
                figure.remove();
            }
            else
            {
                loadPlaceHolder(w_);
            }
        });

        auto update = [=](int progress){
            if (progress != 100)
            {
                p->setIcon(gt::gui::icon::processRunningIcon(progress));
            }
            else
            {
                p->setIcon(gt::gui::icon::python());
            }
        };

        connect(this, &GtpyPythonNode::timePassed, w.get(), update);

        connect(p, &QPushButton::clicked, w.get(), openEditor);

        return w;
    });
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

    /// handle python input variables and add also rest of the input ports
    /// to context
    deserializePythonData(context);

    if (!GtpyContextManager::instance()->evalScript(context, script()))
    {
        gtError() << tr("Script evaluation failed in python node");
        gtError() << script();

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

    auto state = PyGILState_Ensure();

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

    PyGILState_Release(state);
}

void
GtpyPythonNode::serealizePythonData(int context)
{
    gtTrace() << __FUNCTION__;

    std::vector<Node::PortInfo> const poutlist = ports(intelli::PortType::Out);

    auto state = PyGILState_Ensure();

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
                continue;
            }

            if (data_tmp.typeId == GT_CLASSNAME(intelli::ObjectData))
            {
                auto* obj = qvariant_cast<QObject *>(var);

                gtTrace() << "obj" << obj;

                if (auto* objFromPython = qobject_cast<GtObject*>(obj))
                {
                    setNodeData(data_tmp.id(),
                                std::make_shared<intelli::ObjectData>(objFromPython));
                }
            }
            else if (data_tmp.typeId == GT_CLASSNAME(intelli::StringData))
            {
                QString string = var.toString();

                gtTrace() << "string" << string;

                setNodeData(data_tmp.id(),
                            std::make_shared<intelli::StringData>(string));
            }
            else if (data_tmp.typeId == GT_CLASSNAME(intelli::DoubleData))
            {
                double val = var.toDouble();

                gtTrace() << "double" << val;

                setNodeData(data_tmp.id(),
                            std::make_shared<intelli::DoubleData>(val));
            }
            else if (data_tmp.typeId == GT_CLASSNAME(intelli::IntData))
            {
                int val = var.toInt();

                gtTrace() << "int" << val;

                setNodeData(data_tmp.id(),
                            std::make_shared<intelli::IntData>(val));
            }
            else if (data_tmp.typeId == GT_CLASSNAME(intelli::BoolData))
            {
                bool val = var.toBool();

                gtTrace() << "bool" << val;

                setNodeData(data_tmp.id(),
                            std::make_shared<intelli::BoolData>(val));
            }
        }
    }

    PyGILState_Release(state);
}

void
GtpyPythonNode::showCustomContextMenu(const QPoint& pos,
                                      QPlainTextEdit* textEdit)
{
    QMenu* menu = textEdit->createStandardContextMenu();
    QAction* customAction = new QAction("Assistant", textEdit);

    QObject::connect(customAction, &QAction::triggered, textEdit, [&textEdit]()
    {
        auto function = gt::interface::getSharedFunction("Assistant",
                                                         "open_prompt_python");

        if (!function.isNull())
        {
            QTextCursor cursor = textEdit->textCursor();
            QString selectedText = cursor.selectedText();
            auto retVals = function({selectedText});

            if (!retVals.isEmpty())
            {
                cursor.insertText(retVals[0].toString());
            }
        }
    });

    menu->addSeparator();
    menu->addAction(customAction);
    menu->exec(textEdit->mapToGlobal(pos));
    delete menu;
}

void
GtpyPythonNode::loadPlaceHolder(QSvgWidget* w)
{
    w->load(QStringLiteral(":/pyNodes/figure_empty.svg"));
}

void
GtpyPythonNode::appendErrorMessage(QString string, int i)
{
    gtLogOnce(Error) << tr("Python Error: %1").arg(string);
}
