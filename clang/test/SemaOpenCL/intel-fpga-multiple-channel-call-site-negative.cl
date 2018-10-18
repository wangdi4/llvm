// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=1 -verify -fsyntax-only %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=2 -verify -fsyntax-only %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=3 -verify -fsyntax-only %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=4 -verify -fsyntax-only %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=5 -verify -fsyntax-only %s -Wno-error=analyze-channels-usage
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=6 -verify -fsyntax-only %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=7 -verify -fsyntax-only %s -Wno-analyze-channels-usage
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=8 -verify -fsyntax-only %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=9 -verify -fsyntax-only %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=10 -verify -fsyntax-only %s

#pragma OPENCL EXTENSION cl_intel_channels : enable

#if USE_CASE == 1

channel int a;
channel int b[2];

__kernel void k1a(__global int *data) {
  data[0] = read_channel_intel(a);
}

__kernel void k2(__global int *data) {
  write_channel_intel(b[0], data[0]);
}

__kernel void k1b(__global int *data) {
  data[0] = read_channel_intel(a);
  write_channel_intel(b[0], data[0]);
}

// expected-error@-16 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-6 {{usage of channel 'a' for reading found in the kernel 'k1b'. See call stack below:}}
// expected-note@-6 {{channel 'a' is used for reading here}}
// expected-note@-16 {{previous usage of channel 'a' for reading found in the kernel 'k1a'. See call stack below:}}
// expected-note@-16 {{channel 'a' is used for reading here}}
// expected-error@-20 {{usage of the same channel for writing from different kernels is prohibited}}
// expected-note@-11 {{usage of channel 'b[0]' for writing found in the kernel 'k1b'. See call stack below:}}
// expected-note@-10 {{channel 'b[0]' is used for writing here}}
// expected-note@-17 {{previous usage of channel 'b[0]' for writing found in the kernel 'k2'. See call stack below:}}
// expected-note@-17 {{channel 'b[0]' is used for writing here}}

#elif USE_CASE == 2

channel int a[2];

int readAHelper();
void readWriteABHelper();
void wrapper();

int readAHelper() {
  return read_channel_intel(a[0]);
}

void readWriteABHelper() {
  int i = read_channel_intel(a[0]);
  write_channel_intel(a[1], i);
}

void wrapper() {
  readWriteABHelper();
}

__kernel void k1a(__global int *data) {
  data[0] = readAHelper();
}

__kernel void k1b(__global int *data) {
  wrapper();
}

// expected-error@-27 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-5 {{usage of channel 'a[0]' for reading found in the kernel 'k1b'. See call stack below:}}
// expected-note@-5 {{channel 'a[0]' is used for reading here}}
// expected-note@-14 {{channel 'a[0]' is used for reading here}}
// expected-note@-20 {{channel 'a[0]' is used for reading here}}
// expected-note@-13 {{previous usage of channel 'a[0]' for reading found in the kernel 'k1a'. See call stack below:}}
// expected-note@-13 {{channel 'a[0]' is used for reading here}}
// expected-note@-27 {{channel 'a[0]' is used for reading here}}

#elif USE_CASE == 3

channel int a[2];

int readAHelper();
void readWriteABHelper();
void wrapper();

__kernel void k1a(__global int *data) {
  data[0] = readAHelper();
}

__kernel void k1b(__global int *data) {
  wrapper();
}

int readAHelper() {
  return read_channel_intel(a[0]);
}

void readWriteABHelper() {
  int i = read_channel_intel(a[0]);
  write_channel_intel(a[1], i);
}

void wrapper() {
  readWriteABHelper();
}

// expected-error@-27 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-18 {{usage of channel 'a[0]' for reading found in the kernel 'k1b'. See call stack below:}}
// expected-note@-18 {{channel 'a[0]' is used for reading here}}
// expected-note@-6 {{channel 'a[0]' is used for reading here}}
// expected-note@-12 {{channel 'a[0]' is used for reading here}}
// expected-note@-26 {{previous usage of channel 'a[0]' for reading found in the kernel 'k1a'. See call stack below:}}
// expected-note@-26 {{channel 'a[0]' is used for reading here}}
// expected-note@-19 {{channel 'a[0]' is used for reading here}}

#elif USE_CASE == 4

channel int a[2];

int readAHelper();
void readWriteABHelper();
void wrapper();

__kernel void k1a(__global int *data) {
  data[0] = readAHelper();
}

void readWriteABHelper() {
  int i = read_channel_intel(a[0]);
  write_channel_intel(a[1], i);
}

void wrapper() {
  readWriteABHelper();
}

__kernel void k1b(__global int *data) {
  wrapper();
}

