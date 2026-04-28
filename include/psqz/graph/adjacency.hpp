// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#if __has_include(<metall/metall.hpp>)
#include <psqz/metall.hpp>
#endif

#include <psqz/graph/graph.hpp>
#include <psqz/utils/spawner.hpp>

#include <ygm/comm.hpp>

#include <cmath>
#include <sstream>

namespace psqz::graph {

constexpr auto insert_lambda = [](const auto &idx, auto &adj, const auto &elt) {
  adj.push_back(elt);
};

template <typename VecType>
constexpr VecType get_default(const std::size_t vertex_count) {
  VecType default_value{};
  default_value.reserve(static_cast<std::size_t>(__builtin_clz(vertex_count)));
  return default_value;
};

template <typename OptionsType>
struct square_undirected_adjacency {
  using options_type = OptionsType;

  using index_type         = typename options_type::index_type;
  using weight_type        = typename options_type::weight_type;
  using index_vec_type     = typename options_type::index_vec_type;
  using edge_type          = edge<index_type, weight_type>;
  using adjacency_elt_type = std::pair<index_type, weight_type>;
  using adjacency_vec_type =
      typename options_type::vector_type<adjacency_elt_type>;

  template <typename IndexT, typename VecT>
  using container_type = typename options_type::container_type<IndexT, VecT>;

  using row_container_type = container_type<index_type, adjacency_vec_type>;

 private:
  row_container_type _row_container;

 public:
  square_undirected_adjacency(ygm::comm &comm, std::size_t vertex_count)
      : _row_container(
            psqz::spawn<container_type, index_type, adjacency_vec_type>(
                comm, get_default<adjacency_vec_type>(vertex_count),
                vertex_count)) {}

  ygm::comm &comm() { return _row_container.comm(); }

  std::size_t row_count() { return _row_container.size(); }
  std::size_t col_count() { return _row_container.size(); }

  template <typename... Args>
  void for_all_rows(Args &&...args) {
    _row_container.for_all(args...);
  }

  template <typename... Args>
  void for_all_cols(Args &&...args) {
    _row_container.for_all_cols(args...);
  }

  template <typename... Args>
  void async_insert_edge(const edge_type &edge, Args &...args) {
    _row_container.async_visit(edge.src, insert_lambda,
                               adjacency_elt_type{edge.dst, edge.wgt});
    _row_container.async_visit(edge.dst, insert_lambda,
                               adjacency_elt_type{edge.src, edge.wgt});
  }

  template <typename... Args>
  void local_row_visit(const index_type idx, Args &&...args) {
    _row_container.local_visit(idx, args...);
  }

  template <typename... Args>
  void local_col_visit(const index_type idx, Args &&...args) {
    _row_container.local_visit(idx, args...);
  }
};
}  // namespace psqz::graph
