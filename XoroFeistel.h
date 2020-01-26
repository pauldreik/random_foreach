#pragma once

// fix the name of this headers

#include <cstdint>
#include <cstring>

#include "GenericFeistel.h"

template<int ROUNDS_>
class XoroFeistel32
  : public GenericFeistel<XoroFeistel32<ROUNDS_>, std::uint32_t, std::uint16_t>
{
public:
  static constexpr int ROUNDS = ROUNDS_;
  using Base =
    GenericFeistel<XoroFeistel32<ROUNDS>, std::uint32_t, std::uint16_t>;
  explicit XoroFeistel32(int Nbits)
    : Base(Nbits){};

  template<typename URBG>
  void seed(URBG& urbg)
  {
    char seedarray[sizeof(m_key)];
    for (auto& e : seedarray) {
      e = urbg();
    }
    std::memcpy(&m_key, &seedarray[0], sizeof(m_key));
  }

  std::uint16_t roundFunction(const std::uint16_t x, int round) const
  {
    auto tmp = x ^ m_key;
    // from xorshift32
    tmp ^= tmp << 13;
    tmp ^= tmp >> 17;
    tmp ^= tmp << 5;
    return tmp;
  }

private:
  std::uint32_t m_key;
};
