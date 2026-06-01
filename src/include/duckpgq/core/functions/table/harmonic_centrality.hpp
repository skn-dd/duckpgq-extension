//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/harmonic_centrality.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class HarmonicCentralityFunction : public TableFunction {
public:
	HarmonicCentralityFunction() {
		name = "harmonic_centrality";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR,
		             LogicalType::VARCHAR};
		bind_replace = HarmonicCentralityBindReplace;
	}

	static unique_ptr<TableRef> HarmonicCentralityBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct HarmonicCentralityData : TableFunctionData {
	static unique_ptr<FunctionData> HarmonicCentralityBind(ClientContext &context, TableFunctionBindInput &input,
	                                                       vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<HarmonicCentralityData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::DOUBLE);
		names.emplace_back("rowid");
		names.emplace_back("harmonic_centrality");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

struct HarmonicCentralityScanState : GlobalTableFunctionState {
	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		auto result = make_uniq<HarmonicCentralityScanState>();
		return std::move(result);
	}

	bool finished = false;
};

} // namespace duckdb
