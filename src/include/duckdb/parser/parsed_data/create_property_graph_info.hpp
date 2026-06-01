//===----------------------------------------------------------------------===//
//                         DuckPGQ (stock-DuckDB build)
//
// duckdb/parser/parsed_data/create_property_graph_info.hpp
//
// Vendored, lean copy of the fork's CreatePropertyGraphInfo. The fork derived
// this from CreateInfo (full CREATE PROPERTY GRAPH catalog entry); this
// stock-DuckDB build keeps only the four fields the match-rewrite reads
// (property_graph_name, vertex_tables, edge_tables, label_map). graph_match
// synthesizes one of these in-memory from its table-function arguments + the
// labels parsed from the pattern, so the catalog/CreateInfo machinery and
// Serialize/Deserialize are intentionally dropped.
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/vector.hpp"
#include "duckdb/common/case_insensitive_map.hpp"
#include "duckdb/parser/property_graph_table.hpp"

namespace duckdb {

struct CreatePropertyGraphInfo {
	CreatePropertyGraphInfo() = default;
	explicit CreatePropertyGraphInfo(string property_graph_name) : property_graph_name(std::move(property_graph_name)) {
	}

	//! Property graph name
	string property_graph_name;
	//! List of vertex tables
	vector<shared_ptr<PropertyGraphTable>> vertex_tables;
	//! List of edge tables
	vector<shared_ptr<PropertyGraphTable>> edge_tables;

	//! Dictionary mapping a label to its vertex or edge table
	case_insensitive_map_t<shared_ptr<PropertyGraphTable>> label_map;
};

} // namespace duckdb
