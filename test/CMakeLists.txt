add_test_executables(
  decode.c
  decode-invalid.c
  encode.c
  max-length.c
  roundtrip.c
  LINK
    punycode::<TYPE>
    utf::static
)

add_subdirectory(fuzz)
