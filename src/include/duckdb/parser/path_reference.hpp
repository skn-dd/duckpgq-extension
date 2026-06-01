//===----------------------------------------------------------------------===//
//                         DuckPGQ (stock-DuckDB build)
//
// duckdb/parser/path_reference.hpp
//
// Vendored, lean copy of the fork's PathReference AST node. The fork defined
// this in the engine alongside the SQL/PGQ grammar; this stock-DuckDB build
// vendors only the subset the match-rewrite reads. The grammar/parser is NOT
// ported (it lives in libpg_query's bison and cannot run on stock DuckDB) — the
// pattern_parser builds these nodes directly, and graph_match never serializes
// them, so Serialize/Deserialize are intentionally omitted.
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/common.hpp"

namespace duckdb {

enum class PGQPathReferenceType : uint8_t { PATH_ELEMENT = 0, SUBPATH = 1, UNKNOWN = 2 };

class PathReference {
public:
	PGQPathReferenceType path_reference_type;

public:
	explicit PathReference(PGQPathReferenceType path_reference_type) : path_reference_type(path_reference_type) {
	}

	virtual ~PathReference() {
	}

	virtual unique_ptr<PathReference> Copy() = 0;
};

} // namespace duckdb