int readAHelper() {
  return read_channel_intel(a[0]);
}

// expected-error@-27 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-22 {{usage of channel 'a[0]' for reading found in the kernel 'k1a'. See call stack below:}}
// expected-note@-22 {{channel 'a[0]' is used for reading here}}
// expected-note@-6 {{channel 'a[0]' is used for reading here}}
// expected-note@-12 {{previous usage of channel 'a[0]' for reading found in the kernel 'k1b'. See call stack below:}}
// expected-note@-12 {{channel 'a[0]' is used for reading here}}
// expected-note@-17 {{channel 'a[0]' is used for reading here}}
// expected-note@-23 {{channel 'a[0]' is used for reading here}}

#elif USE_CASE == 5

channel int a;
channel int b[2];

__kernel void k1a(__global int *data) {
  data[0] = read_channel_intel(a);
}

__kernel void k2(__global int *data) {
  write_channel_intel(b[0], data[0]);
}

__kernel void k1b(__global int *data) {
  data[0] = read_channel_intel(a);
  write_channel_intel(b[0], data[0]);
}

// expected-warning@-16 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-6 {{usage of channel 'a' for reading found in the kernel 'k1b'. See call stack below:}}
// expected-note@-6 {{channel 'a' is used for reading here}}
// expected-note@-16 {{previous usage of channel 'a' for reading found in the kernel 'k1a'. See call stack below:}}
// expected-note@-16 {{channel 'a' is used for reading here}}
// expected-warning@-20 {{usage of the same channel for writing from different kernels is prohibited}}
// expected-note@-11 {{usage of channel 'b[0]' for writing found in the kernel 'k1b'. See call stack below:}}
// expected-note@-10 {{channel 'b[0]' is used for writing here}}
// expected-note@-17 {{previous usage of channel 'b[0]' for writing found in the kernel 'k2'. See call stack below:}}
// expected-note@-17 {{channel 'b[0]' is used for writing here}}

#elif USE_CASE == 6

channel int a;

__kernel void k1a(__global int *data) {
  data[0] = read_channel_intel(a);
}

__kernel void k2(__global int *data) {
  data[0] = read_channel_intel(a);
}

__kernel void k1b(__global int *data) {
  data[0] = read_channel_intel(a);
}

// expected-error@-14 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-9 {{usage of channel 'a' for reading found in the kernel 'k2'. See call stack below:}}
// expected-note@-9 {{channel 'a' is used for reading here}}
// expected-note@-15 {{previous usage of channel 'a' for reading found in the kernel 'k1a'. See call stack below:}}
// expected-note@-15 {{channel 'a' is used for reading here}}

#elif USE_CASE == 7

// expected-no-diagnostics
channel int a;

__kernel void k1a(__global int *data) {
  data[0] = read_channel_intel(a);
}

__kernel void k2(__global int *data) {
  data[0] = read_channel_intel(a);
}

__kernel void k1b(__global int *data) {
  data[0] = read_channel_intel(a);
}

#elif USE_CASE == 8

// expected-no-diagnostics

channel int a[5];

__attribute__((num_compute_units(4)))
__attribute__((max_global_work_dim(0)))
__attribute__((autorun))
__kernel void kernel_a() {
  int cid = get_compute_id(0);
  int v = read_channel_intel(a[cid]);
  write_channel_intel(a[cid + 1], v);
}

__kernel void kernel_b(__global int *data) {
  *data = read_channel_intel(a[3]);

  write_channel_intel(*(a + 2), *data);
}

#elif USE_CASE == 9

channel int a;

void foo();

__kernel void test() {
  foo();
}

void foo() {
  write_channel_intel(a, 10);
  test();
}

__kernel void test2() {
  test();
}

// expected-error@-17 {{usage of the same channel for writing from different kernels is prohibited}}
// expected-note@-5 {{usage of channel 'a' for writing found in the kernel 'test2'. See call stack below:}}
// expected-note@-5 {{channel 'a' is used for writing here}}
// expected-note@-15 {{channel 'a' is used for writing here}}
// expected-note@-12 {{channel 'a' is used for writing here}}
// expected-note@-18 {{previous usage of channel 'a' for writing found in the kernel 'test'. See call stack below:}}
// expected-note@-18 {{channel 'a' is used for writing here}}
// expected-note@-15 {{channel 'a' is used for writing here}}

#elif USE_CASE == 10

// expected-no-diagnostics

channel int a;

void foo();

__kernel void test() {
  foo();
}

void foo() {
  write_channel_intel(a, 10);
  foo();
}

__kernel void test2() {
  test();
}

#endif
