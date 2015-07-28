// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s

#pragma vtordisp ; // expected-warning {{missing '(' after '#pragma vtordisp' - ignoring}}
#pragma vtordisp ( // expected-warning {{'on' or 'off' is expected}}
#pragma vtordisp (on // expected-warning {{missing ')' after '#pragma vtordisp' - ignoring}}
#pragma vtordisp(on)
#pragma vtordisp (off)
#pragma vtordisp (default) // expected-warning {{'on' or 'off' is expected}}
