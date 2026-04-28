// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/tsv/tsv.hpp>

#include <ygm/container/bag.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>

namespace psqz::kron {

template <typename T>
void if_greater_set(T obs, T &counter) {
  if (obs > counter) {
    counter = obs;
  }
}

void check_probability(const std::string &name, const float probability) {
  if (probability < 0.0 || probability > 1.0) {
    std::stringstream ss;
    ss << name << " probability must be in [0, 1], not " << probability;
    throw std::logic_error(ss.str());
  }
}

template <typename EdgeType>
struct gc_graph {
  using edge_type          = EdgeType;
  using index_type         = edge_type::index_type;
  using weight_type        = edge_type::weight_type;
  using community_type     = index_type;
  using weighted_edge_type = std::tuple<index_type, index_type, weight_type>;
  using edges_type         = std::vector<weighted_edge_type>;
  using truth_type         = std::tuple<index_type, community_type>;
  using truth_vec_type     = std::vector<truth_type>;
  using index_vec_type     = std::vector<index_type>;

  edges_type               edges;
  index_type               vertex_count;
  index_type               edge_count;
  index_type               query_count;
  truth_vec_type           truth;
  index_vec_type           queries;
  community_type           community_count;
  std::vector<std::size_t> community_sizes;
  std::vector<std::size_t> community_intra_counts;
  std::vector<double>      community_intra_densities;
  std::size_t              community_intra_count;
  std::size_t              community_inter_count;
  std::size_t              community_inter_max;
  double                   intra_inter_ratio;

  gc_graph(ygm::comm &comm, const std::string &fname_graph,
           const std::string &fname_truth, const std::string &fname_queries)
      : edges(),
        truth(),
        queries(),
        community_sizes(),
        community_intra_counts(),
        community_intra_densities(),
        vertex_count(0),
        edge_count(0),
        query_count(0),
        community_count(0),
        community_intra_count(0),
        community_inter_count(0),
        community_inter_max(0),
        intra_inter_ratio(0.0) {
    _read_gc_truth(comm, fname_truth);
    _read_gc_queries(comm, fname_queries);
    _read_gc_edges(comm, fname_graph, fname_truth);
    intra_inter_ratio = (double)community_intra_count / community_inter_count;
  }

  void print_state(ygm::comm &world, const std::string &name) {
    world.cout0(name, " vertices: ", vertex_count);
    world.cout0(name, " communities: ", community_count);
    world.cout0(name, " edges: ", edge_count);
    world.cout0(name, " inter edges: ", community_inter_count,
                ", max: ", community_inter_max, ", density: ",
                (double)community_inter_count / community_inter_max);
    world.cout0(name, " intra edges: ", community_intra_count);
    world.cout0(name, " intra/inter ratio: ", intra_inter_ratio);
    world.cout0(name,
                " edge/vertex ratio: ", double(edge_count) / vertex_count);
    world.cout0(name, " community sizes:");
    for (int i(0); i < community_sizes.size(); ++i) {
      std::size_t intra_max =
          (community_sizes[i] * community_sizes[i]) - community_sizes[i];
      double mean_degree =
          (double)community_inter_count / vertex_count +
          (double)community_intra_counts[i] / community_sizes[i];
      world.cout0("\t", i, ": ", community_sizes[i], ", (",
                  community_intra_counts[i], " intra edges, ", intra_max,
                  " intra max, ", community_intra_densities[i], " density, ",
                  ", ", mean_degree, " mean degree, ",
                  mean_degree / vertex_count, " degree/N)");
    }
  }

