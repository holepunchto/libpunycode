# libpunycode

Encoder and decoder for Punycode in C, as specified by [RFC 3492](https://www.rfc-editor.org/rfc/rfc3492). Allocates nothing itself, the length of a result being bounded ahead of converting it.

## Usage

```c
#include <punycode.h>
#include <utf.h>

const utf32_t domain[] = {0x62, 0xfc, 0x63, 0x68, 0x65, 0x72};

size_t len = sizeof(domain) / sizeof(utf32_t);

utf8_t encoded[punycode_max_length_from_utf32(6)];
size_t encoded_len;

punycode_encode_utf8(domain, len, encoded, &encoded_len);
```

## API

See [`include/punycode.h`](include/punycode.h) for the public API.

Punycode is the bootstring encoding alone. The `xn--` prefix that marks a domain label as Punycode encoded belongs to [IDNA](https://www.rfc-editor.org/rfc/rfc3490) rather than to Punycode, and so is neither added by `punycode_encode_utf8()` nor expected by `punycode_decode_utf8()`.

## License

Apache-2.0
