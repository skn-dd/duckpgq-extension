# operator/ and parser/ (SQL/PGQ grammar extensions) are dropped: this builds
# against stock DuckDB, which has no PGQ grammar. Graph patterns are lowered to
# SQL by the DFL compiler; only algorithm/path-finding functions are built.
add_subdirectory(functions)
add_subdirectory(pragma)
add_subdirectory(utils)

set(EXTENSION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp ${EXTENSION_SOURCES}
    PARENT_SCOPE)
