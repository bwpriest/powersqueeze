// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <ygm/comm.hpp>
#include <ygm/detail/collective.hpp>

namespace hdknn::metric {

template <typename HandlerType>
struct search_depth {
  using handler_type                = HandlerType;
  using index_type                  = handler_type::index_type;
  using cmty_type                   = handler_type::cmty_type;
  using parameters_type             = handler_type::parameters_type;
  using truth_type                  = handler_type::truth_type;
  using query_type                  = handler_type::query_type;
  using neighbor_type               = handler_type::neighbor_type;
  using neighborhood_type           = handler_type::neighborhood_type;
  using neighborhood_container_type = handler_type::neighborhood_container_type;

 private:
  handler_type          &_handler;
  ygm::comm             &_comm;
  const parameters_type &_params;

 public:
  search_depth(handler_type &handler)
      : _handler(handler), _comm(_handler.comm()), _params(_handler.params()) {}

  void operator()(truth_type &truth, query_type &queries,
                  neighborhood_container_type &nbhd_container) {
    std::size_t total_queries = queries.size();

    ygm::container::map<index_type, std::size_t> depth_map(_comm);
    queries.for_all([this, &depth_map](const index_type &idx) {
      depth_map.async_insert(idx, this->_params.nn_query());
    });
    _comm.barrier();

    queries.for_all([&truth, &nbhd_container,
                     &depth_map](const index_type &src_idx) {
      // Currently assumes that if we're querying for a point, then we know that
      // there is ground truth
      truth.async_visit_if_contains(
          src_idx,
          [](const index_type &src_idx, const cmty_type &src_cmty,
             auto truth_ptr, auto nbhd_ptr, auto depth_ptr) {
            nbhd_ptr->async_visit(
                src_idx,
                [](const index_type &src_idx, const neighborhood_type &nbhd,
                   const cmty_type &src_cmty, auto truth_ptr, auto depth_ptr) {
                  std::size_t current_depth{0};
                  for (const neighbor_type &nbr : nbhd) {
                    const cmty_type &nbr_idx = nbr.first;
                    truth_ptr->async_visit_if_contains(
                        nbr_idx,
                        [](const index_type &nbr_idx, const cmty_type &nbr_cmty,
                           const index_type &src_idx, const cmty_type &src_cmty,
                           const std::size_t &current_depth, auto depth_ptr) {
                          if (src_cmty == nbr_cmty) {
                            depth_ptr->async_visit(
                                src_idx,
                                [](const index_type  &src_idx,
                                   std::size_t       &depth,
                                   const std::size_t &current_depth) {
                                  if (current_depth < depth) {
                                    depth = current_depth;
                                  }
                                },
                                current_depth);
                          }
                        },
                        src_idx, src_cmty, current_depth, depth_ptr);
                  }
                },
                src_cmty, truth_ptr, depth_ptr);
          },
          truth.get_ygm_ptr(), nbhd_container.get_ygm_ptr(),
          depth_map.get_ygm_ptr());
    });

    _comm.barrier();
    _handler.chirp_metric("search depth time");

    float mean_depth{0.0f};
    depth_map.for_all(
        [&mean_depth](const index_type &idx, const std::size_t &depth) {
          mean_depth += depth;
        });
    mean_depth = ygm::sum(mean_depth, _comm);
    mean_depth /= (float)total_queries;

    _handler.chirp_metric("total queries", total_queries);
    _handler.chirp_metric("mean search depth", mean_depth);
  }
};

}  // namespace hdknn::metric
