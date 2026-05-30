#include "duckpgq/core/functions/table/adamic_adar.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckpgq_extension.hpp"
#include "duckpgq/core/utils/duckpgq_utils.hpp"

#include "duckpgq/core/utils/compressed_sparse_row.hpp"
#include <duckdb/parser/query_node/select_node.hpp>
#include <duckdb/parser/tableref/subqueryref.hpp>
#include <duckpgq/core/functions/table.hpp>

namespace duckdb {

unique_ptr<TableRef> AdamicAdarFunction::AdamicAdarBindReplace(ClientContext &context,
                                                              TableFunctionBindInput &input) {
	auto pg_name = StringUtil::Lower(StringValue::Get(input.inputs[0]));
	auto node_label = StringUtil::Lower(StringValue::Get(input.inputs[1]));
	auto edge_label = StringUtil::Lower(StringValue::Get(input.inputs[2]));

	auto duckpgq_state = GetDuckPGQState(context);
	auto pg_info = GetPropertyGraphInfo(duckpgq_state, pg_name);
	auto edge_pg_entry = ValidateSourceNodeAndEdgeTable(pg_info, node_label, edge_label);

	auto select_node = CreatePairwiseSelectNode(edge_pg_entry, "adamic_adar", "adamic_adar");
	select_node->cte_map.map["csr_cte"] = CreateUndirectedCSRCTE(edge_pg_entry, select_node);

	auto subquery = make_uniq<SelectStatement>();
	subquery->node = std::move(select_node);

	auto result = make_uniq<SubqueryRef>(std::move(subquery));
	result->alias = "adamic_adar";
	return std::move(result);
}

//------------------------------------------------------------------------------
// Register functions
//------------------------------------------------------------------------------
void CoreTableFunctions::RegisterAdamicAdarTableFunction(ExtensionLoader &loader) {
	loader.RegisterFunction(AdamicAdarFunction());
}

} // namespace duckdb
