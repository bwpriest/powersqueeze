// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/handler.hpp>

#include <ygm/container/map.hpp>
#include <ygm/container/set.hpp>

namespace hdknn {

template <typename ParametersType, typename AdjacencyType, typename TruthType,
          typename DistType = float>
class handler
    : public psqz::detail::base_handler_with_truth<ParametersType,
                                                   AdjacencyType, TruthType> {
 public:
  using base_type =
      psqz::detail::base_handler_with_truth<ParametersType, AdjacencyType,
                                            TruthType>;

  using parameters_type = typename base_type::parameters_type;

  using index_type = typename base_type::index_type;

  using dist_type         = DistType;
  using neighbor_type     = std::pair<index_type, dist_type>;
  using neighborhood_type = std::vector<neighbor_type>;
  using neighborhood_container_type =
      ygm::container::map<index_type, neighborhood_type>;
  using query_type = ygm::container::set<index_type>;

 public:
  handler(ygm::comm &comm, const parameters_type &params)
      : base_type(comm, params) {}
};

}  // namespace hdknn
