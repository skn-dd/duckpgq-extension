//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/common_neighbors.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class CommonNeighborsFunction : public TableFunction {
public:
	CommonNeighborsFunction() {
		name = "common_neighbors";
		// common_neighbors(vertex_table, vertex_id_col, edge_table, src_col, dst_col)
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR,
		             LogicalType::VARCHAR};
		bind_replace = CommonNeighborsBindReplace;
	}

	static unique_ptr<TableRef> CommonNeighborsBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct CommonNeighborsData : TableFunctionData {
	static unique_ptr<FunctionData> CommonNeighborsBind(ClientContext &context, TableFunctionBindInput &input,
	                                                    vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<CommonNeighborsData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::BIGINT);
		names.emplace_back("node1");
		names.emplace_back("node2");
		names.emplace_back("common_neighbors");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

} // namespace duckdb
