#ifndef PUNYCODE_H
#define PUNYCODE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <utf.h>

/**
 * The most basic code points that encoding a single code point can produce. A
 * delta is at most `UINT32_MAX`, and every digit but the last divides what
 * remains of it by at least `base - tmax`, being 10, so ten digits exhaust it
 * and an eleventh carries the remainder.
 *
 * https://www.rfc-editor.org/rfc/rfc3492#section-3.3
 */
#define PUNYCODE_MAX_DIGITS 11

/**
 * The most basic code points that encoding `len` code points can produce, for
 * sizing the result of `punycode_encode()`. Unlike the length functions of
 * libutf this is an upper bound rather than an exact length, which for Punycode
 * could only be arrived at by encoding the input; it therefore does not look at
 * the code points themselves.
 *
 * Returns `(size_t) -1` if `len` is large enough that the bound would overflow,
 * such an input being far too long to encode in any case.
 */
size_t
punycode_max_length_from_utf32(size_t len);

/**
 * The most code points that decoding `len` basic code points can produce, for
 * sizing the result of `punycode_decode()`. Every code point of the output is
 * either copied from the literal portion of the input or encoded by at least one
 * digit, so the output is never longer than the input.
 */
size_t
utf32_max_length_from_punycode(size_t len);

/**
 * Encodes the `len` code points of `data` as an extended string, writing the
 * basic code points of it to `result` and their number to `result_len`. As with
 * the functions of libhex, the suffix names the width of the extended string
 * rather than that of the code points beside it.
 *
 * `result` must have room for `punycode_max_length_from_utf32(len)` basic code
 * points. As an extended string consists only of basic code points, it needs no
 * further encoding to be written out as ASCII or UTF-8.
 *
 * Returns 0 on success and -1 if the input cannot be encoded, which is the case
 * when the deltas that it calls for overflow.
 *
 * https://www.rfc-editor.org/rfc/rfc3492#section-6.3
 */
int
punycode_encode_utf8(const utf32_t *data, size_t len, utf8_t *result, size_t *result_len);

/**
 * Decodes the `len` basic code points of `data`, which must be an extended
 * string, writing the code points of it to `result` and their number to
 * `result_len`.
 *
 * `result` must have room for `utf32_max_length_from_punycode(len)` code points.
 *
 * Returns 0 on success and -1 if `data` is not a valid extended string, which
 * covers a code point that is not basic, a digit that is not of the base, a
 * delta that overflows, and a code point that is decoded outside of Unicode or
 * onto a surrogate.
 *
 * https://www.rfc-editor.org/rfc/rfc3492#section-6.2
 */
int
punycode_decode_utf8(const utf8_t *data, size_t len, utf32_t *result, size_t *result_len);

/**
 * As `punycode_decode_utf8()`, but taking the extended string as code points
 * rather than as the bytes that they would narrow to. This is for a caller that
 * already holds it as code points, such as one that has decoded a domain as a
 * whole before breaking it into labels, and saves it narrowing them back down.
 *
 * A code point of `data` that is not basic is rejected in the same way as a byte
 * of `punycode_decode_utf8()` that is not.
 */
int
punycode_decode_utf32(const utf32_t *data, size_t len, utf32_t *result, size_t *result_len);

#ifdef __cplusplus
}
#endif

#endif // PUNYCODE_H
