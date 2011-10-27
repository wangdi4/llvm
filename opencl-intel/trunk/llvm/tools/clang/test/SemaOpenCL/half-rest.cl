// RUN: %clang_cc1 %s -verify -pedantic -fsyntax-only

typedef short half;

void foo(half *h1, half *h2, half *h3) {
  half *h4;
  half h5; // expected-error {{half type variables are not allowed in OpenCL}}

  h3[0] = h1[0] + h2[0]; // expected-error {{half arithmetics are not allowed in OpenCL}}
  h3[1] = h1[1] / h2[1]; // expected-error {{half arithmetics are not allowed in OpenCL}}
  h3[2] = h1[2] * h2[2]; // expected-error {{half arithmetics are not allowed in OpenCL}}
  h3[3] = h1[3] & h2[3]; // expected-error {{half arithmetics are not allowed in OpenCL}}
  h3[4] = h1[4] | h2[4]; // expected-error {{half arithmetics are not allowed in OpenCL}}
  h3[5] = !h1[5]; // expected-error {{half arithmetics are not allowed in OpenCL}}
  ++h1[6]; // expected-error {{half arithmetics are not allowed in OpenCL}}
}
