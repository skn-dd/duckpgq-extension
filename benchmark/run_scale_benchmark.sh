#!/usr/bin/env bash
# Multi-scale perf/memory profiling for DuckPGQ algorithms.
#
# Near-linear algorithms are run at 100k / 500k / 1M / 5M vertices; superlinear
# (per-source BFS) algorithms are run only at a small scale. Each run is a fresh
# sqllogictest process under `/usr/bin/time -v` with a wall-clock timeout, so
# bottlenecks surface as TIMEOUT rows. Graphs are clean ring lattices (degree
# ~5) which the undirected CSR builder accepts.
#
# Usage: benchmark/run_scale_benchmark.sh [timeout_seconds]
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
UNITTEST="$ROOT/build/release/test/unittest"
BENCHDIR="$ROOT/test/bench"
TMP="$(mktemp -d)"
mkdir -p "$BENCHDIR"
trap 'rm -rf "$TMP" "$BENCHDIR"' EXIT
TIMEOUT="${1:-180}"

[[ -x "$UNITTEST" ]] || { echo "build first (make)"; exit 1; }

# gen_test <file> <nv> <deg> <algo> <mem_limit_or_empty>
gen_test() {
  local file="$1" nv="$2" deg="$3" algo="$4" mem="$5" extra="${6:-}" expected="${7:-$nv}"
  {
    echo "require duckpgq"; echo ""
    if [[ -n "$mem" ]]; then echo "statement ok"; echo "SET memory_limit='$mem';"; echo ""; fi
    echo "statement ok"
    echo "CREATE TABLE v AS SELECT i AS id FROM range(0,$nv) t(i);"; echo ""
    echo "statement ok"
    echo "CREATE TABLE e AS SELECT t.i AS src, (t.i + o.off) % $nv AS dst FROM range(0,$nv) t(i) CROSS JOIN range(1,$((deg+1))) o(off);"; echo ""
    echo "statement ok"
    echo "-CREATE PROPERTY GRAPH g VERTEX TABLES (v) EDGE TABLES (e SOURCE KEY (src) REFERENCES v(id) DESTINATION KEY (dst) REFERENCES v(id));"; echo ""
    echo "query I"
    echo "SELECT count(*) FROM ${algo}(g, v, e${extra});"
    echo "----"
    echo "$expected"
  } > "$file"
}

run() {
  local algo="$1" nv="$2" deg="$3" extra="${4:-}" expected="${5:-$nv}"
  local safe="${algo}_${nv}"
  local tf="$BENCHDIR/${safe}.test"; local rel="test/bench/${safe}.test"; local timef="$TMP/${safe}.time"
  gen_test "$tf" "$nv" "$deg" "$algo" "" "$extra" "$expected"
  ( cd "$ROOT" && timeout "$TIMEOUT" /usr/bin/time -v "$UNITTEST" "$rel" ) >/dev/null 2>"$timef"
  local rc=$?
  local wall rss status
  wall=$(awk -F'): ' '/Elapsed \(wall clock\)/{print $2}' "$timef" 2>/dev/null)
  rss=$(awk -F': ' '/Maximum resident set size/{print $2}' "$timef" 2>/dev/null)
  if [[ $rc -eq 124 ]]; then status="TIMEOUT(>${TIMEOUT}s)"; wall=">${TIMEOUT}s"
  elif [[ $rc -eq 0 ]]; then status="OK"
  else status="FAIL(rc=$rc)"; fi
  local rssmb="-"; [[ -n "$rss" ]] && rssmb="$(awk "BEGIN{printf \"%.0f MB\", $rss/1024}")"
  printf "  %-30s %-12s %-10s %s\n" "$algo" "${wall:-?}" "$rssmb" "$status"
}

LINEAR=(degree_centrality triangle_count pagerank weakly_connected_component \
        strongly_connected_component label_propagation eigenvector_centrality \
        katz_centrality article_rank k_core_decomposition \
        global_clustering_coefficient hits_authority hits_hub local_clustering_coefficient \
        topological_sort louvain in_degree_centrality out_degree_centrality)
PARAM=(single_source_shortest_path personalized_pagerank)   # take a 4th arg: source vertex
PAIRWISE=(jaccard_similarity cosine_similarity overlap_similarity common_neighbors \
          preferential_attachment adamic_adar resource_allocation)  # O(V^2) pair output
QUAD=(closeness_centrality betweenness_centrality harmonic_centrality eccentricity)

echo "================================================================"
echo " DuckPGQ Multi-Scale Profile  ($(nproc) cores, timeout ${TIMEOUT}s/run)"
echo "================================================================"
for nv in 100000 500000 1000000 5000000; do
  echo ""; echo "### Near-linear algorithms @ ${nv} vertices (ring lattice, deg 5, ~$((nv*5)) edges)"
  printf "  %-30s %-12s %-10s %s\n" "algorithm" "wall" "peak-RSS" "status"
  for a in "${LINEAR[@]}"; do run "$a" "$nv" 5; done
  echo "  -- parameterized (source vertex = 0) --"
  for a in "${PARAM[@]}"; do run "$a" "$nv" 5 ", 0"; done
done

echo ""; echo "### Superlinear (per-source BFS) algorithms @ 3000 vertices"
printf "  %-30s %-12s %-10s %s\n" "algorithm" "wall" "peak-RSS" "status"
for a in "${QUAD[@]}"; do run "$a" 3000 5; done

echo ""; echo "### Pairwise similarity (O(V^2) output) @ 2000 vertices (=1,999,000 pairs)"
printf "  %-30s %-12s %-10s %s\n" "algorithm" "wall" "peak-RSS" "status"
for a in "${PAIRWISE[@]}"; do run "$a" 2000 5 "" 1999000; done
echo "================================================================"
