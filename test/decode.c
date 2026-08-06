#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <utf.h>

#include "../include/punycode.h"
#include "samples.h"

int
main() {
  int e;

  for (size_t i = 0; i < PUNYCODE_SAMPLES; i++) {
    const punycode_sample_t *sample = &punycode_samples[i];

    printf("%s\n", sample->name);

    utf32_t decoded[256];

    assert(utf32_max_length_from_punycode(sample->encoded_len) <= 256);

    size_t decoded_len;

    e = punycode_decode_utf8(sample->encoded, sample->encoded_len, decoded, &decoded_len);
    assert(e == 0);

    assert(decoded_len == sample->decoded_len);

    for (size_t j = 0; j < decoded_len; j++) {
      assert(decoded[j] == sample->decoded[j]);
    }

    // Widening the extended string to code points must leave the decoder with
    // the very same input, and so give the very same result.
    utf32_t widened[256];

    for (size_t j = 0; j < sample->encoded_len; j++) {
      widened[j] = sample->encoded[j];
    }

    utf32_t wide[256];
    size_t wide_len;

    e = punycode_decode_utf32(widened, sample->encoded_len, wide, &wide_len);
    assert(e == 0);

    assert(wide_len == decoded_len);

    for (size_t j = 0; j < wide_len; j++) {
      assert(wide[j] == decoded[j]);
    }
  }
}
