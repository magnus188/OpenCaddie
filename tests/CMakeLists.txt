add_executable(opencaddie_domain_tests
    domain_tests.cpp
)
target_link_libraries(opencaddie_domain_tests PRIVATE opencaddie_domain)
target_compile_features(opencaddie_domain_tests PRIVATE cxx_std_20)

add_test(NAME domain COMMAND opencaddie_domain_tests)

if(OPENCADDIE_BUILD_UI)
    add_executable(opencaddie_storage_tests
        storage_tests.cpp
    )
    target_link_libraries(opencaddie_storage_tests
        PRIVATE
            opencaddie_storage
            Qt6::Core
            Qt6::Sql
    )
    add_test(NAME storage COMMAND opencaddie_storage_tests)

    add_executable(opencaddie_course_package_tests
        course_package_tests.cpp
    )
    target_link_libraries(opencaddie_course_package_tests
        PRIVATE
            opencaddie_courses
            opencaddie_storage
            Qt6::Core
            Qt6::Sql
    )
    add_test(NAME course_package COMMAND opencaddie_course_package_tests)

    add_executable(opencaddie_course_map_tests
        course_map_tests.cpp
        ${PROJECT_SOURCE_DIR}/src/ui/CourseMapItem.cpp
    )
    target_include_directories(opencaddie_course_map_tests
        PRIVATE ${PROJECT_SOURCE_DIR}/src)
    target_link_libraries(opencaddie_course_map_tests
        PRIVATE
            Qt6::Core
            Qt6::Gui
            Qt6::Qml
            Qt6::Quick
            Qt6::Svg
    )
    add_test(NAME course_map COMMAND opencaddie_course_map_tests)
    set_tests_properties(course_map PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")

    add_executable(opencaddie_provider_tests provider_tests.cpp)
    target_link_libraries(opencaddie_provider_tests
        PRIVATE
            opencaddie_connectivity
            opencaddie_platform
            opencaddie_positioning
            Qt6::Core
    )
    add_test(NAME providers COMMAND opencaddie_provider_tests)
endif()
