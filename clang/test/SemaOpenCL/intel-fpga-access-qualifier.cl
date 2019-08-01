// RUN: %clang_cc1 -x cl -verify -pedantic -fsyntax-only -cl-std=CL1.2 -triple spir64-unknown-unknown-intelfpga %s
// RUN: %clang_cc1 -x cl -verify -pedantic -fsyntax-only -cl-std=CL2.0 -triple spir64-unknown-unknown-intelfpga %s
// RUN: %clang_cc1 -x cl -verify -pedantic -fsyntax-only -cl-std=CL2.0 -triple x86_64-unknown-unknown %s
// RUN: %clang_cc1 -x cl -verify -pedantic -fsyntax-only -cl-std=CL1.2 -triple x86_64-unknown-unknown-intelfpga %s

typedef int Int;
typedef read_only int IntRO; // expected-error {{access qualifier can only be used for pipe and image type}}

kernel void k1(read_write pipe int i){} // expected-error{{access qualifier 'read_write' can not be used for 'read_only pipe int'}}

void myPipeWrite(write_only pipe int);
kernel void k2(read_only pipe int p) {
  myPipeWrite(p); // expected-error {{passing 'read_only pipe int' to parameter of incompatible type 'write_only pipe int'}}
// expected-note@11 {{passing argument to parameter here}}
}

typedef read_only pipe int ROPipeInt;
kernel void k3(ROPipeInt p) {
  myPipeWrite(p); // expected-error {{passing 'ROPipeInt' (aka 'read_only pipe int') to parameter of incompatible type 'write_only pipe int'}}
// expected-note@11 {{passing argument to parameter here}}
}

kernel void pipe_ro_twice(read_only read_only pipe int i){} // expected-warning{{duplicate 'read_only' declaration specifier}}
// Conflicting access qualifiers
kernel void pipe_ro_twice_tw(read_write read_only read_only pipe int i){} // expected-error{{access qualifier 'read_write' can not be used for 'read_only pipe int'}}
kernel void pipe_ro_twice_tw(read_only read_only read_write pipe int i){} // expected-error{{multiple access qualifiers}} expected-warning{{duplicate 'read_only' declaration specifier}}
kernel void pipe_ro_wo(read_only write_only pipe int i){} // expected-error{{multiple access qualifiers}}

kernel void pipe_ro_twice_typedef(read_only ROPipeInt i){} // expected-warning{{duplicate 'read_only' declaration specifier}}
// expected-note@17 {{previously declared 'read_only' here}}
