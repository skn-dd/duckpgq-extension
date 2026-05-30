//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/label_propagation_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct LabelPropagationFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;
	vector<int64_t> label;
	std::mutex state_lock; // Lock for state
	bool state_initialized;

	LabelPropagationFunctionData(ClientContext &context, int32_t csr_id);
	LabelPropagationFunctionData(ClientContext &context, int32_t csr_id, const vector<int64_t> &label);
	static unique_ptr<FunctionData> LabelPropagationBind(ClientContext &context, ScalarFunction &bound_function,
	                                                     vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
