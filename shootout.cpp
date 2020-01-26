/*
 * This function is for performance testing different toy ciphers.
 *
 * See https://www.pauldreik.se/talks/20200123_crypto/
 *
 * By Paul Dreik 2019,2020
 * https://www.pauldreik.se/
 * License: Boost 1.0
 * SPDX-License-Identifier: BSL-1.0
 */
#include <cassert>
#include <functional>
#include <random>
#include <string>
#include <vector>
#include <memory>

#include "AesFunc.h"
#include "GenericFeistel.h"
#include "LazyFisherYates.h"
#include "PlaygroundFeistel.h"
#include "ShaFeistel.h"
#include "XoroFeistel.h"
#include "simdfeistel.h"
#include "Fnv1aCiphers.h"



// empty test function in another translation unit
void
donothing(unsigned int);


template<typename Integer, typename Callback>
void
std_shuffle(Integer N, Callback&& cb)
{
  std::unique_ptr<Integer[]> storage(new Integer[N]);
  Integer* v=storage.get();
  for (Integer i = 0; i < N; ++i) {
    v[i] = i;
  }
  std::shuffle(v, v + N, std::random_device{});
  for (Integer i = 0; i < N; ++i) {
    cb(i);
  }
}

template<typename Integer, typename Callback>
void
std_shuffle_vector(Integer N, Callback&& cb)
{
  std::vector<Integer> v(N);
  for (Integer i = 0; i < N; ++i) {
    v[i] = i;
  }
  std::shuffle(begin(v), end(v) , std::random_device{});
  for (Integer i = 0; i < N; ++i) {
    cb(i);
  }
}


template<typename Integer, typename Callback>
void
ordinary_for(Integer N, Callback&& cb)
{
  for (Integer i = 0; i < N; ++i) {
    cb(i);
  }
}
/**
 * this is to compare with ordinary_for, so the overhead of invoking
 * f can be measured compared to the loop
 */
template<typename Integer, typename Callback>
void
ordinary_for_twice(Integer N, Callback&& cb)
{
  for (Integer i = 0; i < N; ++i) {
    cb(i);
    cb(i);
  }
}

/**
 * This may actually optimize differently than the ordinary for loop
 */
template<typename Integer, typename Callback>
void
do_while(Integer N, Callback&& cb)
{
  Integer i = 0;
  do {
    cb(i);
  } while (++i < N);
}

/**
 * sequential with optional unrolling
 */
template<int Unroll, typename Integer, typename Callback>
void
sequential_for_each(Integer N, Callback&& cb)
{
  if constexpr (Unroll == 1) {
    for (Integer i = 0; i < N; ++i) {
      cb(i);
    }
  } else {
    static_assert(Unroll > 1 && Unroll <= 4, "handle other unroll values");
    Integer i = 0;
    const Integer stop = (N / Unroll) * Unroll;
    for (; i < stop; i += Unroll) {
      cb(i);
      if constexpr (Unroll > 1)
        cb(i + 1);
      if constexpr (Unroll > 2)
        cb(i + 2);
      if constexpr (Unroll > 3)
        cb(i + 3);
      if constexpr (Unroll > 4)
        cb(i + 4);
    }
    for (; i < N; ++i) {
      cb(i);
    }
  }
}

/**
 * sequential with xor (to see if the branch predictor gets
 * a harder time than sequential)
 */
template<typename Integer, typename URNG, typename Callback>
void
xored_for_each(Integer N, URNG&& rng, Callback&& cb)
{
  static_assert(sizeof(decltype(rng())) >= sizeof(Integer));
  Integer key = rng();
  // lazy way of zeroing the upper bits
  while (key > N) {
    key >>= 1;
  }

  Integer count = 0;
  for (Integer i = 0; count < N; ++i) {
    const auto k = i ^ key;
    if (k < N) {
      cb(k);
      ++count;
    }
  }
}

/**
 * invokes cb N times with an integer in [0,N) selected
 * randomly each time
 */
template<typename Integer, typename URBG, typename Callback>
void
random_for_each(Integer N, URBG&& rng, Callback&& cb)
{
  assert(N > 0);
  std::uniform_int_distribution<Integer> dist(0, N - 1);
  for (Integer i = 0; i < N; ++i) {
    Integer j = dist(rng);
    cb(j);
  }
}

/**
 * block cipher based visitation of each integer exactly
 * once
 */
template<typename Feistel, typename Integer, typename URBG, typename Callback>
void
feistel_for_each(Integer M, URBG&& rng, Callback&& cb)
{
  // how many bits do we need?
  int bitsneeded = static_cast<int>(std::ceil(std::log2(M)));
  // round up to even
  bitsneeded /= 2;
  bitsneeded *= 2;

  if (bitsneeded <= 32) {
    Feistel cipher(bitsneeded);
    // auto s=sizeof(cipher);
    cipher.seed(rng);
    Integer count = 0;
    for (Integer i = 0; count < M; ++i) {
      auto encrypted = cipher.encrypt(i);
      if (encrypted < M) {
        cb(encrypted);
        ++count;
      }
    }
    return;
  }
  std::puts("implement switching to 64 bit");
  std::abort();
}

