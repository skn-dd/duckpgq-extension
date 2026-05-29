#pragma once
#include "duckpgq/common.hpp"

namespace duckdb {

struct CoreTableFunctions {
	static void Register(ExtensionLoader &loader) {
		RegisterCreatePropertyGraphTableFunction(loader);
		RegisterMatchTableFunction(loader);
		RegisterDropPropertyGraphTableFunction(loader);
		RegisterDescribePropertyGraphTableFunction(loader);
		RegisterDegreeCentralityTableFunction(loader);
		RegisterTriangleCountTableFunction(loader);
		RegisterClosenessCentralityTableFunction(loader);
		RegisterBetweennessCentralityTableFunction(loader);
		RegisterStronglyConnectedComponentTableFunction(loader);
		RegisterLabelPropagationTableFunction(loader);
		RegisterEigenvectorCentralityTableFunction(loader);
		RegisterLocalClusteringCoefficientTableFunction(loader);
		RegisterScanTableFunctions(loader);
		RegisterSummarizePropertyGraphTableFunction(loader);
		RegisterWeaklyConnectedComponentTableFunction(loader);
		RegisterPageRankTableFunction(loader);
	}

private:
	static void RegisterCreatePropertyGraphTableFunction(ExtensionLoader &loader);
	static void RegisterMatchTableFunction(ExtensionLoader &loader);
	static void RegisterDropPropertyGraphTableFunction(ExtensionLoader &loader);
	static void RegisterDescribePropertyGraphTableFunction(ExtensionLoader &loader);
	static void RegisterDegreeCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterTriangleCountTableFunction(ExtensionLoader &loader);
	static void RegisterClosenessCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterBetweennessCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterStronglyConnectedComponentTableFunction(ExtensionLoader &loader);
	static void RegisterLabelPropagationTableFunction(ExtensionLoader &loader);
	static void RegisterEigenvectorCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterLocalClusteringCoefficientTableFunction(ExtensionLoader &loader);
	static void RegisterScanTableFunctions(ExtensionLoader &loader);
	static void RegisterWeaklyConnectedComponentTableFunction(ExtensionLoader &loader);
	static void RegisterPageRankTableFunction(ExtensionLoader &loader);
	static void RegisterSummarizePropertyGraphTableFunction(ExtensionLoader &loader);
};

} // namespace duckdb
