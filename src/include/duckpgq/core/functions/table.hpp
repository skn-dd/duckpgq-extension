#pragma once
#include "duckpgq/common.hpp"

namespace duckdb {

struct CoreTableFunctions {
	static void Register(ExtensionLoader &loader) {
		// Vertical slice: only pagerank is decoupled to the MakeEdgeSpec(args)
		// pattern so far. The other algorithm + path-finding registrations are
		// re-enabled one-by-one as each bind is decoupled (the parallel fan-out).
		// Grammar table functions (CREATE/MATCH/DROP/DESCRIBE/scan/summarize) are
		// excluded entirely — this targets stock DuckDB.
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
	static void RegisterScanTableFunctions(ExtensionLoader &loader);
	static void RegisterWeaklyConnectedComponentTableFunction(ExtensionLoader &loader);
	static void RegisterPageRankTableFunction(ExtensionLoader &loader);
	static void RegisterSummarizePropertyGraphTableFunction(ExtensionLoader &loader);
};

} // namespace duckdb
