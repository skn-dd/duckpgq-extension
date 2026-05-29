//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/louvain_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct LouvainFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;
	vector<int64_t> community;
	std::mutex state_lock; // Lock for state
	bool state_initialized;

	LouvainFunctionData(ClientContext &context, int32_t csr_id);
	LouvainFunctionData(ClientContext &context, int32_t csr_id, const vector<int64_t> &community);
	static unique_ptr<FunctionData> LouvainBind(ClientContext &context, ScalarFunction &bound_function,
	                                            vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
