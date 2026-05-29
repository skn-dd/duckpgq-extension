# DuckPGQ Algorithm Test Plan & Profiling Results

Complements [ADR-001](../docs/adr/001-property-graph-algorithm-library.md).

## Build / environment

- DuckDB pairing: `duckdb` submodule on duckdb-pgq `main` @ `7970b59` (the repo's
  recorded submodule SHA is stale; this is the commit the extension compiles/runs against).
- Toolchain: gcc 11.5, cmake/ninja (user-installed), system OpenSSL 3.5.1.
- Host: 32 cores, 62 GiB RAM, Linux. Build: `make` (release). Tests: `build/release/test/unittest <path>`.

## Algorithm coverage (28 new functions)

All exposed as scalar + table functions over the in-memory CSR, each with a
sqllogictest using hand-derived oracles on a small property graph. Three
bind-replace SELECT patterns back them: per-vertex `CreateSelectNode`,
per-vertex-with-parameter `CreateParamSelectNode`, and pairwise
`CreatePairwiseSelectNode` (utils).

| Family | Algorithms |
|---|---|
| Centrality | degree, in-degree, out-degree, closeness, harmonic, betweenness (Brandes), eigenvector, Katz, ArticleRank, HITS (hub + authority), personalized PageRank |
| Community | strongly connected components, label propagation, Louvain, k-core decomposition, triangle count, global clustering coefficient |
| Similarity (pairwise) | Jaccard, cosine, overlap, common-neighbors, Adamic-Adar, preferential attachment, resource allocation |
| Pathfinding / ordering | single-source shortest path (BFS), topological sort (Kahn) |

Pairwise similarity emits O(V²) node pairs (a<b) by design — intended for
small/medium graphs or filtered use. Parameterized functions take the source
vertex as a 4th table-function argument, e.g. `single_source_shortest_path(pg, v, e, 0)`.

Genuinely deferred (need substantially more than a new algorithm file): A*
(needs a spatial heuristic / coordinates), Yen K-shortest and MST (edge-list
output, not per-vertex), Leiden (multi-level refinement), and node embeddings
FastRP / Node2Vec / DeepWalk (vector outputs + RNG / skip-gram training, not
deterministically testable). These remain future work per ADR-001.

## Memory limits

`memory_limit` is honored end-to-end: DuckDB's buffer manager enforces it during
CSR construction and errors gracefully (verified: `SET memory_limit='64MB'` on a
2M-vertex build yields a clean `Out of Memory Error`, no crash). Additionally,
`CheckAlgorithmMemoryBudget` (utils) pre-checks each algorithm's non-spillable
working-vector allocation against the configured limit and throws
`OutOfMemoryException` rather than risking an OS OOM.

## Multi-scale profile (ring-lattice graph, degree 5, `run_scale_benchmark.sh`)

Near-linear algorithms — wall-clock (peak-RSS) by vertex count:

| Algorithm | 100k | 500k | 1M | 5M (25M edges) |
|---|---|---|---|---|
| pagerank | 0.08s | 0.24s | 0.42s | 1.76s (1.3GB) |
| strongly_connected_component | 0.08s | 0.23s | 0.41s | 1.72s (1.3GB) |
| article_rank | 0.09s | 0.24s | 0.43s | 1.73s (1.3GB) |
| hits_authority / hits_hub | 0.08s | 0.24s | 0.42s | 1.73s (1.3GB) |
| degree_centrality | 0.23s | 0.81s | 1.72s | 5.57s (6.6GB) |
| weakly_connected_component | 0.25s | 0.95s | 1.68s | 6.14s (6.8GB) |
| eigenvector_centrality | 0.25s | 0.83s | 1.83s | 5.85s (6.7GB) |
| k_core_decomposition | 0.25s | 0.92s | 1.83s | 5.85s (6.7GB) |
| triangle_count | 0.28s | 0.97s | 2.04s | 9.20s (6.6GB) |
| local_clustering_coefficient | 0.26s | 0.94s | 1.81s | 9.09s (6.7GB) |
| katz_centrality | 0.36s | 1.44s | 2.86s | 12.98s (6.6GB) |
| topological_sort | — | — | 0.43s | 1.79s (1.3GB) |
| label_propagation (cap 20 iters) | 2.6s | 13.9s | 6.9s | 33.7s (6.6GB) |
| global_clustering_coefficient | 0.32s | 1.28s | 2.94s | 52.5s (6.6GB) |

Superlinear per-source-BFS algorithms (O(V·(V+E)); run at 3k vertices): closeness
0.31s, betweenness 0.73s, harmonic 0.31s, eccentricity 0.29s — all <1s, ~74MB.

