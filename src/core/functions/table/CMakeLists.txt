# Stock-DuckDB build — vertical slice.
# Only pagerank is decoupled (operates on edge/vertex tables directly). The other
# 27 algorithm + path-finding table binds still reference the old property-graph
# registry helpers; they are re-added here one-by-one as each is decoupled to the
# MakeEdgeSpec(args) pattern (the parallelizable fan-out). Grammar/PG table
# functions (create/describe/drop/match/scan/summarize) are excluded entirely.
set(EXTENSION_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/pagerank.cpp
    ${EXTENSION_SOURCES}
    PARENT_SCOPE)
