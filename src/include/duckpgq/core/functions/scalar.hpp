#pragma once
#include "duckpgq/common.hpp"

namespace duckdb {

struct CoreScalarFunctions {
	static void Register(ExtensionLoader &loader) {
		RegisterCheapestPathLengthScalarFunction(loader);
		RegisterCSRCreationScalarFunctions(loader);
		RegisterDegreeCentralityScalarFunction(loader);
		RegisterTriangleCountScalarFunction(loader);
		RegisterClosenessCentralityScalarFunction(loader);
		RegisterBetweennessCentralityScalarFunction(loader);
		RegisterStronglyConnectedComponentScalarFunction(loader);
		RegisterLabelPropagationScalarFunction(loader);
		RegisterEigenvectorCentralityScalarFunction(loader);
		RegisterCSRDeletionScalarFunction(loader);
		RegisterGetCSRWTypeScalarFunction(loader);
		RegisterIterativeLengthScalarFunction(loader);
		RegisterIterativeLength2ScalarFunction(loader);
		RegisterIterativeLengthBidirectionalScalarFunction(loader);
		RegisterLocalClusteringCoefficientScalarFunction(loader);
		RegisterReachabilityScalarFunction(loader);
		RegisterShortestPathScalarFunction(loader);
		RegisterWeaklyConnectedComponentScalarFunction(loader);
		RegisterPageRankScalarFunction(loader);
	}

private:
	static void RegisterCheapestPathLengthScalarFunction(ExtensionLoader &loader);
	static void RegisterCSRCreationScalarFunctions(ExtensionLoader &loader);
	static void RegisterDegreeCentralityScalarFunction(ExtensionLoader &loader);
	static void RegisterTriangleCountScalarFunction(ExtensionLoader &loader);
	static void RegisterClosenessCentralityScalarFunction(ExtensionLoader &loader);
	static void RegisterBetweennessCentralityScalarFunction(ExtensionLoader &loader);
	static void RegisterStronglyConnectedComponentScalarFunction(ExtensionLoader &loader);
	static void RegisterLabelPropagationScalarFunction(ExtensionLoader &loader);
	static void RegisterEigenvectorCentralityScalarFunction(ExtensionLoader &loader);
	static void RegisterCSRDeletionScalarFunction(ExtensionLoader &loader);
	static void RegisterGetCSRWTypeScalarFunction(ExtensionLoader &loader);
	static void RegisterIterativeLengthScalarFunction(ExtensionLoader &loader);
	static void RegisterIterativeLength2ScalarFunction(ExtensionLoader &loader);
	static void RegisterIterativeLengthBidirectionalScalarFunction(ExtensionLoader &loader);
	static void RegisterLocalClusteringCoefficientScalarFunction(ExtensionLoader &loader);
	static void RegisterReachabilityScalarFunction(ExtensionLoader &loader);
	static void RegisterShortestPathScalarFunction(ExtensionLoader &loader);
	static void RegisterWeaklyConnectedComponentScalarFunction(ExtensionLoader &loader);
	static void RegisterPageRankScalarFunction(ExtensionLoader &loader);
};

} // namespace duckdb
