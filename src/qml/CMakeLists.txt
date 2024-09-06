# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

ecm_add_qml_module(KF6ColorScheme URI org.kde.colorscheme VERSION 1.0 GENERATE_PLUGIN_SOURCE)

target_link_libraries(KF6ColorScheme PRIVATE
    Qt6::Qml
)

ecm_finalize_qml_module(KF6ColorScheme DESTINATION ${KDE_INSTALL_QMLDIR})
