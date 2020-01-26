
#include "DynamicFeistel32.h"

#include <iomanip>
#include <iostream>
#include <random>

namespace {
#include "popcount.cpp"
}

int
main(int argc, char* argv[])
{
  const std::size_t unroll = 64;

  assert(argc > 1);
  const std::size_t M = std::stoull(argv[1]) / unroll;

  std::random_device rd;
  const auto seed = rd();
  const std::size_t nbits = 32;
  DynamicFeistel32 b((const char*)&seed, sizeof(seed), nbits);

  std::array<unsigned, unroll> ii;
  for (unsigned int i = 0; i < M; ++i) {
    std::iota(ii.begin(), ii.end(), i);
    auto xx = b.encryptmany<unroll>(ii);
    for (auto x : xx) {
      auto actual = count_set_bits(x);
      auto expected = __builtin_popcount(x);
      assert(actual == expected);
    }
  }
}
