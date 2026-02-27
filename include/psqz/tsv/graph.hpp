// Copyright 2023-2026 Lawrence Livermore National Security, LLC and other
// powersqueeze Project Developers.See the top-level COPYRIGHT file for details.

#pragma once

#include <psqz/graph/adjacency.hpp>
#include <psqz/graph/truth.hpp>
#include <psqz/tsv/adjacency.hpp>
#include <psqz/tsv/truth.hpp>

namespace psqz::tsv {

namespace core {
template <typename HandlerType>
using adjacency =
    psqz::tsv::detail::adjacency<HandlerType, psqz::graph::detail::adjacency>;
template <typename HandlerType>
using normalize =
    psqz::tsv::detail::normalize<HandlerType, psqz::graph::detail::normalize>;
template <typename HandlerType>
using truth = psqz::tsv::detail::truth<HandlerType, psqz::graph::detail::truth>;
template <typename HandlerType>
using queries =
    psqz::tsv::detail::queries<HandlerType, psqz::graph::detail::queries>;
}  // namespace core

template <typename HandlerType>
using adjacency = psqz::graph::adjacency<HandlerType, core::adjacency>;
template <typename HandlerType>
using normalized_adjacency =
    psqz::graph::normalized_adjacency<HandlerType, core::adjacency,
                                      core::normalize>;
template <typename HandlerType>
using truth = psqz::graph::truth<HandlerType, core::truth>;
template <typename HandlerType>
using queries = psqz::graph::queries<HandlerType, core::queries>;
}  // namespace psqz::tsv