

#include "MurmurCryptFixed64.h"
#include <cassert>
#include <random>

// verify the encrypt/decrypt works as intended
void
testFixed64()
{
  MurmurCryptFixed64 c;

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
    auto e = c.encrypt(i);
    assert(e == c.encrypt_and_debug(i));
    auto d = c.decrypt(e);
    assert(d == i);
  } while (++count > 0);
}

int
main()
{
  testFixed64();
}
