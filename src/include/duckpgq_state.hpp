#pragma once

#include "duckpgq/common.hpp"
#include "duckpgq/core/utils/compressed_sparse_row.hpp"

#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace duckdb {

// Stock-DuckDB build: DuckPGQState carries only the CSR cache used by the graph
// algorithms / path-finding functions. The property-graph registry and parser-
// extension state were dropped together with the SQL/PGQ grammar front-end —
// graphs are described by table-function arguments, not registered objects.
class DuckPGQState : public ClientContextState {
public:
	explicit DuckPGQState() {};

	void QueryEnd() override;
	CSR *GetCSR(int32_t id);

	//! CSR data structures built for graph-algorithm / path-finding queries
	std::unordered_map<int32_t, unique_ptr<CSR>> csr_list;
	std::mutex csr_lock;
	std::unordered_set<int32_t> csr_to_delete;
};

} // namespace duckdb
