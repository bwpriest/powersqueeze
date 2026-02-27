// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/utils/parameters.hpp>

#include <unistd.h>

namespace psqz::tsv {

namespace detail {

namespace query_only {
template <typename BaseType>
struct parameters : public BaseType {
  using base_type = BaseType;

  parameter<std::string> index_filename;
  parameter<std::string> query_filename;

  parameters()
      : base_type(),
        index_filename("index_filename",
                       "File containing data to build index (required)", 'i',
                       true, ""),
        query_filename("query_filename",
                       "Optional file containing indices to query", 'q', true,
                       "") {
    this->_params.push_back(&index_filename);
    this->_params.push_back(&query_filename);
  }

  bool _help_needed() const override {
    bool ret = base_type::_help_needed();
    if (index_filename().empty()) {
      std::cout << "Must specify file to build index from" << std::endl;
      return true;
    }
    return ret;
  }
};
}  // namespace query_only

template <typename BaseType>
struct parameters : public query_only::parameters<BaseType> {
  using base_type = query_only::parameters<BaseType>;

  parameter<std::string> truth_filename;

  parameters()
      : base_type(),
        truth_filename("index_filename", "File containing true communities",
                       'g', true, "") {
    this->_params.push_back(&truth_filename);
  }

  bool _help_needed() const override {
    bool ret = base_type::_help_needed();
    if (truth_filename().empty()) {
      std::cout << "Must specify file containing ground truth" << std::endl;
      return true;
    }
    return ret;
  }
};
}  // namespace detail

using parameters = detail::parameters<psqz::parameters>;

namespace query_only {
using parameters = psqz::tsv::detail::query_only::parameters<psqz::parameters>;
}

}  // namespace psqz::tsv
