/*
    SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "kquickcolorschemeplugin.h"

#include <kcolorschememanager.h>

void KQuickColorSchemePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.colorscheme"));
    qmlRegisterSingletonType<KColorSchemeManager>(uri, 1, 0, "ColorSchemeManager", [](auto engine, auto scriptEngine) {
        (void)engine;
        auto instance = KColorSchemeManager::instance();
        scriptEngine->setObjectOwnership(instance, QJSEngine::CppOwnership);
        return instance;
    });
}

#include "moc_kquickcolorschemeplugin.cpp"
