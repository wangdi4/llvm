// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify -I %S/intel %s

#pragma start_map_region wwww // expected-warning {{missing '(' after '#pragma start_map_region' - ignoring}}
#pragma start_map_region( // expected-warning {{expected a string}}
#pragma start_map_region("www" // expected-warning {{missing ')' after '#pragma start_map_region' - ignoring}}
#pragma stop_map_region // expected-warning {{no #pragma start_map_region is currently active: pragma ignored}}
#pragma start_map_region("ttt.h")
#pragma stop_map_region
#pragma start_map_region("www__1@") // expected-error {{'www__1@' file not found}}

