//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/betweenness_centrality_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct BetweennessCentralityFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;
	vector<double_t> betweenness;
	std::mutex state_lock; // Lock for state
	bool state_initialized;

	BetweennessCentralityFunctionData(ClientContext &context, int32_t csr_id);
	static unique_ptr<FunctionData> BetweennessCentralityBind(ClientContext &context, ScalarFunction &bound_function,
	                                                          vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
