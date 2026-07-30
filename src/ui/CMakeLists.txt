set_source_files_properties(qml/Theme.qml
    PROPERTIES QT_QML_SINGLETON_TYPE TRUE
)
set_source_files_properties(assets/fonts/OFL.txt
    PROPERTIES QT_EXCLUDE_FROM_TRANSLATION TRUE
)

qt_add_executable(opencaddie
    AppController.cpp
    CourseMapItem.cpp
    TranslationManager.cpp
    main.cpp
)

set_target_properties(opencaddie PROPERTIES
    OUTPUT_NAME "OpenCaddie"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "OpenCaddie"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
    MACOSX_BUNDLE_GUI_IDENTIFIER "io.opencaddie.device"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    QT_EXCLUDE_SOURCES_FROM_TRANSLATION
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/OFL.txt"
)

qt_add_qml_module(opencaddie
    URI OpenCaddie
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/components/AppButton.qml
        qml/components/AppTextField.qml
        qml/components/BottomNav.qml
        qml/components/ConfirmDialog.qml
        qml/components/MetricCard.qml
        qml/components/OnScreenKeyboard.qml
        qml/components/SectionCard.qml
        qml/components/TopBar.qml
        qml/screens/BagScreen.qml
        qml/screens/CourseLibraryScreen.qml
        qml/screens/HistoryScreen.qml
        qml/screens/LiveHoleScreen.qml
        qml/screens/RoundSetupScreen.qml
        qml/screens/ScorecardScreen.qml
        qml/screens/SettingsScreen.qml
        qml/screens/WelcomeScreen.qml
        qml/screens/WifiScreen.qml
    RESOURCES
        assets/demo/manifest.json
        assets/demo/navigation.json
        assets/demo/render-model.json
        assets/routes/demo-route.csv
        assets/fonts/Inter-Bold.ttf
        assets/fonts/Inter-Medium.ttf
        assets/fonts/Inter-Regular.ttf
        assets/fonts/Inter-SemiBold.ttf
        assets/fonts/OFL.txt
)

target_include_directories(opencaddie PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(opencaddie
    PRIVATE
        opencaddie_connectivity
        opencaddie_courses
        opencaddie_domain
        opencaddie_platform
        opencaddie_positioning
        opencaddie_storage
        Qt6::Core
        Qt6::Gui
        Qt6::Network
        Qt6::Qml
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Sql
        Qt6::Svg
)

qt_add_translations(opencaddie
    TS_FILES
        i18n/opencaddie_en.ts
        i18n/opencaddie_nb.ts
    RESOURCE_PREFIX "/i18n"
    LUPDATE_OPTIONS -no-obsolete
)

install(TARGETS opencaddie
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
