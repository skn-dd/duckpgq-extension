#!/usr/bin/env python3
"""Time graph operations on a live Oracle 23ai ADB for comparison with DuckPGQ.

Credentials are read from the environment (never hard-code them):
    export ORACLE_USER=ADMIN
    export ORACLE_PW=...                       # do not commit
    export ORACLE_DSN='(description=(address=(protocol=tcps)(port=1521)
                        (host=...))(connect_data=(service_name=...))
                        (security=(ssl_server_dn_match=yes)))'
    python3 benchmark/oracle_compare.py

Generates identical ring-lattice graphs (N vertices, ~5N directed edges) used by
run_scale_benchmark.sh, builds a SQL property graph, and times degree centrality
(via Oracle's GRAPH_TABLE graph engine) and a 2-hop pattern count.

Note: Oracle 23ai also exposes in-database PageRank/WCC/Bellman-Ford via
DBMS_OGA inside GRAPH_TABLE; wiring the GAF output-property syntax here is left
as a TODO to add a direct algorithm-library time comparison.
"""
import os, sys, time
try:
    import oracledb
except ImportError:
    sys.exit("pip install --user oracledb")

USER = os.environ.get("ORACLE_USER", "ADMIN")
PW = os.environ.get("ORACLE_PW")
DSN = os.environ.get("ORACLE_DSN")
if not (PW and DSN):
    sys.exit("Set ORACLE_PW and ORACLE_DSN in the environment.")

conn = oracledb.connect(user=USER, password=PW, dsn=DSN)
cur = conn.cursor()

def ex(sql):
    cur.execute(sql)

def timed(sql):
    t = time.time(); cur.execute(sql); cur.fetchall(); return time.time() - t

def safe(sql):
    try: ex(sql)
    except Exception: pass

DEG = 5
print(f"{'scale':>9} | {'build+PG(s)':>11} | {'degree(s)':>10} | {'2-hop(s)':>9}")
print("-" * 50)
for N in (100_000, 500_000, 1_000_000, 5_000_000):
    safe("DROP PROPERTY GRAPH rlg")
    safe("DROP TABLE rl_edges PURGE"); safe("DROP TABLE rl_nodes PURGE")
    t0 = time.time()
    ex(f"CREATE TABLE rl_nodes AS SELECT level-1 AS id FROM dual CONNECT BY level<={N}")
    ex(f"CREATE TABLE rl_edges AS SELECT n.id AS src, MOD(n.id+o.off,{N}) AS dst "
       f"FROM rl_nodes n CROSS JOIN (SELECT level AS off FROM dual CONNECT BY level<={DEG}) o")
    ex("ALTER TABLE rl_nodes ADD PRIMARY KEY (id)")
    ex("CREATE PROPERTY GRAPH rlg VERTEX TABLES (rl_nodes KEY (id) LABEL n PROPERTIES (id)) "
       "EDGE TABLES (rl_edges KEY (src,dst) SOURCE KEY (src) REFERENCES rl_nodes (id) "
       "DESTINATION KEY (dst) REFERENCES rl_nodes (id) LABEL e)")
    build = time.time() - t0
    tdeg = timed("SELECT count(*) FROM (SELECT a_id, count(*) d FROM "
                 "GRAPH_TABLE(rlg MATCH (a)-[]->(b) COLUMNS (a.id AS a_id)) GROUP BY a_id)")
    t2 = timed("SELECT count(*) FROM GRAPH_TABLE(rlg MATCH (a)-[]->(b)-[]->(c) COLUMNS (1 AS one))")
    print(f"{N:>9} | {build:>11.2f} | {tdeg:>10.2f} | {t2:>9.2f}")

safe("DROP PROPERTY GRAPH rlg"); safe("DROP TABLE rl_edges PURGE"); safe("DROP TABLE rl_nodes PURGE")
conn.commit(); cur.close(); conn.close()
print("DONE")
