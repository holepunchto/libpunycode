list(APPEND benches
  decode
  encode
)

foreach(bench IN LISTS benches)
  add_executable(bench_${bench} ${bench}.c)

  target_link_libraries(
    bench_${bench}
    PRIVATE
      punycode_static
  )
endforeach()
