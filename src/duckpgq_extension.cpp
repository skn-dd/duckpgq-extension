#define DUCKDB_EXTENSION_MAIN

#include "duckpgq_extension.hpp"
#include "duckpgq/common.hpp"
#include "duckpgq/core/module.hpp"
#include "duckdb/main/connection_manager.hpp"

namespace duckdb {

static void LoadInternal(ExtensionLoader &loader) {
	CoreModule::Register(loader);
}

void DuckpgqExtension::Load(DuckDB &db) {
	ExtensionLoader loader(*db.instance);
	LoadInternal(loader);
}

std::string DuckpgqExtension::Name() {
	return "duckpgq";
}

} // namespace duckdb

extern "C" {

DUCKDB_EXTENSION_API void duckpgq_init(duckdb::DatabaseInstance &db) {
	duckdb::DuckDB db_wrapper(db);
	db_wrapper.LoadExtension<duckdb::DuckpgqExtension>();
}

DUCKDB_EXTENSION_API const char *duckpgq_version() {
	return duckdb::DuckDB::LibraryVersion();
}
}
