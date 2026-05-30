//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/katz_centrality_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct KatzCentralityFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;
	vector<double_t> centrality;
	double_t alpha;
	double_t beta;
	int64_t max_iterations;
	double_t convergence_threshold;
	int64_t iteration_count;
	std::mutex state_lock; // Lock for state
	bool state_initialized;
	bool converged;

	KatzCentralityFunctionData(ClientContext &context, int32_t csr_id);
	static unique_ptr<FunctionData> KatzCentralityBind(ClientContext &context, ScalarFunction &bound_function,
	                                                   vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
