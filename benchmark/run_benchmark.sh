#!/usr/bin/env bash
# Perf + memory profiling harness for DuckPGQ graph algorithms.
#
# The DuckPGQ property-graph table functions must be driven through the
# sqllogictest `unittest` runner (the bare CLI does not wire up the PGQ
# parser/operator extensions). We therefore generate a tiny .test per
# algorithm that builds a random graph and forces a full computation, then
# run each under `/usr/bin/time -v` to capture wall-clock and peak RSS.
#
# Usage: benchmark/run_benchmark.sh
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
UNITTEST="$ROOT/build/release/test/unittest"
TMP="$(mktemp -d)"
# The sqllogictest runner only executes .test files discovered under the repo
# test/ tree, so generated benchmark tests must live there.
BENCHDIR="$ROOT/test/bench"
mkdir -p "$BENCHDIR"
trap 'rm -rf "$TMP" "$BENCHDIR"' EXIT

if [[ ! -x "$UNITTEST" ]]; then
  echo "ERROR: $UNITTEST not found. Build first (make)." >&2
  exit 1
fi

# gen_test <file> <num_vertices> <num_edges> <algo_call_or_empty>
# If algo is empty, only builds the graph + CSR (baseline).
gen_test() {
  local file="$1" nv="$2" ne="$3" algo="$4"
  # Clean ring-lattice graph: vertex i connects to (i+1..i+k) mod N. This yields
  # unique edges, no self-loops, a connected graph, and every edge references an
  # existing unique vertex -- which the (undirected) CSR builder requires.
  local k=$(( ne / nv )); [[ $k -lt 1 ]] && k=1
  {
    echo "# generated benchmark (ring lattice, degree ~${k})"
    echo "require duckpgq"
    echo ""
    echo "statement ok"
    echo "CREATE TABLE v AS SELECT i AS id FROM range(0,$nv) t(i);"
    echo ""
    echo "statement ok"
    echo "CREATE TABLE e AS SELECT t.i AS src, (t.i + o.off) % $nv AS dst FROM range(0,$nv) t(i) CROSS JOIN range(1,$((k+1))) o(off);"
    echo ""
    echo "statement ok"
    echo "-CREATE PROPERTY GRAPH g VERTEX TABLES (v) EDGE TABLES (e SOURCE KEY (src) REFERENCES v(id) DESTINATION KEY (dst) REFERENCES v(id));"
    echo ""
    if [[ -n "$algo" ]]; then
      echo "query I"
      echo "SELECT count(*) FROM ${algo}(g, v, e);"
      echo "----"
      echo "$nv"
    else
      echo "query I"
      echo "SELECT count(*) FROM v;"
      echo "----"
      echo "$nv"
    fi
  } > "$file"
}

# run_one <label> <num_vertices> <num_edges> <algo>
run_one() {
  local label="$1" nv="$2" ne="$3" algo="$4"
  local safe="${label//[^A-Za-z0-9_]/_}"
  local test_file="$BENCHDIR/${safe}.test"
  gen_test "$test_file" "$nv" "$ne" "$algo"
  local relpath="test/bench/${safe}.test"
  local timef="$TMP/${safe}.time"
  ( cd "$ROOT" && /usr/bin/time -v "$UNITTEST" "$relpath" ) >/dev/null 2>"$timef"
  local rc=$?
  local wall rss status
  wall=$(awk -F'): ' '/Elapsed \(wall clock\)/{print $2}' "$timef")
  rss=$(awk -F': ' '/Maximum resident set size/{print $2}' "$timef")
  if [[ $rc -eq 0 ]]; then status="OK"; else status="FAIL(rc=$rc)"; fi
  printf "%-34s %12s %12s   %s\n" "$label" "$wall" "$(awk "BEGIN{printf \"%.1f MB\", $rss/1024}")" "$status"
}

echo "==================================================================================="
echo " DuckPGQ Algorithm Perf / Memory Profile"
echo " host: $(nproc) cores, $(free -h | awk '/Mem:/{print $2}') RAM"
echo "==================================================================================="
printf "%-34s %12s %12s   %s\n" "benchmark (algorithm @ scale)" "wall" "peak-RSS" "status"
echo "-----------------------------------------------------------------------------------"

# --- Near-linear algorithms: large graph (N=50k vertices, M=500k edges) ---
LV=50000; LE=500000
run_one "baseline_graphgen@${LV}v/${LE}e"        $LV $LE ""
run_one "degree_centrality@${LV}v/${LE}e"        $LV $LE "degree_centrality"
run_one "triangle_count@${LV}v/${LE}e"           $LV $LE "triangle_count"
run_one "pagerank@${LV}v/${LE}e"                 $LV $LE "pagerank"
run_one "weakly_connected_component@${LV}v/${LE}e" $LV $LE "weakly_connected_component"
run_one "strongly_connected_component@${LV}v/${LE}e" $LV $LE "strongly_connected_component"
run_one "label_propagation@${LV}v/${LE}e"        $LV $LE "label_propagation"
run_one "eigenvector_centrality@${LV}v/${LE}e"   $LV $LE "eigenvector_centrality"
run_one "local_clustering_coefficient@${LV}v/${LE}e" $LV $LE "local_clustering_coefficient"

echo "-----------------------------------------------------------------------------------"
# --- Quadratic (per-source BFS) algorithms: smaller graph (N=5k, M=25k) ---
QV=5000; QE=25000
run_one "baseline_graphgen@${QV}v/${QE}e"        $QV $QE ""
run_one "closeness_centrality@${QV}v/${QE}e"     $QV $QE "closeness_centrality"
run_one "betweenness_centrality@${QV}v/${QE}e"   $QV $QE "betweenness_centrality"
echo "==================================================================================="
echo "Notes: wall/peak-RSS include sqllogictest harness + graph generation + CSR build."
echo "Subtract the matching baseline_graphgen row to isolate algorithm cost."
