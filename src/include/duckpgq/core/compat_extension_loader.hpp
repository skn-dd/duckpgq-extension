//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/compat_extension_loader.hpp
//
// Compatibility shim for DuckDB 1.2.2, which predates the `ExtensionLoader`
// registration API (introduced ~1.4). The extension only uses
// `RegisterFunction(...)` and `GetDatabaseInstance()`, so we emulate that subset
// by forwarding to `ExtensionUtil`. This lets the rest of the extension code
// (written against the newer `ExtensionLoader &`) compile unchanged.
//===----------------------------------------------------------------------===//

#pragma once

#include "duckdb.hpp"
#include "duckdb/main/extension_util.hpp"

namespace duckdb {

class ExtensionLoader {
public:
	explicit ExtensionLoader(DatabaseInstance &db_p) : db(db_p) {
	}

	DatabaseInstance &GetDatabaseInstance() {
		return db;
	}

	void RegisterFunction(ScalarFunction function) {
		ExtensionUtil::RegisterFunction(db, std::move(function));
	}
	void RegisterFunction(ScalarFunctionSet function) {
		ExtensionUtil::RegisterFunction(db, std::move(function));
	}
	void RegisterFunction(TableFunction function) {
		ExtensionUtil::RegisterFunction(db, std::move(function));
	}
	void RegisterFunction(TableFunctionSet function) {
		ExtensionUtil::RegisterFunction(db, std::move(function));
	}
	void RegisterFunction(PragmaFunction function) {
		ExtensionUtil::RegisterFunction(db, std::move(function));
	}
	void RegisterFunction(PragmaFunctionSet function) {
		ExtensionUtil::RegisterFunction(db, std::move(function));
	}

private:
	DatabaseInstance &db;
};

} // namespace duckdb
