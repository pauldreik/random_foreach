#pragma once

/*
 * By Paul Dreik 2019,2020
 * https://www.pauldreik.se/
 * License: Boost 1.0
 * SPDX-License-Identifier: BSL-1.0
 */

#include <cstdint>
#include <immintrin.h> // intrinsics for sha

#include "GenericFeistel.h"

/*
 * Feistel ceipher using hardware sha instructions (requires hw support,
 * look for the sha_ni flag in /proc/cpuinfo
 */
template<int ROUNDS_>
class ShaFeistel32
  : public GenericFeistel<ShaFeistel32<ROUNDS_>, std::uint32_t, std::uint16_t>
{
public:
  static constexpr int ROUNDS = ROUNDS_;
  using Base =
    GenericFeistel<ShaFeistel32<ROUNDS>, std::uint32_t, std::uint16_t>;
  explicit ShaFeistel32(int Nbits)
    : Base(Nbits)
  {}

  template<typename URBG>
  void seed(URBG& urbg)
  {
    char seedarray[sizeof(m_key)];
    for (auto& e : seedarray) {
      e = urbg();
    }
    m_key = _mm_loadu_si128((__m128i*)seedarray);
  }

  std::uint16_t roundFunction(const std::uint16_t x, int round) const
  {
    assert(round >= 0 && round < 8);
    __m128i a = _mm_set1_epi16(x);
    // requires the sha extensions, look for the sha_ni flag in /proc/cpuinfo
    auto res = _mm_sha1rnds4_epu32(m_key, a, 0);

    return _mm_extract_epi16(res, 0);
  }

private:
  __m128i m_key;
};
