//===----------------------------------------------------------------------===//
//                         DuckPGQ (stock-DuckDB build)
//
// src/core/functions/table/graph_match.cpp
//
// Ported from the fork's extension/duckpgq/src/core/functions/table/match.cpp.
// The relational-rewrite logic (ProcessPathList / AddEdgeJoins / EdgeType* /
// CreateMatchJoinExpression / bounded-path expansion) is pure (no catalog /
// binder / ClientContext coupling) and is reproduced essentially unchanged.
//
// What is adapted: the fork's MatchBindReplace pulled a parsed MatchExpression
// and a registered CreatePropertyGraphInfo out of DuckPGQState. Here,
// GraphMatchBindReplace instead:
//   1. reads the table-function args (pattern, vertex_table, vertex_id,
//      edge_table, src, dst),
//   2. parses the pattern string into a MatchExpression (pattern_parser),
//   3. SYNTHESIZES a one-vertex-table / one-edge-table CreatePropertyGraphInfo,
//      populated exactly like MakeEdgeSpec and keyed by the parsed labels so
//      FindGraphTable resolves, then
//   4. runs the ported rewrite to return a SubqueryRef.
//
// Variable-length edges (-[e:E]->{lo,hi}) are lowered via the relational
// bounded-path expansion (AddBoundedPathExpansion) only — the CSR/shortestpath
// route is intentionally not wired here (see TODO at the bottom).
//===----------------------------------------------------------------------===//

#include "duckpgq/core/functions/table/graph_match.hpp"
#include "duckpgq/core/parser/pattern_parser.hpp"

#include "duckdb/common/string_util.hpp"
#include "duckdb/common/case_insensitive_map.hpp"
#include "duckdb/parser/tableref/subqueryref.hpp"
#include "duckdb/parser/tableref/joinref.hpp"
#include "duckdb/parser/tableref/basetableref.hpp"
#include "duckdb/parser/expression/function_expression.hpp"
#include "duckdb/parser/expression/constant_expression.hpp"
#include "duckdb/parser/expression/comparison_expression.hpp"
#include "duckdb/parser/expression/conjunction_expression.hpp"
#include "duckdb/parser/expression/star_expression.hpp"
#include "duckdb/parser/expression/columnref_expression.hpp"
#include "duckdb/parser/query_node/set_operation_node.hpp"
#include "duckdb/parser/query_node/select_node.hpp"

#include <duckpgq/core/functions/table.hpp>
#include <duckpgq/core/utils/duckpgq_utils.hpp>

#include <algorithm>

