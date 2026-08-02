add_library(opencaddie_integrations STATIC
    IntegrationCatalog.cpp
)

target_include_directories(opencaddie_integrations
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(opencaddie_integrations
    PUBLIC
        Qt6::Core
)
