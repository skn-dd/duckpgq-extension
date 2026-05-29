//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/function_data/overlap_similarity_function_data.hpp
//
//
//===----------------------------------------------------------------------===//

#pragma once
#include "duckdb/main/client_context.hpp"
#include "duckpgq/common.hpp"

namespace duckdb {

struct OverlapSimilarityFunctionData final : FunctionData {
	ClientContext &context;
	int32_t csr_id;

	OverlapSimilarityFunctionData(ClientContext &context, int32_t csr_id);
	static unique_ptr<FunctionData> OverlapSimilarityBind(ClientContext &context, ScalarFunction &bound_function,
	                                                      vector<unique_ptr<Expression>> &arguments);

	unique_ptr<FunctionData> Copy() const override;
	bool Equals(const FunctionData &other_p) const override;
};

} // namespace duckdb
