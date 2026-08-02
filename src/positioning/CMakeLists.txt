add_library(opencaddie_positioning STATIC
    PositionProvider.h
    ProviderSelection.h
    GpsdPositionProvider.cpp
    ManualPositionProvider.cpp
    NoFixPositionProvider.cpp
    RouteReplayPositionProvider.cpp
)

target_include_directories(opencaddie_positioning
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(opencaddie_positioning
    PUBLIC
        opencaddie_domain
        Qt6::Core
        Qt6::Network
)
