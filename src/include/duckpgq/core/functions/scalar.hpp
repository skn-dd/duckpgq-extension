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
		RegisterHarmonicCentralityScalarFunction(loader);
		RegisterKCoreDecompositionScalarFunction(loader);
		RegisterEccentricityScalarFunction(loader);
		RegisterArticleRankScalarFunction(loader);
		RegisterKatzCentralityScalarFunction(loader);
		RegisterHitsAuthorityScalarFunction(loader);
		RegisterHitsHubScalarFunction(loader);
		RegisterGlobalClusteringCoefficientScalarFunction(loader);
		RegisterTopologicalSortScalarFunction(loader);
		RegisterJaccardSimilarityScalarFunction(loader);
		RegisterCosineSimilarityScalarFunction(loader);
		RegisterOverlapSimilarityScalarFunction(loader);
		RegisterCommonNeighborsScalarFunction(loader);
		RegisterPreferentialAttachmentScalarFunction(loader);
		RegisterAdamicAdarScalarFunction(loader);
		RegisterResourceAllocationScalarFunction(loader);
		RegisterSingleSourceShortestPathScalarFunction(loader);
		RegisterPersonalizedPagerankScalarFunction(loader);
		RegisterOutDegreeCentralityScalarFunction(loader);
		RegisterInDegreeCentralityScalarFunction(loader);
		RegisterLouvainScalarFunction(loader);
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
	static void RegisterHarmonicCentralityScalarFunction(ExtensionLoader &loader);
	static void RegisterKCoreDecompositionScalarFunction(ExtensionLoader &loader);
	static void RegisterEccentricityScalarFunction(ExtensionLoader &loader);
	static void RegisterArticleRankScalarFunction(ExtensionLoader &loader);
	static void RegisterKatzCentralityScalarFunction(ExtensionLoader &loader);
	static void RegisterHitsAuthorityScalarFunction(ExtensionLoader &loader);
	static void RegisterHitsHubScalarFunction(ExtensionLoader &loader);
	static void RegisterGlobalClusteringCoefficientScalarFunction(ExtensionLoader &loader);
	static void RegisterTopologicalSortScalarFunction(ExtensionLoader &loader);
	static void RegisterJaccardSimilarityScalarFunction(ExtensionLoader &loader);
	static void RegisterCosineSimilarityScalarFunction(ExtensionLoader &loader);
	static void RegisterOverlapSimilarityScalarFunction(ExtensionLoader &loader);
	static void RegisterCommonNeighborsScalarFunction(ExtensionLoader &loader);
	static void RegisterPreferentialAttachmentScalarFunction(ExtensionLoader &loader);
	static void RegisterAdamicAdarScalarFunction(ExtensionLoader &loader);
	static void RegisterResourceAllocationScalarFunction(ExtensionLoader &loader);
	static void RegisterSingleSourceShortestPathScalarFunction(ExtensionLoader &loader);
	static void RegisterPersonalizedPagerankScalarFunction(ExtensionLoader &loader);
	static void RegisterOutDegreeCentralityScalarFunction(ExtensionLoader &loader);
	static void RegisterInDegreeCentralityScalarFunction(ExtensionLoader &loader);
	static void RegisterLouvainScalarFunction(ExtensionLoader &loader);
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
