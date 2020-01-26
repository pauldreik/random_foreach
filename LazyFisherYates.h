/*
 *
 * Lazy Fisher-Yates iteration
 *
 * This steps through a range randomly, using the idea from the Fisher Yates
 * shuffle. It does not shuffle the range and then iterate, instead it
 * stores a sparse representation of the index range.
 *
 * The amount of memory reguired increases the closer to the middle one is.
 * Experimentally, it seems like the expected amount of memory at step i
 * out of N is i*(N-i)/2N (so at most N/4 when i=N/2).
 * The underlying store is however a std::map, so expect N log N memory need.
 *
 * By Paul Dreik 2019  https://www.pauldreik.se/
 * License: Boost 1.0
 * SPDX-License-Identifier: BSL-1.0
 */
#pragma once

#include <cassert>
#include <map>
#include <random>
#include <utility>

// invokes cb with a each integer in [0,N) in random order
template<typename Integer, typename URBG, typename Callback>
void
lazy_fisher_yates(Integer N, URBG&& rng, Callback cb)
{
  if (N == 0) {
    return;
  }

  std::map<Integer, Integer> a;

  for (Integer i = 0; i < N - 1; ++i) {
    std::uniform_int_distribution<Integer> dist(i, N - 1);
    Integer j = dist(rng);

    // find a[i]
    Integer a_i;
    if (!a.empty() && a.begin()->first == i) {
      a_i = a.begin()->second;
    } else {
      a_i = i;
    }

    // find a[j]
    Integer a_j;
    auto j_it = a.lower_bound(j);
    if (j_it != a.end() && j_it->first == j) {
      a_j = j_it->second;
      j_it->second = a_i;
    } else {
      a_j = j;
      a.insert(j_it, { j, a_i });
    }

    // drop the first entry to save space if possible
    if (a.begin()->first <= i) {
      a.erase(a.begin());
    }
    cb(a_j);
  }
  assert(a.size() <= 1);
  // the last iteration
  auto a_j = a.empty() ? N - 1 : a.rbegin()->second;
  cb(a_j);
}
