// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/kron/parameters.hpp>
#include <psqz/sketch/parameters.hpp>
#include <psqz/tsv/parameters.hpp>

#include <unistd.h>

namespace hdknn::dhnsw {

template <typename BaseType>
struct parameters : public BaseType {
  using base_type = BaseType;

  psqz::parameter<int> voronoi_rank;
  psqz::parameter<int> seed_count;
  psqz::parameter<int> hop_count;
  psqz::parameter<int> nn_query;

  parameters()
      : base_type(),
        voronoi_rank("voronoi_rank", "Voronoi rank", 'v', true, -1),
        seed_count("seed_count", "Number of seeds", 's', true, -1),
        hop_count("hop_count", "Number of hops", 'p', true, -1),
        nn_query("nn_query", "Number of nearest neighbors", 'k', true, -1) {
    this->_params.push_back(&voronoi_rank);
    this->_params.push_back(&seed_count);
    this->_params.push_back(&hop_count);
    this->_params.push_back(&nn_query);
  }

  bool _help_needed() const {
    bool ret = base_type::_help_needed();
    if (voronoi_rank() < 1) {
      std::cout << "Voronoi rank must be a positive integer, not "
                << voronoi_rank() << std::endl;
      return true;
    }
    if (seed_count() < 1) {
      std::cout << "Seed count must be a positive integer, not " << seed_count()
                << std::endl;
      return true;
    }
    if (hop_count() < 1) {
      std::cout << "Hop count must be a positive integer, not " << hop_count()
                << std::endl;
      return true;
    }
    if (nn_query() < 1) {
      std::cout
          << "Number of nearest neighbors must be a positive integer, not "
          << nn_query() << std::endl;
      return true;
    }
    return ret;
  }
};

namespace tsv {
using parameters = hdknn::dhnsw::parameters<psqz::tsv::parameters>;

namespace query_only {
using parameters = hdknn::dhnsw::parameters<psqz::tsv::query_only::parameters>;
}

namespace sketch {
using parameters = hdknn::dhnsw::parameters<psqz::sketch::tsv::parameters>;

namespace query_only {
using parameters =
    hdknn::dhnsw::parameters<psqz::sketch::tsv::query_only::parameters>;
}
}  // namespace sketch
}  // namespace tsv
namespace kron {
using parameters = hdknn::dhnsw::parameters<psqz::kron::parameters>;

namespace sketch {
using parameters = hdknn::dhnsw::parameters<psqz::sketch::kron::parameters>;
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
  std::string str = params.parse_str() + " ";
  while ((c = getopt(argc, argv, str.c_str())) != -1) {
    params.parse(c, optarg);
  }
  if (params.check_help_needed()) {
    params.usage();
    exit(-1);
  }

  return params;
}

}  // namespace hdknn::dhnsw
