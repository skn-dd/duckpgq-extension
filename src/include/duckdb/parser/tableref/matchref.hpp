//===----------------------------------------------------------------------===//
//                         DuckPGQ (stock-DuckDB build)
//
// duckdb/parser/tableref/matchref.hpp
//
// Vendored, lean copy of the fork's MatchExpression. It carries the parsed
// path patterns + the projected column list that the match-rewrite consumes.
// The fork built this from the SQL/PGQ grammar; this stock-DuckDB build builds
// it directly from the pattern_parser. The base-class virtuals required to
// instantiate a ParsedExpression are stubbed inline (graph_match never
// serializes, hashes, or re-binds this node).
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/vector.hpp"
#include "duckdb/parser/path_pattern.hpp"
#include "duckdb/parser/parsed_expression.hpp"
#include "duckdb/common/enums/expression_type.hpp"

namespace duckdb {

class MatchExpression : public ParsedExpression {
public:
	static constexpr const ExpressionClass TYPE = ExpressionClass::BOUND_EXPRESSION;

public:
	MatchExpression() : ParsedExpression(ExpressionType::FUNCTION_REF, ExpressionClass::BOUND_EXPRESSION) {
	}

	string pg_name;
	string alias;
	vector<unique_ptr<PathPattern>> path_patterns;

	vector<unique_ptr<ParsedExpression>> column_list;

	unique_ptr<ParsedExpression> where_clause;

public:
	string ToString() const override {
		return "MATCH";
	}
	bool Equals(const BaseExpression &other_p) const override {
		return false;
	}
	bool IsAggregate() const override {
		return false;
	}
	bool IsWindow() const override {
		return false;
	}
	bool HasSubquery() const override {
		return false;
	}
	bool IsScalar() const override {
		return false;
	}
	bool HasParameter() const override {
		return false;
	}
	hash_t Hash() const override {
		return 0;
	}

	unique_ptr<ParsedExpression> Copy() const override {
		auto result = make_uniq<MatchExpression>();
		result->pg_name = pg_name;
		result->alias = alias;
		for (auto &pattern : path_patterns) {
			result->path_patterns.push_back(const_cast<PathPattern &>(*pattern).Copy());
		}
		for (auto &col : column_list) {
			result->column_list.push_back(col->Copy());
		}
		if (where_clause) {
			result->where_clause = where_clause->Copy();
		}
		return std::move(result);
	}
};

} // namespace duckdb