 private:
  void _read_gc_truth(ygm::comm &world, const std::string &filename) {
    truth.clear();
    auto vertex_count_ptr    = world.make_ygm_ptr(vertex_count);
    auto community_count_ptr = world.make_ygm_ptr(community_count);
    auto truth_ptr           = world.make_ygm_ptr(truth);

    if (world.rank0()) {
      std::ifstream filestream(filename);

      if (filestream.is_open()) {
        std::string line;
        if (!std::getline(filestream, line)) {
          std::cerr << "Empty file\n";
          exit(-1);
        }
        do {
          std::istringstream iss(line);
          index_type         vtx;
          community_type     cmty;
          if (!(iss >> vtx >> cmty)) {
            std::cerr << "Malformed line in input\n";
            exit(-1);
          } else {
            if_greater_set(vtx, vertex_count);
            if_greater_set(cmty, community_count);
            if (vtx < 1) {
              throw std::logic_error(
                  "found non-positive vertex index in GC truth stream");
            }
            --vtx;
            --cmty;
            // The correctness of this step depends having the ground truth
            // arrive in ascending order.
            truth.push_back({vtx, cmty});
          }
        } while (std::getline(filestream, line));
        filestream.close();
      } else {
        std::cerr << "Unable to open file " << filename << std::endl;
        exit(-1);
      }

      world.async_bcast(
          [](const auto vertex_count, const auto community_count,
             const auto &truth, auto vertex_count_ptr, auto community_count_ptr,
             auto truth_ptr) {
            *vertex_count_ptr    = vertex_count;
            *community_count_ptr = community_count;
            *truth_ptr           = truth;
          },
          vertex_count, community_count, truth, vertex_count_ptr,
          community_count_ptr, truth_ptr);
    }

    world.barrier();

    community_sizes.resize(community_count);
    community_intra_counts.resize(community_count);
    for (const std::tuple<index_type, community_type> &cmty_tuple : truth) {
      ++community_sizes[std::get<1>(cmty_tuple)];
    }
    community_inter_max =
        vertex_count * vertex_count -
        std::accumulate(
            community_sizes.begin(), community_sizes.end(), 0,
            [](const std::size_t cum_sqr_sum, const std::size_t next) {
              return cum_sqr_sum + (next * next);
            });

    return;
  }

  void _read_gc_queries(ygm::comm &world, const std::string &filename) {
    queries.clear();
    auto query_count_ptr = world.make_ygm_ptr(query_count);
    auto queries_ptr     = world.make_ygm_ptr(queries);
    if (world.rank0()) {
      std::ifstream filestream(filename);

      if (filestream.is_open()) {
        std::string line;
        if (!std::getline(filestream, line)) {
          std::cerr << "Empty file\n";
          exit(-1);
        }
        do {
          std::istringstream iss(line);
          index_type         vtx;
          if (!(iss >> vtx)) {
            std::cerr << "Malformed line in input\n";
            exit(-1);
          } else {
            ++query_count;
            if (vtx < 1) {
              throw std::logic_error(
                  "found non-positive vertex index in GC truth stream");
            }
            --vtx;
            queries.push_back({vtx});
          }
        } while (std::getline(filestream, line));
        filestream.close();
      } else {
        std::cerr << "Unable to open file " << filename << std::endl;
        exit(-1);
      }

      world.async_bcast(
          [](const auto query_count, const auto &queries, auto query_count_ptr,
             auto queries_ptr) {
            *query_count_ptr = query_count;
            *queries_ptr     = queries;
          },
          query_count, queries, query_count_ptr, queries_ptr);
    }

    world.barrier();
  }

  void _read_gc_edges(ygm::comm &world, const std::string &fname_graph,
                      const std::string &fname_truth) {
    index_type vertex_count_check{0};

    edges.clear();
    auto vertex_count_check_ptr    = world.make_ygm_ptr(vertex_count_check);
    auto edge_count_ptr            = world.make_ygm_ptr(edge_count);
    auto edges_ptr                 = world.make_ygm_ptr(edges);
    auto community_inter_count_ptr = world.make_ygm_ptr(community_inter_count);
    auto community_intra_counts_ptr =
        world.make_ygm_ptr(community_intra_counts);

    if (world.rank0()) {
      _read_one_gc_file(fname_graph, vertex_count_check);

      world.async_bcast(
          [](const auto vertex_count_check, const auto edge_count,
             const auto &edges, const auto community_inter_count,
             const auto &community_intra_counts, auto vertex_count_check_ptr,
             const auto edge_count_ptr, auto edges_ptr,
             auto community_inter_count_ptr, auto community_intra_counts_ptr) {
            *vertex_count_check_ptr     = vertex_count_check;
            *edge_count_ptr             = edge_count;
            *edges_ptr                  = edges;
            *community_inter_count_ptr  = community_inter_count;
            *community_intra_counts_ptr = community_intra_counts;
          },
          vertex_count_check, edge_count, edges, community_inter_count,
          community_intra_counts, vertex_count_check_ptr, edge_count_ptr,
          edges_ptr, community_inter_count_ptr, community_intra_counts_ptr);
    }

    world.barrier();

    if (vertex_count_check != vertex_count) {
      world.cerr0("edge stream file ", fname_graph, " vertex count (",
                  vertex_count_check, ") disagress with community stream file ",
                  fname_truth, " vertex count (", vertex_count, ")");
      exit(-1);
    }

    community_intra_count = std::accumulate(community_intra_counts.begin(),
                                            community_intra_counts.end(), 0);
    community_intra_densities.resize(community_sizes.size());
    for (int i(0); i < community_intra_densities.size(); ++i) {
      std::size_t community_max =
          community_sizes[i] * community_sizes[i] - community_sizes[i];
      community_intra_densities[i] =
          (double)community_intra_counts[i] / community_max;
    }

    return;
  }

