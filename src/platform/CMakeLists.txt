add_library(opencaddie_platform STATIC
    PowerProvider.h
    MockPowerProvider.cpp
    PiPowerProvider.cpp
)

target_include_directories(opencaddie_platform
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(opencaddie_platform PUBLIC Qt6::Core)
