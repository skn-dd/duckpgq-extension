//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/article_rank.hpp
//
//
//===----------------------------------------------------------------------===//

#include "duckpgq/common.hpp"

namespace duckdb {

class ArticleRankFunction : public TableFunction {
public:
	ArticleRankFunction() {
		name = "article_rank";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR};
		bind_replace = ArticleRankBindReplace;
	}

	static unique_ptr<TableRef> ArticleRankBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct ArticleRankData : TableFunctionData {
	static unique_ptr<FunctionData> ArticleRankBind(ClientContext &context, TableFunctionBindInput &input,
	                                                vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<ArticleRankData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::DOUBLE);
		names.emplace_back("rowid");
		names.emplace_back("article_rank");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

struct ArticleRankScanState : GlobalTableFunctionState {
	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		auto result = make_uniq<ArticleRankScanState>();
		return std::move(result);
	}

	bool finished = false;
};

} // namespace duckdb
