//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/katz_centrality.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class KatzCentralityFunction : public TableFunction {
public:
	KatzCentralityFunction() {
		name = "katz_centrality";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR};
		bind_replace = KatzCentralityBindReplace;
	}

	static unique_ptr<TableRef> KatzCentralityBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct KatzCentralityData : TableFunctionData {
	static unique_ptr<FunctionData> KatzCentralityBind(ClientContext &context, TableFunctionBindInput &input,
	                                                   vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<KatzCentralityData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::DOUBLE);
		names.emplace_back("rowid");
		names.emplace_back("katz_centrality");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

struct KatzCentralityScanState : GlobalTableFunctionState {
	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		auto result = make_uniq<KatzCentralityScanState>();
		return std::move(result);
	}

	bool finished = false;
};

} // namespace duckdb
