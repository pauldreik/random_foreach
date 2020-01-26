/*
 * By Paul Dreik 2019,2020
 * https://www.pauldreik.se/
 * License: Boost 1.0
 * SPDX-License-Identifier: BSL-1.0
 */

#include "Fnv1aCiphers.h"
#include <cstdint>
#include <iostream>
#include <random>

/*
 * Example of generating random numbers, suitable to feed
 * into a random number statistics program
 *
 */
int
main()
{
  Dynamic64 cipher(64);

  cipher.seed(std::random_device{});

  for (std::uint64_t i = 0; i < 0xFFFFFFFFU; ++i) {
    auto e = cipher.encrypt(i);
    std::uint32_t small = e;
    std::cout.write((const char*)&small, sizeof(small));
  }
}
