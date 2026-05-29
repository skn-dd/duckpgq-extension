//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/adamic_adar.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class AdamicAdarFunction : public TableFunction {
public:
	AdamicAdarFunction() {
		name = "adamic_adar";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR};
		bind_replace = AdamicAdarBindReplace;
	}

	static unique_ptr<TableRef> AdamicAdarBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct AdamicAdarData : TableFunctionData {
	static unique_ptr<FunctionData> AdamicAdarBind(ClientContext &context, TableFunctionBindInput &input,
	                                               vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<AdamicAdarData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::DOUBLE);
		names.emplace_back("node1");
		names.emplace_back("node2");
		names.emplace_back("adamic_adar");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

} // namespace duckdb
