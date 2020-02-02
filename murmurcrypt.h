#ifndef MURMURCRYPT_H
#define MURMURCRYPT_H

#include <cassert>
#include <cstdint>
#include <initializer_list>
#include <type_traits>
/**
 * @brief The MurmurCrypt class
 * Encrypts using the same idea as the murmur crypt is based on.
 * Suggest by Martin Ankerl
 * See https://github.com/pauldreik/random_foreach/issues/4
 */
class MurmurCrypt
{
public:
  MurmurCrypt();

  std::uint64_t encrypt_original(std::uint64_t h)
  {
    h ^= h >> 33;
    h *= prime1;
    h ^= h >> 33;
    h *= prime2;
    h ^= h >> 33;
    return h;
  }
  std::uint64_t encrypt(std::uint64_t h)
  {
    xor_and_shift<33>(h);
    multiply_with_prime<prime1>(h);
    xor_and_shift<33>(h);
    multiply_with_prime<prime2>(h);
    xor_and_shift<33>(h);
    return h;
  }

  // encrypts, while asserting all the steps can be decrypted.
  // for debugging.
  std::uint64_t encrypt_and_debug(std::uint64_t h)
  {
    const std::uint64_t h1 = h;

    xor_and_shift<33>(h);
    const std::uint64_t h2 = h;
    assert(h1 == undo_xor_and_shift<33>(h));

    multiply_with_prime<prime1>(h);
    const std::uint64_t h3 = h;
    assert(h2 == undo_multiply_with_prime<prime1>(h));

    xor_and_shift<33>(h);
    const std::uint64_t h4 = h;
    assert(h3 == undo_xor_and_shift<33>(h));

    multiply_with_prime<prime2>(h);
    const std::uint64_t h5 = h;
    assert(h4 == undo_multiply_with_prime<prime2>(h));

    xor_and_shift<33>(h);
    const std::uint64_t h6 = h;
    assert(h5 == undo_xor_and_shift<33>(h));

    return h;
  }
  std::uint64_t decrypt(std::uint64_t h)
  {
    h = undo_xor_and_shift<33>(h);
    h = undo_multiply_with_prime<prime2>(h);
    h = undo_xor_and_shift<33>(h);
    h = undo_multiply_with_prime<prime1>(h);
    h = undo_xor_and_shift<33>(h);
    return h;
  }

  // private:
  template<int shift>
  void xor_and_shift(std::uint64_t& h)
  {
    static_assert(shift >= 32, "shift should be >=32 for this to work");
    static_assert(shift < 64, "shift should be less than 64");
    h ^= (h >> shift);
  }

  template<int shift>
  std::uint64_t undo_xor_and_shift(std::uint64_t h)
  {
    static_assert(shift >= 32, "shift should be >=32 for this to work");
    static_assert(shift < 64, "shift should be less than 64");
    h ^= (h >> (shift));
    return h;
  }
  template<std::uint64_t prime>
  void multiply_with_prime(std::uint64_t& h)
  {
    h *= prime;
  }
  template<std::uint64_t prime>
  std::uint64_t undo_multiply_with_prime(std::uint64_t h)
  {
    multiply_with_prime<inv_prime(prime)>(h);
    return h;
  }

  /**
   * This calculates the factor k such that any unkown number u multipled with
   * prime can be recovered, by multiplying with k modulo 2^64 (if Uinteger
   * is 64 bit). It does so by running the extended euclid algorithm, but
   * it is a bit shaky because I had to improvise to avoid overflows
   */
  template<typename UInteger>
  static constexpr UInteger inv_prime(const UInteger prime)
  {
    static_assert(std::is_integral_v<UInteger>, "");
    static_assert(std::is_unsigned_v<UInteger>, "");

    using Integer = std::make_signed_t<UInteger>;

    // a is 2^64
    // b is prime. the multiplicative inverse is the y in
    // ax+by=gcd(a,b)
    Integer xk = 0;
    Integer yk = 1;
    Integer xkm1 = 1;
    Integer ykm1 = 0;
    // found by the extended euclid algorithm.
    static_assert(prime > 2, "avoid edge case");

    // count of iterations
    int k = 1;

    // do the first iteration of the loop manually to avoid overflow.
    ++k;

    // UInteger tmpa=2**64
    UInteger tmpb = prime;
    // for 64 bit, this should be 2**64 / b, which is calculated without
    // overflow
    const UInteger m_minus_1 = ~UInteger{ 0 };
    UInteger q = (m_minus_1) / tmpb;
    UInteger r = (m_minus_1 - q * tmpb) + 1;

    Integer tmp = q * xk + xkm1;
    xkm1 = xk;
    xk = tmp;

    tmp = q * yk + ykm1;
    ykm1 = yk;
    yk = tmp;
    Integer tmpa = tmpb;
    tmpb = r;
    while (r != 0) {
      ++k;
      q = tmpa / tmpb;
      r = tmpa - q * tmpb;

      tmp = q * xk + xkm1;
      xkm1 = xk;
      xk = tmp;

      tmp = q * yk + ykm1;
      ykm1 = yk;
      yk = tmp;
      tmpa = tmpb;
      tmpb = r;
    }
    Integer gcd = tmpa;
    // static_assert(gcd==1,"assumption on prime violated");

    UInteger factor = 0;
    if (k % 2 == 0) {
      factor = ykm1;
    } else {
      // the factor is -ykml, but we cant
      // use that with unsigned, so use 2^64-ykml instead calculated
      // in a non-overflowing way
      factor = (m_minus_1 - ykm1) + 1;
    }
    return factor;
  }

  static constexpr std::uint64_t prime1 = 0xff51afd7ed558ccd;
  static constexpr std::uint64_t prime2 = 0xc4ceb9fe1a85ec53;
};

#endif // MURMURCRYPT_H
