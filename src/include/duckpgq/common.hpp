#pragma once

#include "duckdb.hpp"
#include "duckdb/common/helper.hpp"
// DuckDB 1.2.2 has no ExtensionLoader; this shim provides the subset we use.
#include "duckpgq/core/compat_extension_loader.hpp"
// TODO doc util