namespace duckdb {

namespace {

constexpr int64_t MAX_BOUNDED_PATH_EXPANSION_UPPER = 16;

unique_ptr<ParsedExpression> BuildConjunction(vector<unique_ptr<ParsedExpression>> &conditions) {
	unique_ptr<ParsedExpression> where_clause;
	for (auto &condition : conditions) {
		if (where_clause) {
			where_clause = make_uniq<ConjunctionExpression>(ExpressionType::CONJUNCTION_AND, std::move(where_clause),
			                                                std::move(condition));
		} else {
			where_clause = std::move(condition);
		}
	}
	return where_clause;
}

void CrossJoinTableRef(unique_ptr<TableRef> &from_clause, unique_ptr<TableRef> table_ref) {
	if (from_clause) {
		auto join = make_uniq<JoinRef>(JoinRefType::CROSS);
		join->left = std::move(from_clause);
		join->right = std::move(table_ref);
		from_clause = std::move(join);
	} else {
		from_clause = std::move(table_ref);
	}
}

} // namespace

shared_ptr<PropertyGraphTable> GraphMatchFunction::FindGraphTable(const string &label,
                                                                 CreatePropertyGraphInfo &pg_table) {
	const auto graph_table_entry = pg_table.label_map.find(label);
	if (graph_table_entry == pg_table.label_map.end()) {
		throw BinderException("graph_match: the label '%s' is not present in the pattern's tables", label);
	}
	return graph_table_entry->second;
}

unique_ptr<ParsedExpression> GraphMatchFunction::CreateMatchJoinExpression(vector<string> vertex_keys,
                                                                          vector<string> edge_keys,
                                                                          const string &vertex_alias,
                                                                          const string &edge_alias) {
	vector<unique_ptr<ParsedExpression>> conditions;
	if (vertex_keys.size() != edge_keys.size()) {
		throw BinderException("Vertex columns and edge columns size mismatch");
	}
	for (idx_t i = 0; i < vertex_keys.size(); i++) {
		auto vertex_colref = make_uniq<ColumnRefExpression>(vertex_keys[i], vertex_alias);
		auto edge_colref = make_uniq<ColumnRefExpression>(edge_keys[i], edge_alias);
		conditions.push_back(make_uniq<ComparisonExpression>(ExpressionType::COMPARE_EQUAL, std::move(vertex_colref),
		                                                     std::move(edge_colref)));
	}
	return BuildConjunction(conditions);
}

PathElement *GraphMatchFunction::GetPathElement(const unique_ptr<PathReference> &path_reference) {
	if (path_reference->path_reference_type == PGQPathReferenceType::PATH_ELEMENT) {
		return reinterpret_cast<PathElement *>(path_reference.get());
	}
	if (path_reference->path_reference_type == PGQPathReferenceType::SUBPATH) {
		return nullptr;
	}
	throw InternalException("Unknown path reference type detected");
}

SubPath *GraphMatchFunction::GetSubPath(const unique_ptr<PathReference> &path_reference) {
	if (path_reference->path_reference_type == PGQPathReferenceType::PATH_ELEMENT) {
		return nullptr;
	}
	if (path_reference->path_reference_type == PGQPathReferenceType::SUBPATH) {
		return reinterpret_cast<SubPath *>(path_reference.get());
	}
	throw InternalException("Unknown path reference type detected");
}

unique_ptr<ParsedExpression> GraphMatchFunction::CreateWhereClause(vector<unique_ptr<ParsedExpression>> &conditions) {
	return BuildConjunction(conditions);
}

void GraphMatchFunction::CheckEdgeTableConstraints(const string &src_reference, const string &dst_reference,
                                                  const shared_ptr<PropertyGraphTable> &edge_table) {
	if (src_reference != edge_table->source_reference) {
		throw BinderException("Label %s is not registered as a source reference for edge pattern of table %s",
		                      src_reference, edge_table->table_name);
	}
	if (dst_reference != edge_table->destination_reference) {
		throw BinderException("Label %s is not registered as a destination reference for edge pattern of table %s",
		                      src_reference, edge_table->table_name);
	}
}

void GraphMatchFunction::EdgeTypeAny(const shared_ptr<PropertyGraphTable> &edge_table, const string &edge_binding,
                                    const string &prev_binding, const string &next_binding,
                                    vector<unique_ptr<ParsedExpression>> &conditions,
                                    unique_ptr<TableRef> &from_clause) {
	// (SELECT src, dst, * FROM edge UNION ALL SELECT dst, src, * FROM edge) edge_binding
	auto src_dst_select_node = make_uniq<SelectNode>();
	src_dst_select_node->from_table = edge_table->CreateBaseTableRef(edge_binding);
	{
		vector<unique_ptr<ParsedExpression>> src_dst_children;
		src_dst_children.push_back(make_uniq<ColumnRefExpression>(edge_table->source_fk[0], edge_binding));
		src_dst_children.push_back(make_uniq<ColumnRefExpression>(edge_table->destination_fk[0], edge_binding));
		src_dst_children.push_back(make_uniq<StarExpression>());
		src_dst_select_node->select_list = std::move(src_dst_children);
	}

	auto dst_src_select_node = make_uniq<SelectNode>();
	dst_src_select_node->from_table = edge_table->CreateBaseTableRef(edge_binding);
	{
		vector<unique_ptr<ParsedExpression>> dst_src_children;
		dst_src_children.push_back(make_uniq<ColumnRefExpression>(edge_table->destination_fk[0], edge_binding));
		dst_src_children.push_back(make_uniq<ColumnRefExpression>(edge_table->source_fk[0], edge_binding));
		dst_src_children.push_back(make_uniq<StarExpression>());
		dst_src_select_node->select_list = std::move(dst_src_children);
	}

	auto union_node = make_uniq<SetOperationNode>();
	union_node->setop_type = SetOperationType::UNION;
	union_node->setop_all = true;
	union_node->children.push_back(std::move(src_dst_select_node));
	union_node->children.push_back(std::move(dst_src_select_node));
	auto union_select = make_uniq<SelectStatement>();
	union_select->node = std::move(union_node);
	auto union_subquery = make_uniq<SubqueryRef>(std::move(union_select));
	union_subquery->alias = edge_binding;

	CrossJoinTableRef(from_clause, std::move(union_subquery));

	auto src_left_expr =
	    CreateMatchJoinExpression(edge_table->source_pk, edge_table->source_fk, prev_binding, edge_binding);
	auto dst_left_expr =
	    CreateMatchJoinExpression(edge_table->destination_pk, edge_table->destination_fk, next_binding, edge_binding);
	auto combined_left_expr = make_uniq<ConjunctionExpression>(ExpressionType::CONJUNCTION_AND,
	                                                           std::move(src_left_expr), std::move(dst_left_expr));
	conditions.push_back(std::move(combined_left_expr));
}

void GraphMatchFunction::EdgeTypeLeft(const shared_ptr<PropertyGraphTable> &edge_table, const string &next_table_name,
                                     const string &prev_table_name, const string &edge_binding,
                                     const string &prev_binding, const string &next_binding,
                                     vector<unique_ptr<ParsedExpression>> &conditions) {
	CheckEdgeTableConstraints(next_table_name, prev_table_name, edge_table);
	conditions.push_back(
	    CreateMatchJoinExpression(edge_table->source_pk, edge_table->source_fk, next_binding, edge_binding));
	conditions.push_back(
	    CreateMatchJoinExpression(edge_table->destination_pk, edge_table->destination_fk, prev_binding, edge_binding));
}

void GraphMatchFunction::EdgeTypeRight(const shared_ptr<PropertyGraphTable> &edge_table, const string &next_table_name,
                                      const string &prev_table_name, const string &edge_binding,
                                      const string &prev_binding, const string &next_binding,
                                      vector<unique_ptr<ParsedExpression>> &conditions) {
	CheckEdgeTableConstraints(prev_table_name, next_table_name, edge_table);
	conditions.push_back(
	    CreateMatchJoinExpression(edge_table->source_pk, edge_table->source_fk, prev_binding, edge_binding));
	conditions.push_back(
	    CreateMatchJoinExpression(edge_table->destination_pk, edge_table->destination_fk, next_binding, edge_binding));
}

void GraphMatchFunction::EdgeTypeLeftRight(const shared_ptr<PropertyGraphTable> &edge_table, const string &edge_binding,
                                          const string &prev_binding, const string &next_binding,
                                          vector<unique_ptr<ParsedExpression>> &conditions,
                                          case_insensitive_map_t<shared_ptr<PropertyGraphTable>> &alias_map,
                                          int32_t &extra_alias_counter) {
	auto src_left_expr =
	    CreateMatchJoinExpression(edge_table->source_pk, edge_table->source_fk, next_binding, edge_binding);
	auto dst_left_expr =
	    CreateMatchJoinExpression(edge_table->destination_pk, edge_table->destination_fk, prev_binding, edge_binding);
	auto combined_left_expr = make_uniq<ConjunctionExpression>(ExpressionType::CONJUNCTION_AND,
	                                                           std::move(src_left_expr), std::move(dst_left_expr));

	const auto additional_edge_alias = edge_binding + std::to_string(extra_alias_counter);
	extra_alias_counter++;
	alias_map[additional_edge_alias] = edge_table;

	auto src_right_expr =
	    CreateMatchJoinExpression(edge_table->source_pk, edge_table->source_fk, prev_binding, additional_edge_alias);
	auto dst_right_expr = CreateMatchJoinExpression(edge_table->destination_pk, edge_table->destination_fk,
	                                                next_binding, additional_edge_alias);
	auto combined_right_expr = make_uniq<ConjunctionExpression>(ExpressionType::CONJUNCTION_AND,
	                                                            std::move(src_right_expr), std::move(dst_right_expr));

	auto combined_expr = make_uniq<ConjunctionExpression>(ExpressionType::CONJUNCTION_AND, std::move(combined_left_expr),
	                                                      std::move(combined_right_expr));
	conditions.push_back(std::move(combined_expr));
}

void GraphMatchFunction::AddEdgeJoins(const shared_ptr<PropertyGraphTable> &edge_table,
                                     const shared_ptr<PropertyGraphTable> &previous_vertex_table,
                                     const shared_ptr<PropertyGraphTable> &next_vertex_table, PGQMatchType edge_type,
                                     const string &edge_binding, const string &prev_binding,
                                     const string &next_binding, vector<unique_ptr<ParsedExpression>> &conditions,
                                     case_insensitive_map_t<shared_ptr<PropertyGraphTable>> &alias_map,
                                     int32_t &extra_alias_counter, unique_ptr<TableRef> &from_clause) {
	if (edge_type != PGQMatchType::MATCH_EDGE_ANY) {
		alias_map[edge_binding] = edge_table;
	}
	switch (edge_type) {
	case PGQMatchType::MATCH_EDGE_ANY:
		EdgeTypeAny(edge_table, edge_binding, prev_binding, next_binding, conditions, from_clause);
		break;
	case PGQMatchType::MATCH_EDGE_LEFT:
		EdgeTypeLeft(edge_table, next_vertex_table->table_name, previous_vertex_table->table_name, edge_binding,
		             prev_binding, next_binding, conditions);
		break;
	case PGQMatchType::MATCH_EDGE_RIGHT:
		EdgeTypeRight(edge_table, next_vertex_table->table_name, previous_vertex_table->table_name, edge_binding,
		              prev_binding, next_binding, conditions);
		break;
	case PGQMatchType::MATCH_EDGE_LEFT_RIGHT:
		EdgeTypeLeftRight(edge_table, edge_binding, prev_binding, next_binding, conditions, alias_map,
		                  extra_alias_counter);
		break;
	default:
		throw InternalException("Unknown match type found");
	}
}

bool GraphMatchFunction::CanExpandBoundedSubpath(const shared_ptr<PropertyGraphTable> &edge_table, SubPath *subpath,
                                                PGQMatchType edge_type) {
	if (edge_type != PGQMatchType::MATCH_EDGE_RIGHT) {
		return false;
	}
	if (subpath->upper <= 1 || subpath->upper > MAX_BOUNDED_PATH_EXPANSION_UPPER) {
		return false;
	}
	if (edge_table->source_reference != edge_table->destination_reference) {
		return false;
	}
	if (edge_table->source_pk.size() != edge_table->destination_pk.size()) {
		return false;
	}
	return edge_table->source_fk.size() == edge_table->source_pk.size() &&
	       edge_table->destination_fk.size() == edge_table->destination_pk.size();
}

void GraphMatchFunction::AddBoundedPathExpansion(const shared_ptr<PropertyGraphTable> &edge_table, SubPath *subpath,
                                                const string &prev_binding, const string &edge_binding,
                                                const string &next_binding,
                                                vector<unique_ptr<ParsedExpression>> &conditions,
                                                unique_ptr<TableRef> &from_clause, int32_t &extra_alias_counter) {
	auto path_alias = edge_binding + "_bounded_path_" + std::to_string(static_cast<uint32_t>(extra_alias_counter++));
	vector<string> source_columns;
	vector<string> destination_columns;
	for (idx_t key_idx = 0; key_idx < edge_table->source_pk.size(); key_idx++) {
		source_columns.push_back("__duckpgq_src_" + std::to_string(key_idx));
		destination_columns.push_back("__duckpgq_dst_" + std::to_string(key_idx));
	}

	auto union_node = make_uniq<SetOperationNode>();
	union_node->setop_type = SetOperationType::UNION;
	union_node->setop_all = true;

	for (int64_t path_length = subpath->lower; path_length <= subpath->upper; path_length++) {
		auto branch = make_uniq<SelectNode>();
		branch->AddDistinct();
		vector<unique_ptr<ParsedExpression>> branch_conditions;

		if (path_length == 0) {
			auto vertex_alias = path_alias + "_v_0";
			branch->from_table = edge_table->source_pg_table->CreateBaseTableRef(vertex_alias);
			for (idx_t key_idx = 0; key_idx < edge_table->source_pk.size(); key_idx++) {
				auto source_ref = make_uniq<ColumnRefExpression>(edge_table->source_pk[key_idx], vertex_alias);
				source_ref->alias = source_columns[key_idx];
				branch->select_list.push_back(std::move(source_ref));

				auto destination_ref = make_uniq<ColumnRefExpression>(edge_table->destination_pk[key_idx], vertex_alias);
				destination_ref->alias = destination_columns[key_idx];
				branch->select_list.push_back(std::move(destination_ref));
			}
		} else {
			vector<string> edge_aliases;
			for (int64_t edge_idx = 0; edge_idx < path_length; edge_idx++) {
				auto current_edge_alias =
				    path_alias + "_e_" + std::to_string(path_length) + "_" + std::to_string(edge_idx);
				edge_aliases.push_back(current_edge_alias);
				CrossJoinTableRef(branch->from_table, edge_table->CreateBaseTableRef(current_edge_alias));
			}

			for (int64_t vertex_idx = 0; vertex_idx + 1 < path_length; vertex_idx++) {
				auto vertex_alias = path_alias + "_v_" + std::to_string(path_length) + "_" + std::to_string(vertex_idx);
				CrossJoinTableRef(branch->from_table, edge_table->destination_pg_table->CreateBaseTableRef(vertex_alias));
				branch_conditions.push_back(CreateMatchJoinExpression(edge_table->destination_pk,
				                                                      edge_table->destination_fk, vertex_alias,
				                                                      edge_aliases[static_cast<idx_t>(vertex_idx)]));
				branch_conditions.push_back(CreateMatchJoinExpression(edge_table->source_pk, edge_table->source_fk,
				                                                      vertex_alias,
				                                                      edge_aliases[static_cast<idx_t>(vertex_idx + 1)]));
			}

			for (idx_t key_idx = 0; key_idx < edge_table->source_fk.size(); key_idx++) {
				auto source_ref = make_uniq<ColumnRefExpression>(edge_table->source_fk[key_idx], edge_aliases[0]);
				source_ref->alias = source_columns[key_idx];
				branch->select_list.push_back(std::move(source_ref));

				auto destination_ref =
				    make_uniq<ColumnRefExpression>(edge_table->destination_fk[key_idx], edge_aliases[path_length - 1]);
				destination_ref->alias = destination_columns[key_idx];
				branch->select_list.push_back(std::move(destination_ref));
			}
		}

		branch->where_clause = BuildConjunction(branch_conditions);
		union_node->children.push_back(std::move(branch));
	}

	auto union_select = make_uniq<SelectStatement>();
	union_select->node = std::move(union_node);
	auto union_inner_alias = path_alias + "_union";
	auto union_subquery = make_uniq<SubqueryRef>(std::move(union_select));
	union_subquery->alias = union_inner_alias;

	auto distinct_node = make_uniq<SelectNode>();
	distinct_node->AddDistinct();
	distinct_node->from_table = std::move(union_subquery);
	for (idx_t key_idx = 0; key_idx < source_columns.size(); key_idx++) {
		auto source_ref = make_uniq<ColumnRefExpression>(source_columns[key_idx], union_inner_alias);
		source_ref->alias = source_columns[key_idx];
		distinct_node->select_list.push_back(std::move(source_ref));

		auto destination_ref = make_uniq<ColumnRefExpression>(destination_columns[key_idx], union_inner_alias);
		destination_ref->alias = destination_columns[key_idx];
		distinct_node->select_list.push_back(std::move(destination_ref));
	}
	auto distinct_select = make_uniq<SelectStatement>();
	distinct_select->node = std::move(distinct_node);
	auto distinct_subquery = make_uniq<SubqueryRef>(std::move(distinct_select));
	distinct_subquery->alias = path_alias;
	CrossJoinTableRef(from_clause, std::move(distinct_subquery));

	conditions.push_back(CreateMatchJoinExpression(edge_table->source_pk, source_columns, prev_binding, path_alias));
	conditions.push_back(
	    CreateMatchJoinExpression(edge_table->destination_pk, destination_columns, next_binding, path_alias));
}

void GraphMatchFunction::ProcessPathList(vector<unique_ptr<PathReference>> &path_list,
                                        vector<unique_ptr<ParsedExpression>> &conditions,
                                        unique_ptr<SelectNode> &final_select_node,
                                        case_insensitive_map_t<shared_ptr<PropertyGraphTable>> &alias_map,
                                        CreatePropertyGraphInfo &pg_table, int32_t &extra_alias_counter,
                                        MatchExpression &original_ref) {
	PathElement *previous_vertex_element = GetPathElement(path_list[0]);
	if (!previous_vertex_element) {
		throw NotImplementedException("graph_match: a path may not begin with a quantified subpath");
	}
	auto previous_vertex_table = FindGraphTable(previous_vertex_element->label, pg_table);
	alias_map[previous_vertex_element->variable_binding] = previous_vertex_table;

	for (idx_t idx_j = 1; idx_j < path_list.size(); idx_j = idx_j + 2) {
		PathElement *next_vertex_element = GetPathElement(path_list[idx_j + 1]);
		if (!next_vertex_element) {
			throw NotImplementedException("graph_match: a quantified subpath may not appear on a vertex");
		}
		if (next_vertex_element->match_type != PGQMatchType::MATCH_VERTEX ||
		    previous_vertex_element->match_type != PGQMatchType::MATCH_VERTEX) {
			throw BinderException("Vertex and edge patterns must be alternated.");
		}
		auto next_vertex_table = FindGraphTable(next_vertex_element->label, pg_table);
		alias_map[next_vertex_element->variable_binding] = next_vertex_table;

		PathElement *edge_element = GetPathElement(path_list[idx_j]);
		if (!edge_element) {
			// Quantified (variable-length) edge — a SubPath wrapping one edge element.
			auto edge_subpath = reinterpret_cast<SubPath *>(path_list[idx_j].get());
			if (edge_subpath->path_list.size() > 1) {
				throw NotImplementedException("graph_match: nested subpaths are not supported");
			}
			edge_element = GetPathElement(edge_subpath->path_list[0]);
			auto edge_table = FindGraphTable(edge_element->label, pg_table);

			if (edge_subpath->upper > 1) {
				if (CanExpandBoundedSubpath(edge_table, edge_subpath, edge_element->match_type)) {
					AddBoundedPathExpansion(edge_table, edge_subpath, previous_vertex_element->variable_binding,
					                        edge_element->variable_binding, next_vertex_element->variable_binding,
					                        conditions, final_select_node->from_table, extra_alias_counter);
				} else {
					// TODO: wire the CSR/shortestpath route (AddPathFinding) for
					// left/undirected or large-bound variable-length edges. The CSR
					// scalar functions (create_csr_*, iterativelength, shortestpath) are
					// registered, so this is a contained follow-up; for now only
					// right-directed, bounded (upper <= 16) var-length is lowered.
					throw NotImplementedException(
					    "graph_match: variable-length edges are only supported as right-directed with bounded "
					    "upper limit <= %lld (use -[e:E]->{lo,hi})",
					    static_cast<long long>(MAX_BOUNDED_PATH_EXPANSION_UPPER));
				}
			} else {
				AddEdgeJoins(edge_table, previous_vertex_table, next_vertex_table, edge_element->match_type,
				             edge_element->variable_binding, previous_vertex_element->variable_binding,
				             next_vertex_element->variable_binding, conditions, alias_map, extra_alias_counter,
				             final_select_node->from_table);
			}
		} else {
			auto edge_table = FindGraphTable(edge_element->label, pg_table);
			AddEdgeJoins(edge_table, previous_vertex_table, next_vertex_table, edge_element->match_type,
			             edge_element->variable_binding, previous_vertex_element->variable_binding,
			             next_vertex_element->variable_binding, conditions, alias_map, extra_alias_counter,
			             final_select_node->from_table);
		}
		previous_vertex_element = next_vertex_element;
		previous_vertex_table = next_vertex_table;
	}
	(void)original_ref;
}

//------------------------------------------------------------------------------
// BindReplace: synthesize the property graph from args, parse, rewrite.
//------------------------------------------------------------------------------
unique_ptr<TableRef> GraphMatchFunction::GraphMatchBindReplace(ClientContext &context,
                                                              TableFunctionBindInput &input) {
	// graph_match(pattern, vertex_table, vertex_id, edge_table, src, dst)
	auto pattern = StringValue::Get(input.inputs[0]);
	auto vertex_table = StringUtil::Lower(StringValue::Get(input.inputs[1]));
	auto vertex_id = StringUtil::Lower(StringValue::Get(input.inputs[2]));
	auto edge_table = StringUtil::Lower(StringValue::Get(input.inputs[3]));
	auto src_col = StringUtil::Lower(StringValue::Get(input.inputs[4]));
	auto dst_col = StringUtil::Lower(StringValue::Get(input.inputs[5]));
	(void)context;

	// 1. Parse the pattern string into a MatchExpression + the distinct labels.
	vector<string> vertex_labels;
	vector<string> edge_labels;
	auto match_expr = ParseGraphPattern(pattern, vertex_labels, edge_labels);

	if (edge_labels.size() != 1 || vertex_labels.size() != 1) {
		throw NotImplementedException(
		    "graph_match: exactly one vertex label and one edge label are supported per call "
		    "(got %llu vertex / %llu edge labels); call once per distinct label pair",
		    static_cast<unsigned long long>(vertex_labels.size()),
		    static_cast<unsigned long long>(edge_labels.size()));
	}

	// 2. Synthesize a CreatePropertyGraphInfo: one vertex table + one edge table,
	//    populated exactly like MakeEdgeSpec, keyed by the parsed labels so
	//    FindGraphTable resolves. Source and destination vertices are the same
	//    vertex table; the CSR/join builders disambiguate via the bindings.
	auto edge_spec = MakeEdgeSpec(vertex_table, vertex_id, edge_table, src_col, dst_col);
	edge_spec->main_label = edge_labels[0];

	auto vertex_spec = make_shared_ptr<PropertyGraphTable>();
	vertex_spec->table_name = vertex_table;
	vertex_spec->is_vertex_table = true;
	vertex_spec->main_label = vertex_labels[0];
	vertex_spec->source_pk = {vertex_id};
	vertex_spec->destination_pk = {vertex_id};

	CreatePropertyGraphInfo pg_info("__graph_match");
	pg_info.vertex_tables.push_back(vertex_spec);
	pg_info.edge_tables.push_back(edge_spec);
	pg_info.label_map[vertex_labels[0]] = vertex_spec;
	pg_info.label_map[edge_labels[0]] = edge_spec;

	// 3. Run the ported rewrite.
	vector<unique_ptr<ParsedExpression>> conditions;
	auto final_select_node = make_uniq<SelectNode>();
	case_insensitive_map_t<shared_ptr<PropertyGraphTable>> alias_map;
	int32_t extra_alias_counter = 0;

	for (auto &path_pattern : match_expr->path_patterns) {
		ProcessPathList(path_pattern->path_elements, conditions, final_select_node, alias_map, pg_info,
		                extra_alias_counter, *match_expr);
	}

	// Cross-join all the vertex/edge relations encountered.
	for (auto &table_alias_entry : alias_map) {
		auto table_ref = table_alias_entry.second->CreateBaseTableRef();
		table_ref->alias = table_alias_entry.first;
		if (final_select_node->from_table) {
			auto new_root = make_uniq<JoinRef>(JoinRefType::CROSS);
			new_root->left = std::move(final_select_node->from_table);
			new_root->right = std::move(table_ref);
			final_select_node->from_table = std::move(new_root);
		} else {
			final_select_node->from_table = std::move(table_ref);
		}
	}

	// Projection. Because the result is wrapped in a SubqueryRef, the inner
	// relation aliases (a, b, e, ...) are not visible to the outer query, so we
	// expose a stable, collision-free set of output columns named
	// "<binding>_<column>" for the key columns of every bound relation:
	//   - vertex binding  -> "<binding>_<vertex_id>"
	//   - edge binding    -> "<binding>_<src>" and "<binding>_<dst>"
	// (Undirected/ANY edges are not bound and therefore not projected.) This is
	// the contract DFL codegen selects from. Ordered by binding name for
	// determinism.
	vector<string> ordered_bindings;
	ordered_bindings.reserve(alias_map.size());
	for (auto &entry : alias_map) {
		ordered_bindings.push_back(entry.first);
	}
	std::sort(ordered_bindings.begin(), ordered_bindings.end());
	for (auto &binding : ordered_bindings) {
		const auto &tbl = alias_map[binding];
		if (tbl->is_vertex_table) {
			auto colref = make_uniq<ColumnRefExpression>(tbl->source_pk[0], binding);
			colref->alias = binding + "_" + tbl->source_pk[0];
			final_select_node->select_list.push_back(std::move(colref));
		} else {
			auto src_ref = make_uniq<ColumnRefExpression>(tbl->source_fk[0], binding);
			src_ref->alias = binding + "_" + tbl->source_fk[0];
			final_select_node->select_list.push_back(std::move(src_ref));
			auto dst_ref = make_uniq<ColumnRefExpression>(tbl->destination_fk[0], binding);
			dst_ref->alias = binding + "_" + tbl->destination_fk[0];
			final_select_node->select_list.push_back(std::move(dst_ref));
		}
	}
	if (final_select_node->select_list.empty()) {
		final_select_node->select_list.push_back(make_uniq<StarExpression>());
	}
	final_select_node->where_clause = CreateWhereClause(conditions);

	auto subquery = make_uniq<SelectStatement>();
	subquery->node = std::move(final_select_node);
	auto result = make_uniq<SubqueryRef>(std::move(subquery), "graph_match");
	return std::move(result);
}

//------------------------------------------------------------------------------
// Register
//------------------------------------------------------------------------------
void CoreTableFunctions::RegisterGraphMatchTableFunction(ExtensionLoader &loader) {
	loader.RegisterFunction(GraphMatchFunction());
}

} // namespace duckdb
