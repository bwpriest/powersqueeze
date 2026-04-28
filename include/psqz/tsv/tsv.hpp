// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/utils/string_help.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace psqz::tsv {

template <typename EdgeType>
inline bool try_read_edge(EdgeType &edge, std::ifstream &ifs) {
  using index_type  = EdgeType::index_type;
  using weight_type = EdgeType::weight_type;

  std::string line;
  index_type  src;
  index_type  dst;
  weight_type wgt;
  if (std::getline(ifs, line)) {
    std::stringstream ssline(line);
    ssline >> src >> dst >> wgt;
#ifdef TSV_DECREMENT
    src -= 1;
    dst -= 1;
#endif
    edge = EdgeType{src, dst, wgt};
    return true;
  } else {
    return false;
  }
}

/**
 * @brief Here we are only using  the `src` field of the edge argument for
 * consistency with the kronecker impl.
 *
 * @tparam EdgeType
 * @param edge
 * @param ifs
 * @return true
 * @return false
 */
template <typename EdgeType>
inline bool try_read_index(EdgeType &edge, std::ifstream &ifs) {
  using edge_type  = EdgeType;
  using index_type = edge_type::index_type;

  std::string line;
  index_type  idx;
  if (std::getline(ifs, line)) {
    std::stringstream ssline(line);
    ssline >> idx;
#ifdef TSV_DECREMENT
    idx -= 1;
#endif
    edge = EdgeType{idx, 0, 0};
    return true;
  } else {
    return false;
  }
}

inline bool is_tsv(const std::string &filename) {
  std::ifstream ifs(filename);
  if (!ifs.good()) {
    std::stringstream ss;
    ss << "error opening filename: " << filename << std::endl;
    throw std::invalid_argument(ss.str());
  }

  std::string line;
  if (std::getline(ifs, line)) {
    std::vector<std::string> parts(split(line, '\t'));
    return (parts.size() > 1) ? true : false;
  } else {
    return false;
  }
}

}  // namespace psqz::tsv