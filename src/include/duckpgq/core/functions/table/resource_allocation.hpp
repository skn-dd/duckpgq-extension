//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/resource_allocation.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once

#include "duckpgq/common.hpp"
#include "duckdb/function/table_function.hpp"

namespace duckdb {

class ResourceAllocationFunction : public TableFunction {
public:
	ResourceAllocationFunction() {
		name = "resource_allocation";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR};
		bind_replace = ResourceAllocationBindReplace;
	}

	static unique_ptr<TableRef> ResourceAllocationBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

struct ResourceAllocationData : TableFunctionData {
	static unique_ptr<FunctionData> ResourceAllocationBind(ClientContext &context, TableFunctionBindInput &input,
	                                                       vector<LogicalType> &return_types, vector<string> &names) {
		auto result = make_uniq<ResourceAllocationData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::DOUBLE);
		names.emplace_back("node1");
		names.emplace_back("node2");
		names.emplace_back("resource_allocation");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

} // namespace duckdb