/**
 * like feistel_for_each, but simd parallelized
 */
template<typename Integer, typename URBG, typename Callback>
void
simdfeistel_for_each(Integer M, URBG&& rng, Callback&& cb)
{
  // how many bits do we need?
  int bitsneeded = static_cast<int>(std::ceil(std::log2(M)));
  // round up to even
  bitsneeded /= 2;
  bitsneeded *= 2;

  if (bitsneeded <= 32) {
    ParallelFeistel cipher(bitsneeded);
    cipher.seed(rng);
    ManyU32 II(0, 1, 2, 3, 4, 5, 6, 7);
    const ManyU32 ones(1);
    for (Integer count = 0; count < M; II += ones) {
      auto ea = cipher.encrypt(II).toArray();
      for (auto encrypted : ea) {
        if (encrypted < M) {
          cb(encrypted);
          ++count;
          if (count >= M) {
            return;
          }
        }
      }
    }
    return;
  }
  std::puts("implement switching to 64 bit");
  std::abort();
}

int
main(int argc, char* argv[])
{
  assert(argc > 1 && "first arg should be algo name");
  const std::string algoname{ argv[1] };

  // arg 2 - size of test
  const unsigned long long Ntmp = argc > 2 ? std::stoull(argv[2]) : (1U << 30);

  using Integer = std::uint32_t;
  if (Ntmp >= std::numeric_limits<Integer>::max()) {
    std::puts("sorry, too large for the test");
    return EXIT_FAILURE;
  }
  const Integer N = static_cast<Integer>(Ntmp);

  std::map<std::string, std::function<void()>> functions;

  auto work = [](auto x) {
#if 1
      donothing(x);
#else
    auto actual = count_set_bits(x);
    auto expected = __builtin_popcount(x);
    assert(actual == expected);
#endif
  };

  functions["xor"] = [&]() { xored_for_each(N, std::random_device{}, work); };
  functions["dowhile"] = [&]() { do_while(N, work); };
  functions["sequential"] = [&]() { ordinary_for(N, work); };
  functions["sequential_twice"] = [&]() { ordinary_for_twice(N, work); };
  functions["sequential_unroll1"] = [&]() { sequential_for_each<1>(N, work); };
  functions["sequential_unroll2"] = [&]() { sequential_for_each<2>(N, work); };
  functions["sequential_unroll3"] = [&]() { sequential_for_each<3>(N, work); };
  functions["sequential_unroll4"] = [&]() { sequential_for_each<4>(N, work); };
  functions["random_minstd"] = [&]() {
    random_for_each(N, std::minstd_rand{ std::random_device{}() }, work);
  };

  functions["random_mt19937"] = [&]() {
    random_for_each(N, std::mt19937{ std::random_device{}() }, work);
  };
  functions["random_mt19937_64"] = [&]() {
    random_for_each(N, std::mt19937_64{ std::random_device{}() }, work);
  };

  functions["lazy_fisher_yates_19937"] = [&]() {
    lazy_fisher_yates(N, std::mt19937{ std::random_device{}() }, work);
  };
  functions["std_shuffle"] = [&]() { std_shuffle(N, work); };

  functions["std_shuffle_vector"] = [&]() { std_shuffle_vector(N, work); };

  functions["fn1va_feistel"] = [&]() {
    feistel_for_each<Dynamic32>(N, std::random_device{}, work);
  };

  functions["aes_feistel"] = [&]() {
    feistel_for_each<Aes32<2>>(N, std::random_device{}, work);
  };
  functions["aes_feistel_rounds4"] = [&]() {
    feistel_for_each<Aes32<4>>(N, std::random_device{}, work);
  };
  functions["sha1_feistel"] = [&]() {
    feistel_for_each<ShaFeistel32<2>>(N, std::random_device{}, work);
  };

  functions["xoro_feistel"] = [&]() {
    feistel_for_each<XoroFeistel32<2>>(N, std::random_device{}, work);
  };
  functions["playground_feistel"] = [&]() {
    feistel_for_each<PlaygroundFeistel<2>>(N, std::random_device{}, work);
  };

  functions["simd_feistel"] = [&]() {
    simdfeistel_for_each(N, std::random_device{}, work);
  };
  if (algoname == "--list") {
    for (auto& e : functions) {
      std::puts(e.first.c_str());
    }
    std::exit(EXIT_SUCCESS);
  }
  if (algoname == "--compiler") {
#define STRINGIFY2(x) #x
#define STRINGIFY(x) STRINGIFY2(x)
#if defined(__clang__)
    std::puts("clang-" STRINGIFY(__clang_major__));
#elif defined(__GNUC__)
    std::puts("gcc-" STRINGIFY(__GNUC__));
#else
    std::puts("unknown");
#endif
    std::exit(EXIT_SUCCESS);
  }
  auto it = functions.find(algoname);
  if (it == functions.end()) {
    std::puts("could not find that function");
    std::exit(EXIT_FAILURE);
  }
  it->second();
}
