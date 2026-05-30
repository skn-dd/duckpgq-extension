//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/strongly_connected_component.hpp
//
//
//===----------------------------------------------------------------------===//

#include "duckpgq/common.hpp"

namespace duckdb {

class StronglyConnectedComponentFunction : public TableFunction {
public:
	StronglyConnectedComponentFunction() {
		name = "strongly_connected_component";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR};
		bind_replace = StronglyConnectedComponentBindReplace;
	}

	static unique_ptr<TableRef> StronglyConnectedComponentBindReplace(ClientContext &context,
	                                                                  TableFunctionBindInput &input);
};

struct StronglyConnectedComponentData : TableFunctionData {
	static unique_ptr<FunctionData> StronglyConnectedComponentBind(ClientContext &context,
	                                                               TableFunctionBindInput &input,
	                                                               vector<LogicalType> &return_types,
	                                                               vector<string> &names) {
		auto result = make_uniq<StronglyConnectedComponentData>();
		result->pg_name = StringValue::Get(input.inputs[0]);
		result->node_table = StringValue::Get(input.inputs[1]);
		result->edge_table = StringValue::Get(input.inputs[2]);
		return_types.emplace_back(LogicalType::BIGINT);
		return_types.emplace_back(LogicalType::BIGINT);
		names.emplace_back("rowid");
		names.emplace_back("componentId");
		return std::move(result);
	}

	string pg_name;
	string node_table;
	string edge_table;
};

struct StronglyConnectedComponentScanState : GlobalTableFunctionState {
	static unique_ptr<GlobalTableFunctionState> Init(ClientContext &context, TableFunctionInitInput &input) {
		auto result = make_uniq<StronglyConnectedComponentScanState>();
		return std::move(result);
	}

	bool finished = false;
};

} // namespace duckdb
