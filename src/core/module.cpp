
#include "duckpgq/core/module.hpp"
#include "duckpgq/common.hpp"
#include "duckpgq/core/functions/scalar.hpp"
#include "duckpgq/core/functions/table.hpp"
#include "duckpgq/core/pragma/duckpgq_pragma.hpp"

namespace duckdb {

// Stock-DuckDB graph extension: only the graph-algorithm + path-finding
// functions are registered. The SQL/PGQ grammar front-end (parser + operator
// extensions; CREATE/MATCH/GRAPH_TABLE) is intentionally dropped so this builds
// against unmodified mainline DuckDB and the public extension ecosystem stays
// usable. Graph patterns are lowered to SQL by the DFL compiler; algorithms run
// as table functions over edge/vertex tables directly.
void CoreModule::Register(ExtensionLoader &loader) {
	CoreTableFunctions::Register(loader);
	CoreScalarFunctions::Register(loader);
	CorePGQPragma::Register(loader);
}

} // namespace duckdb
