add_fuzz_executables(
  decode.c
  encode.c
  PREFIX fuzz_
  DURATION 600
  LINK
    punycode::static
    utf::static
)
