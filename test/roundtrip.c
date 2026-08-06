#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <utf.h>

#include "../include/punycode.h"
#include "samples.h"

// The longest run that any of the cases below round trips, and the buffer that
// encoding a run that long calls for.
#define MAX_LEN     512
#define MAX_ENCODED (MAX_LEN * PUNYCODE_MAX_DIGITS + 1)

// Encodes `data` and decodes the result, asserting that the code points come back
// unchanged.
static void
roundtrip(const utf32_t *data, size_t len) {
  int e;

  assert(len <= MAX_LEN);

  size_t bound = punycode_max_length_from_utf32(len);
  assert(bound != (size_t) -1);
  assert(bound <= MAX_ENCODED);

  utf8_t encoded[MAX_ENCODED];

  size_t encoded_len;

  e = punycode_encode_utf8(data, len, encoded, &encoded_len);
  assert(e == 0);
  assert(encoded_len <= bound);

  // An extended string holds nothing but basic code points, and so is valid
  // ASCII.
  assert(ascii_validate(encoded, encoded_len));

  // Decoding never produces more code points than the basic ones it consumes.
  utf32_t decoded[MAX_ENCODED];

  assert(utf32_max_length_from_punycode(encoded_len) <= MAX_ENCODED);

  size_t decoded_len;

  e = punycode_decode_utf8(encoded, encoded_len, decoded, &decoded_len);
  assert(e == 0);

  assert(decoded_len == len);

  for (size_t i = 0; i < len; i++) {
    assert(decoded[i] == data[i]);
  }
}

int
main() {
  // Every one of the samples survives a round trip.
  for (size_t i = 0; i < PUNYCODE_SAMPLES; i++) {
    printf("%s\n", punycode_samples[i].name);

    roundtrip(punycode_samples[i].decoded, punycode_samples[i].decoded_len);
  }

  // So does nothing at all.
  roundtrip(NULL, 0);

  // A single code point, taken from across the whole of Unicode and skipping the
  // surrogates, which are not code points that may stand on their own.
  {
    for (utf32_t c = 0; c <= 0x10ffff; c++) {
      if (c >= 0xd800 && c <= 0xdfff) continue;

      roundtrip(&c, 1);
    }
  }

  // A run of the same code point, which drives the deltas of the encoder up
  // without changing what it has to encode.
  {
    utf32_t data[MAX_LEN];

    for (size_t len = 1; len <= MAX_LEN; len *= 2) {
      for (size_t i = 0; i < len; i++) {
        data[i] = 0x10348;
      }

      roundtrip(data, len);
    }
  }

  // A run that climbs, so that every code point of it is a fresh one for the
  // encoder to handle.
  {
    utf32_t data[MAX_LEN];

    for (size_t i = 0; i < MAX_LEN; i++) {
      data[i] = (utf32_t) (0x4e00 + i);
    }

    roundtrip(data, MAX_LEN);
  }

  // A run that descends, the encoder having to come back around for each.
  {
    utf32_t data[MAX_LEN];

    for (size_t i = 0; i < MAX_LEN; i++) {
      data[i] = (utf32_t) (0x4e00 + MAX_LEN - i);
    }

    roundtrip(data, MAX_LEN);
  }

  // Basic and non-basic code points interleaved, exercising both the literal
  // portion and the encoded one.
  {
    utf32_t data[MAX_LEN];

    for (size_t i = 0; i < MAX_LEN; i++) {
      data[i] = i % 2 == 0 ? (utf32_t) ('a' + i % 26) : (utf32_t) (0x300 + i);
    }

    roundtrip(data, MAX_LEN);
  }
}
