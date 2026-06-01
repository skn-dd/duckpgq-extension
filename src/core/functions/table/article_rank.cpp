#include "duckpgq/core/functions/table/article_rank.hpp"
#include "duckdb/function/table_function.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"

#include <duckpgq/core/functions/table.hpp>
#include <duckpgq/core/utils/duckpgq_utils.hpp>
#include "duckdb/parser/tableref/basetableref.hpp"

namespace duckdb {

// Main binding function
unique_ptr<TableRef> ArticleRankFunction::ArticleRankBindReplace(ClientContext &context,
                                                                 TableFunctionBindInput &input) {
	auto vertex_table = StringUtil::Lower(StringValue::Get(input.inputs[0]));
	auto vertex_id = StringUtil::Lower(StringValue::Get(input.inputs[1]));
	auto edge_table = StringUtil::Lower(StringValue::Get(input.inputs[2]));
	auto src_col = StringUtil::Lower(StringValue::Get(input.inputs[3]));
	auto dst_col = StringUtil::Lower(StringValue::Get(input.inputs[4]));
	(void)context;

	auto edge_pg_entry = MakeEdgeSpec(vertex_table, vertex_id, edge_table, src_col, dst_col);

	auto select_node = CreateSelectNode(edge_pg_entry, "article_rank", "article_rank");

	select_node->cte_map.map["csr_cte"] = CreateDirectedCSRCTE(edge_pg_entry, "src", "edge", "dst");

	auto subquery = make_uniq<SelectStatement>();
	subquery->node = std::move(select_node);

	auto result = make_uniq<SubqueryRef>(std::move(subquery));
	result->alias = "article_rank";
	return std::move(result);
}

//------------------------------------------------------------------------------
// Register functions
//------------------------------------------------------------------------------
void CoreTableFunctions::RegisterArticleRankTableFunction(ExtensionLoader &loader) {
	loader.RegisterFunction(ArticleRankFunction());
}

} // namespace duckdb
