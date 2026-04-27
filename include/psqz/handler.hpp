// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/utils/metrics.hpp>
#include <psqz/utils/spawner.hpp>

#include <ygm/container/set.hpp>
#include <ygm/utility/timer.hpp>

#include <krowkee/cereal/eigen.hpp>

#include <Eigen/Dense>

#include <iomanip>

namespace psqz {
template <typename ParametersType, typename SketchType, typename AdjacencyType,
          typename TruthType>
class handler {
 public:
  using parameters_type = ParametersType;

  using sketch_type    = SketchType;
  using adjacency_type = AdjacencyType;
  using truth_type     = TruthType;

  static_assert(std::is_same<typename adjacency_type::index_type,
                             typename truth_type::key_type>::value);
  // static_assert(std::is_same<typename adjacency_type::weight_type,
  //                            typename sketch_type::register_type>()::value);

  using feature_type = typename sketch_type::register_type;

  using cmty_type = truth_type::mapped_type;

  using index_type         = typename adjacency_type::index_type;
  using weight_type        = typename adjacency_type::weight_type;
  using index_vec_type     = typename adjacency_type::index_vec_type;
  using adjacency_elt_type = std::pair<index_type, weight_type>;
  using adjacency_vec_type =
      typename adjacency_type::vector_type<adjacency_elt_type>;

  using query_type       = ygm::container::set<index_type>;
  using feature_vec_type = typename sketch_type::registers_type;
  using sketch_container_type =
      typename adjacency_type::container_type<index_type, feature_vec_type>;
  using degree_type = float;
  using degree_container_type =
      typename adjacency_type::container_type<index_type, degree_type>;
  using metrics_type = Metrics;
  using timer_type   = ygm::utility::timer;

 protected:
  ygm::comm             &_comm;
  const parameters_type &_params;
  metrics_type           _metrics;
  timer_type             _timer;

 public:
  handler(ygm::comm &comm, const parameters_type &params)
      : _comm(comm), _params(params), _metrics(), _timer() {}

  ygm::comm             &comm() { return _comm; }
  const parameters_type &params() { return _params; }

  template <typename T>
  void chirp_line(const std::string &name, const T val) {
    if (_params.verbose()) {
      std::stringstream ss;
      ss << std::setw(50) << std::right << name << " -- " << val;
      _comm.cout0(ss.str());
    }
  }

  void chirp_metric(const std::string name) {
    auto time = _timer.elapsed();
    _metrics.set(name, time);
    chirp_line(name, time);
    _timer.reset();
  }

  template <typename T>
  void chirp_metric(const std::string name, const T val) {
    _metrics.set(name, val);
    chirp_line(name, val);
  }

  void repeat_metrics() {
    std::stringstream ss;
    ss << _params.csv_names() << "," << csv_metric_names(_metrics) << std::endl
       << _params.csv_values() << "," << csv_metric_vals(_metrics);
    _comm.cout0(ss.str());
  }

  void reset_timer() { _timer.reset(); }
};
}  // namespace psqz
