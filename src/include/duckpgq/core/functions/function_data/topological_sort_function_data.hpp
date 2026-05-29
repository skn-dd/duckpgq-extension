//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/topological_sort_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct TopologicalSortFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;
	vector<int64_t> order; // Topological order index per vertex (size v_size)
	std::mutex state_lock; // Lock for state
	bool state_initialized;

	TopologicalSortFunctionData(ClientContext &context, int32_t csr_id);
	TopologicalSortFunctionData(ClientContext &context, int32_t csr_id, const vector<int64_t> &order);
	static unique_ptr<FunctionData> TopologicalSortBind(ClientContext &context, ScalarFunction &bound_function,
	                                                    vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
