#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <utf.h>

#include "../include/punycode.h"

int
main() {
  // Decoding never produces more code points than it consumes.
  {
    assert(utf32_max_length_from_punycode(0) == 0);
    assert(utf32_max_length_from_punycode(1) == 1);
    assert(utf32_max_length_from_punycode(63) == 63);
    assert(utf32_max_length_from_punycode(SIZE_MAX) == SIZE_MAX);
  }

  // Encoding allows for the digits of every code point plus the one delimiter.
  {
    assert(punycode_max_length_from_utf32(0) == 1);
    assert(punycode_max_length_from_utf32(1) == PUNYCODE_MAX_DIGITS + 1);
    assert(punycode_max_length_from_utf32(63) == 63 * PUNYCODE_MAX_DIGITS + 1);
  }

  // A length that cannot be scaled without overflowing is rejected rather than
  // wrapping to a bound too small for the encoder to write within.
  {
    assert(punycode_max_length_from_utf32(SIZE_MAX) == (size_t) -1);

    // The largest length that does fit, and the first that does not.
    size_t fits = (SIZE_MAX - 1) / PUNYCODE_MAX_DIGITS;

    size_t bound = punycode_max_length_from_utf32(fits);

    assert(bound != (size_t) -1);

    // A bound that had wrapped would come back smaller than the length it was
    // asked about, which is the failure the guard exists to prevent.
    assert(bound > fits);

    assert(punycode_max_length_from_utf32(fits + 1) == (size_t) -1);
  }
}
