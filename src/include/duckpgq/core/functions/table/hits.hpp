//===----------------------------------------------------------------------===//
//                         DuckPGQ
//
// duckpgq/core/functions/table/hits.hpp
//
//
//===----------------------------------------------------------------------===//

#include "duckpgq/common.hpp"

namespace duckdb {

class HitsAuthorityFunction : public TableFunction {
public:
	HitsAuthorityFunction() {
		name = "hits_authority";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR};
		bind_replace = HitsAuthorityBindReplace;
	}

	static unique_ptr<TableRef> HitsAuthorityBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

class HitsHubFunction : public TableFunction {
public:
	HitsHubFunction() {
		name = "hits_hub";
		arguments = {LogicalType::VARCHAR, LogicalType::VARCHAR, LogicalType::VARCHAR};
		bind_replace = HitsHubBindReplace;
	}

	static unique_ptr<TableRef> HitsHubBindReplace(ClientContext &context, TableFunctionBindInput &input);
};

} // namespace duckdb
