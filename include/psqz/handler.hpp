// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/utils/metrics.hpp>

#include <ygm/comm.hpp>
#include <ygm/utility/timer.hpp>

#include <Eigen/Dense>

#include <krowkee/cereal/eigen.hpp>

#include <iomanip>

namespace psqz {

namespace detail {
template <typename ParametersType>
class base_handler {
 public:
  using parameters_type = ParametersType;
  using metrics_type    = Metrics;
  using timer_type      = ygm::utility::timer;

 protected:
  ygm::comm             &_comm;
  const parameters_type &_params;
  metrics_type           _metrics;
  timer_type             _timer;

 public:
  base_handler(ygm::comm &comm, const parameters_type &params)
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

template <typename AdjacencyType>
struct base_adjacency_handler {
  using adjacency_type     = AdjacencyType;
  using index_type         = typename adjacency_type::index_type;
  using feature_type       = typename adjacency_type::weight_type;
  using weight_type        = typename adjacency_type::weight_type;
  using index_vec_type     = typename adjacency_type::index_vec_type;
  using adjacency_elt_type = std::pair<index_type, weight_type>;
  using adjacency_vec_type =
      typename adjacency_type::vector_type<adjacency_elt_type>;

  using feature_vec_type = Eigen::Vector<feature_type, Eigen::Dynamic>;

  using sketch_container_type =
      typename adjacency_type::container_type<index_type, feature_vec_type>;
};

template <typename TruthType>
struct base_truth_handler {
  using truth_type = TruthType;
  using cmty_type  = truth_type::mapped_type;
};

template <typename SketchType>
struct base_sketch_handler {
  using sketch_type = SketchType;
};

template <typename TruthHandlerType, typename AdjacencyHandlerType>
struct base_truth_handler_checker {
  static_assert(
      std::is_same<typename AdjacencyHandlerType::index_type,
                   typename TruthHandlerType::truth_type::key_type>::value);
  static_assert(
      std::is_same<typename AdjacencyHandlerType::index_type,
                   typename TruthHandlerType::truth_type::mapped_type>::value);
};

template <typename SketchHandlerType, typename AdjacencyHandlerType>
struct base_sketch_handler_checker {
  static_assert(std::is_same<
                typename AdjacencyHandlerType::feature_type,
                typename SketchHandlerType::sketch_type::register_type>::value);
  static_assert(
      std::is_same<
          typename AdjacencyHandlerType::feature_vec_type,
          typename SketchHandlerType::sketch_type::registers_type>::value);
};

}  // namespace detail

template <typename SketchType, typename ParametersType, typename AdjacencyType,
          typename TruthType>
class handler_with_truth : public detail::base_handler<ParametersType>,
                           public detail::base_sketch_handler<SketchType>,
                           public detail::base_adjacency_handler<AdjacencyType>,
                           public detail::base_truth_handler<TruthType> {
  detail::base_sketch_handler_checker<
      detail::base_sketch_handler<SketchType>,
      detail::base_adjacency_handler<AdjacencyType>>
      sketch_checker;
  detail::base_truth_handler_checker<
      detail::base_truth_handler<TruthType>,
      detail::base_adjacency_handler<AdjacencyType>>
      truth_checker;

 public:
  using base_type       = detail::base_handler<ParametersType>;
  using parameters_type = typename base_type::parameters_type;
  handler_with_truth(ygm::comm &comm, const parameters_type &params)
      : base_type(comm, params) {}
};
}  // namespace psqz
