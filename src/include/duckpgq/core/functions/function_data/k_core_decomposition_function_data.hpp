//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/k_core_decomposition_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct KCoreDecompositionFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;
	vector<int64_t> core;
	std::mutex state_lock; // Lock for state
	bool state_initialized;

	KCoreDecompositionFunctionData(ClientContext &context, int32_t csr_id);
	KCoreDecompositionFunctionData(ClientContext &context, int32_t csr_id, const vector<int64_t> &core);
	static unique_ptr<FunctionData> KCoreDecompositionBind(ClientContext &context, ScalarFunction &bound_function,
	                                                       vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