  void _read_one_gc_file(const std::string &filename,
                         index_type        &vertex_count_check) {
    std::ifstream filestream(filename);

    if (filestream.is_open()) {
      std::string line;
      if (!std::getline(filestream, line)) {
        std::cerr << "Empty file\n";
        exit(-1);
      }
      do {
        std::istringstream iss(line);
        index_type         src, dst;
        weight_type        wgt;
        if (!(iss >> src >> dst >> wgt)) {
          std::cerr << "Malformed line in input\n";
          exit(-1);
        } else {
          if_greater_set(src, vertex_count_check);
          if_greater_set(dst, vertex_count_check);
          if (src < 1 || dst < 1) {
            throw std::logic_error(
                "found non-positive vertex index in GC edge stream");
          }
          --src;
          --dst;
          _add_one_edge(src, dst, wgt);
          // forcing to be symmetric, at least for now...
          _add_one_edge(dst, src, wgt);
          // update the intra/inter community edge counts
          community_type src_cmty = std::get<1>(truth[src]);
          community_type dst_cmty = std::get<1>(truth[dst]);
          if (src_cmty == dst_cmty) {
            ++community_intra_counts[src_cmty];
            ++community_intra_counts[src_cmty];
          } else {
            ++community_inter_count;
            ++community_inter_count;
          }
        }
      } while (std::getline(filestream, line));
      filestream.close();
    } else {
      std::cerr << "Unable to open file " << filename << std::endl;
      exit(-1);
    }
    for (index_type i{0}; i < vertex_count_check; ++i) {
      _add_one_edge(i, i, (weight_type)1);
    }
  }

  void _add_one_edge(index_type src, index_type dst, weight_type wgt) {
    edges.push_back(std::make_tuple(src, dst, wgt));
    ++edge_count;
  }
};

template <typename EdgeType>
struct reader {
  using input_type = std::string;

  using edge_type          = EdgeType;
  using graph_type         = gc_graph<edge_type>;
  using index_type         = graph_type::index_type;
  using weight_type        = graph_type::weight_type;
  using community_type     = graph_type::community_type;
  using weighted_edge_type = graph_type::weighted_edge_type;
  using truth_type         = graph_type::truth_type;

  enum class mode_type : std::uint8_t { index, query, truth };

 private:
  ygm::comm    &_comm;
  graph_type    _graph1;
  graph_type    _graph2;
  mode_type     _mode;
  std::uint32_t _random_seed;
  float         _target_degree_power;
  float         _noise_ratio;
  double        _intra_probability;
  double        _inter_probability;

  double _compute_intra_probability() const {
    double expected_mean_degree =
        (double(_graph1.edge_count) / _graph1.vertex_count) *
        (double(_graph2.edge_count) / _graph2.vertex_count);
    double target_mean_degree =
        std::pow(std::log(_graph1.vertex_count * _graph2.vertex_count),
                 _target_degree_power);
    _comm.cout0("expected mean degree: ", expected_mean_degree);
    _comm.cout0("target mean degree: ", target_mean_degree);
    double intra_probability = target_mean_degree / expected_mean_degree;
    return (intra_probability < 1.0) ? intra_probability : 1.0;
  }

 public:
  reader(ygm::comm &comm, const input_type &fname_graph1,
         const input_type &fname_graph2, const input_type &fname_truth1,
         const input_type &fname_truth2, const input_type &fname_queries1,
         const input_type &fname_queries2, const float target_degree_power,
         const float noise_ratio, const std::uint32_t random_seed)
      : _comm(comm),
        _graph1(_comm, fname_graph1, fname_truth1, fname_queries1),
        _graph2(_comm, fname_graph2, fname_truth2, fname_queries2),
        _mode(mode_type::index),
        _random_seed(random_seed),
        _target_degree_power(target_degree_power),
        _noise_ratio(noise_ratio),
        _intra_probability(_compute_intra_probability()),
        _inter_probability(_intra_probability / _noise_ratio) {
    if (!fname_graph1.empty() && !fname_graph2.empty()) {
      check_probability("intra-cluster preservation", _intra_probability);
      check_probability("inter-cluster preservation", _inter_probability);

      // _graph1.print_state(_comm, "Graph 1");
      // _graph2.print_state(_comm, "Graph 2");
      comm.cout0("intra probability: ", _intra_probability);
      comm.cout0("inter probability: ", _inter_probability);
    }
  }

