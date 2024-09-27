/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORSCHEMEMANAGER_P_H
#define KCOLORSCHEMEMANAGER_P_H

#include <memory>

#include "kcolorschememodel.h"

#include <KColorSchemeWatcher>

class KColorSchemeManager;

class KColorSchemeManagerPrivate
{
public:
    KColorSchemeManagerPrivate();

    std::unique_ptr<KColorSchemeModel> model;
    bool m_autosaveChanges = true;
    QString m_activatedScheme;

    static QIcon createPreview(const QString &path);
    void activateSchemeInternal(const QString &colorSchemePath);
    QString automaticColorSchemePath() const;
    QModelIndex indexForSchemeId(const QString &id) const;

    const QString &getLightColorScheme() const
    {
        return m_lightColorScheme;
    }
    const QString &getDarkColorScheme() const
    {
        return m_darkColorScheme;
    }

    QString m_lightColorScheme = QStringLiteral("BreezeLight");
    QString m_darkColorScheme = QStringLiteral("BreezeDark");
    std::optional<KColorSchemeWatcher> m_colorSchemeWatcher;
};

#endif
