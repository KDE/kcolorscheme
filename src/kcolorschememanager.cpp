/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolorschememanager.h"
#include "kcolorschememanager_p.h"

#include "kcolorscheme.h"
#include "kcolorschememodel.h"

#include <KColorSchemeWatcher>
#include <KConfigGroup>
#include <KConfigGui>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QPainter>
#include <QPointer>
#include <QStandardPaths>

#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>

// ensure we are linking KConfigGui, so QColor I/O from KConfig works
KCONFIGGUI_EXPORT int initKConfigGroupGui();
static int s_init = initKConfigGroupGui();

constexpr int defaultSchemeRow = 0;

static bool isKdePlatformTheme()
{
    if (!QGuiApplicationPrivate::platformTheme()) {
        return false;
    }

    if (QGuiApplicationPrivate::platformTheme()->name() == QLatin1String("kde")) {
        return true;
    }

    if (qgetenv("XDG_CURRENT_DESKTOP") == "KDE" && QGuiApplicationPrivate::platformTheme()->name() == QLatin1String("xdgdesktopportal")) {
        return true;
    }

    return false;
}

void KColorSchemeManagerPrivate::activateSchemeInternal(const QString &colorSchemePath)
{
    // hint for plasma-integration to synchronize the color scheme with the window manager/compositor
    // The property needs to be set before the palette change because is is checked upon the
    // ApplicationPaletteChange event.
    qApp->setProperty("KDE_COLOR_SCHEME_PATH", colorSchemePath);
    if (colorSchemePath.isEmpty()) {
        qApp->setPalette(QPalette());
    } else {
        qApp->setPalette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig(colorSchemePath)));
    }
}

QString KColorSchemeManagerPrivate::automaticColorSchemeId() const
{
    if (!m_colorSchemeWatcher) {
        return QString();
    }

    switch (m_colorSchemeWatcher->systemPreference()) {
    case KColorSchemeWatcher::PreferHighContrast:
        return QString();
    case KColorSchemeWatcher::PreferDark:
        return getDarkColorScheme();
    case KColorSchemeWatcher::PreferLight:
    case KColorSchemeWatcher::NoPreference:
        return getLightColorScheme();
    };
    return QString();
}

// The meaning of the Default entry depends on the platform
// On KDE we apply a default KColorScheme
// On other platforms we automatically apply Breeze/Breeze Dark depending on the system preference
QString KColorSchemeManagerPrivate::automaticColorSchemePath() const
{
    const QString colorSchemeId = automaticColorSchemeId();
    if (colorSchemeId.isEmpty()) {
        return QString();
    } else {
        return indexForSchemeId(colorSchemeId).data(KColorSchemeModel::PathRole).toString();
    }
}

QIcon KColorSchemeManagerPrivate::createPreview(const QString &path)
{
    KSharedConfigPtr schemeConfig = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    QIcon result;

    KColorScheme activeWindow(QPalette::Active, KColorScheme::Window, schemeConfig);
    KColorScheme activeButton(QPalette::Active, KColorScheme::Button, schemeConfig);
    KColorScheme activeView(QPalette::Active, KColorScheme::View, schemeConfig);
    KColorScheme activeSelection(QPalette::Active, KColorScheme::Selection, schemeConfig);

    auto pixmap = [&](int size) {
        QPixmap pix(size, size);
        pix.fill(Qt::black);
        QPainter p;
        p.begin(&pix);
        const int itemSize = size / 2 - 1;
        p.fillRect(1, 1, itemSize, itemSize, activeWindow.background());
        p.fillRect(1 + itemSize, 1, itemSize, itemSize, activeButton.background());
        p.fillRect(1, 1 + itemSize, itemSize, itemSize, activeView.background());
        p.fillRect(1 + itemSize, 1 + itemSize, itemSize, itemSize, activeSelection.background());
        p.end();
        result.addPixmap(pix);
    };
    // 16x16
    pixmap(16);
    // 24x24
    pixmap(24);

    return result;
}

KColorSchemeManagerPrivate::KColorSchemeManagerPrivate()
    : model(new KColorSchemeModel())
{
}

KColorSchemeManager::KColorSchemeManager(GuardApplicationConstructor, QGuiApplication *app)
    : QObject(app)
    , d(new KColorSchemeManagerPrivate)
{
    init();
}

#if KCOLORSCHEME_BUILD_DEPRECATED_SINCE(6, 6)
KColorSchemeManager::KColorSchemeManager(QObject *parent)
    : QObject(parent)
    , d(new KColorSchemeManagerPrivate())
{
    init();
}
#endif

KColorSchemeManager::~KColorSchemeManager()
{
}

void KColorSchemeManager::init()
{
    QString platformThemeSchemePath = qApp->property("KDE_COLOR_SCHEME_PATH").toString();
    if (!isKdePlatformTheme() && platformThemeSchemePath.isEmpty()) {
        d->m_colorSchemeWatcher.emplace();
        QObject::connect(&*d->m_colorSchemeWatcher, &KColorSchemeWatcher::systemPreferenceChanged, this, [this]() {
            if (!d->m_activatedScheme.isEmpty()) {
                // Don't override what has been manually set
                return;
            }

            d->activateSchemeInternal(d->automaticColorSchemePath());
        });
    }

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, QStringLiteral("UiSettings"));

    QString schemeId;

    // Migrate the deprecated ColorScheme to ColorSchemeId, if needed
    // TODO: KF7 keep only else branch
    if(!cg.hasKey("ColorSchemeId") && cg.hasKey("ColorScheme")) {
        const QString scheme = cg.readEntry("ColorScheme", QString());
        const auto index = indexForScheme(scheme);
        if (index.isValid()) {
            schemeId = index.data(KColorSchemeModel::IdRole).toString();
            saveSchemeIdToConfigFile(schemeId);
        }
    } else {
        QString schemeId = cg.readEntry("ColorSchemeId", QString());
    }

    QString schemePath;

    if (schemeId.isEmpty()) {
        // Color scheme might be already set from a platform theme
        // This is used for example by QGnomePlatform that can set color scheme
        // matching GNOME settings. This avoids issues where QGnomePlatform sets
        // QPalette for dark theme, but end up mixing it also with Breeze light
        // that is going to be used as a fallback for apps using KColorScheme.
        // BUG: 447029
        if (platformThemeSchemePath.isEmpty()) {
            schemePath = d->automaticColorSchemePath();
        }
    } else {
        auto index = d->indexForSchemeId(schemeId);
        schemePath = index.data(KColorSchemeModel::PathRole).toString();
        d->m_activatedScheme = index.data(KColorSchemeModel::IdRole).toString();
    }

    if (!schemePath.isEmpty()) {
        d->activateSchemeInternal(schemePath);
    }
}