  void set_mode(const mode_type mode) { _mode = mode; }

  mode_type mode() const { return _mode; }

  template <typename Function, typename... Args>
  void for_all(Function fn, Args... args) {
    switch (_mode) {
      case mode_type::index:
        return for_all_index(fn);
        break;
      case mode_type::query:
        return for_all_query(fn);
        break;
      case mode_type::truth:
        return for_all_truth(fn);
        break;
    }
  }

  template <typename Function>
  void for_all_index(Function fn) {
    std::mt19937                gen(_random_seed + _comm.rank());
    std::bernoulli_distribution intra_dist(_intra_probability);
    std::bernoulli_distribution inter_dist(_inter_probability);

    index_type pos1 = _comm.rank();

    while (pos1 < _graph1.edges.size()) {
      const weighted_edge_type &edge1  = _graph1.edges.at(pos1);
      const index_type          row1   = std::get<0>(edge1);
      const index_type          col1   = std::get<1>(edge1);
      const weight_type         wgt1   = std::get<2>(edge1);
      const community_type     &cmty11 = std::get<1>(_graph1.truth.at(row1));
      const community_type     &cmty12 = std::get<1>(_graph1.truth.at(col1));

      std::for_each(
          _graph2.edges.begin(), _graph2.edges.end(),
          [fn, &row1, &col1, &wgt1, &cmty11, &cmty12, &intra_dist, &inter_dist,
           &gen, this](const weighted_edge_type &edge2) {
            const index_type      row2   = std::get<0>(edge2);
            const index_type      col2   = std::get<1>(edge2);
            const weight_type     wgt2   = std::get<2>(edge2);
            const community_type &cmty21 = std::get<1>(_graph2.truth.at(row2));
            const community_type &cmty22 = std::get<1>(_graph2.truth.at(col2));

            if (cmty11 == cmty12 && cmty21 == cmty22) {
              // spawn edge with intra_probability
              if (!intra_dist(gen)) {
                return;
              }
            } else {
              // spawn edge with inter_probability
              if (!inter_dist(gen)) {
                return;
              }
            }
            const index_type  row = row1 * this->_graph2.vertex_count + row2;
            const index_type  col = col1 * this->_graph2.vertex_count + col2;
            const weight_type wgt = wgt1 * wgt2;
            if (row != col) {
              fn(edge_type{row, col, wgt});
            }
          });

      pos1 += _comm.size();
    }
  }

  template <typename Function>
  void for_all_query(Function fn) {
    std::mt19937                gen(_random_seed + _comm.rank());
    std::bernoulli_distribution intra_dist(_intra_probability);
    std::bernoulli_distribution inter_dist(_inter_probability);

    index_type pos1 = _comm.rank();

    while (pos1 < _graph1.edges.size()) {
      const index_type     &idx1  = _graph1.queries.at(pos1);
      const community_type &cmty1 = std::get<1>(_graph1.truth.at(idx1));

      std::for_each(
          _graph2.queries.begin(), _graph2.queries.end(),
          [fn, &idx1, &cmty1, &intra_dist, &inter_dist, &gen,
           this](const index_type &idx2) {
            const community_type &cmty2 = std::get<1>(_graph2.truth.at(idx2));

            if (cmty1 == cmty2) {
              // spawn edge with intra_probability
              if (!intra_dist(gen)) {
                return;
              }
            } else {
              // spawn edge with inter_probability
              if (!inter_dist(gen)) {
                return;
              }
            }
            const index_type idx = idx1 * this->_graph2.vertex_count + idx2;
            fn(edge_type{idx, 0, 0});
          });

      pos1 += _comm.size();
    }
  }

  template <typename Function>
  void for_all_truth(Function fn) {
    index_type pos1 = _comm.rank();

    while (pos1 < _graph1.truth.size()) {
      const truth_type &truth1 = _graph1.truth.at(pos1);
      index_type        idx1   = std::get<0>(truth1);
      community_type    cmty1  = std::get<1>(truth1);
      std::for_each(_graph2.truth.begin(), _graph2.truth.end(),
                    [fn, &idx1, &cmty1, this](const truth_type &truth2) {
                      index_type     idx2  = std::get<0>(truth2);
                      community_type cmty2 = std::get<1>(truth2);

                      index_type idx = idx1 * this->_graph2.vertex_count + idx2;
                      community_type cmty =
                          cmty1 * this->_graph2.community_count + cmty2;

                      fn(edge_type{idx, cmty});
                    });

      pos1 += _comm.size();
    }
  }
};

}  // namespace psqz::kron
