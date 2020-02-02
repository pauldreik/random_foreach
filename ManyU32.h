#pragma once
/*
 * By Paul Dreik 2019,2020
 * https://www.pauldreik.se/
 * License: Boost 1.0
 * SPDX-License-Identifier: BSL-1.0
 */
#include <immintrin.h>
#include <cstdint>
#include <array>

struct ManyU32
{
  using Int = std::uint32_t;
  // sets all elements to the same value x
  explicit ManyU32(Int x) noexcept { m_x = _mm256_set1_epi32(x); }

  ManyU32(Int a, Int b, Int c, Int d, Int e, Int f, Int g, Int h)
  {
    // perhaps the wrong order
    m_x = _mm256_set_epi32(a, b, c, d, e, f, g, h);
  }
  explicit ManyU32(__m256i x) noexcept
    : m_x(x)
  {}

  ManyU32& operator^=(const ManyU32& other) noexcept
  {
    m_x = _mm256_xor_si256(m_x, other.m_x);
    return *this;
  }
  ManyU32& operator&=(const ManyU32& other) noexcept
  {
    m_x = _mm256_and_si256(m_x, other.m_x);
    return *this;
  }
  ManyU32& operator*=(const ManyU32& other) noexcept
  {
    m_x = _mm256_mullo_epi32(m_x, other.m_x);
    return *this;
  }
  ManyU32& operator+=(const ManyU32& other) noexcept
  {
    m_x = _mm256_add_epi32(m_x, other.m_x);
    return *this;
  }
  // avoid accidental use of if(obj), since
  // it is unclear if it means "any" or "all"
  // for a vector.
  operator bool() const = delete;

  template<int i>
  Int get() const noexcept
  {
    static_assert(i >= 0 && i < 8, "outside bounds");
    Int tmp[8];
    static_assert(sizeof(*this) == sizeof(tmp), "");
    _mm256_storeu_si256((__m256i*)&tmp[0], m_x);
    return tmp[i];
  }
  std::array<Int, 8> toArray() const
  {
    std::array<Int, 8> ret;
    _mm256_storeu_si256((__m256i*)&ret[0], m_x);
    return ret;
  }
  __m256i m_x;
};
ManyU32
operator^(const ManyU32& a, const ManyU32& b) noexcept
{
  return ManyU32{ _mm256_xor_si256(a.m_x, b.m_x) };
}
ManyU32 operator&(const ManyU32& a, const ManyU32& b) noexcept
{
  return ManyU32{ _mm256_and_si256(a.m_x, b.m_x) };
}
ManyU32
operator|(const ManyU32& a, const ManyU32& b) noexcept
{
  return ManyU32{ _mm256_or_si256(a.m_x, b.m_x) };
}
ManyU32
operator>>(const ManyU32& a, int n) noexcept
{
  assert(n >= 0);
  assert(n < 32);
  return ManyU32{ _mm256_srli_epi32(a.m_x, n) };
}
ManyU32
operator<<(const ManyU32& a, int n) noexcept
{
  assert(n >= 0);
  assert(n < 32);
  return ManyU32{ _mm256_slli_epi32(a.m_x, n) };
}
ManyU32
operator==(const ManyU32& a, const ManyU32& b) noexcept
{
  return ManyU32{ _mm256_cmpeq_epi32(a.m_x, b.m_x) };
}
