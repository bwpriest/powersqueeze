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
struct adjacency_view {
  using options_type = OptionsType;

  using index_type         = options_type::index_type;
  using weight_type        = options_type::weight_type;
  using index_vec_type     = options_type::index_vec_type;
  using edge_type          = edge<index_type, weight_type>;
  using adjacency_elt_type = std::pair<index_type, weight_type>;
  using adjacency_vec_type =
      typename options_type::vector_type<adjacency_elt_type>;

  template <typename IndexT, typename VecT>
  using container_type = typename options_type::container_type<IndexT, VecT>;

  using row_container_type = container_type<index_type, adjacency_vec_type>;

 private:
  row_container_type &_container;

 public:
  adjacency_view(row_container_type &container) : _container(container) {}

  row_container_type &container() { return _container; }
};

template <typename OptionsType>
struct square_undirected_adjacency {
  using options_type = OptionsType;
  using view_type    = adjacency_view<OptionsType>;

  using index_type         = view_type::index_type;
  using weight_type        = view_type::weight_type;
  using index_vec_type     = view_type::index_vec_type;
  using edge_type          = view_type::edge_type;
  using adjacency_elt_type = view_type::adjacency_elt_type;
  using adjacency_vec_type = view_type::adjacency_vec_type;

  template <typename IndexT, typename VecT>
  using container_type = typename options_type::container_type<IndexT, VecT>;

  using row_container_type = view_type::row_container_type;

 private:
  row_container_type _row_container;
  view_type          _row_view;

 public:
  square_undirected_adjacency(ygm::comm &comm, std::size_t vertex_count)
      : _row_container(
            psqz::spawn<container_type, index_type, adjacency_vec_type>(
                comm, get_default<adjacency_vec_type>(vertex_count),
                vertex_count)),
        _row_view(_row_container) {}

  ygm::comm &comm() { return _row_container.comm(); }

  row_container_type &row_container() { return _row_view.container(); }
  row_container_type &col_container() { return _row_view.container(); }

  view_type &row_view() { return _row_view; }
  view_type &col_view() { return _row_view; }

  static constexpr std::string name() { return "square_undirected"; }
  static constexpr bool        rectangular() { return false; }

  template <typename... Args>
  void async_insert_edge(const edge_type &edge, Args &...args) {
    _row_container.async_visit(edge.src, insert_lambda,
                               adjacency_elt_type{edge.dst, edge.wgt});
    _row_container.async_visit(edge.dst, insert_lambda,
                               adjacency_elt_type{edge.src, edge.wgt});
  }
};
}  // namespace psqz::graph
