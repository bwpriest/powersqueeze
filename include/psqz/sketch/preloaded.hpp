// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <type_traits>

namespace psqz::sketch::preloaded {

template <typename AdjacencyViewType, typename SketchContainerType>
void spMV(AdjacencyViewType   &adjacency_view,
          SketchContainerType &current_sketch,
          SketchContainerType &next_sketch) {
  using index_type         = AdjacencyViewType::index_type;
  using adjacency_vec_type = AdjacencyViewType::adjacency_vec_type;
  using adjacency_elt_type = AdjacencyViewType::adjacency_elt_type;
  using weight_type        = AdjacencyViewType::weight_type;
  using feature_vec_type   = SketchContainerType::mapped_type;
  using feature_type       = feature_vec_type::value_type;
  static_assert(
      std::is_same<index_type, typename SketchContainerType::key_type>());
  static_assert(
      std::is_same<index_type, typename adjacency_elt_type::first_type>());

  auto next_sketch_ptr = next_sketch.get_ygm_ptr();

  static std::unordered_map<index_type, feature_vec_type> buffer;

  auto kv_lambda = [&adjacency_view, next_sketch_ptr](
                       const index_type       &col_idx,
                       const feature_vec_type &col_sketch) {
    auto csc_visit_lambda =
        [](const index_type &col_idx, const adjacency_vec_type &col_adj,
           const feature_vec_type &col_sketch, const auto next_sketch_ptr) {
          for (const adjacency_elt_type &row : col_adj) {
            const index_type &row_idx = row.first;
            auto [itr, inserted]      = buffer.try_emplace(row_idx, col_sketch);
            if (!inserted) {
              itr->second += col_sketch;
            }
          }
        };
    adjacency_view.container().local_visit(col_idx, csc_visit_lambda,
                                           col_sketch, next_sketch_ptr);
  };

  current_sketch.for_all(kv_lambda);

  current_sketch.comm().barrier();

  for (const auto [row_idx, sum_sketch] : buffer) {
    next_sketch.async_visit(
        row_idx,
        [](const index_type &row_idx, feature_vec_type &row_sketch,
           const feature_vec_type &sum_sketch) { row_sketch += sum_sketch; },
        sum_sketch);
  }

  current_sketch.comm().barrier();

  buffer.clear();
}
}  // namespace psqz::sketch::preloaded
