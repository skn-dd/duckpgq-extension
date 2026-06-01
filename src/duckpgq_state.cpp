#include "duckpgq_state.hpp"

namespace duckdb {

void DuckPGQState::QueryEnd() {
	std::lock_guard<std::mutex> guard(csr_lock);
	for (const auto &csr_id : csr_to_delete) {
		csr_list.erase(csr_id);
	}
	csr_to_delete.clear();
}

CSR *DuckPGQState::GetCSR(int32_t id) {
	auto csr_entry = csr_list.find(id);
	if (csr_entry == csr_list.end()) {
		throw ConstraintException("CSR not found with ID %d", id);
	}
	return csr_entry->second.get();
}

} // namespace duckdb
