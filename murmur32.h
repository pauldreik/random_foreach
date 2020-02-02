#pragma once

#include <cstdint>

class Murmur32 {
public:
    explicit Murmur32(int nbits) {
        m_nbits=nbits;
        m_shift=nbits-nbits/2;
        m_mask=1;
        while(__builtin_popcount(m_mask)<nbits) {
            m_mask<<=1;
            m_mask |=1;
        }
    }
    template<typename URBG>
    void seed(URBG&& rng) {
        for(auto& e: m_keys) {
            e=rng();
        }
    }

    std::uint32_t encrypt(std::uint32_t x) const {
    assert( (x & ~mask)==0 && "high bits are set");
        x ^= (x>>m_shift);
        //x &= m_mask;

        x *= 0xcc9e2d51;
        x &= m_mask;

        x ^= (x>>m_shift);
        //x &= m_mask;

        x *= 0x1b873593;
        x &= m_mask;

        x ^= (x>>m_shift);
        //x &= m_mask;
        return x;
    }
private:
    int m_nbits;
    int m_shift;
    std::uint32_t m_mask;
    std::array<std::uint32_t,3> m_keys;
};
