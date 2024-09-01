/*
    SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KQUICKCOLORSCHEMEPLUGIN_H
#define KQUICKCOLORSCHEMEPLUGIN_H

#include <QQmlEngine>
#include <QQmlExtensionPlugin>

class KQuickColorSchemePlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
    void registerTypes(const char *uri) override;
};

#endif // KQUICKCOLORSCHEMEPLUGIN_H
