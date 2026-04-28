// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;
namespace psqz {

void strip(std::string &str) {
  if (str[0] == '\"') {
    str.erase(0, 1);
    str.erase(str.size() - 1);
  }
}

template <typename ContainerType>
void read_truth_file(ContainerType &container, const fs::path &path) {
  using index_type = ContainerType::key_type;
  using cmty_type  = ContainerType::mapped_type;
  if (!fs::exists(path)) {
    container.comm().cerr("truth file path ", path, " does not exist!");
    exit(1);
  }
  std::ifstream ifs(path);
  std::string   line;
  while (std::getline(ifs, line)) {
    index_type         idx;
    cmty_type          cmty;
    std::istringstream iss(line);
    iss >> idx;
    iss >> cmty;
    container.async_visit(
        idx,
        [](const index_type &idx, cmty_type &target, const cmty_type &cmty) {
          target = cmty;
        },
        cmty);
  }
}

template <typename ContainerType>
void read_truth(ContainerType &container, const fs::path &path) {
  if (!fs::exists(path)) {
    container.comm().cerr0("truth file index path ", path, " does not exist!");
    exit(1);
  }
  int           rank = container.comm().rank();
  int           size = container.comm().size();
  std::ifstream ifs(path);
  std::string   line;
  int           counter{0};
  while (std::getline(ifs, line)) {
    if (counter % size == rank) {
      strip(line);
      read_truth_file(container, line);
    }
    ++counter;
  }
}

template <typename ContainerType>
void read_features_file(ContainerType &container, const int count,
                        const fs::path &path) {
  using index_type       = ContainerType::key_type;
  using feature_vec_type = ContainerType::mapped_type;
  using feature_type     = feature_vec_type::value_type;

  if (!fs::exists(path)) {
    container.comm().cout("feature file path ", path, " does not exist!");
  }
  std::ifstream ifs(path);
  std::string   line;
  feature_type  dummy;
  while (std::getline(ifs, line)) {
    std::istringstream iss(line);
    index_type         idx;
    feature_vec_type   sketch(count);
    iss >> idx;
    for (int i{0}; i < count; ++i) {
      iss >> sketch[i];
    }

    container.async_visit(
        idx,
        [](const index_type &idx, feature_vec_type &features,
           const feature_vec_type &sketch) { features = sketch; },
        sketch);
  }
}
template <typename ContainerType>
void read_features(ContainerType &container, const int count,
                   const fs::path &path) {
  if (!fs::exists(path)) {
    container.comm().cerr0("features file index path ", path,
                           " does not exist!");
    exit(1);
  }
  int           rank = container.comm().rank();
  int           size = container.comm().size();
  std::ifstream ifs(path);
  std::string   line;
  int           counter{0};
  while (std::getline(ifs, line)) {
    if (counter % size == rank) {
      strip(line);
      read_features_file(container, count, line);
    }
    ++counter;
  }
}

}  // namespace psqz