#pragma once
#include "duckpgq_state.hpp"
#include "duckdb/parser/expression/columnref_expression.hpp"
#include "duckdb/parser/property_graph_table.hpp"

namespace duckdb {

#define LANE_LIMIT         512
#define VISIT_SIZE_DIVISOR 2

// Function to get DuckPGQState from ClientContext
shared_ptr<DuckPGQState> GetDuckPGQState(ClientContext &context, bool throw_error_not_found = false);

// Honors DuckDB's configured memory_limit: throws OutOfMemoryException with a
// clear, actionable message when an algorithm's estimated working-set
// allocation would exceed the buffer manager's max memory. Algorithms call this
// before allocating their O(V) / O(V^2) state so that large graphs fail
// gracefully instead of triggering the OOM killer.
void CheckAlgorithmMemoryBudget(ClientContext &context, idx_t estimated_bytes, const string &algorithm_name);
// Build an edge-spec from table-function arguments (no property-graph registry):
// source/destination vertices are the same vertex_table; the CSR builders
// disambiguate the self-join via "src"/"dst" bindings.
shared_ptr<PropertyGraphTable> MakeEdgeSpec(const string &vertex_table, const string &vertex_id,
                                            const string &edge_table, const string &src_col, const string &dst_col);
unique_ptr<SelectNode> CreateSelectNode(const shared_ptr<PropertyGraphTable> &edge_pg_entry,
                                        const string &function_name, const string &function_alias);
// Per-vertex SELECT that threads an extra constant scalar argument (e.g. a source
// vertex id) into the algorithm call: add(__x.temp, fn(0, src.rowid, extra_arg)).
unique_ptr<SelectNode> CreateParamSelectNode(const shared_ptr<PropertyGraphTable> &edge_pg_entry,
                                             const string &function_name, const string &function_alias,
                                             const Value &extra_arg);
// Pairwise SELECT over the source vertex table cross-joined with itself (aliases
// "a"/"b", a.rowid < b.rowid): emits node1, node2, fn(0, a.rowid, b.rowid).
unique_ptr<SelectNode> CreatePairwiseSelectNode(const shared_ptr<PropertyGraphTable> &edge_pg_entry,
                                                const string &function_name, const string &function_alias);
unique_ptr<BaseTableRef> CreateBaseTableRef(const string &table_name, const string &alias = "");
unique_ptr<ColumnRefExpression> CreateColumnRefExpression(const string &column_name, const string &table_name = "",
                                                          const string &alias = "");

} // namespace duckdb
