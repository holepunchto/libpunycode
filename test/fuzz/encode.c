#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <utf.h>

#include "../../include/punycode.h"

int
LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  // Take the input as UTF-8 so that the fuzzer arrives at sequences of code
  // points, rather than at the raw words that reading it as UTF-32 would give.
  if (!utf8_validate((const utf8_t *) data, size)) return 0;

  size_t len = utf32_length_from_utf8((const utf8_t *) data, size);

  utf32_t *decoded = (utf32_t *) malloc(len * sizeof(utf32_t) + 1);

  if (decoded == NULL) return 0;

  len = utf8_convert_to_utf32((const utf8_t *) data, size, decoded);

  size_t bound = punycode_max_length_from_utf32(len);

  if (bound == (size_t) -1) {
    free(decoded);

    return 0;
  }

  utf8_t *encoded = (utf8_t *) malloc(bound + 1);

  if (encoded == NULL) {
    free(decoded);

    return 0;
  }

  size_t encoded_len;

  int err = punycode_encode_utf8(decoded, len, encoded, &encoded_len);

  if (err == 0) {
    // The bound must hold, or the encoder wrote past what a caller would have
    // allocated for it.
    assert(encoded_len <= bound);

    // An extended string holds nothing but basic code points.
    assert(ascii_validate(encoded, encoded_len));

    // Encoding is lossless, so the code points must survive being decoded again.
    utf32_t *again = (utf32_t *) malloc(utf32_max_length_from_punycode(encoded_len) * sizeof(utf32_t) + 1);

    if (again != NULL) {
      size_t again_len;

      err = punycode_decode_utf8(encoded, encoded_len, again, &again_len);
      assert(err == 0);
      assert(again_len == len);

      for (size_t i = 0; i < len; i++) {
        assert(again[i] == decoded[i]);
      }

      free(again);
    }
  }

  free(encoded);
  free(decoded);

  return 0;
}
