// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/graph/graph.hpp>
#include <psqz/utils/spawner.hpp>

#include <Eigen/Dense>

#include <krowkee/cereal/eigen.hpp>

namespace psqz::graph {

template <template <typename, typename> class ContainerType = psqz::ygm_array,
          template <typename> class VecType                 = std::vector,
          typename IndexType = std::size_t, typename WeightType = float>
struct options {
  template <typename T>
  using vector_type = VecType<T>;

  template <typename IndexT, typename VecT>
  using container_type = ContainerType<IndexT, VecT>;

  using index_type     = IndexType;
  using weight_type    = WeightType;
  using feature_type   = weight_type;
  using index_vec_type = vector_type<index_type>;

  using feature_vec_type = Eigen::Vector<feature_type, Eigen::Dynamic>;

  using sketch_container_type = container_type<index_type, feature_vec_type>;
};

}  // namespace psqz::graph
