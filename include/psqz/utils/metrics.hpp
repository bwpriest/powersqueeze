// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <unistd.h>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <vector>

namespace psqz {

struct Metric {
  std::string name;
  double      value;

  Metric(std::string _name) : name(_name), value(0.0) {}
};

struct Metrics {
  Metrics() {}

  void set(std::string name, double value) {
    try {
      lookup.at(name)->value = value;
    } catch (std::out_of_range &e) {
      values.push_back(std::make_shared<Metric>(name));
      values.back()->value = value;
      lookup[name]         = values.back();
    }
  }

  Metric get(std::string name) const {
    try {
      return *lookup.at(name);
    } catch (std::out_of_range &e) {
      std::stringstream ss;
      ss << "metric name " << name << " has not been set!";
      throw std::out_of_range(ss.str());
    }
  }
  double get_value(std::string name) const { return get(name).value; }

  template <typename Func>
  void for_each(Func &func) const {
    for (const std::shared_ptr<Metric> &metric : values) {
      func(*metric);
    }
  }

 private:
  std::vector<std::shared_ptr<Metric>>           values;
  std::map<std::string, std::shared_ptr<Metric>> lookup;
};

std::ostream &operator<<(std::ostream &os, const Metric &metric) {
  os << metric.name << ":\t" << metric.value;
  return os;
}

std::ostream &operator<<(std::ostream &os, const Metrics &metrics) {
  os << "metrics:" << std::endl;
  auto print_fn = [&os](const Metric &metric) {
    os << "\t" << metric << std::endl;
  };
  metrics.for_each(print_fn);
  return os;
}

template <typename... Args>
std::string csv_format_list(const char delim, const Args &...args) {
  constexpr std::size_t elem_count = sizeof...(Args);
  std::size_t           counter(0);
  std::stringstream     ss;
  // process lambda for each element
  auto process_one_lambda = [&ss, &counter, &elem_count, &delim](auto element) {
    ss << element;
    if (++counter < elem_count) {
      ss << delim;
    }
  };
  // process all elements;
  (process_one_lambda(args), ...);

  return ss.str();
}

template <typename Arg>
std::string csv_format_list(const char delim, std::vector<Arg> &args) {
  std::stringstream ss;
  std::size_t       elem_count(args.size());
  std::size_t       counter(0);
  for (const Arg &element : args) {
    ss << element;
    if (++counter < elem_count) {
      ss << delim;
    }
  }
  return ss.str();
}

std::string csv_metric_names(const Metrics &metrics, const char delim = ',') {
  std::vector<std::string> names;
  auto                     names_fn = [&names](const Metric &metric) {
    names.push_back(metric.name);
  };
  metrics.for_each(names_fn);
  std::string ret = csv_format_list(delim, names);
  return ret;
}

std::string csv_metric_vals(const Metrics &metrics, const char delim = ',') {
  std::vector<double> values;
  auto                vals_fn = [&values](const Metric &metric) {
    values.push_back(metric.value);
  };
  metrics.for_each(vals_fn);
  std::string ret = csv_format_list(delim, values);
  return ret;
}

std::string csv_metrics(const Metrics &metrics, const char delim = ',') {
  std::stringstream ss;
  ss << csv_format_list(delim, csv_metric_names(metrics, delim)) << std::endl;
  ss << csv_format_list(delim, csv_metric_vals(metrics, delim));
  return ss.str();
}

}  // namespace psqz
