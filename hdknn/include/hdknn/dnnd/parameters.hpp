// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/kron/parameters.hpp>
#include <psqz/sketch/parameters.hpp>
#include <psqz/tsv/parameters.hpp>

#include <unistd.h>

namespace hdknn::dnnd {

template <typename BaseType>
struct parameters : public BaseType {
  using base_type = BaseType;

  psqz::parameter<int>           nn_count;
  psqz::parameter<int>           nn_query;
  psqz::parameter<double>        rho;
  psqz::parameter<double>        delta;
  psqz::parameter<std::uint64_t> batch_size;
  psqz::parameter<bool>          make_index_undirected;
  psqz::parameter<double>        pruning_degree_multiplier;
  psqz::parameter<double>        epsilon;

  parameters()
      : base_type(),
        nn_count("nn_count", "Number of nearest neighbors in index", 'k', true,
                 -1),
        nn_query("nn_query", "Number of nearest neighbors to query", 'K', true,
                 -1),
        rho("rho", "DNND: rho parameter", 'P', true, 0.8),
        delta("delta", "DNND: delta parameter", 'D', true, 0.001),
        batch_size("batch_size", "DNND: batch size parameter", 'S', true,
                   1ULL << 31),
        make_index_undirected("batch_size", "DNND: make index undirected?", 'U',
                              false, true),  // currently always true
        pruning_degree_multiplier("pruning_degree_multiplier",
                                  "DNND: pruning degree multiplier", 'L', true,
                                  1.5),
        epsilon("epsilon", "DNND: epsilon parameter", 'E', true, 0.1) {
    this->_params.push_back(&nn_count);
    this->_params.push_back(&nn_query);
    this->_params.push_back(&rho);
    this->_params.push_back(&delta);
    this->_params.push_back(&batch_size);
    this->_params.push_back(&make_index_undirected);
    this->_params.push_back(&pruning_degree_multiplier);
    this->_params.push_back(&epsilon);
  }

  bool _help_needed() const {
    bool ret = base_type::_help_needed();
    if (nn_count() < 1) {
      std::cout << "Number of nearest neighbors in index must be a positive "
                   "integer, not "
                << nn_count() << std::endl;
      return true;
    }
    if (nn_query() < 1) {
      std::cout << "Number of nearest neighbors to query must be a positive "
                   "integer, not "
                << nn_query() << std::endl;
      return true;
    }
    if (rho() <= 0.0) {
      std::cout << "DNND: rho parameter must be positive, not " << rho()
                << std::endl;
      return true;
    }
    if (delta() <= 0.0) {
      std::cout << "DNND: delta parameter must be positive, not " << delta()
                << std::endl;
      return true;
    }
    if (pruning_degree_multiplier() <= 0.0) {
      std::cout
          << "DNND: pruning degree multiplier parameter must be positive, not "
          << pruning_degree_multiplier() << std::endl;
      return true;
    }
    if (epsilon() <= 0.0) {
      std::cout << "DNND: epsilon parameter must be positive, not " << epsilon()
                << std::endl;
      return true;
    }
    return ret;
  }
};

namespace tsv {

using parameters = hdknn::dnnd::parameters<psqz::tsv::parameters>;

namespace query_only {
using parameters = hdknn::dnnd::parameters<psqz::tsv::query_only::parameters>;
}

namespace sketch {

using parameters = hdknn::dnnd::parameters<psqz::sketch::tsv::parameters>;
namespace query_only {
using parameters =
    hdknn::dnnd::parameters<psqz::sketch::tsv::query_only::parameters>;
}
}  // namespace sketch
}  // namespace tsv

namespace kron {

using parameters = hdknn::dnnd::parameters<psqz::kron::parameters>;

namespace sketch {

using parameters = hdknn::dnnd::parameters<psqz::sketch::kron::parameters>;
}
}  // namespace kron

template <typename BaseType>
std::ostream &operator<<(std::ostream &os, const parameters<BaseType> &params) {
  return params.print(os);
}

template <typename BaseType>
BaseType parse_cmd_line(int argc, char **argv) {
  BaseType    params{};
  int         c;
  std::string str = BaseType::parse_str() + " ";
  while ((c = getopt(argc, argv, str.c_str())) != -1) {
    params.parse(c, optarg);
  }
  if (params.check_help_needed()) {
    params.usage();
    exit(-1);
  }

  return params;
}

}  // namespace hdknn::dnnd
