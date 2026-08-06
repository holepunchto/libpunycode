#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <utf.h>

#include "../include/punycode.h"

/**
 * The bootstring parameters for Punycode.
 *
 * https://www.rfc-editor.org/rfc/rfc3492#section-5
 */
enum {
  punycode__base = 36,
  punycode__tmin = 1,
  punycode__tmax = 26,
  punycode__skew = 38,
  punycode__damp = 700,
  punycode__initial_bias = 72,
  punycode__initial_n = 0x80,
};

// https://www.rfc-editor.org/rfc/rfc3492#section-5
static inline utf8_t
punycode__encode_digit(uint32_t digit) {
  // 0..25 map to "a".."z" and 26..35 map to "0".."9".
  return (utf8_t) (digit + (digit < 26 ? 'a' : '0' - 26));
}

// https://www.rfc-editor.org/rfc/rfc3492#section-5
static inline uint32_t
punycode__decode_digit(utf32_t c) {
  if (c >= '0' && c <= '9') return c - '0' + 26;
  if (c >= 'a' && c <= 'z') return c - 'a';
  if (c >= 'A' && c <= 'Z') return c - 'A';

  return punycode__base;
}

// https://www.rfc-editor.org/rfc/rfc3492#section-6.1
static inline uint32_t
punycode__adapt(uint32_t delta, uint32_t points, bool first) {
  delta = first ? delta / punycode__damp : delta / 2;

  delta += delta / points;

  uint32_t k = 0;

  while (delta > ((punycode__base - punycode__tmin) * punycode__tmax) / 2) {
    delta /= punycode__base - punycode__tmin;

    k += punycode__base;
  }

  return k + (((punycode__base - punycode__tmin + 1) * delta) / (delta + punycode__skew));
}

// https://www.rfc-editor.org/rfc/rfc3492#section-6.1
static inline uint32_t
punycode__threshold(uint32_t k, uint32_t bias) {
  if (k <= bias + punycode__tmin) return punycode__tmin;
  if (k >= bias + punycode__tmax) return punycode__tmax;

  return k - bias;
}

size_t
punycode_max_length_from_utf32(size_t len) {
  // Every code point is either basic, and so copied as a single one, or encoded
  // as at most `PUNYCODE_MAX_DIGITS` of them. The one extra is the delimiter
  // that separates the literal portion from the encoded one.
  if (len > (SIZE_MAX - 1) / PUNYCODE_MAX_DIGITS) return (size_t) -1;

  return len * PUNYCODE_MAX_DIGITS + 1;
}

size_t
utf32_max_length_from_punycode(size_t len) {
  return len;
}

int
punycode_encode_utf8(const utf32_t *data, size_t len, utf8_t *result, size_t *result_len) {
  size_t out = 0;

  size_t basic = 0;

  for (size_t i = 0; i < len; i++) {
    if (data[i] < punycode__initial_n) {
      basic++;

      result[out++] = (utf8_t) data[i];
    }
  }

  if (basic > 0) result[out++] = '-';

  uint32_t n = punycode__initial_n, bias = punycode__initial_bias, delta = 0;

  // The smallest code point in the input that has yet to be handled.
  uint32_t m = UINT32_MAX;

  for (size_t i = 0; i < len; i++) {
    if (data[i] >= n && data[i] < m) m = data[i];
  }

  for (size_t handled = basic; handled < len;) {
    if (m - n > (UINT32_MAX - delta) / (uint32_t) (handled + 1)) return -1;

    delta += (m - n) * (uint32_t) (handled + 1);

    n = m;

    // The code point to handle once this one is done, found along the way so
    // that each round over the input takes a single pass.
    m = UINT32_MAX;

    for (size_t i = 0; i < len; i++) {
      if (data[i] > n) {
        if (data[i] < m) m = data[i];
      } else if (data[i] < n) {
        if (delta == UINT32_MAX) return -1;

        delta++;
      } else {
        uint32_t q = delta;

        for (uint32_t k = punycode__base;; k += punycode__base) {
          uint32_t t = punycode__threshold(k, bias);

          if (q < t) break;

          result[out++] = punycode__encode_digit(t + (q - t) % (punycode__base - t));

          q = (q - t) / (punycode__base - t);
        }

        result[out++] = punycode__encode_digit(q);

        bias = punycode__adapt(delta, (uint32_t) handled + 1, handled == basic);

        delta = 0;

        handled++;
      }
    }

    delta++;
    n++;
  }

  *result_len = out;

  return 0;
}

/**
 * Reads the basic code point at `i` of an extended string that is held either as
 * bytes or as code points. Both carry the same information, an extended string
 * being basic code points throughout, so the two differ only in how wide a word
 * each of them takes.
 */
static inline utf32_t
punycode__at(const void *data, size_t i, bool wide) {
  return wide ? ((const utf32_t *) data)[i] : (utf32_t) ((const utf8_t *) data)[i];
}

/**
 * The decoder itself, over an input of either width, so that the two entry points
 * below share the one copy of it rather than each carrying its own.
 *
 * https://www.rfc-editor.org/rfc/rfc3492#section-6.2
 */
static inline int
punycode__decode(const void *data, size_t len, bool wide, utf32_t *result, size_t *result_len) {
  size_t out = 0;

  // Everything up to and including the last delimiter, if any, is the literal
  // portion of the input.
  size_t literal = 0;

  for (size_t i = 0; i < len; i++) {
    if (punycode__at(data, i, wide) == '-') literal = i;
  }

  for (size_t i = 0; i < literal; i++) {
    utf32_t c = punycode__at(data, i, wide);

    if (c >= punycode__initial_n) return -1;

    result[out++] = c;
  }

  uint32_t n = punycode__initial_n, bias = punycode__initial_bias, i = 0;

  size_t pointer = literal > 0 ? literal + 1 : 0;

  while (pointer < len) {
    uint32_t previous = i, weight = 1;

    for (uint32_t k = punycode__base;; k += punycode__base) {
      if (pointer == len) return -1;

      uint32_t digit = punycode__decode_digit(punycode__at(data, pointer++, wide));

      if (digit >= punycode__base) return -1;

      if (digit > (UINT32_MAX - i) / weight) return -1;

      i += digit * weight;

      uint32_t t = punycode__threshold(k, bias);

      if (digit < t) break;

      if (weight > UINT32_MAX / (punycode__base - t)) return -1;

      weight *= punycode__base - t;
    }

    // The number of code points decoded so far, including the one about to be
    // inserted.
    uint32_t points = (uint32_t) out + 1;

    bias = punycode__adapt(i - previous, points, previous == 0);

    if (i / points > UINT32_MAX - n) return -1;

    n += i / points;
    i %= points;

    if (n > 0x10ffff || (n >= 0xd800 && n <= 0xdfff)) return -1;

    // Taking the remainder above leaves the insertion point within what has been
    // decoded so far, so the tail that it shifts along is never a negative
    // length.
    memmove(&result[i + 1], &result[i], (out - i) * sizeof(utf32_t));

    result[i] = n;

    out++;

    i++;
  }

  *result_len = out;

  return 0;
}

int
punycode_decode_utf8(const utf8_t *data, size_t len, utf32_t *result, size_t *result_len) {
  return punycode__decode(data, len, false, result, result_len);
}

int
punycode_decode_utf32(const utf32_t *data, size_t len, utf32_t *result, size_t *result_len) {
  return punycode__decode(data, len, true, result, result_len);
}
