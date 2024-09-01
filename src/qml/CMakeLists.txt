# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_qml_module(kquickcolorschemeplugin URI "org.kde.colorscheme")

target_sources(kquickcolorschemeplugin PRIVATE
    kquickcolorschemeplugin.cpp
)

target_link_libraries(kquickcolorschemeplugin PRIVATE
    KF6ColorScheme
    Qt6::Quick
)

ecm_finalize_qml_module(kquickcolorschemeplugin DESTINATION ${KDE_INSTALL_QMLDIR})
