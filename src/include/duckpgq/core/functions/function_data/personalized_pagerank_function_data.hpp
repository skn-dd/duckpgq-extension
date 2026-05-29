//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/personalized_pagerank_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct PersonalizedPageRankFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;
	vector<double_t> rank;
	vector<double_t> temp_rank;
	double_t damping_factor;
	double_t convergence_threshold;
	int64_t iteration_count;
	std::mutex state_lock; // Lock for state
	bool state_initialized;
	bool converged;

	PersonalizedPageRankFunctionData(ClientContext &context, int32_t csr_id);
	static unique_ptr<FunctionData> PersonalizedPageRankBind(ClientContext &context, ScalarFunction &bound_function,
	                                                         vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
