# ADR-001: Architecture for a Comprehensive Property-Graph Algorithm Library

## Status
Accepted

## Context

DuckPGQ today ships a handful of graph algorithms (shortest path via bitset BFS,
bidirectional BFS, cheapest/weighted path, reachability, weakly connected
components, local clustering coefficient, PageRank). They are exposed two ways:

1. **Scalar functions** (e.g. `pagerank(csr_id, src)`) that operate on a
   Compressed Sparse Row (CSR) graph held in `DuckPGQState::csr_list`, compute
   the whole-graph result once (guarded by a `state_initialized` flag + mutex in
   the bind data), cache it, then emit one value per source row of the calling
   `DataChunk`.
2. **Table functions** (e.g. `pagerank(pg, vertex, edge)`) implemented as
   `bind_replace` rewrites that synthesize a SQL subquery: a `csr_cte` builds the
   CSR from the property graph's vertex/edge tables, then the scalar algorithm is
   invoked per vertex `rowid`.

We want to grow this into a broad, SOTA algorithm library (centrality, community
detection, traversal, similarity, path finding) comparable to Neo4j GDS,
NetworkX, and igraph. Getting the *extension shape* wrong is expensive: every
algorithm shares the CSR build path, the registration scaffolding, and the test
harness, so an inconsistent approach would multiply maintenance cost across
dozens of functions and force painful migrations later.

## Decision Drivers

- **Consistency at scale**: dozens of algorithms must share one mental model so
  contributors (and agents) can add an algorithm by analogy, not invention.
- **Reuse the CSR**: building the CSR from base tables is the dominant cost;
  algorithms must operate on the in-memory CSR, not re-scan SQL.
- **SQL-native ergonomics**: results must compose in SQL (`FROM algo(pg, v, e)`)
  and join back to vertex properties.
- **Correctness & testability**: each algorithm needs a deterministic oracle for
  sqllogictest, plus perf/memory profiling before release.
- **Parallelism**: 32-core hosts are common; the design should not preclude
  intra-algorithm parallelism even if the first cut is single-threaded.
- **Minimal divergence from upstream**: stay on the documented
  `ExtensionLoader` + `CoreScalarFunctions`/`CoreTableFunctions` registration
  pattern so DuckDB version bumps stay cheap.

## Considered Options

### Option A: Keep the established "scalar-over-CSR + table-function rewrite" pattern (per-algorithm files)
Each algorithm gets: a scalar `.cpp` computing over `csr_list[csr_id]`, a
`*_function_data` bind-data class holding cached state, a table-function `.hpp`
(`BindReplace` + `Bind` schema) and `.cpp`, registration entries in
`scalar.hpp`/`table.hpp`, and a CMake entry. Output is per-vertex values
(centralities, components) or pair inputs (similarity, path).

- **Pros**: Zero new infrastructure; identical to PageRank/WCC/LCC so it is
  copy-and-adapt; composes in SQL today; CSR reused; reviewers already understand
  it; trivially parallelizable across contributors/agents since each algorithm is
  mostly its own files.
- **Cons**: Boilerplate per algorithm (~5 files); the "compute-once, cache in bind
  data, emit per row" idiom is subtle (lock + `state_initialized`); whole-graph
  results recomputed if bind data is not reused across chunks.
- **Cost**: Low per algorithm (~0.5–1 day each once the template is set).

### Option B: A generic "graph algorithm" table-function framework (registry + strategy objects)
Introduce one `graph_algo(pg, v, e, 'name', params...)` table function backed by
a registry of algorithm strategy objects implementing a common
`Compute(CSR&, params) -> ResultColumns` interface, with a shared executor that
handles CSR lookup, parallel scheduling, and result materialization.

- **Pros**: One registration point; uniform parameter handling and parallel
  executor written once; less per-algorithm boilerplate; easier to add cross-cutting
  features (progress, cancellation, memory accounting) in one place.
- **Cons**: Large up-front build; diverges from the existing pattern (two idioms
  in the tree until everything migrates); a single stringly-typed entry point is
  worse SQL ergonomics and weaker bind-time validation than first-class function
  names; harder for parallel contributors to work without colliding on the shared
  executor.
- **Cost**: High up front (framework), lower marginal cost later.

### Option C: Push algorithms into SQL/recursive-CTE macros (no new C++)
Express algorithms as recursive CTEs / SQL macros over the edge tables.

- **Pros**: No C++; portable; leverages DuckDB's optimizer.
- **Cons**: Recursive CTEs are far slower and memory-heavier than CSR kernels;
  many algorithms (Brandes betweenness, Tarjan SCC, Louvain) don't map cleanly to
  SQL; loses the CSR investment; hard to profile/optimize. Non-starter for SOTA
  performance.
- **Cost**: Low to start, but a dead end for the stated goal.

## Decision

**We will use Option A** — extend the existing scalar-over-CSR + table-function
rewrite pattern, one set of files per algorithm — for the initial library, and
**defer Option B** (a shared executor) as a future refactor once we have ~15+
algorithms and can extract the genuinely common machinery (parallel vertex loops,
result materialization, memory accounting) without guessing.

