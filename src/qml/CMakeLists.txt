# SPDX-FileCopyrightText: 2018 Eike Hein <hein@kde.org>
# SPDX-FileCopyrightText: 2021 Volker Krause <vkrause@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

ecm_add_qml_module(kquickcolorschemeplugin URI "org.kde.colorscheme")

target_sources(kquickcolorschemeplugin PRIVATE
    kquickcolorschemeplugin.cpp
)

target_link_libraries(kquickcolorschemeplugin PRIVATE
    KF6ColorScheme
    Qt6::Quick
)

ecm_finalize_qml_module(kquickcolorschemeplugin DESTINATION ${KDE_INSTALL_QMLDIR})
