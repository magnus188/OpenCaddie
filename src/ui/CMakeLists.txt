set_source_files_properties(qml/Theme.qml qml/KeyboardController.qml
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

# Dock and Finder icon for the macOS development bundle. Regenerate the .icns
# from packaging/macos/icon.svg with tools/make-macos-icon.sh.
if(APPLE)
    set(opencaddie_icns "${PROJECT_SOURCE_DIR}/packaging/macos/OpenCaddie.icns")
    set_source_files_properties(${opencaddie_icns} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources"
        QT_EXCLUDE_FROM_TRANSLATION TRUE
    )
    target_sources(opencaddie PRIVATE ${opencaddie_icns})
    set_target_properties(opencaddie PROPERTIES
        MACOSX_BUNDLE_ICON_FILE "OpenCaddie.icns"
    )
endif()

qt_add_qml_module(opencaddie
    URI OpenCaddie
    VERSION 1.0
    QML_FILES
        qml/Main.qml
        qml/Theme.qml
        qml/KeyboardController.qml
        qml/components/AppButton.qml
        qml/components/AppSelectSheet.qml
        qml/components/AppSlider.qml
        qml/components/AppSwitch.qml
        qml/components/AppTextField.qml
        qml/components/BottomNav.qml
        qml/components/BrandLockup.qml
        qml/components/ClubArtwork.qml
        qml/components/ClubPickerSheet.qml
        qml/components/ClubDetailsSheet.qml
        qml/components/ConfirmDialog.qml
        qml/components/HoleHeader.qml
        qml/components/FlyingBird.qml
        qml/components/IconButton.qml
        qml/components/MetricCard.qml
        qml/components/OnScreenKeyboard.qml
        qml/components/PageScaffold.qml
        qml/components/RadialMapGlow.qml
        qml/components/ScoreCelebrationOverlay.qml
        qml/components/ScorePromptBanner.qml
        qml/components/SectionCard.qml
        qml/components/SplashOverlay.qml
        qml/components/StaticMapPreview.qml
        qml/components/TopBar.qml
        qml/components/SystemStatusBar.qml
        qml/screens/BagScreen.qml
        qml/screens/CourseLibraryScreen.qml
        qml/screens/CourseSearchScreen.qml
        qml/screens/CoursePlannerScreen.qml
        qml/screens/CoursePlannerMapScreen.qml
        qml/screens/HistoryScreen.qml
        qml/screens/LiveHoleScreen.qml
        qml/screens/HoleScoreScreen.qml
        qml/screens/RoundDetailScreen.qml
        qml/screens/RoundMapScreen.qml
        qml/screens/RoundSetupScreen.qml
        qml/screens/ScorecardScreen.qml
        qml/screens/SettingsConnectivityScreen.qml
        qml/screens/SettingsDisplayScreen.qml
        qml/screens/SettingsIntegrationsScreen.qml
        qml/screens/SettingsMapScreen.qml
        qml/screens/SettingsRoundScreen.qml
        qml/screens/SettingsScreen.qml
        qml/screens/StatsScreen.qml
        qml/screens/WelcomeScreen.qml
        qml/screens/WifiScreen.qml
    RESOURCES
        assets/brand/mark.svg
        assets/club-heads/bag-rack.png
        assets/club-heads/driver.png
        assets/club-heads/hybrid.png
        assets/club-heads/iron.png
        assets/club-heads/other.png
        assets/club-heads/putter.png
        assets/club-heads/wedge.png
        assets/club-heads/wood.png
        assets/clubs/bag.png
        assets/clubs/driver.png
        assets/clubs/hybrid.png
        assets/clubs/iron.png
        assets/clubs/other.png
        assets/clubs/putter.png
        assets/clubs/wedge.png
        assets/clubs/wood.png
        assets/demo/manifest.json
        assets/demo/navigation.json
        assets/demo/home-render-model.json
        assets/demo/render-model.json
        assets/routes/demo-route.csv
        assets/fonts/Inter-Bold.ttf
        assets/fonts/Inter-Medium.ttf
        assets/fonts/Inter-Regular.ttf
        assets/fonts/Inter-SemiBold.ttf
        assets/fonts/OFL.txt
        assets/illustrations/home-course-cubist.png
        assets/icons/close.svg
        assets/icons/compass.svg
        assets/icons/dogleg.svg
        assets/icons/expand.svg
        assets/icons/measure.svg
        assets/icons/reset.svg
        assets/icons/rotate.svg
        assets/icons/figma/weather-direction.svg
        assets/icons/figma/weather-rain.svg
        assets/icons/figma/weather-wind.svg
        assets/wildlife/bird-body.svg
        assets/wildlife/bird-wing-lower.svg
        assets/wildlife/bird-wing-upper.svg
        assets/wildlife/eagle-body.svg
        assets/wildlife/eagle-wing-lower.svg
        assets/wildlife/eagle-wing-upper.svg
        assets/icons/lucide/LICENSE.txt
        assets/icons/lucide/battery-charging.svg
        assets/icons/lucide/battery-charging-dark.svg
        assets/icons/lucide/battery-full.svg
        assets/icons/lucide/battery-full-dark.svg
        assets/icons/lucide/battery-low.svg
        assets/icons/lucide/battery-low-dark.svg
        assets/icons/lucide/battery-medium.svg
        assets/icons/lucide/battery-medium-dark.svg
        assets/icons/lucide/bluetooth.svg
        assets/icons/lucide/bluetooth-dark.svg
        assets/icons/lucide/check.svg
        assets/icons/lucide/chevron-left.svg
        assets/icons/lucide/cloud-rain.svg
        assets/icons/lucide/chevron-right.svg
        assets/icons/lucide/menu.svg
        assets/icons/lucide/minus.svg
        assets/icons/lucide/navigation.svg
        assets/icons/lucide/plug-zap.svg
        assets/icons/lucide/plug-zap-dark.svg
        assets/icons/lucide/plus.svg
        assets/icons/lucide/ruler.svg
        assets/icons/lucide/search.svg
        assets/icons/lucide/thermometer.svg
        assets/icons/lucide/trash-2.svg
        assets/icons/lucide/undo-2.svg
        assets/icons/lucide/wifi-high.svg
        assets/icons/lucide/wifi-high-dark.svg
        assets/icons/lucide/wifi-low.svg
        assets/icons/lucide/wifi-low-dark.svg
        assets/icons/lucide/wifi.svg
        assets/icons/lucide/wifi-dark.svg
        assets/icons/lucide/wind-arrow.svg
        assets/icons/lucide/x.svg
)

target_include_directories(opencaddie PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(opencaddie
    PRIVATE
        opencaddie_connectivity
        opencaddie_courses
        opencaddie_domain
        opencaddie_integrations
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