What tipped the balance: the existing pattern is already proven for three
whole-graph algorithms (PageRank, WCC, LCC), composes natively in SQL, fully
reuses the CSR, and — critically for the agent-swarm execution plan — lets many
algorithms be built in parallel because each lives in its own files. The
framework (Option B) is the right *eventual* shape, but building it before we
understand the variety of algorithm signatures (per-vertex vs. pairwise vs.
path-returning vs. iterative-to-convergence) would be premature abstraction.

## Algorithm Catalog (SOTA target)

Legend: ✅ exists · ⭐ Phase 1 (this effort) · ➕ Phase 2 · 🔬 Phase 3 (research/ML)

### Centrality
- ✅ PageRank
- ⭐ Degree centrality (in / out / total)
- ⭐ Closeness centrality (Wasserman–Faust normalized)
- ⭐ Betweenness centrality (Brandes)
- ➕ Harmonic centrality
- ➕ Eigenvector centrality
- ➕ Katz centrality
- ➕ ArticleRank
- ➕ Personalized PageRank
- ➕ HITS (hubs & authorities)

### Community Detection
- ✅ Weakly Connected Components
- ✅ Local Clustering Coefficient
- ⭐ Triangle Count (per-vertex + global)
- ⭐ Strongly Connected Components (Tarjan)
- ⭐ Label Propagation (LPA)
- ➕ Louvain (modularity)
- ➕ Leiden
- ➕ K-core decomposition
- ➕ Global clustering coefficient / transitivity

### Path Finding & Traversal
- ✅ BFS shortest-path length / path / bidirectional
- ✅ Cheapest (weighted) path
- ⭐ Single-Source Shortest Path (Dijkstra, weighted)
- ➕ Delta-stepping SSSP (parallel)
- ➕ Bellman–Ford (negative weights)
- ➕ Yen's K-shortest paths
- ➕ A*
- ➕ Minimum Spanning Tree (Prim / Kruskal)
- ➕ Topological sort / cycle detection
- ➕ Random walk (node2vec sampling)

### Similarity
- ⭐ Jaccard similarity (neighborhood)
- ➕ Overlap / Cosine similarity
- ➕ Adamic–Adar
- ➕ Common neighbors / Preferential attachment / Resource allocation

### Node Embeddings
- 🔬 FastRP
- 🔬 Node2Vec / DeepWalk

## Phase 1 scope (this execution)

Implement, build, end-to-end test (sqllogictest), and profile:

1. **Degree centrality** — establishes/validates the template (cheap, exact oracle).
2. **Triangle count** (per-vertex) — feeds clustering; intersection kernel.
3. **Closeness centrality** — multi-source BFS over CSR.
4. **Betweenness centrality** (Brandes) — BFS + dependency accumulation.
5. **Strongly Connected Components** (Tarjan, iterative) — directed components.
6. **Label Propagation** — iterative-to-convergence community detection.
7. **Single-Source Shortest Path** (Dijkstra) — weighted, uses CSR `w`/`w_double`.
8. **Jaccard similarity** — pairwise neighborhood similarity.

## Execution model (agent swarm)

- **Scaffolding first (leader, serial)**: implement Degree centrality fully
  (all 5 file types + test) to lock the template and the central wiring files
  (`scalar.hpp`, `table.hpp`, `module`, CMake). These are shared edit points and
  must not be edited by parallel agents simultaneously.
- **Breadth next (swarm, parallel)**: each remaining algorithm is built by a
  worker agent in its *own* files. The leader serializes the few-line edits to
  the shared registration headers/CMake to avoid merge conflicts.
- **Integrate & test**: incremental rebuild, load into a freshly installed DuckDB
  CLI, run the new sqllogictest suite end-to-end.
- **Profile**: perf (wall-clock per algorithm on a generated graph) and memory
  (peak RSS / valgrind massif or /usr/bin/time -v) before marking complete.

## Consequences

### Positive
- New algorithms are copy-and-adapt; low cognitive load; reviewable by analogy.
- Full CSR reuse; SQL-native composition; first-class function names give
  bind-time validation and discoverability.
- Naturally parallel to build (one algorithm ≈ one set of files).

### Negative
- Per-algorithm boilerplate (~5 files) until/unless Option B is extracted.
- The "compute-once, cache, emit-per-row" idiom must be replicated carefully;
  easy to get the lock/`state_initialized` handshake wrong.
- Shared registration files (`scalar.hpp`, `table.hpp`, CMake) are contention
  points for parallel work; require serialized edits.

### Risks
- **Correctness of complex kernels** (Brandes, Tarjan): mitigated by
  deterministic sqllogictest oracles computed against NetworkX/hand-derived values
  on tiny graphs.
- **Memory blow-up on large graphs** (e.g. APSP, betweenness O(V·E)): mitigated by
  Phase-1 scope avoiding all-pairs materialization and by mandatory memory
  profiling before release.
- **cmake 4.x vs. DuckDB minimum**: mitigated by pinning a 3.x cmake if configure
  rejects the policy version.

## Verification
- All Phase-1 algorithms registered, building in the loadable extension, and
  green under `make test` (new `test/sql/scalar/*.test` files).
- Each algorithm validated against a known oracle on a small graph.
- Perf + memory numbers captured for each algorithm on a generated graph and
  recorded in the test plan before the project is marked complete.
