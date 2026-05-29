# DuckPGQ Algorithm Test Plan & Profiling Results

This complements [ADR-001](../docs/adr/001-property-graph-algorithm-library.md).

## Build / environment

- DuckDB pairing: `duckdb` submodule pinned to duckdb-pgq `main` @ `7970b59`
  (the repo's recorded submodule SHA was stale; this is the commit the current
  extension source actually compiles and runs against).
- Toolchain: gcc 11.5, cmake/ninja (user-installed), system OpenSSL 3.5.1.
- Host: 32 cores, 62 GiB RAM, Linux.
- Build: `make` (release). Run tests: `build/release/test/unittest <path>`.

## Test plan / coverage

End-to-end sqllogictest, one file per algorithm under `test/sql/scalar/`, each
asserting hand-derived values on a small property graph (the SNB-style 5-vertex
"Student/know" graph used by the existing `pagerank.test`).

| Algorithm | Test | Status |
|---|---|---|
| Degree centrality | `test/sql/scalar/degree_centrality.test` | PASS |
| Triangle count | `test/sql/scalar/triangle_count.test` | PASS |
| Closeness centrality | `test/sql/scalar/closeness_centrality.test` | PASS |
| Betweenness centrality (Brandes) | `test/sql/scalar/betweenness_centrality.test` | PASS |
| Strongly connected components (Tarjan) | `test/sql/scalar/strongly_connected_component.test` | PASS |
| Label propagation | `test/sql/scalar/label_propagation.test` | PASS |
| Eigenvector centrality | `test/sql/scalar/eigenvector_centrality.test` | PASS |

Full regression: **all 65 pre-existing sqllogictests + 7 new algorithm tests pass**
(72 total), no regressions.

## Perf / memory profile

Run with `benchmark/run_benchmark.sh` (generates a clean ring-lattice graph,
runs each algorithm via the sqllogictest harness under `/usr/bin/time -v`).
`wall` and `peak-RSS` include harness startup + graph generation + CSR build;
subtract the matching `baseline_graphgen` row to isolate algorithm cost.

Near-linear algorithms @ 50,000 vertices / ~250,000 edges:

| Benchmark | Wall | Peak RSS |
|---|---|---|
| baseline_graphgen | 0.04 s | 37 MB |
| degree_centrality | 0.25 s | 281 MB |
| triangle_count | 0.30 s | 290 MB |
| pagerank | 0.08 s | 60 MB |
| weakly_connected_component | 0.25 s | 326 MB |
| strongly_connected_component | 0.08 s | 59 MB |
| label_propagation | 4.14 s | 278 MB |
| eigenvector_centrality | 0.26 s | 287 MB |
| local_clustering_coefficient | 0.29 s | 324 MB |

Quadratic (per-source BFS) algorithms @ 5,000 vertices / ~25,000 edges:

| Benchmark | Wall | Peak RSS |
|---|---|---|
| baseline_graphgen | 0.02 s | 29 MB |
| closeness_centrality | 0.70 s | 83 MB |
| betweenness_centrality | 2.01 s | 78 MB |

### Observations
- No memory blow-ups: peak RSS stays < 330 MB across all algorithms.
- `label_propagation` is the slowest near-linear algorithm (4.1 s): synchronous
  majority-vote iterates many rounds before converging on a ring lattice.
- `closeness`/`betweenness` are O(V·(V+E)); kept on a smaller graph by design
  (Phase-1 avoids all-pairs materialization).
- The undirected CSR builder requires every edge to reference an existing,
  unique vertex (csr_creation.cpp:121); benchmark graphs must avoid
  duplicate/self-loop edges (the ring-lattice generator satisfies this).

## Known limitations
- DuckPGQ property-graph table functions must be driven through the
  sqllogictest harness; the bare `duckdb` CLI crashes on the PGQ table-function
  rewrite path (pre-existing, affects shipped functions too).
