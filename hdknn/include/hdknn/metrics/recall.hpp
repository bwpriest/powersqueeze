// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <hdknn/handler.hpp>

#include <ygm/comm.hpp>
#include <ygm/detail/collective.hpp>

#include <numeric>

namespace ygm::detail {

template <typename T>
std::vector<T> sum(const std::vector<T> &value, comm &c) {
  std::size_t    size = value.size();
  std::vector<T> to_return(size);
  c.barrier();
  MPI_Comm mpi_comm = c.get_mpi_comm();
  YGM_ASSERT_MPI(MPI_Allreduce(&value[0], &to_return[0], size,
                               detail::mpi_typeof(T()), MPI_SUM, mpi_comm));
  return to_return;
}
}  // namespace ygm::detail

namespace hdknn::metric {

template <typename HandlerType>
struct recall {
  using handler_type      = HandlerType;
  using index_type        = typename handler_type::index_type;
  using cmty_type         = typename handler_type::cmty_type;
  using parameters_type   = typename handler_type::parameters_type;
  using truth_type        = typename handler_type::truth_type;
  using query_type        = typename handler_type::query_type;
  using neighbor_type     = typename handler_type::neighbor_type;
  using neighborhood_type = typename handler_type::neighborhood_type;
  using neighborhood_container_type =
      typename handler_type::neighborhood_container_type;

 private:
  handler_type          &_handler;
  ygm::comm             &_comm;
  const parameters_type &_params;

 public:
  recall(handler_type &handler)
      : _handler(handler), _comm(_handler.comm()), _params(_handler.params()) {}

  void operator()(truth_type &truth, query_type &queries,
                  neighborhood_container_type &nbhd_container) {
    std::size_t cmty_count{0};
    truth.for_all([&cmty_count](const index_type &idx, const cmty_type &cmty) {
      if (cmty > cmty_count) {
        cmty_count = cmty;
      }
    });
    cmty_count = ygm::max(cmty_count, _comm) + 1;
    static std::vector<std::size_t> cmty_sizes(cmty_count);
    truth.for_all([](const index_type &idx, const cmty_type &cmty) {
      ++cmty_sizes[cmty];
    });
    cmty_sizes = ygm::detail::sum(cmty_sizes, _comm);

    std::size_t query_count = queries.size();

    ygm::container::map<index_type, std::pair<std::size_t, std::size_t>>
        recall_map(_comm);

    static std::size_t total_queries(0);

    queries.for_all([&truth, &nbhd_container,
                     &recall_map](const index_type &src_idx) {
      truth.async_visit_if_contains(
          src_idx,
          [](const index_type &src_idx, const cmty_type &src_cmty,
             auto truth_ptr, auto recall_ptr, auto nbhd_ptr) {
            recall_ptr->async_visit(
                src_idx,
                [](const index_type                    &src_idx,
                   std::pair<std::size_t, std::size_t> &recall_pair,
                   const cmty_type                     &src_cmty) {
                  recall_pair.second = cmty_sizes[src_cmty];
                },
                src_cmty);

            nbhd_ptr->async_visit(
                src_idx,
                [](const index_type &src_idx, const neighborhood_type &nbhd,
                   const cmty_type &src_cmty, auto truth_ptr, auto recall_ptr) {
                  for (const neighbor_type &nbr : nbhd) {
                    const index_type &nbr_idx = nbr.first;
                    truth_ptr->async_visit_if_contains(
                        nbr_idx,
                        [](const index_type &nbr_idx, const cmty_type &nbr_cmty,
                           const index_type src_idx, const cmty_type &src_cmty,
                           auto recall_ptr) {
                          if (nbr_cmty == src_cmty) {
                            recall_ptr->async_visit(
                                src_idx,
                                [](const index_type &src_idx,
                                   std::pair<std::size_t, std::size_t>
                                       &recall_pair) { ++recall_pair.first; });
                          }
                        },
                        src_idx, src_cmty, recall_ptr);
                  }
                },
                src_cmty, truth_ptr, recall_ptr);
          },
          truth.get_ygm_ptr(), recall_map.get_ygm_ptr(),
          nbhd_container.get_ygm_ptr());
    });

    _comm.barrier();
    _handler.chirp_metric("recall time");

    float mean_recall{0.0f};
    recall_map.for_all(
        [&mean_recall](const index_type                          &idx,
                       const std::pair<std::size_t, std::size_t> &recall_pair) {
          mean_recall += recall_pair.first / (float)recall_pair.second;
        });
    mean_recall = ygm::sum(mean_recall, _comm);
    mean_recall /= (float)query_count;

    total_queries = ygm::sum(total_queries, _comm);
    _handler.chirp_metric("mean recall", mean_recall);
    float mean_cmty_size =
        std::accumulate(cmty_sizes.begin(), cmty_sizes.end(), 0) /
        (float)cmty_sizes.size();
    _handler.chirp_metric("mean community size", mean_cmty_size);
  }
};

}  // namespace hdknn::metric
