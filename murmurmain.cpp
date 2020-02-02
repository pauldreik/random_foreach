

#include "murmurcrypt.h"
#include <cassert>
#include <random>

int
main()
{

  constexpr auto blah = MurmurCrypt::inv_prime(MurmurCrypt::prime1);
  constexpr auto blah2 = MurmurCrypt::inv_prime(MurmurCrypt::prime2);
  MurmurCrypt c;

  std::uint64_t count = 0;

  auto rd = std::random_device{};
  std::uint64_t offset = rd();
  offset <<= 32;
  offset |= rd();
  std::uint64_t flip = rd();
  flip <<= 32;
  flip |= rd();
  do {
    auto i = (count + offset) ^ flip;
    auto e = c.encrypt1(i);
    assert(e == c.encrypt2(i));
    auto d = c.decrypt(e);
    assert(d == i);
  } while (++count > 0);
}
