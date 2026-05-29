#include "duckdb/planner/expression/bound_function_expression.hpp"
#include "duckpgq/common.hpp"
#include "duckpgq/core/functions/function_data/k_core_decomposition_function_data.hpp"
#include <duckpgq/core/functions/scalar.hpp>
#include <duckpgq/core/functions/table/k_core_decomposition.hpp>
#include <duckpgq/core/utils/duckpgq_utils.hpp>

namespace duckdb {

static void KCoreDecompositionFunction(DataChunk &args, ExpressionState &state, Vector &result) {
	auto &func_expr = state.expr.Cast<BoundFunctionExpression>();
	auto &info = func_expr.bind_info->Cast<KCoreDecompositionFunctionData>();
	auto duckpgq_state = GetDuckPGQState(info.context);

	// Locate the CSR representation of the graph
	auto csr_entry = duckpgq_state->csr_list.find(info.csr_id);
	if (csr_entry == duckpgq_state->csr_list.end()) {
		throw ConstraintException("CSR not found. Is the graph populated?");
	}

	if (!(csr_entry->second->initialized_v && csr_entry->second->initialized_e)) {
		throw ConstraintException("Need to initialize CSR before running k-core decomposition.");
	}

	auto *v = reinterpret_cast<int64_t *>(duckpgq_state->csr_list[info.csr_id]->v);
	vector<int64_t> &e = duckpgq_state->csr_list[info.csr_id]->e;
	size_t v_size = duckpgq_state->csr_list[info.csr_id]->vsize;
	CheckAlgorithmMemoryBudget(info.context, (idx_t)v_size * sizeof(int64_t) * 3, "k_core_decomposition");

	// State initialization and computation (only once)
	if (!info.state_initialized) {
		std::lock_guard<std::mutex> guard(info.state_lock); // Thread safety
		if (!info.state_initialized) {
			// vsize = num_vertices + 2 (padding); real vertices are those with u + 2 < v_size.
			info.core.assign(v_size, 0);

			// Residual degree per vertex (current degree in the remaining subgraph).
			vector<int64_t> degree(v_size, 0);
			vector<bool> removed(v_size, false);
			for (size_t u = 0; u + 2 < v_size; u++) {
				int64_t start_edge = v[u];
				int64_t end_edge = (u + 1 < v_size) ? v[u + 1] : static_cast<int64_t>(e.size());
				degree[u] = end_edge - start_edge;
			}

			// Repeatedly remove the minimum residual-degree vertex.
			int64_t seen_so_far = 0;
			for (size_t iter = 0; iter + 2 < v_size; iter++) {
				// Find the remaining vertex with the smallest residual degree.
				int64_t min_vertex = -1;
				int64_t min_degree = 0;
				for (size_t u = 0; u + 2 < v_size; u++) {
					if (removed[u]) {
						continue;
					}
					if (min_vertex == -1 || degree[u] < min_degree) {
						min_vertex = static_cast<int64_t>(u);
						min_degree = degree[u];
					}
				}
				if (min_vertex == -1) {
					break;
				}

				// Core number is the running maximum of removed residual degrees.
				if (degree[min_vertex] > seen_so_far) {
					seen_so_far = degree[min_vertex];
				}
				info.core[min_vertex] = seen_so_far;
				removed[min_vertex] = true;

				// Remove it: decrement residual degree of its remaining neighbors.
				int64_t start_edge = v[min_vertex];
				int64_t end_edge =
				    (static_cast<size_t>(min_vertex) + 1 < v_size) ? v[min_vertex + 1] : static_cast<int64_t>(e.size());
				for (int64_t j = start_edge; j < end_edge; j++) {
					int64_t neighbor = e[j];
					if (!removed[neighbor] && degree[neighbor] > 0) {
						degree[neighbor]--;
					}
				}
			}
			info.state_initialized = true;
		}
	}

	// Get the source vector for the current DataChunk
	auto &src = args.data[1];
	UnifiedVectorFormat vdata_src;
	src.ToUnifiedFormat(args.size(), vdata_src);
	auto src_data = reinterpret_cast<int64_t *>(vdata_src.data);

	// Create result vector
	ValidityMask &result_validity = FlatVector::Validity(result);
	result.SetVectorType(VectorType::FLAT_VECTOR);
	auto result_data = FlatVector::GetData<int64_t>(result);

	// Output the core number corresponding to each source ID in the DataChunk
	for (idx_t i = 0; i < args.size(); i++) {
		auto id_pos = vdata_src.sel->get_index(i);
		if (!vdata_src.validity.RowIsValid(id_pos)) {
			result_validity.SetInvalid(i);
			continue;
		}
		auto node_id = src_data[id_pos];
		if (node_id < 0 || static_cast<size_t>(node_id) >= v_size) {
			result_validity.SetInvalid(i);
			continue;
		}
		result_data[i] = info.core[node_id];
	}

	duckpgq_state->csr_to_delete.insert(info.csr_id);
}

//------------------------------------------------------------------------------
// Register functions
//------------------------------------------------------------------------------
void CoreScalarFunctions::RegisterKCoreDecompositionScalarFunction(ExtensionLoader &loader) {
	loader.RegisterFunction(ScalarFunction("k_core_decomposition", {LogicalType::INTEGER, LogicalType::BIGINT},
	                                       LogicalType::BIGINT, KCoreDecompositionFunction,
	                                       KCoreDecompositionFunctionData::KCoreDecompositionBind));
}

} // namespace duckdb
