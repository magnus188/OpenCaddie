add_library(opencaddie_domain STATIC
    ClubRecommendation.cpp
    Geo.cpp
    HoleSelector.cpp
    NearGreenTrigger.cpp
    PlaysLike.cpp
    Scoring.cpp
    Settings.cpp
    Statistics.cpp
)

target_include_directories(opencaddie_domain
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_compile_features(opencaddie_domain PUBLIC cxx_std_20)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(opencaddie_domain PRIVATE
        -Wall -Wextra -Wpedantic -Wconversion -Wshadow
    )
endif()
