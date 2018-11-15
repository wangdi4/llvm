// RUN: %clang_cc1 -x cl -cl-std=CL2.0 -triple spir-unknown-unknown -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown -fsyntax-only -verify %s

#pragma OPENCL EXTENSION cl_intel_channels : enable

struct st {
  int i1;
  int i2;
};

channel int ich __attribute__((io("eth0"))) __attribute__((depth(-1))); // expected-warning{{'depth' attribute parameter 0 is negative and will be ignored}}
channel int ich1 __attribute__((io(1))); // expected-error{{'io' attribute requires a string}}
channel long lch __attribute__((depth(3)));
channel struct st sch __attribute__((depth(0)));

channel int arr[5] __attribute__((io("eth0")));
channel int arr1[5] __attribute__((io(1))); // expected-error{{'io' attribute requires a string}}
channel int multiarr[2][7] __attribute__((depth(0)));

__constant int a1 __attribute((depth(3))) = 2; // expected-warning{{'depth' attribute only applies to OpenCL channels}}
__constant long a2 __attribute((io("eth1"))) = 2; // expected-warning{{'io' attribute only applies to OpenCL channels}}

__kernel void k1() __attribute__((depth(100))) { // expected-warning{{'depth' attribute only applies to parameters and global variables}}
}

__kernel void k2() {
  int i1 = read_channel_intel(arr[1]);
  int i2 = read_channel_intel(multiarr[1][2]);
}

__kernel void k3() __attribute__((io("tmp"))) { // expected-warning{{'io' attribute only applies to variables}}
}

channel foo; // expected-error{{missing actual type specifier for channel}}

struct incomplete;  // expected-note{{forward declaration of 'struct incomplete'}}

channel struct incomplete ch_arr[10]; // expected-error{{array has incomplete element type '__global channel struct incomplete'}}
channel struct incomplete ch; // expected-error{{tentative definition has type '__global channel struct incomplete' that is never completed}}
// expected-note@-4{{forward declaration of 'struct incomplete'}}
channel struct incomplete; // expected-warning{{declaration does not declare anything}}
channel struct st; // expected-warning{{declaration does not declare anything}}
channel int; // expected-warning{{declaration does not declare anything}}

void write_wrapper(__global channel char *inp, char data) {
  write_channel_intel(*inp, data); // expected-error{{invalid argument type 'channel char *' to unary expression}}
  void *tmp = &inp; // expected-error{{invalid argument type 'channel char *' to unary expression}}
}

void read_channel(__global channel int *a) {
}
channel char INPUT_CHANNEL;
__kernel void k4(__global const char *src)
{
  char tmp = src[1];
  write_wrapper(&INPUT_CHANNEL, tmp); // expected-error{{invalid argument type '__global channel char' to unary expression}}
  read_channel(&arr[1]); // expected-error{{invalid argument type '__global channel int' to unary expression}}
  read_channel(&multiarr[1][2]); // expected-error{{invalid argument type '__global channel int' to unary expression}}
}

void negative_test_kernel(__global int *data) {
  int i = read_channel_intel(wrong_channel_variable_name); // expected-error{{use of undeclared identifier 'wrong_channel_variable_name'}}
  write_channel_intel(wrong_channel_variable_name_2, i); // expected-error{{use of undeclared identifier 'wrong_channel_variable_name_2'}}
}
