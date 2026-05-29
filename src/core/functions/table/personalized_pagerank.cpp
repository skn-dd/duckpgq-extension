#include "duckpgq/core/functions/table/personalized_pagerank.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"

#include <duckpgq/core/functions/table.hpp>
#include <duckpgq/core/utils/duckpgq_utils.hpp>
#include "duckpgq/core/utils/compressed_sparse_row.hpp"
#include "duckdb/parser/tableref/basetableref.hpp"

namespace duckdb {

// Main binding function
unique_ptr<TableRef> PersonalizedPageRankFunction::PersonalizedPageRankBindReplace(ClientContext &context,
                                                                                   TableFunctionBindInput &input) {
	auto pg_name = StringUtil::Lower(StringValue::Get(input.inputs[0]));
	auto node_table = StringUtil::Lower(StringValue::Get(input.inputs[1]));
	auto edge_table = StringUtil::Lower(StringValue::Get(input.inputs[2]));
	int64_t source = BigIntValue::Get(input.inputs[3]);

	auto duckpgq_state = GetDuckPGQState(context);
	auto pg_info = GetPropertyGraphInfo(duckpgq_state, pg_name);
	auto edge_pg_entry = ValidateSourceNodeAndEdgeTable(pg_info, node_table, edge_table);

	auto select_node = CreateParamSelectNode(edge_pg_entry, "personalized_pagerank", "personalized_pagerank",
	                                          Value::BIGINT(source));

	select_node->cte_map.map["csr_cte"] = CreateDirectedCSRCTE(edge_pg_entry, "src", "edge", "dst");

	auto subquery = make_uniq<SelectStatement>();
	subquery->node = std::move(select_node);

	auto result = make_uniq<SubqueryRef>(std::move(subquery));
	result->alias = "personalized_pagerank";
	return std::move(result);
}

//------------------------------------------------------------------------------
// Register functions
//------------------------------------------------------------------------------
void CoreTableFunctions::RegisterPersonalizedPagerankTableFunction(ExtensionLoader &loader) {
	loader.RegisterFunction(PersonalizedPageRankFunction());
}

} // namespace duckdb
