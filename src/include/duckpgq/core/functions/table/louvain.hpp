//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/louvain.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class LouvainFunction : public TableFunction {
public:
	LouvainFunction() {
		name = "louvain";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR};
		bind_replace = LouvainBindReplace;
	}

	static unique_ptr<TableRef> LouvainBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct LouvainData : TableFunctionData {
	static unique_ptr<FunctionData> LouvainBind(ClientContext &context, TableFunctionBindInput &input,
	                                            vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<LouvainData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::BIGINT);
		names.emplace_back("rowid");
		names.emplace_back("community");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

struct LouvainScanState : GlobalTableFunctionState {
	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		auto result = make_uniq<LouvainScanState>();
		return std::move(result);
	}

	bool finished = false;
};

} // namespace duckdb
