/*
    SPDX-FileCopyrightText: 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOLORSCHEME_P_H
#define KCOLORSCHEME_P_H

#include <KSharedConfig>

#include <QCoreApplication>
#include <QPalette>

#include <array>

#ifdef Q_OS_WIN
#include "windows.h"
#endif

#ifdef Q_OS_WIN
static bool isHighContrastModeActive()
{
    HIGHCONTRAST result;
    result.cbSize = sizeof(HIGHCONTRAST);
    if (SystemParametersInfo(SPI_GETHIGHCONTRAST, result.cbSize, &result, 0)) {
        return (result.dwFlags & HCF_HIGHCONTRASTON);
    }
    return false;
}
#endif

static KSharedConfigPtr defaultConfig()
{
    // cache the value we'll return, since usually it's going to be the same value
    static thread_local KSharedConfigPtr config;
    // Read from the application's color scheme file (as set by KColorSchemeManager).
    // If unset, this is equivalent to openConfig() and the system scheme is used.
    const auto colorSchemePath = qApp->property("KDE_COLOR_SCHEME_PATH").toString();
#ifdef Q_OS_WIN
    // If no color scheme is set and high-contrast is active then use the system colors
    if (colorSchemePath.isEmpty() && isHighContrastModeActive()) {
        return {};
    }
#endif
    if (!config || config->name() != colorSchemePath) {
        config = KSharedConfig::openConfig(colorSchemePath);
    }
    return config;
}

class StateEffects
{
public:
    explicit StateEffects(QPalette::ColorGroup state, const KSharedConfigPtr &);
    ~StateEffects()
    {
    }

    QBrush brush(const QBrush &background) const;
    QBrush brush(const QBrush &foreground, const QBrush &background) const;

private:
    enum EffectTypes {
        Intensity,
        Color,
        Contrast,
        NEffectTypes,
    };

    enum IntensityEffects {
        IntensityNoEffect,
        IntensityShade,
        IntensityDarken,
        IntensityLighten,
        NIntensityEffects,
    };

    enum ColorEffects {
        ColorNoEffect,
        ColorDesaturate,
        ColorFade,
        ColorTint,
        NColorEffects,
    };

    enum ContrastEffects {
        ContrastNoEffect,
        ContrastFade,
        ContrastTint,
        NContrastEffects,
    };

    int _effects[NEffectTypes];
    double _amount[NEffectTypes];
    QColor _color;
};

#endif