Note: wall/peak-RSS include the sqllogictest harness + graph generation + CSR
build; the directed-CSR algorithms (pagerank/SCC/article_rank/HITS) keep a single
CSR copy (~1.3GB at 5M) while the undirected-CSR algorithms materialize both
edge directions (~6.6GB at 5M).

## Bottlenecks found and fixed (via the multi-scale run)

1. **k_core_decomposition — O(V²) → TIMEOUT at 500k.** Replaced the naive
   "scan for min-degree vertex each step" with the linear-time Batagelj-Zaversnik
   bucket algorithm. Now 1.8s @1M, 5.9s @5M.
2. **eigenvector_centrality & katz_centrality — SEGFAULT at ≥500k.** Root cause:
   state vectors were `resize()`d *outside* the `state_lock`; DuckDB parallelizes
   the scan and shares bind data across threads, so the unlocked resize raced and
   corrupted the heap (only triggered once the graph was large enough to
   parallelize). Fixed by moving initialization inside the lock. The same latent
   race was hardened proactively in **pagerank** and **article_rank**.
3. **eigenvector_centrality — out-of-bounds.** Its accumulation loop iterated all
   `vsize` vertices (incl. the 2 CSR padding slots) and read their undefined edge
   ranges; restricted to real vertices (`u + 2 < v_size`). (label_propagation had
   the same latent issue, also fixed.)
4. **label_propagation — TIMEOUT at 5M.** Reused the per-vertex frequency map
   across iterations (avoids millions of allocations) and capped iterations at 20
   (heuristic; cf. Neo4j GDS default ~10). Now 33.7s @5M (was >150s).

Known characteristic (not a defect): `global_clustering_coefficient` computes the
whole-graph transitivity single-threaded under its compute-once lock, so it is the
slowest at 5M (52s); per-vertex algorithms that run inside DuckDB's parallel scan
are faster.

## Regression status
All pre-existing sqllogictests plus the 28 new algorithm tests pass; no regressions.

## Cross-system comparison vs Oracle AI Database 26ai (23ai ADB)

Live comparison against an Oracle Autonomous Database (`23.26.2.2.0`, "high"
service). Identical ring-lattice graphs (N vertices, ~5N directed edges) were
generated on both systems. `benchmark/oracle_compare.py` (credentials via env
vars) drives the Oracle side using its `GRAPH_TABLE` graph engine on a SQL
property graph.

**Degree centrality** — wall-clock seconds:

| Scale | DuckPGQ (total: gen+CSR+compute) | Oracle build+PG | Oracle degree (GRAPH_TABLE) | Oracle end-to-end |
|---|---|---|---|---|
| 100k / 0.5M edges | 0.23 | 1.65 | 0.25 | ~1.9 |
| 500k / 2.5M edges | 0.89 | 3.75 | 0.83 | ~4.6 |
| 1M / 5M edges | 1.71 | 6.70 | 1.62 | ~8.3 |
| 5M / 25M edges | 5.44 | 31.12 | 10.73 | ~41.8 |

Oracle 2-hop pattern count (`(a)->(b)->(c)`, its graph engine): 0.51 / 2.92 /
4.76 / 31.30 s at 100k/500k/1M/5M.

**Read this comparison with caveats** — it is indicative, not controlled:
- Different architectures/goals: DuckPGQ is in-process, in-memory, ephemeral CSR
  on a 32-core box; Oracle ADB is a durable, transactional, shared autonomous
  instance (unknown OCPU allocation) accessed over the network.
- Semantics differ slightly: DuckPGQ degree is undirected; the Oracle query is
  directed out-degree. DuckPGQ's number includes graph generation + CSR build;
  Oracle's build and query are timed separately.
- Takeaway: the engines are comparable up to ~1M; DuckPGQ's in-memory CSR pulls
  ahead end-to-end at 5M (≈5s vs ≈42s incl. Oracle's durable build).

**Algorithm-library head-to-head (PageRank / WCC / Bellman-Ford):** Oracle 23ai
*does* provide these in-database via `DBMS_OGA` (invoked through the Graph
Analytics Framework `GRAPH_TABLE` syntax). Finalizing the exact GAF
output-property grammar (`ORA_VERTEX_OUTPUT_PROPERTY`) over a blind SQL
connection was not completed this session (ORA-40975); completing it is the
next step to add a direct PageRank/WCC time comparison. There is a pre-built
`BENCH_GRAPH` (10M vertices / 100M edges) on the instance available for that.

