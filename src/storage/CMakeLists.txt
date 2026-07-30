add_library(opencaddie_storage STATIC
    Database.cpp
    Repositories.cpp
)

target_include_directories(opencaddie_storage
    PUBLIC
        ${PROJECT_SOURCE_DIR}/src
)

target_link_libraries(opencaddie_storage
    PUBLIC
        opencaddie_domain
        Qt6::Core
        Qt6::Sql
)

