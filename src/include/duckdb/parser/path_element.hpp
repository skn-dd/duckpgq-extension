//===----------------------------------------------------------------------===//
//                         DuckPGQ (stock-DuckDB build)
//
// duckdb/parser/path_element.hpp
//
// Vendored, lean copy of the fork's PathElement AST node (a single
// vertex/edge token in a path pattern). Only the fields the match-rewrite reads
// are kept; Serialize/Deserialize/Equals/ToString are omitted (graph_match
// never serializes and the rewrite dispatches on path_reference_type via
// reinterpret_cast, not virtual calls).
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/parser/path_reference.hpp"

namespace duckdb {

enum class PGQMatchType : uint8_t {
	MATCH_VERTEX = 0,
	MATCH_EDGE_ANY = 1,
	MATCH_EDGE_LEFT = 2,
	MATCH_EDGE_RIGHT = 3,
	MATCH_EDGE_LEFT_RIGHT = 4
};

class PathElement : public PathReference {
public:
	PGQMatchType match_type;

	std::string label;

	std::string variable_binding;

public:
	explicit PathElement(PGQPathReferenceType path_reference_type = PGQPathReferenceType::PATH_ELEMENT)
	    : PathReference(path_reference_type), match_type(PGQMatchType::MATCH_VERTEX) {
	}

	unique_ptr<PathReference> Copy() override {
		auto result = make_uniq<PathElement>(path_reference_type);
		result->match_type = match_type;
		result->label = label;
		result->variable_binding = variable_binding;
		return std::move(result);
	}
};

} // namespace duckdb
