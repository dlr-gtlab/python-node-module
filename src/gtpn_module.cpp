/* GTlab - Gas Turbine laboratory
 * Source File: gtpy_pythonnode.h
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)
 *
 *  Created on: 22.03.2024
 *  Author: Jens Schmeink (AT-TWK)
 */
#include <intelli/nodefactory.h>
#include <intelli/nodedatafactory.h>
#include <intelli/graph.h>
#include <intelli/gui/nodeui.h>

#include "gtpn_module.h"
/// Nodes
#include "nodes/gtpy_pythonnode.h"


GtVersionNumber
GtpnModule::version()
{
    return GtVersionNumber{0, 3, 0};
}

QString
GtpnModule::description() const
{
    return QStringLiteral("Module for Python scripting in"
                          " the graph environment");
}

void
GtpnModule::init()
{
    /// Nodes
    intelli::NodeFactory::registerNode<GtpyPythonNode>("Scripting");
}

GtpnModule::MetaInformation
GtpnModule::metaInformation() const
{
    MetaInformation m;

    m.author =    QStringLiteral("J. Schmeink");
    m.authorContact = QStringLiteral("");

    // TODO: set license
    // m.licenseShort = ...;

    return m;
}


QMap<const char*, QMetaObject>
GtpnModule::uiItems()
{
    QMap<const char*, QMetaObject> map;

    map.insert(GT_CLASSNAME(GtpyPythonNode),
               GT_METADATA(intelli::NodeUI));
    return map;
}