QAbstractItemModel *KColorSchemeManager::model() const
{
    return d->model.get();
}

QModelIndex KColorSchemeManagerPrivate::indexForSchemeId(const QString &id) const
{
    // Empty string is mapped to "reset to the system scheme"
    if (id.isEmpty()) {
        return model->index(defaultSchemeRow);
    }
    for (int i = 1; i < model->rowCount(); ++i) {
        QModelIndex index = model->index(i);
        if (index.data(KColorSchemeModel::IdRole).toString() == id) {
            return index;
        }
    }
    return QModelIndex();
}

void KColorSchemeManager::setAutosaveChanges(bool autosaveChanges)
{
    d->m_autosaveChanges = autosaveChanges;
}

QModelIndex KColorSchemeManager::indexForSchemeId(const QString &id) const
{
    return d->indexForSchemeId(id);
}

QModelIndex KColorSchemeManager::indexForScheme(const QString &name) const
{
    // Empty string is mapped to "reset to the system scheme"
    if (name.isEmpty()) {
        return d->model->index(defaultSchemeRow);
    }
    for (int i = 1; i < d->model->rowCount(); ++i) {
        QModelIndex index = d->model->index(i);
        if (index.data(KColorSchemeModel::NameRole).toString() == name) {
            return index;
        }
    }
    return QModelIndex();
}

void KColorSchemeManager::activateScheme(const QModelIndex &index)
{
    const bool isDefaultEntry = index.data(KColorSchemeModel::PathRole).toString().isEmpty();

    if (index.isValid() && index.model() == d->model.get() && !isDefaultEntry) {
        d->activateSchemeInternal(index.data(KColorSchemeModel::PathRole).toString());
        d->m_activatedScheme = index.data(KColorSchemeModel::IdRole).toString();
        if (d->m_autosaveChanges) {
#if KCOLORSCHEME_ENABLE_DEPRECATED_SINCE(6, 17)
            saveSchemeToConfigFile(index.data(KColorSchemeModel::NameRole).toString());
#endif
            saveSchemeIdToConfigFile(index.data(KColorSchemeModel::IdRole).toString());
        }
    } else {
        d->activateSchemeInternal(d->automaticColorSchemePath());
        d->m_activatedScheme = QString();
        if (d->m_autosaveChanges) {
#if KCOLORSCHEME_ENABLE_DEPRECATED_SINCE(6, 17)
            saveSchemeToConfigFile(QString());
#endif
            saveSchemeIdToConfigFile(QString());
        }
    }
}

void KColorSchemeManager::activateScheme(const QString &schemeId)
{
    auto index = d->indexForSchemeId(schemeId);
    const bool isDefaultEntry = index.data(KColorSchemeModel::IdRole).toString().isEmpty();

    if (index.isValid() && !isDefaultEntry) {
        d->activateSchemeInternal(index.data(KColorSchemeModel::PathRole).toString());
        d->m_activatedScheme = index.data(KColorSchemeModel::IdRole).toString();
        if (d->m_autosaveChanges) {
            saveSchemeIdToConfigFile(index.data(KColorSchemeModel::IdRole).toString());
        }
    } else {
        d->activateSchemeInternal(d->automaticColorSchemePath());
        d->m_activatedScheme = QString();
        if (d->m_autosaveChanges) {
            saveSchemeIdToConfigFile(QString());
        }
    }
}

#if KCOLORSCHEME_ENABLE_DEPRECATED_SINCE(6, 17)
void KColorSchemeManager::saveSchemeToConfigFile(const QString &schemeName) const
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, QStringLiteral("UiSettings"));

    if (schemeName.isEmpty() && !cg.hasDefault("ColorScheme")) {
        cg.revertToDefault("ColorScheme");
    } else {
        cg.writeEntry("ColorScheme", KLocalizedString::removeAcceleratorMarker(schemeName));
    }

    cg.sync();
}
#endif

void KColorSchemeManager::saveSchemeIdToConfigFile(const QString &schemeId) const
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, QStringLiteral("UiSettings"));
    cg.writeEntry("ColorSchemeId", schemeId);
    cg.sync();
}

QString KColorSchemeManager::activeSchemeId() const
{
    return d->m_activatedScheme;
}

QString KColorSchemeManager::activeSchemeName() const
{
    return d->indexForSchemeId(d->m_activatedScheme).data(KColorSchemeModel::NameRole).toString();
}

KColorSchemeManager *KColorSchemeManager::instance()
{
    Q_ASSERT(qApp);
    static QPointer<KColorSchemeManager> manager;
    if (!manager) {
        manager = new KColorSchemeManager(GuardApplicationConstructor{}, qApp);
    }
    return manager;
}

#include "moc_kcolorschememanager.cpp"
