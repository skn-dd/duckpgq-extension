//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/eccentricity.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class EccentricityFunction : public TableFunction {
public:
	EccentricityFunction() {
		name = "eccentricity";
		// eccentricity(vertex_table, vertex_id_col, edge_table, src_col, dst_col)
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR,
		             LogicalType::VARCHAR};
		bind_replace = EccentricityBindReplace;
	}

	static unique_ptr<TableRef> EccentricityBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct EccentricityData : TableFunctionData {
	static unique_ptr<FunctionData> EccentricityBind(ClientContext &context, TableFunctionBindInput &input,
	                                                 vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<EccentricityData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::BIGINT);
		names.emplace_back("rowid");
		names.emplace_back("eccentricity");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

struct EccentricityScanState : GlobalTableFunctionState {
	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		auto result = make_uniq<EccentricityScanState>();
		return std::move(result);
	}

	bool finished = false;
};

} // namespace duckdb
