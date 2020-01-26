/*
 * By Paul Dreik 2019,2020
 * https://www.pauldreik.se/
 * License: Boost 1.0
 * SPDX-License-Identifier: BSL-1.0
 */
#pragma once

#include "GenericFeistel.h"
#include <array>
#include <cstdint>

namespace {
/**
 * internals for the ciphers further down this file
 */
namespace DynamicInternals {
std::uint16_t
hashfnv1a(const std::uint16_t value)
{
  const std::uint32_t prime = 0x1000193;
  std::uint32_t hash = 0x811c9dc5;
  for (unsigned i = 0; i < sizeof(value); ++i) {
    const unsigned char byte = (value >> (8 * i)) & 0xFF;
    hash ^= byte;
    hash *= prime;
  }
  // fold the 32 bit hash into 16 by xoring the halves, as the authors
  // of fnv1a recommend.
  return hash ^ (hash >> 16);
}

std::uint32_t
hashfnv1a(const std::uint32_t value)
{
  const std::uint32_t prime = 0x1000193;
  std::uint32_t hash = 0x811c9dc5;
  for (unsigned i = 0; i < sizeof(value); ++i) {
    const unsigned char byte = (value >> (8 * i)) & 0xFF;
    hash ^= byte;
    hash *= prime;
  }
  return hash;
}
}
}

/**
 * @brief The Dynamic32 class
 * A block cipher with Fnv1a as round function, dynamic bit width <=32
 * restricted to even.
 *
 * motivation for using fn1v as a building block:
 * https://aras-p.info/blog/2016/08/09/More-Hash-Function-Tests/
 */
class Dynamic32 : public GenericFeistel<Dynamic32, std::uint32_t, std::uint16_t>
{
public:
  static constexpr int ROUNDS = 2;
  using Base = GenericFeistel<Dynamic32, std::uint32_t, std::uint16_t>;
  explicit Dynamic32(int Nbits)
    : Base(Nbits)
  {}

  template<typename URBG>
  void seed(URBG&& urbg)
  {
    for (auto& e : m_key) {
      e = urbg();
    }
  }

  std::uint16_t roundFunction(const std::uint16_t x, int round) const
  {
    return DynamicInternals::hashfnv1a(x) ^ m_key[round];
  }

private:
  std::array<std::uint16_t, ROUNDS> m_key;
};

/**
 * @brief The Dynamic64 class
 * A block cipher with Fnv1a as round function, dynamic bit width <=64
 * restricted to even.
 */
class Dynamic64 : public GenericFeistel<Dynamic64, std::uint64_t, std::uint32_t>
{
public:
  static constexpr int ROUNDS = 2;
  using Base = GenericFeistel<Dynamic64, std::uint64_t, std::uint32_t>;
  explicit Dynamic64(int Nbits)
    : Base(Nbits)
  {}
  template<typename URBG>
  void seed(URBG&& urbg)
  {
    for (auto& e : m_key) {
      e = urbg();
    }
  }
  std::uint32_t roundFunction(const std::uint32_t x, int round) const
  {
    return DynamicInternals::hashfnv1a(x) ^ m_key[round];
  }

private:
  std::array<std::uint32_t, ROUNDS> m_key;
};
