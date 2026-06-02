/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2026 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORSCHEMEMANAGER_P_H
#define KCOLORSCHEMEMANAGER_P_H

#include <memory>

#include "kcolorschememodel.h"

#include <QBrush>
#include <QIconEngine>

class KColorSchemeManager;

class PreviewIconEngine : public QIconEngine
{
public:
    explicit PreviewIconEngine(const QString &path);

    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    QIconEngine *clone() const override;

    QSize actualSize(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QPixmap pixmap(const QSize &size, QIcon::Mode mode, QIcon::State state) override;
    QPixmap scaledPixmap(const QSize &size, QIcon::Mode mode, QIcon::State state, qreal scale) override;

private:
    void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state, bool includeFrame);

    QBrush m_window;
    QBrush m_button;
    QBrush m_view;
    QBrush m_selection;
};

class KColorSchemeManagerPrivate
{
public:
    KColorSchemeManagerPrivate();

    std::unique_ptr<KColorSchemeModel> model;
    bool m_autosaveChanges = true;
    QString m_activatedScheme;

    static QIcon createPreview(const QString &path);
    void activateSchemeInternal(const QString &colorSchemePath);
    QString automaticColorSchemeId() const;
    QString automaticColorSchemePath() const;
    QModelIndex indexForSchemeId(const QString &id) const;

    enum ContrastPreference {
        NoPreference,
        HighContrast,
    };
    static ContrastPreference contrastPreference();

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
};

#endif
