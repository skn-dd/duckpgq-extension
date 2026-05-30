//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/in_degree_centrality_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct InDegreeCentralityFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;
	vector<int64_t> indeg; // In-degree per vertex (size v_size)
	std::mutex state_lock; // Lock for state
	bool state_initialized;

	InDegreeCentralityFunctionData(ClientContext &context, int32_t csr_id);
	InDegreeCentralityFunctionData(ClientContext &context, int32_t csr_id, const vector<int64_t> &indeg);
	static unique_ptr<FunctionData> InDegreeCentralityBind(ClientContext &context, ScalarFunction &bound_function,
	                                                       vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
