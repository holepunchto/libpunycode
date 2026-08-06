#include <stddef.h>
#include <utf.h>

#include "../include/punycode.h"

int
main() {
  // The code points of the Japanese sample of RFC 3492 section 7.1, being a label
  // of the sort that a domain name holds.
  static const utf32_t decoded[] = {
    0x306a, 0x305c, 0x307f, 0x3093, 0x306a, 0x65e5, 0x672c, 0x8a9e, 0x3092, 0x8a71, 0x3057, 0x3066, 0x304f, 0x308c, 0x306a, 0x3044, 0x306e, 0x304b
  };

  size_t len = sizeof(decoded) / sizeof(utf32_t);

  utf8_t encoded[256];
  size_t encoded_len;

  for (size_t i = 0; i < 1000000; i++) {
    punycode_encode_utf8(decoded, len, encoded, &encoded_len);
  }
}
