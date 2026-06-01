#include "duckpgq/core/functions/table/single_source_shortest_path.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"

#include <duckpgq/core/functions/table.hpp>
#include <duckpgq/core/utils/duckpgq_utils.hpp>
#include "duckdb/parser/tableref/basetableref.hpp"

namespace duckdb {

// Main binding function
unique_ptr<TableRef>
SingleSourceShortestPathFunction::SingleSourceShortestPathBindReplace(ClientContext &context,
                                                                      TableFunctionBindInput &input) {
	// single_source_shortest_path(vertex_table, vertex_id_col, edge_table, src_col, dst_col, source_vertex) —
	// runs directly on the given tables; no CREATE PROPERTY GRAPH / registry required.
	auto vertex_table = StringUtil::Lower(StringValue::Get(input.inputs[0]));
	auto vertex_id = StringUtil::Lower(StringValue::Get(input.inputs[1]));
	auto edge_table = StringUtil::Lower(StringValue::Get(input.inputs[2]));
	auto src_col = StringUtil::Lower(StringValue::Get(input.inputs[3]));
	auto dst_col = StringUtil::Lower(StringValue::Get(input.inputs[4]));
	int64_t source = BigIntValue::Get(input.inputs[5]);
	(void)context;

	auto edge_pg_entry = MakeEdgeSpec(vertex_table, vertex_id, edge_table, src_col, dst_col);

	auto select_node =
	    CreateParamSelectNode(edge_pg_entry, "single_source_shortest_path", "distance", Value::BIGINT(source));

	select_node->cte_map.map["csr_cte"] = CreateDirectedCSRCTE(edge_pg_entry, "src", "edge", "dst");

	auto subquery = make_uniq<SelectStatement>();
	subquery->node = std::move(select_node);

	auto result = make_uniq<SubqueryRef>(std::move(subquery));
	result->alias = "single_source_shortest_path";
	return std::move(result);
}

//------------------------------------------------------------------------------
// Register functions
//------------------------------------------------------------------------------
void CoreTableFunctions::RegisterSingleSourceShortestPathTableFunction(ExtensionLoader &loader) {
	loader.RegisterFunction(SingleSourceShortestPathFunction());
}

} // namespace duckdb
