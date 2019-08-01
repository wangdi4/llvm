// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -pedantic -verify %s

int main(void) {
  int i = 0, x = 0;
  // Check that we have *NOT* allowed such erroneous constructs.
  #pragma unroll(=4) // expected-error{{expected expression}} \
                     // expected-warning{{extra tokens at end of '#pragma unroll' - ignored}}
  for (i = 0; i < 20; ++i)
    ++x;

  // Check that we have allowed arguments via '='.
  #pragma unroll = 2
  for (i = 0; i < 10; ++i)
    ++x;

  // Check that default spelling is handled without any diagnostics.
  #pragma unroll(4)
  for (i = 0; i < 20; ++i)
    ++x;

  #pragma unroll 4
  for (i = 0; i < 20; ++i)
    ++x;

  return 0;
}
