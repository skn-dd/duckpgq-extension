#include "duckpgq/core/functions/table/hits.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"

#include <duckpgq/core/functions/table.hpp>
#include <duckpgq/core/utils/duckpgq_utils.hpp>
#include <duckpgq/core/utils/compressed_sparse_row.hpp>
#include "duckdb/parser/tableref/basetableref.hpp"

namespace duckdb {

// Authority binding function
unique_ptr<TableRef> HitsAuthorityFunction::HitsAuthorityBindReplace(ClientContext &context,
                                                                     TableFunctionBindInput &input) {
	auto pg_name = StringUtil::Lower(StringValue::Get(input.inputs[0]));
	auto node_table = StringUtil::Lower(StringValue::Get(input.inputs[1]));
	auto edge_table = StringUtil::Lower(StringValue::Get(input.inputs[2]));

	auto duckpgq_state = GetDuckPGQState(context);
	auto pg_info = GetPropertyGraphInfo(duckpgq_state, pg_name);
	auto edge_pg_entry = ValidateSourceNodeAndEdgeTable(pg_info, node_table, edge_table);

	auto select_node = CreateSelectNode(edge_pg_entry, "hits_authority", "hits_authority");

	select_node->cte_map.map["csr_cte"] = CreateDirectedCSRCTE(edge_pg_entry, "src", "edge", "dst");

	auto subquery = make_uniq<SelectStatement>();
	subquery->node = std::move(select_node);

	auto result = make_uniq<SubqueryRef>(std::move(subquery));
	result->alias = "hits_authority";
	return std::move(result);
}

// Hub binding function
unique_ptr<TableRef> HitsHubFunction::HitsHubBindReplace(ClientContext &context, TableFunctionBindInput &input) {
	auto pg_name = StringUtil::Lower(StringValue::Get(input.inputs[0]));
	auto node_table = StringUtil::Lower(StringValue::Get(input.inputs[1]));
	auto edge_table = StringUtil::Lower(StringValue::Get(input.inputs[2]));

	auto duckpgq_state = GetDuckPGQState(context);
	auto pg_info = GetPropertyGraphInfo(duckpgq_state, pg_name);
	auto edge_pg_entry = ValidateSourceNodeAndEdgeTable(pg_info, node_table, edge_table);

	auto select_node = CreateSelectNode(edge_pg_entry, "hits_hub", "hits_hub");

	select_node->cte_map.map["csr_cte"] = CreateDirectedCSRCTE(edge_pg_entry, "src", "edge", "dst");

	auto subquery = make_uniq<SelectStatement>();
	subquery->node = std::move(select_node);

	auto result = make_uniq<SubqueryRef>(std::move(subquery));
	result->alias = "hits_hub";
	return std::move(result);
}

//------------------------------------------------------------------------------
// Register functions
//------------------------------------------------------------------------------
void CoreTableFunctions::RegisterHitsAuthorityTableFunction(ExtensionLoader &loader) {
	loader.RegisterFunction(HitsAuthorityFunction());
}

void CoreTableFunctions::RegisterHitsHubTableFunction(ExtensionLoader &loader) {
	loader.RegisterFunction(HitsHubFunction());
}

} // namespace duckdb
