//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/cosine_similarity.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class CosineSimilarityFunction : public TableFunction {
public:
	CosineSimilarityFunction() {
		name = "cosine_similarity";
		// cosine_similarity(vertex_table, vertex_id_col, edge_table, src_col, dst_col)
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR,
		             LogicalType::VARCHAR};
		bind_replace = CosineSimilarityBindReplace;
	}

	static unique_ptr<TableRef> CosineSimilarityBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct CosineSimilarityData : TableFunctionData {
	static unique_ptr<FunctionData> CosineSimilarityBind(ClientContext &context, TableFunctionBindInput &input,
	                                                     vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<CosineSimilarityData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::DOUBLE);
		names.emplace_back("node1");
		names.emplace_back("node2");
		names.emplace_back("cosine_similarity");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

} // namespace duckdb
