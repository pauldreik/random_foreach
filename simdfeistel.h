#pragma once
/*
 * By Paul Dreik 2019,2020
 * https://www.pauldreik.se/
 * License: Boost 1.0
 * SPDX-License-Identifier: BSL-1.0
 */
#include <immintrin.h>

#include "GenericFeistel.h"

#include "ManyU32.h"

// 16 bit fnv1a hash, assuming the input is
// zero in the most significant bits.
static ManyU32
hashfnv1a_16(ManyU32 value) noexcept
{
  const ManyU32 prime{ 0x1000193 };
  const ManyU32 lastbytemask{ 0xFF };

  ManyU32 hash{ 0x811c9dc5 };

  /*  This is what is supposed to happen:
  for (int shift = 0; shift < 32; shift += 8) {
    hash ^= ((value >> shift) & lastbytemask);
    hash *= prime;
  }
   but unroll it manually and remove steps not neccessary
   because we know we are interested in the bottom 16 bits only
   and know the upper ones are zero
  */
  hash ^= (value & lastbytemask);
  hash *= prime;

  hash ^= (value >> 8);
  hash *= prime;

  // there is no 16 bit fnv hash, we fold the halves
  // onto each other to make it one
  return hash ^ (hash >> 16);
}

/**
 * @brief The ParallelFeistel class
 * A try to encrypt multiple values in parallel, using simd, to gain
 * some speed.
 *
 * cycles for 2**30 values, skylake:
 * capto16  9152170408 # incorrect fnv (truncated)
 * proper16 9462775491 # correct fnv (folded)
 * aes      9566842315
 */
class ParallelFeistel : public GenericFeistel<ParallelFeistel, ManyU32, ManyU32>
{
public:
  static constexpr int ROUNDS = 2;
  using Base = GenericFeistel<ParallelFeistel, ManyU32, ManyU32>;
  explicit ParallelFeistel(int Nbits)
    : Base(Nbits)
  {
    m_key.fill(0);
  }
  template<typename URBG>
  void seed(URBG&& urbg)
  {
    for (auto& e : m_key) {
      e = urbg();
    }
  }

  ManyU32 roundFunction( ManyU32 x, int round) const
  {
    return hashfnv1a_16(x) ^ ManyU32 { m_key[round] };
  }

private:
  std::array<std::uint16_t, ROUNDS> m_key;
};
