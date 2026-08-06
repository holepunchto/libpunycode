#include <stddef.h>
#include <utf.h>

#include "../include/punycode.h"

int
main() {
  // The Japanese sample of RFC 3492 section 7.1, being a label of the sort that a
  // domain name holds.
  static const utf8_t encoded[] = "n8jok5ay5dzabd5bym9f0cm5685rrjetr6pdxa";

  size_t len = sizeof(encoded) - 1;

  utf32_t decoded[64];
  size_t decoded_len;

  for (size_t i = 0; i < 1000000; i++) {
    punycode_decode_utf8(encoded, len, decoded, &decoded_len);
  }
}
