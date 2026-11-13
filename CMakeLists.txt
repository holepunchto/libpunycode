cmake_minimum_required(VERSION 4.0)

find_package(cmake-fetch REQUIRED PATHS node_modules/cmake-fetch)
find_package(cmake-fuzz REQUIRED PATHS node_modules/cmake-fuzz)

project(punycode C)

fetch_package("github:holepunchto/libutf")

get_property(shared GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS)

add_library(punycode OBJECT)

set_target_properties(
  punycode
  PROPERTIES
  C_STANDARD 11
  POSITION_INDEPENDENT_CODE ${shared}
)

target_sources(
  punycode
  INTERFACE
    include/punycode.h
  PRIVATE
    src/punycode.c
)

target_include_directories(
  punycode
  PUBLIC
    include
    $<TARGET_PROPERTY:utf,INTERFACE_INCLUDE_DIRECTORIES>
)

if(shared)
  add_library(punycode_shared SHARED)

  set_target_properties(
    punycode_shared
    PROPERTIES
    OUTPUT_NAME punycode
    WINDOWS_EXPORT_ALL_SYMBOLS ON
  )

  target_link_libraries(
    punycode_shared
    PUBLIC
      punycode
      utf_shared
  )
endif()

add_library(punycode_static STATIC)

set_target_properties(
  punycode_static
  PROPERTIES
  OUTPUT_NAME punycode
  PREFIX lib
)

target_link_libraries(
  punycode_static
  PUBLIC
    punycode
    utf_static
)

install(TARGETS punycode_static)

if(shared)
  install(TARGETS punycode_shared)
endif()

install(FILES include/punycode.h DESTINATION include)

if(PROJECT_IS_TOP_LEVEL)
  enable_testing()

  add_subdirectory(bench)
  add_subdirectory(test)
endif()
