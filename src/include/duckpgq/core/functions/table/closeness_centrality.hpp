//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/closeness_centrality.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class ClosenessCentralityFunction : public TableFunction {
public:
	ClosenessCentralityFunction() {
		name = "closeness_centrality";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR};
		bind_replace = ClosenessCentralityBindReplace;
	}

	static unique_ptr<TableRef> ClosenessCentralityBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct ClosenessCentralityData : TableFunctionData {
	static unique_ptr<FunctionData> ClosenessCentralityBind(ClientContext &context, TableFunctionBindInput &input,
	                                                        vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<ClosenessCentralityData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::DOUBLE);
		names.emplace_back("rowid");
		names.emplace_back("closeness_centrality");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

struct ClosenessCentralityScanState : GlobalTableFunctionState {
	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		auto result = make_uniq<ClosenessCentralityScanState>();
		return std::move(result);
	}

	bool finished = false;
};

} // namespace duckdb
