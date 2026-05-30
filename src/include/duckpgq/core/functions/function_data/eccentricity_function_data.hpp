//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/eccentricity_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct EccentricityFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;
	vector<int64_t> ecc;
	std::mutex state_lock; // Lock for state
	bool state_initialized;

	EccentricityFunctionData(ClientContext &context, int32_t csr_id);
	static unique_ptr<FunctionData> EccentricityBind(ClientContext &context, ScalarFunction &bound_function,
	                                                 vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
