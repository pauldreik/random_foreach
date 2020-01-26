/*
* By Paul Dreik 2019,2020
* https://www.pauldreik.se/
* License: Boost 1.0
* SPDX-License-Identifier: BSL-1.0
*/
#pragma once

#include <cstdint>
#include <wmmintrin.h> //for intrinsics for AES-NI

#include "GenericFeistel.h"

/**
 * Hardware accelerated cipher, using the AES hw support
 * in the rounding function
 */
template<int ROUNDS_>
class Aes32
  : public GenericFeistel<Aes32<ROUNDS_>, std::uint32_t, std::uint16_t>
{
public:
  static constexpr int ROUNDS = ROUNDS_;
  using Base = GenericFeistel<Aes32<ROUNDS>, std::uint32_t, std::uint16_t>;
  explicit Aes32(int Nbits)
    : Base(Nbits){}

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
    __m128i m = _mm_set1_epi16(x);
    m = _mm_aesenc_si128(m, m_key);
    return _mm_extract_epi16(m, 0);
  }

private:
  __m128i m_key;
};
