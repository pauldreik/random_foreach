/*
 * runs the small/big-crush statistical tests
 *
 * By Paul Dreik 2019,2020
 * https://www.pauldreik.se/
 * License: Boost 1.0
 * SPDX-License-Identifier: BSL-1.0
 */
#include "GenericFeistel.h"
#include "PlaygroundFeistel.h"
#include <cstdint>
#include <cstdlib>
#include <random>

extern "C"
{
#include "TestU01.h"
}

static std::uint64_t count = 0;
// static Dynamic64 cipher(64);

// static PlaygroundFeistel64<2> cipher(64);
static PlaygroundFeistel64<20> cipher(64);

unsigned int
doit()
{
  return cipher.encrypt(count++);
}

int
main()
{
  cipher.seed(std::random_device{});

  // avoid having the c function using a non-const pointer to a char literal,
  // which causes warnings.
  std::string name("dynamic64");
  name.push_back('\0');
  unif01_Gen* gen = unif01_CreateExternGenBits(name.data(), doit);

  // Run the tests.
  //  bbattery_SmallCrush(gen);
  bbattery_BigCrush(gen);

  // Clean up.
  unif01_DeleteExternGenBits(gen);

  return 0;
}
