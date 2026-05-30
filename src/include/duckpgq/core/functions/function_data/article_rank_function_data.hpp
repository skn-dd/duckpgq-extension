//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/article_rank_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct ArticleRankFunctionData final : FunctionData {
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

	ArticleRankFunctionData(ClientContext &context, int32_t csr_id);
	ArticleRankFunctionData(ClientContext &context, int32_t csr_id, const vector<int64_t> &componentId);
	static unique_ptr<FunctionData> ArticleRankBind(ClientContext &context, ScalarFunction &bound_function,
	                                                vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
