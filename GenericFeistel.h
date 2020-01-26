/*
 * By Paul Dreik 2019,2020
 * https://www.pauldreik.se/
 * License: Boost 1.0
 * SPDX-License-Identifier: BSL-1.0
 */

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>

/**
 * CRTP base class for a Feistel block crypto
 *
 * see https://en.wikipedia.org/wiki/Feistel_cipher
 *
 * EncryptTypeFull is the block type (typically unsigned int).
 * EncryptTypeHalf is what will be used to store the left and right halves
 * respectively.
 * Derived must implement ROUNDS to tell how many rounds there are, as well
 * as the round function with signature compatible with:
 * EncryptTypeHalf roundFunction(EncryptTypeHalf x, int round);
 * Thanks to the Feistel construction, anything deterministic that compiles
 * will do, even if the cryptographic quality and cpu consumption depend on
 * it.
 */
template<typename Derived, typename EncryptTypeFull, typename EncryptTypeHalf>
class GenericFeistel
{
public:
  enum Direction
  {
    Encrypt,
    Decrypt
  };
  constexpr static int rounds()
  {
    static_assert(Derived::ROUNDS >= 0, "ROUNDS must be 0 or larger");
    return Derived::ROUNDS;
  }
  EncryptTypeFull encrypt(const EncryptTypeFull& cleartext)
  {
    return commonEncryptAndDecrypt<Encrypt>(cleartext);
  }
  EncryptTypeFull decrypt(const EncryptTypeFull& ciphertext)
  {
    return commonEncryptAndDecrypt<Decrypt>(ciphertext);
  }

  template<Direction direction>
  EncryptTypeFull commonEncryptAndDecrypt(const EncryptTypeFull& input)
  {
    if constexpr (rounds() == 0)
      return input;
    EncryptTypeHalf left = (input >> m_Nbitshalf) & m_mask;
    EncryptTypeHalf right = input & m_mask;
    int round;
    for (int i = 0; i < rounds(); ++i) {
      if constexpr (direction == Encrypt)
        round = i;
      else
        round = rounds() - 1 - i;
      EncryptTypeHalf Fresult = This()->roundFunction(right, round);
      left ^= Fresult;
      left &= m_mask;
      std::swap(left, right);
    }
    std::swap(left, right);
    return (EncryptTypeFull{ left } << m_Nbitshalf) | right;
  }

protected:
  const int m_Nbitshalf;
  const EncryptTypeHalf m_mask;
  explicit GenericFeistel(int Nbits)
    : m_mask((1ULL << (Nbits / 2)) - 1)
    , m_Nbitshalf(Nbits / 2)
  {
    assert(Nbits <= 8 * sizeof(EncryptTypeFull));
  }

private:
  Derived* This() { return static_cast<Derived*>(this); }
  const Derived* This() const { return static_cast<const Derived*>(this); }
};
