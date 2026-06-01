//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/in_degree_centrality.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class InDegreeCentralityFunction : public TableFunction {
public:
	InDegreeCentralityFunction() {
		name = "in_degree_centrality";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR,
		             LogicalType::VARCHAR};
		bind_replace = InDegreeCentralityBindReplace;
	}

	static unique_ptr<TableRef> InDegreeCentralityBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct InDegreeCentralityData : TableFunctionData {
	static unique_ptr<FunctionData> InDegreeCentralityBind(ClientContext &context, TableFunctionBindInput &input,
	                                                       vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<InDegreeCentralityData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::BIGINT);
		names.emplace_back("rowid");
		names.emplace_back("in_degree_centrality");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

struct InDegreeCentralityScanState : GlobalTableFunctionState {
	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		auto result = make_uniq<InDegreeCentralityScanState>();
		return std::move(result);
	}

	bool finished = false;
};

} // namespace duckdb
