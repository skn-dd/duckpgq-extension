//===----------------------------------------------------------------------===//
//                         DuckPGQ (stock-DuckDB build)
//
// duckpgq/core/parser/pattern_parser.hpp
//
// A small, bounded recursive-descent parser for the DFL-emitted PGQ pattern
// subset. The full SQL/PGQ grammar lives in libpg_query's bison and cannot run
// on stock DuckDB, so graph_match takes the pattern as a plain string and this
// parser turns it into the (vendored) MatchExpression AST that the ported
// match-rewrite consumes.
//
// Supported grammar (whitespace-insensitive):
//   pattern   := node (edge node)*
//   node      := '(' var (':' Label)? ')'
//   edge      := '-' '[' var (':' Label)? ']' '->' '{' lo ',' hi '}'?     (right)
//             |  '<-' '[' var (':' Label)? ']' '-'                         (left)
//             |  '-' '[' var (':' Label)? ']' '-'                         (any/undirected)
//   var/Label := [A-Za-z_][A-Za-z0-9_]*
// Variable-length quantifier '{lo,hi}' is only accepted on a right edge.
// Anything else raises a specific ParserException.
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/parser/tableref/matchref.hpp"

namespace duckdb {

//! Parse a single PGQ path pattern string into a MatchExpression. The set of
//! distinct labels encountered (vertex + edge) is returned in [out_labels] so
//! the caller can synthesize the matching CreatePropertyGraphInfo. Throws
//! ParserException with a specific message on unsupported / malformed input.
unique_ptr<MatchExpression> ParseGraphPattern(const string &pattern, vector<string> &out_vertex_labels,
                                              vector<string> &out_edge_labels);

} // namespace duckdb
