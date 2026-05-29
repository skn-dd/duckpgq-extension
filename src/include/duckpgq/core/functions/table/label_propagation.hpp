//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/label_propagation.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class LabelPropagationFunction : public TableFunction {
public:
	LabelPropagationFunction() {
		name = "label_propagation";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR};
		bind_replace = LabelPropagationBindReplace;
	}

	static unique_ptr<TableRef> LabelPropagationBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct LabelPropagationData : TableFunctionData {
	static unique_ptr<FunctionData> LabelPropagationBind(ClientContext &context, TableFunctionBindInput &input,
	                                                     vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<LabelPropagationData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::BIGINT);
		names.emplace_back("rowid");
		names.emplace_back("label");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

struct LabelPropagationScanState : GlobalTableFunctionState {
	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		auto result = make_uniq<LabelPropagationScanState>();
		return std::move(result);
	}

	bool finished = false;
};

} // namespace duckdb
