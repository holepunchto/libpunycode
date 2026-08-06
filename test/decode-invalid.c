#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <utf.h>

#include "../include/punycode.h"

// Each of these was checked against a reference implementation, which rejects
// every one of them.
static const char *invalid[] = {
  // A code point that is not a digit of the base.
  "!",
  "a!",
  " ",

  // A code point of the literal portion that is not basic.
  "\xff-a",

  // A number that runs out of input before its last digit.
  "zzzzzzzzzzzz",

  // A delta too large to be held in the 32 bits that the bootstring parameters
  // allow for.
  "99999999999",

  // A delimiter with nothing before it leaves no basic code points, which both
  // this and the reference implementation take as leaving the delimiter itself to
  // be decoded as a digit, so it is rejected.
  "-",
  "-abc",
};

#define INVALID (sizeof(invalid) / sizeof(const char *))

int
main() {
  int e;

  for (size_t i = 0; i < INVALID; i++) {
    size_t len = strlen(invalid[i]);

    printf("%s\n", invalid[i]);

    utf32_t decoded[256];

    assert(utf32_max_length_from_punycode(len) <= 256);

    size_t decoded_len;

    e = punycode_decode_utf8((const utf8_t *) invalid[i], len, decoded, &decoded_len);
    assert(e == -1);

    // The two widths must reject in step with one another.
    utf32_t widened[256];

    for (size_t j = 0; j < len; j++) {
      widened[j] = (utf8_t) invalid[i][j];
    }

    e = punycode_decode_utf32(widened, len, decoded, &decoded_len);
    assert(e == -1);
  }

  // A code point above the basic ones is rejected by the wide decoder just as a
  // byte above them is by the narrow one. Only the wide decoder can be handed
  // such an input, a byte having nowhere to put it.
  {
    static const utf32_t beyond_basic[] = {0x100, '-', 'a'};

    utf32_t decoded[256];
    size_t decoded_len;

    e = punycode_decode_utf32(beyond_basic, 3, decoded, &decoded_len);
    assert(e == -1);
  }

  // An input that holds nothing but a literal portion is not invalid, there being
  // no number in it to be malformed. These are the counterparts of the rejected
  // inputs above, and a reference implementation accepts each of them.
  {
    utf32_t decoded[256];
    size_t decoded_len;

    e = punycode_decode_utf8((const utf8_t *) "", 0, decoded, &decoded_len);
    assert(e == 0);
    assert(decoded_len == 0);

    e = punycode_decode_utf8((const utf8_t *) "abc-", 4, decoded, &decoded_len);
    assert(e == 0);
    assert(decoded_len == 3);
    assert(decoded[0] == 'a' && decoded[1] == 'b' && decoded[2] == 'c');

    // The last delimiter is the one that ends the literal portion, so the two
    // before it are literal code points.
    e = punycode_decode_utf8((const utf8_t *) "---", 3, decoded, &decoded_len);
    assert(e == 0);
    assert(decoded_len == 2);
    assert(decoded[0] == '-' && decoded[1] == '-');
  }
}
