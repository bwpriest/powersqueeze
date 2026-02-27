// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/tsv/tsv.hpp>

#include <ygm/comm.hpp>
#include <ygm/container/bag.hpp>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace psqz {
namespace tsv {

template <typename EdgeType>
struct reader {
  using input_type = std::string;
  using edge_type  = EdgeType;

  reader(ygm::comm &comm, const input_type &filename,
         const bool verbose = false)
      : _comm(comm), _bag(comm), _filled(false), _verbose(verbose) {
    if (!filename.empty()) {
      fill_bag(_bag, filename);
      _filled = true;
    }
  }

  template <typename Function>
  void for_all(Function fn, const bool directed = true) {
    if (!_filled) {
      throw std::logic_error("reader has no filenmae!");
    }
    auto read_file_lambda = [&fn, &directed](const input_type &filename) {
      std::ifstream ifs(filename);
      if (!ifs.good()) {
        std::cerr << "error opening filename: " << filename << std::endl;
      }
      edge_type edge{0, 0, 0};
      while (try_read_edge(edge, ifs)) {
        fn(edge);
        if (directed == false) {
          fn(edge.swap());
        }
      }
    };
    _bag.for_all(read_file_lambda);
  }

  void fill_bag(ygm::container::bag<input_type> &bag,
                const input_type                &filename) const {
    if (bag.comm().rank0()) {
      if (is_tsv(filename) == false) {
        if (_verbose) {
          std::cout << "File " << filename << " is not a tsv. Assuming it "
                    << "contains a list of tsv files." << std::endl;
        }

        std::ifstream ifs(filename.c_str());
        std::string   line;
        while (std::getline(ifs, line)) {
          std::stringstream ss(line);
          std::string       fname;
          ss >> fname;
          bag.async_insert(fname);
        }
      } else {
        bag.async_insert(filename);
      }
    }
    bag.comm().barrier();
  }

  ygm::comm &comm() { return _comm; }

 private:
  ygm::comm                      &_comm;
  ygm::container::bag<input_type> _bag;
  bool                            _filled;
  bool                            _verbose;
};

template <typename IndexType>
struct query_reader {
  using index_type = IndexType;
  using input_type = std::string;

  query_reader(ygm::comm &comm, const input_type &filename,
               const bool verbose = false)
      : _comm(comm), _bag(comm), _filled(false), _verbose(verbose) {
    if (!filename.empty()) {
      fill_bag(_bag, filename);
      _filled = true;
    }
  }

  template <typename Function>
  void for_all(Function fn) {
    if (!_filled) {
      throw std::logic_error("reader has no filenmae!");
    }
    auto read_file_lambda = [&fn](const input_type &filename) {
      std::ifstream ifs(filename);
      if (!ifs.good()) {
        std::cerr << "error opening filename: " << filename << std::endl;
      }
      index_type idx{0};
      while (try_read_index(idx, ifs)) {
        fn(idx);
      }
    };
    _bag.for_all(read_file_lambda);
  }

  void fill_bag(ygm::container::bag<input_type> &bag,
                const input_type                &filename) const {
    if (bag.comm().rank0()) {
      if (is_tsv(filename) == false) {
        if (_verbose) {
          std::cout << "File " << filename << " is not a tsv. Assuming it "
                    << "contains a list of tsv files." << std::endl;
        }

        std::ifstream ifs(filename.c_str());
        std::string   line;
        while (std::getline(ifs, line)) {
          std::stringstream ss(line);
          std::string       fname;
          ss >> fname;
          bag.async_insert(fname);
        }
      } else {
        bag.async_insert(filename);
      }
    }
    bag.comm().barrier();
  }

  ygm::comm &comm() { return _comm; }

 private:
  ygm::comm                      &_comm;
  ygm::container::bag<input_type> _bag;
  bool                            _filled;
  bool                            _verbose;
};

}  // namespace tsv
}  // namespace psqz
