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

void create_directory_if_not_exists(const fs::path &path) {
  if (!fs::is_directory(path) || !fs::exists(path)) {
    fs::create_directory(path);
  }
}

template <typename ContainerType>
void write_features(ContainerType &container, const fs::path &path) {
  create_directory_if_not_exists(path);
  int               rank = container.comm().rank();
  std::stringstream ss;
  ss << rank << "_features.txt";
  std::string   fname  = ss.str();
  fs::path      target = path / fname;
  std::ofstream ofs(target);
  if (!ofs.good()) {
    std::cerr << "error opening filename: " << (target) << std::endl;
  }

  container.for_all([&ofs](const auto &index, const auto &point) {
    ofs << index;
    for (const auto &feature : point) {
      ofs << " " << feature;
    }
    ofs << std::endl;
  });
  container.comm().barrier();
  ofs.close();
}

template <typename ContainerType>
void write_truth(ContainerType &container, const fs::path &path) {
  create_directory_if_not_exists(path);
  int               rank = container.comm().rank();
  std::stringstream ss;
  ss << rank << "_labels.txt";
  std::string   fname  = ss.str();
  fs::path      target = path / fname;
  std::ofstream ofs(target);
  if (!ofs.good()) {
    std::cerr << "error opening filename: " << (target) << std::endl;
  }

  container.for_all([&ofs](const auto &index, const auto &label) {
    ofs << index << " " << label << std::endl;
  });
  container.comm().barrier();
  ofs.close();
}

void write_index_file(const fs::path &path, const fs::path &target,
                      const std::string name) {
  std::stringstream ss;
  ss << name << ".txt";
  std::string   fname = ss.str();
  std::ofstream ofs(path / fname);
  if (!ofs.good()) {
    std::cerr << "error opening filename: " << (path / fname) << std::endl;
  }
  for (const auto &entry : fs::directory_iterator(target)) {
    ofs << fs::absolute(entry).c_str() << std::endl;
  }
  ofs.close();
}

template <typename ContainerType>
void write_truth_files(ContainerType &container, const fs::path &path) {
  fs::path target = path / "truth";
  create_directory_if_not_exists(target);
  psqz::write_truth(container, target);
  if (container.comm().rank0()) {
    write_index_file(path, target, "truth");
  }
}

template <typename ContainerType>
void write_feature_files(ContainerType &container, const fs::path &path,
                         const std::string name) {
  psqz::create_directory_if_not_exists(path / "features");
  fs::path features_dir = path / "features";
  fs::path target       = features_dir / name;
  create_directory_if_not_exists(target);
  psqz::write_features(container, target);
  if (container.comm().rank0()) {
    write_index_file(features_dir, target, name);
  }
}
}  // namespace psqz
