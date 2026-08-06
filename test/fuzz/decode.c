#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <utf.h>

#include "../../include/punycode.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  size_t bound = utf32_max_length_from_punycode(size);

  utf32_t *decoded = (utf32_t *) malloc(bound * sizeof(utf32_t) + 1);

  if (decoded == NULL) return 0;

  size_t decoded_len;

  int err = punycode_decode_utf8((const utf8_t *) data, size, decoded, &decoded_len);

  // Widening the input must not change what the decoder makes of it, whether it
  // accepts it or not.
  {
    utf32_t *widened = (utf32_t *) malloc(size * sizeof(utf32_t) + 1);
    utf32_t *wide = (utf32_t *) malloc(bound * sizeof(utf32_t) + 1);

    if (widened != NULL && wide != NULL) {
      for (size_t i = 0; i < size; i++) {
        widened[i] = data[i];
      }

      size_t wide_len;

      int wide_err = punycode_decode_utf32(widened, size, wide, &wide_len);

      assert((wide_err == 0) == (err == 0));

      if (wide_err == 0) {
        assert(wide_len == decoded_len);

        for (size_t i = 0; i < wide_len; i++) {
          assert(wide[i] == decoded[i]);
        }
      }
    }

    free(wide);
    free(widened);
  }

  if (err == 0) {
    // The bound must hold, or the decoder wrote past what a caller would have
    // allocated for it.
    assert(decoded_len <= bound);

    for (size_t i = 0; i < decoded_len; i++) {
      // Every code point decoded must be one that Unicode allows to stand on its
      // own.
      assert(decoded[i] <= 0x10ffff);
      assert(decoded[i] < 0xd800 || decoded[i] > 0xdfff);
    }

    // Whatever was decoded must encode back to something that decodes to the very
    // same code points. The encoded form itself need not match the input, which
    // may hold digits in uppercase or a number that is not the shortest one.
    size_t encoded_bound = punycode_max_length_from_utf32(decoded_len);

    assert(encoded_bound != (size_t) -1);

    utf8_t *encoded = (utf8_t *) malloc(encoded_bound + 1);

    if (encoded != NULL) {
      size_t encoded_len;

      err = punycode_encode_utf8(decoded, decoded_len, encoded, &encoded_len);
      assert(err == 0);
      assert(encoded_len <= encoded_bound);

      assert(ascii_validate(encoded, encoded_len));

      utf32_t *again = (utf32_t *) malloc(utf32_max_length_from_punycode(encoded_len) * sizeof(utf32_t) + 1);

      if (again != NULL) {
        size_t again_len;

        err = punycode_decode_utf8(encoded, encoded_len, again, &again_len);
        assert(err == 0);
        assert(again_len == decoded_len);

        for (size_t i = 0; i < decoded_len; i++) {
          assert(again[i] == decoded[i]);
        }

        free(again);
      }

      free(encoded);
    }
  }

  free(decoded);

  return 0;
}
