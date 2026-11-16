if(TARGET punycode_shared)
  set(library punycode::shared)
else()
  set(library punycode::static)
endif()

add_test_executables(
  decode.c
  decode-invalid.c
  encode.c
  max-length.c
  roundtrip.c
  LINK
    ${library}
    utf::static
)

add_subdirectory(fuzz)
