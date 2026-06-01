//===----------------------------------------------------------------------===//
//                         DuckPGQ (stock-DuckDB build)
//
// duckdb/parser/path_pattern.hpp
//
// Vendored, lean copy of the fork's PathPattern AST node (one comma-separated
// path in a MATCH). Only fields the match-rewrite reads are kept;
// Serialize/Deserialize/Equals omitted.
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/vector.hpp"
#include "duckdb/parser/parsed_expression.hpp"
#include "duckdb/parser/path_reference.hpp"
#include "duckdb/parser/path_element.hpp"
#include "duckdb/parser/subpath_element.hpp"

namespace duckdb {

class PathPattern {
public:
	unique_ptr<ParsedExpression> where_clause;
	vector<unique_ptr<PathReference>> path_elements;

	bool all = false;
	bool shortest = false;
	bool group = false;
	int32_t topk = false;

	PathPattern() = default;

	unique_ptr<PathPattern> Copy() {
		auto result = make_uniq<PathPattern>();
		if (where_clause) {
			result->where_clause = where_clause->Copy();
		}
		for (auto &element : path_elements) {
			result->path_elements.push_back(element->Copy());
		}
		result->all = all;
		result->shortest = shortest;
		result->group = group;
		result->topk = topk;
		return result;
	}
};

} // namespace duckdb
