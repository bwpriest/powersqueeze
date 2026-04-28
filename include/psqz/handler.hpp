// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/utils/metrics.hpp>

#include <ygm/comm.hpp>
#include <ygm/utility/timer.hpp>

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
  using adjacency_elt_type = adjacency_type::adjacency_elt_type;
  using adjacency_vec_type = adjacency_type::adjacency_vec_type;
};

template <typename TruthType>
struct base_truth_handler {
  using truth_type = TruthType;
  using cmty_type  = truth_type::mapped_type;
};

template <typename TruthHandlerType, typename OptionsType>
struct base_truth_handler_checker {
  static_assert(
      std::is_same<typename OptionsType::index_type,
                   typename TruthHandlerType::truth_type::key_type>::value);
  static_assert(std::is_same<typename OptionsType::index_type,
                             typename TruthHandlerType::cmty_type>::value);
};

template <typename SketchType, typename OptionsType>
struct base_sketch_handler_checker {
  static_assert(std::is_same<typename OptionsType::feature_type,
                             typename SketchType::register_type>::value);
  static_assert(std::is_same<typename OptionsType::feature_vec_type,
                             typename SketchType::registers_type>::value);
};

}  // namespace detail

template <typename OptionsType, typename SketchType, typename ParametersType,
          template <typename> class AdjacencyType, typename TruthType>
class handler_with_truth
    : public detail::base_handler<ParametersType>,
      public OptionsType,
      public detail::base_adjacency_handler<AdjacencyType<OptionsType>>,
      public detail::base_truth_handler<TruthType> {
 public:
  using sketch_type = SketchType;

 private:
  detail::base_sketch_handler_checker<sketch_type, OptionsType> sketch_checker;
  detail::base_truth_handler_checker<detail::base_truth_handler<TruthType>,
                                     OptionsType>
      truth_checker;

 public:
  using base_type       = detail::base_handler<ParametersType>;
  using parameters_type = base_type::parameters_type;
  handler_with_truth(ygm::comm &comm, const parameters_type &params)
      : base_type(comm, params) {}
};
}  // namespace psqz
