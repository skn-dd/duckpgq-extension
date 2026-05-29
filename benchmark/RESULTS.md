# DuckPGQ Algorithm Test Plan & Profiling Results

Complements [ADR-001](../docs/adr/001-property-graph-algorithm-library.md).

## Build / environment

- DuckDB pairing: `duckdb` submodule on duckdb-pgq `main` @ `7970b59` (the repo's
  recorded submodule SHA is stale; this is the commit the extension compiles/runs against).
- Toolchain: gcc 11.5, cmake/ninja (user-installed), system OpenSSL 3.5.1.
- Host: 32 cores, 62 GiB RAM, Linux. Build: `make` (release). Tests: `build/release/test/unittest <path>`.

## Algorithm coverage (16 functions)

All exposed as scalar + table functions over the in-memory CSR, each with a
sqllogictest using hand-derived oracles on a small property graph.

| Family | Algorithms |
|---|---|
| Centrality | degree, closeness, betweenness (Brandes), harmonic, eigenvector, Katz, ArticleRank, HITS (hub + authority) |
| Community | weakly/strongly connected components, label propagation, k-core decomposition, triangle count, global clustering coefficient, local clustering coefficient |
| Ordering | topological sort (Kahn) |

Deferred (require new table-function infrastructure — pair output / parameters /
embeddings, flagged as future work in ADR-001): pairwise similarity (Jaccard,
cosine, overlap, Adamic-Adar, common-neighbors, preferential attachment,
resource allocation), parameterized pathfinding (personalized PageRank, Dijkstra
SSSP, A*, Yen K-shortest, MST), and node embeddings (FastRP, Node2Vec/DeepWalk).

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
All pre-existing sqllogictests plus the 16 new algorithm tests pass; no regressions.
