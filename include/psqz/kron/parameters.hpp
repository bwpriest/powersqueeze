// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/utils/parameters.hpp>

#include <unistd.h>

namespace psqz::kron {

namespace detail {
template <typename BaseType>
struct parameters : public BaseType {
  using base_type = BaseType;

  parameter<std::string> left_index_filename;
  parameter<std::string> right_index_filename;
  parameter<std::string> left_truth_filename;
  parameter<std::string> right_truth_filename;
  parameter<float>       target_degree_power;
  parameter<float>       noise_ratio;

  parameters()
      : base_type(),
        left_index_filename(
            "left_index_filename",
            "File containing data for left kronecker graph (required)", 'i',
            true, ""),
        right_index_filename(
            "right_index_filename",
            "File containing data for right kronecker graph (required)", 'I',
            true, ""),
        left_truth_filename("left_truth_filename",
                            "File containing community data for left kronecker "
                            "graph (required)",
                            'g', true, ""),
        right_truth_filename("right_truth_filename",
                             "File containing community data for right "
                             "kronecker graph (required)",
                             'G', true, ""),
        target_degree_power(
            "target_degree_power",
            "power (a>1.0) of expected mean degree log(|V(A)| * |V(B)|)^a", 'a',
            true, 1.4),
        noise_ratio(
            "noise_ratio",
            "ratio (b>1.0) of intra-community to inter-community preservation",
            'b', true, 4.0) {
    this->_params.push_back(&left_index_filename);
    this->_params.push_back(&right_index_filename);
    this->_params.push_back(&left_truth_filename);
    this->_params.push_back(&right_truth_filename);
    this->_params.push_back(&target_degree_power);
    this->_params.push_back(&noise_ratio);
  }

  bool _help_needed() const override {
    bool ret = base_type::_help_needed();
    if (left_index_filename().empty()) {
      std::cout << "Must specify left index file" << std::endl;
      return true;
    }
    if (right_index_filename().empty()) {
      std::cout << "Must specify right index file" << std::endl;
      return true;
    }
    if (left_truth_filename().empty()) {
      std::cout << "Must specify left ground truth file" << std::endl;
      return true;
    }
    if (right_truth_filename().empty()) {
      std::cout << "Must specify right ground truth file" << std::endl;
      return true;
    }
    if (target_degree_power() < 1.0) {
      std::cout << "target degree power must be a > 1.0 exponent, not "
                << target_degree_power() << std::endl;
      return true;
    }
    if (noise_ratio() < 1.0) {
      std::cout << "noise ratio must be > 1.0, not " << noise_ratio()
                << std::endl;
      return true;
    }
    return ret;
  }
};
}  // namespace detail

using parameters = detail::parameters<psqz::parameters>;

}  // namespace psqz::kron
