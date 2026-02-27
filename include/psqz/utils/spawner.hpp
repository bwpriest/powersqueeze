// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <ygm/container/array.hpp>
#include <ygm/container/map.hpp>
#include <ygm/detail/ygm_traits.hpp>

#include <type_traits>

namespace psqz {

template <typename IndexType, typename Point>
using ygm_array = ygm::container::array<Point, IndexType>;

template <typename IndexType, typename Point>
using ygm_map = ygm::container::map<IndexType, Point>;

template <template <typename, typename> class ContainerType, typename IndexType,
          typename Point>
ContainerType<IndexType, Point> spawn(ygm::comm         &comm,
                                      const Point       &default_value,
                                      const std::size_t &array_size) {
  if constexpr (std::is_same<ContainerType<IndexType, Point>,
                             ygm_map<IndexType, Point>>::value) {
    return {comm};
  } else if constexpr (std::is_same<ContainerType<IndexType, Point>,
                                    ygm_array<IndexType, Point>>::value) {
    return {comm, array_size, default_value};
  } else {
    static_assert(ygm::detail::always_false<>,
                  "spawn of template type is not supported");
  }
}

}  // namespace psqz
