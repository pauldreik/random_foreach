/*
* By Paul Dreik 2019,2020
* https://www.pauldreik.se/
* License: Boost 1.0
* SPDX-License-Identifier: BSL-1.0
*/
#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <immintrin.h>
#include <wmmintrin.h> //for intrinsics for AES-NI

#include "GenericFeistel.h"

/*
 * For experimenting with different rounding functions to see how much
 * it takes to pass the randomness tests
 */
template<int ROUNDS_>
class PlaygroundFeistel
  : public GenericFeistel<PlaygroundFeistel<ROUNDS_>,
                          std::uint32_t,
                          std::uint16_t>
{
public:
  static constexpr int ROUNDS = ROUNDS_;
  using Base =
    GenericFeistel<PlaygroundFeistel<ROUNDS>, std::uint32_t, std::uint16_t>;
  explicit PlaygroundFeistel(int Nbits)
    : Base(Nbits){};

  template<typename URBG>
  void seed(URBG& urbg)
  {
    char seedarray[sizeof(m_key)];
    for (auto& e : seedarray) {
      e = urbg();
    }
    std::memcpy(&m_key, &seedarray[0], sizeof(m_key));
    while (m_key == 0) {
      m_key = urbg();
    }
  }

  std::uint16_t roundFunction(const std::uint16_t x, int round) const
  {
    // 2.25
    return _mm_crc32_u32(m_key, x);

    // 2.22
    return _mm_crc32_u16(m_key, x);

    auto rotate = [](std::uint32_t x, unsigned n) {
      return (x << n) | (x >> ((-n) & 31));
    };
    // 2.8
    return rotate(x, m_key & 0x1F);

    // 1.96
    return _mm_popcnt_u32(x);

    // 1.96 (requires bmi2)
    return _pdep_u32(x, m_key);

    // 1.96 (requires bmi2)
    return _pext_u32(m_key, x);

    // 7.55
    return x / m_key;

    // 2.22
    return (std::uint16_t)m_key - x;

    // 2.24
    return m_key - x;

    // 1.95
    return m_key * x;

    // 1.96
    return m_key + x;
    // 1.97
    auto tmp = x ^ m_key;
    return tmp;
  }

private:
  std::uint32_t m_key;
};

template<int ROUNDS_>
class PlaygroundFeistel64
  : public GenericFeistel<PlaygroundFeistel64<ROUNDS_>,
                          std::uint64_t,
                          std::uint32_t>
{
public:
  static constexpr int ROUNDS = ROUNDS_;
  using Base =
    GenericFeistel<PlaygroundFeistel64<ROUNDS>, std::uint64_t, std::uint32_t>;
  explicit PlaygroundFeistel64(int Nbits)
    : Base(Nbits)
  {
    m_selector.fill(NONE);
    // m_selector={CRC32,XOR,MULTIPLY,CRC32, FN1VA};
    // this passes
    // m_selector={FN1VA,FN1VA,FN1VA,FN1VA};
    // m_selector={FN1VA,FN1VA,FN1VA};
    // m_selector={CRC32,FN1VA,FN1VA};
    // m_selector={FN1VA,CRC32,FN1VA};
    // m_selector={AES,AES,AES};
    // m_selector={AES,FN1VA,AES};

    // this does not pass
    // m_selector={FN1VA,FN1VA};
    /*
    m_selector={XOR,ROTATE,
                XOR,ROTATE,
                XOR,ROTATE,
                XOR,ROTATE,
                XOR,ROTATE,
                XOR,ROTATE,
                XOR,ROTATE,
                XOR,ROTATE};
                */
    // m_selector={CRC32,CRC32,CRC32,CRC32,CRC32,CRC32,CRC32,CRC32,CRC32,CRC32,CRC32,CRC32};
    // m_selector={AES,AES};

    /* fails bigcrush with:
    31  CouponCollector, r = 10         2.0e-6
    94  HammingCorr, L = 1200          1 - 3.7e-11
    m_selector = { AES, FN1VA, CRC32 };
    */


  }

  template<typename URBG>
  void seed(URBG&& urbg)
  {
    char seedarray[sizeof(m_key)];
    for (auto& e : seedarray) {
      e = urbg();
    }
    std::memcpy(&m_key, &seedarray[0], sizeof(m_key));
  }

  enum RoundFuncs
  {
    NONE,
    CRC32,
    FN1VA,
    ROTATE,
    POPCOUNT,
    PDEP1,
    PDEP2,
    PEXT1,
    PEXT2,
    DIVIDE,
    SUBTRACT,
    MULTIPLY,
    ADD,
    XOR,
    AES
  };
  static std::uint32_t hashfnv1a(const std::uint32_t value)
  {
    const std::uint32_t prime = 0x01000193;
    std::uint32_t hash = 0x811c9dc5;
    for (unsigned i = 0; i < sizeof(value); ++i) {
      const unsigned char byte = (value >> (8 * i)) & 0xFF;
      hash ^= byte;
      hash *= prime;
    }
    return hash;
  }

  std::uint32_t applyAes(const std::uint32_t x) const
  {
    static_assert(sizeof(m_key) >= 128 / 8, "key is too small");
    auto key = _mm_loadu_si128((__m128i*)m_key.data());

    __m128i m = _mm_set1_epi32(x);
    m = _mm_aesenc_si128(m, key);
    return _mm_extract_epi32(m, 0);
  }

  std::uint32_t roundFunction(const std::uint32_t x, int round) const
  {
    auto rf = m_selector[round];
    switch (rf) {
      case RoundFuncs::NONE:
        return x;
      case RoundFuncs::AES:
        return applyAes(x);
      case RoundFuncs::CRC32:
        return _mm_crc32_u32(m_key[round], x);
      case RoundFuncs::FN1VA:
        return hashfnv1a(x) ^ m_key[round];
      case RoundFuncs::ROTATE: {
        auto rotate = [](std::uint32_t x, unsigned n) {
          return (x << n) | (x >> ((-n) & 31));
        };
        return rotate(x, m_key[round] & 0x1F);
      }
      case RoundFuncs::POPCOUNT:
        return _mm_popcnt_u32(x);

      case RoundFuncs::PDEP1:
        return _pdep_u32(x, m_key[round]);
      case RoundFuncs::PDEP2:
        return _pdep_u32(m_key[round], x);
      case RoundFuncs::PEXT1:
        return _pext_u32(m_key[round], x);
      case RoundFuncs::PEXT2:
        return _pext_u32(x, m_key[round]);

      case RoundFuncs::DIVIDE:
        return x / m_key[round];

      case RoundFuncs::SUBTRACT:
        return m_key[round] - x;

      case RoundFuncs::MULTIPLY:
        return m_key[round] * x;
      case RoundFuncs::ADD:
        return m_key[round] + x;
      case RoundFuncs::XOR:
        return x ^ m_key[round];
    } // switch
    std::abort();
  } // func

private:
  std::array<RoundFuncs, ROUNDS_> m_selector;
  std::array<std::uint32_t, ROUNDS_> m_key;
  // int m_key;
};
