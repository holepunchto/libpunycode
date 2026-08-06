#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <utf.h>

#include "../include/punycode.h"
#include "samples.h"

int
main() {
  int e;

  for (size_t i = 0; i < PUNYCODE_SAMPLES; i++) {
    const punycode_sample_t *sample = &punycode_samples[i];

    printf("%s\n", sample->name);

    utf8_t encoded[1024];

    size_t bound = punycode_max_length_from_utf32(sample->decoded_len);

    assert(bound != (size_t) -1);
    assert(bound <= 1024);

    size_t encoded_len;

    e = punycode_encode_utf8(sample->decoded, sample->decoded_len, encoded, &encoded_len);
    assert(e == 0);

    // The bound must hold for every one of the samples, or the encoder wrote
    // past what a caller would have allocated.
    assert(encoded_len <= bound);

    assert(encoded_len == sample->encoded_len);
    assert(memcmp(encoded, sample->encoded, encoded_len) == 0);
  }
}
