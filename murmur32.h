#pragma once
/*
 * By Paul Dreik 2019,2020
 * https://www.pauldreik.se/
 * License: Boost 1.0
 * SPDX-License-Identifier: BSL-1.0
 */
#include <cstdint>

#include "ManyU32.h"

class Murmur32 {
public:
    explicit Murmur32(int nbits)  {
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
            e=rng() & m_mask;
        }
    }

    std::uint32_t encrypt(std::uint32_t x) const {
    assert( (x & ~mask)==0 && "high bits are set");
        x ^= m_keys[0];
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



class SimdMurmur32 {
public:
    explicit SimdMurmur32(int nbits) : m_mask{0U} {
        m_nbits=nbits;
        m_shift=nbits-nbits/2;
        std::uint32_t mask=1;
        while(__builtin_popcount(mask)<nbits) {
            mask<<=1;
            mask |=1;
        }
        m_mask=ManyU32{mask};
    }
    template<typename URBG>
    void seed(URBG&& rng) {
        auto mask=m_mask.get<0>();
        for(auto& e: m_keys) {
            e=rng() & mask;
        }
                m_mask=ManyU32{mask};
    }

    ManyU32 encrypt(ManyU32 x) const {
        x ^= ManyU32{m_keys[0]};
        x ^= (x>>m_shift);
        //x &= m_mask;

        x *= ManyU32{0xcc9e2d51};
        x &= m_mask;

        x ^= (x>>m_shift);
        //x &= m_mask;

        x *= ManyU32{0x1b873593};
        x &= m_mask;

        x ^= (x>>m_shift);
        //x &= m_mask;
        return x;
    }
private:
    int m_nbits;
    int m_shift;
    ManyU32 m_mask;
    std::array<std::uint32_t,3> m_keys;
};
