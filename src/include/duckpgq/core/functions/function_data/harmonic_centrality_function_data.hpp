//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/harmonic_centrality_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct HarmonicCentralityFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;
	vector<double_t> harmonic;
	std::mutex state_lock; // Lock for state
	bool state_initialized;

	HarmonicCentralityFunctionData(ClientContext &context, int32_t csr_id);
	static unique_ptr<FunctionData> HarmonicCentralityBind(ClientContext &context, ScalarFunction &bound_function,
	                                                       vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
