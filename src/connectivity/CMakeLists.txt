add_library(opencaddie_connectivity STATIC
    NetworkManager.h
    MockNetworkManager.cpp
    NetworkManagerDbus.cpp
)

target_include_directories(opencaddie_connectivity
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(opencaddie_connectivity
    PUBLIC
        Qt6::Core
        Qt6::DBus
        Qt6::Network
)
