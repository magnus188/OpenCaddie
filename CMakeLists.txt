cmake_minimum_required(VERSION 3.24)

project(OpenCaddie
    VERSION 0.1.0
    DESCRIPTION "An open-source, local-first golf computer"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(OPENCADDIE_BUILD_UI "Build the Qt Quick device application" ON)
option(OPENCADDIE_BUILD_TESTS "Build unit and integration tests" ON)
option(OPENCADDIE_ENABLE_SANITIZERS "Enable Address/Undefined sanitizers" OFF)

include(GNUInstallDirs)
include(CTest)

if(OPENCADDIE_ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_subdirectory(src/domain)

if(OPENCADDIE_BUILD_UI)
    find_package(Qt6 6.8 REQUIRED COMPONENTS
        Core
        DBus
        Gui
        LinguistTools
        Network
        Qml
        Quick
        QuickControls2
        Sql
        Svg
    )
    qt_standard_project_setup(REQUIRES 6.8 I18N_TRANSLATED_LANGUAGES en nb)

    add_subdirectory(src/storage)
    add_subdirectory(src/courses)
    add_subdirectory(src/positioning)
    add_subdirectory(src/connectivity)
    add_subdirectory(src/platform)
    add_subdirectory(src/ui)
endif()

if(OPENCADDIE_BUILD_TESTS)
    add_subdirectory(tests)
endif()

install(FILES
    README.md
    LICENSE
    docs/architecture.md
    docs/hardware.md
    docs/third-party.md
    docs/v2-extension-path.md
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
)
if(UNIX AND NOT APPLE)
    install(FILES packaging/systemd/opencaddie.service
        DESTINATION lib/systemd/system
    )
    install(FILES packaging/tmpfiles/opencaddie.conf
        DESTINATION lib/tmpfiles.d
    )
endif()
install(FILES deploy/raspberry-pi/config.txt.fragment
    DESTINATION ${CMAKE_INSTALL_DOCDIR}/raspberry-pi
)

set(CPACK_PACKAGE_NAME "opencaddie")
set(CPACK_PACKAGE_VENDOR "OpenCaddie contributors")
set(CPACK_PACKAGE_CONTACT "https://github.com/magnus188/OpenCaddie/issues")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CHECKSUM "SHA256")
set(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenCaddie contributors")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_ARCHIVE_FILE_NAME
    "opencaddie-${PROJECT_VERSION}-${CMAKE_SYSTEM_PROCESSOR}")
set(CPACK_GENERATOR "DEB;TGZ")
include(CPack)
