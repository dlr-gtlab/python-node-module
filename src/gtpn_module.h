/* GTlab - Gas Turbine laboratory
 * Source File: gtpy_pythonnode.h
 *
 * SPDX-License-Identifier: Apache-2.0
 * SPDX-FileCopyrightText: 2024 German Aerospace Center (DLR)
 *
 *  Created on: 22.03.2024
 *  Author: Jens Schmeink (AT-TWK)
 */

#ifndef GTPN_MODULE_H
#define GTPN_MODULE_H

#include "gt_moduleinterface.h"
#include "gt_mdiinterface.h"

/**
 * @brief The GtpgModule class
 */
class GtpnModule : public QObject,
        public GtModuleInterface,
        public GtMdiInterface
{
    Q_OBJECT

    GT_MODULE("gtpn_module.json")

    Q_INTERFACES(GtModuleInterface)
    Q_INTERFACES(GtMdiInterface)
    
public:

    /**
     * @brief Returns current version number of module
     * @return version number
     */
    GtVersionNumber version() override;

    /**
     * @brief Returns module description
     * @return description
     */
    QString description() const override;

    /**
     * @brief Initializes module. Called on application startup.
     */
    void init() override;

    /**
     * @brief Passes additional module information to the framework.
     *
     * NOTE: A reference to the author can significantly help the user to
     * know who to contact in case of issues or other request.
     * @return module meta information.
     */
    MetaInformation metaInformation() const override;

    /**
     * @brief uiItems
     * @return data class names mapped to ui item objects
     */
    QMap<const char*, QMetaObject> uiItems() override;
};

#endif // GTPN_MODULE_H
