add_library(opencaddie_courses STATIC
    CourseProvider.h
    CoursePackageManager.cpp
    FixtureCourseProvider.cpp
    OpenGolfMapProvider.cpp
    StoredZipReader.cpp
)

target_include_directories(opencaddie_courses
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(opencaddie_courses
    PUBLIC
        opencaddie_storage
        Qt6::Core
        Qt6::Network
)
