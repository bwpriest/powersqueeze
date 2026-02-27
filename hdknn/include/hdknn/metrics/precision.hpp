// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <ygm/comm.hpp>
#include <ygm/detail/collective.hpp>

namespace hdknn::metric {

template <typename HandlerType>
struct precision {
  using handler_type      = HandlerType;
  using index_type        = typename handler_type::index_type;
  using cmty_type         = typename handler_type::cmty_type;
  using truth_type        = typename handler_type::truth_type;
  using query_type        = typename handler_type::query_type;
  using neighbor_type     = typename handler_type::neighbor_type;
  using neighborhood_type = typename handler_type::neighborhood_type;
  using neighborhood_container_type =
      typename handler_type::neighborhood_container_type;

 private:
  handler_type &_handler;
  ygm::comm    &_comm;

 public:
  precision(handler_type &handler)
      : _handler(handler), _comm(_handler.comm()) {}

  void operator()(truth_type &truth, query_type &queries,
                  neighborhood_container_type &nbhd_container) {
    std::size_t        total_queries = queries.size();
    static std::size_t true_positives{0};
    static std::size_t total_neighbors{0};

    auto nbhd_ptr = nbhd_container.get_ygm_ptr();
    queries.for_all([&truth, &nbhd_ptr](const index_type &src_idx) {
      truth.async_visit_if_contains(
          src_idx,
          [](const index_type &src_idx, const cmty_type &src_cmty,
             auto nbhd_ptr, auto truth_ptr) {
            nbhd_ptr->async_visit(
                src_idx,
                [](const index_type &src_idx, const neighborhood_type &nbhd,
                   const cmty_type &src_cmty, auto truth_ptr) {
                  for (const neighbor_type &nbr : nbhd) {
                    const index_type &nbr_idx = nbr.first;
                    ++total_neighbors;
                    truth_ptr->async_visit_if_contains(
                        nbr_idx,
                        [](const index_type &nbr_idx, const cmty_type &nbr_cmty,
                           const cmty_type &src_cmty) {
                          if (nbr_cmty == src_cmty) {
                            ++true_positives;
                          }
                        },
                        src_cmty);
                  }
                },
                src_cmty, truth_ptr);
          },
          nbhd_ptr, truth.get_ygm_ptr());
    });

    _comm.barrier();
    _handler.chirp_metric("precision time");

    true_positives  = ygm::sum(true_positives, _comm);
    total_neighbors = ygm::sum(total_neighbors, _comm);
    auto precision  = true_positives / ((float)total_neighbors);
    _handler.chirp_metric("precision", precision);

    true_positives  = 0;
    total_neighbors = 0;
  }
};

}  // namespace hdknn::metric
