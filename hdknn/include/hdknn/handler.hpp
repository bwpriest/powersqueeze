// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/handler.hpp>

#include <ygm/container/map.hpp>
#include <ygm/container/set.hpp>

namespace hdknn {

namespace detail {

template <typename IndexType, typename DistType>
struct base_knn_handler {
  using dist_type         = DistType;
  using neighbor_type     = std::pair<IndexType, dist_type>;
  using neighborhood_type = std::vector<neighbor_type>;
  using neighborhood_container_type =
      ygm::container::map<IndexType, neighborhood_type>;
  using query_type = ygm::container::set<IndexType>;
};

template <typename KnnHandlerType, typename OptionsType>
struct base_knn_handler_checker {
  static_assert(std::is_same<typename KnnHandlerType::neighbor_type::first_type,
                             typename OptionsType::index_type>::value);
};
}  // namespace detail

template <typename OptionsType, typename ParametersType, typename TruthType,
          typename DistType = float>
class handler_with_truth
    : public psqz::detail::base_handler<ParametersType>,
      public OptionsType,
      public detail::base_knn_handler<typename OptionsType::index_type,
                                      DistType>,
      public psqz::detail::base_truth_handler<TruthType> {
  psqz::detail::base_truth_handler_checker<
      psqz::detail::base_truth_handler<TruthType>, OptionsType>
      truth_checker;
  detail::base_knn_handler_checker<
      detail::base_knn_handler<typename OptionsType::index_type, DistType>,
      OptionsType>
      knn_checker;

 public:
  using base_type       = psqz::detail::base_handler<ParametersType>;
  using parameters_type = base_type::parameters_type;
  handler_with_truth(ygm::comm &comm, const parameters_type &params)
      : base_type(comm, params) {}
};

}  // namespace hdknn
