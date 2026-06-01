//===----------------------------------------------------------------------===//
//                         DuckPGQ (stock-DuckDB build)
//
// duckdb/parser/property_graph_table.hpp
//
// Vendored, lean copy of the fork's PropertyGraphTable. The fork defined this
// in the engine; this stock-DuckDB build vendors only the subset the CSR +
// bind-replace code reads (edge/vertex table + key columns + CreateBaseTableRef)
// so those code paths compile unchanged while being fed an edge-spec built
// directly from table-function arguments (no property-graph registration).
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb/common/vector.hpp"
#include "duckdb/parser/tableref/basetableref.hpp"

namespace duckdb {

//! Resolved description of one edge (or vertex) table used to build the CSR.
//! Populated from table-function arguments — see MakeEdgeSpec in duckpgq_utils.
class PropertyGraphTable {
public:
	PropertyGraphTable() = default;

	string table_name;
	string table_name_alias;

	vector<string> column_names;
	vector<string> column_aliases;
	vector<string> except_columns;

	vector<string> sub_labels;
	string main_label;

	string catalog_name;
	string schema_name;

	bool all_columns = false;
	bool no_columns = false;
	bool is_vertex_table = false;

	string discriminator;

	vector<string> source_fk;
	vector<string> source_pk;
	string source_catalog;
	string source_schema;
	string source_reference;
	shared_ptr<PropertyGraphTable> source_pg_table;

	vector<string> destination_fk;
	vector<string> destination_pk;
	string destination_catalog;
	string destination_schema;
	string destination_reference;
	shared_ptr<PropertyGraphTable> destination_pg_table;

	bool hasTableNameAlias() const {
		return !table_name_alias.empty();
	}

	unique_ptr<BaseTableRef> CreateBaseTableRef(const string &alias = "") const {
		auto base_table_ref = make_uniq<BaseTableRef>();
		base_table_ref->catalog_name = catalog_name;
		base_table_ref->schema_name = schema_name;
		base_table_ref->table_name = table_name;
		base_table_ref->alias = alias.empty() ? "" : alias;
		return base_table_ref;
	}
};

} // namespace duckdb
