// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

namespace psqz::graph {

template <typename IndexType, typename WeightType>
struct edge {
  using index_type  = IndexType;
  using weight_type = WeightType;

  index_type  src;
  index_type  dst;
  weight_type wgt;

  edge<index_type, weight_type> swap() const { return {dst, src, wgt}; }
};

}  // namespace psqz::graph