#pragma once
#include "duckpgq/common.hpp"

namespace duckdb {

struct CoreTableFunctions {
	static void Register(ExtensionLoader &loader) {
		// All algorithm + path-finding table functions, decoupled to take
		// edge/vertex tables directly (MakeEdgeSpec). Grammar table functions
		// (CREATE/MATCH/DROP/DESCRIBE/scan/summarize) are excluded — stock DuckDB.
		RegisterDegreeCentralityTableFunction(loader);
		RegisterTriangleCountTableFunction(loader);
		RegisterClosenessCentralityTableFunction(loader);
		RegisterBetweennessCentralityTableFunction(loader);
		RegisterStronglyConnectedComponentTableFunction(loader);
		RegisterLabelPropagationTableFunction(loader);
		RegisterEigenvectorCentralityTableFunction(loader);
		RegisterHarmonicCentralityTableFunction(loader);
		RegisterKCoreDecompositionTableFunction(loader);
		RegisterEccentricityTableFunction(loader);
		RegisterArticleRankTableFunction(loader);
		RegisterKatzCentralityTableFunction(loader);
		RegisterHitsAuthorityTableFunction(loader);
		RegisterHitsHubTableFunction(loader);
		RegisterGlobalClusteringCoefficientTableFunction(loader);
		RegisterTopologicalSortTableFunction(loader);
		RegisterJaccardSimilarityTableFunction(loader);
		RegisterCosineSimilarityTableFunction(loader);
		RegisterOverlapSimilarityTableFunction(loader);
		RegisterCommonNeighborsTableFunction(loader);
		RegisterPreferentialAttachmentTableFunction(loader);
		RegisterAdamicAdarTableFunction(loader);
		RegisterResourceAllocationTableFunction(loader);
		RegisterSingleSourceShortestPathTableFunction(loader);
		RegisterPersonalizedPagerankTableFunction(loader);
		RegisterOutDegreeCentralityTableFunction(loader);
		RegisterInDegreeCentralityTableFunction(loader);
		RegisterLouvainTableFunction(loader);
		RegisterLocalClusteringCoefficientTableFunction(loader);
		RegisterWeaklyConnectedComponentTableFunction(loader);
		RegisterPageRankTableFunction(loader);
		RegisterGraphMatchTableFunction(loader);
	}

private:
	static void RegisterDegreeCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterTriangleCountTableFunction(ExtensionLoader &loader);
	static void RegisterClosenessCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterBetweennessCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterStronglyConnectedComponentTableFunction(ExtensionLoader &loader);
	static void RegisterLabelPropagationTableFunction(ExtensionLoader &loader);
	static void RegisterEigenvectorCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterHarmonicCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterKCoreDecompositionTableFunction(ExtensionLoader &loader);
	static void RegisterEccentricityTableFunction(ExtensionLoader &loader);
	static void RegisterArticleRankTableFunction(ExtensionLoader &loader);
	static void RegisterKatzCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterHitsAuthorityTableFunction(ExtensionLoader &loader);
	static void RegisterHitsHubTableFunction(ExtensionLoader &loader);
	static void RegisterGlobalClusteringCoefficientTableFunction(ExtensionLoader &loader);
	static void RegisterTopologicalSortTableFunction(ExtensionLoader &loader);
	static void RegisterJaccardSimilarityTableFunction(ExtensionLoader &loader);
	static void RegisterCosineSimilarityTableFunction(ExtensionLoader &loader);
	static void RegisterOverlapSimilarityTableFunction(ExtensionLoader &loader);
	static void RegisterCommonNeighborsTableFunction(ExtensionLoader &loader);
	static void RegisterPreferentialAttachmentTableFunction(ExtensionLoader &loader);
	static void RegisterAdamicAdarTableFunction(ExtensionLoader &loader);
	static void RegisterResourceAllocationTableFunction(ExtensionLoader &loader);
	static void RegisterSingleSourceShortestPathTableFunction(ExtensionLoader &loader);
	static void RegisterPersonalizedPagerankTableFunction(ExtensionLoader &loader);
	static void RegisterOutDegreeCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterInDegreeCentralityTableFunction(ExtensionLoader &loader);
	static void RegisterLouvainTableFunction(ExtensionLoader &loader);
	static void RegisterLocalClusteringCoefficientTableFunction(ExtensionLoader &loader);
	static void RegisterWeaklyConnectedComponentTableFunction(ExtensionLoader &loader);
	static void RegisterPageRankTableFunction(ExtensionLoader &loader);
	static void RegisterGraphMatchTableFunction(ExtensionLoader &loader);
};

} // namespace duckdb
