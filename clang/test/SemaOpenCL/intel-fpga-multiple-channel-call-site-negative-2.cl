// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=1 -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=2 -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=3 -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=4 -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=5 -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=6 -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=7 -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=8 -fsyntax-only -verify %s
// RUN: %clang_cc1 -x cl -triple spir64-unknown-unknown-intelfpga -DUSE_CASE=9 -fsyntax-only -verify %s

#pragma OPENCL EXTENSION cl_intel_channels : enable

#if USE_CASE == 1

channel int a;
channel int b[2];

void writeHelper(channel int ch, int data) {
  write_channel_intel(ch, data);
}

int readHelper(channel int ch) {
  return read_channel_intel(ch);
}

__kernel void kernel1(__global int *data) {
  data[0] = readHelper(a);
}

__kernel void kernel2(__global int *data) {
  writeHelper(b[0], data[0]);
}

__kernel void kernel3(__global int *data) {
  data[0] = read_channel_intel(a);
  write_channel_intel(b[0], data[0]);
}

// expected-error@-24 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-6 {{usage of channel 'a' for reading found in the kernel 'kernel3'. See call stack below:}}
// expected-note@-6 {{channel 'a' is used for reading here}}
// expected-note@-16 {{previous usage of channel 'a' for reading found in the kernel 'kernel1'. See call stack below:}}
// expected-note@-16 {{channel 'a' is used for reading here}}
// expected-note@-21 {{channel 'a' is used for reading here}}
// expected-error@-29 {{usage of the same channel for writing from different kernels is prohibited}}
// expected-note@-12 {{usage of channel 'b[0]' for writing found in the kernel 'kernel3'. See call stack below:}}
// expected-note@-11 {{channel 'b[0]' is used for writing here}}
// expected-note@-18 {{previous usage of channel 'b[0]' for writing found in the kernel 'kernel2'. See call stack below:}}
// expected-note@-18 {{channel 'b[0]' is used for writing here}}
// expected-note@-31 {{channel 'b[0]' is used for writing here}}

#elif USE_CASE == 2

channel int a[2];

int readAHelper(channel int ch);
void readWriteABHelper(channel int ch1, channel int ch2);
void wrapper();

int readAHelper(channel int ch) {
  return read_channel_intel(ch);
}

void readWriteABHelper(channel int ch1, channel int ch2) {
  int i = read_channel_intel(ch2);
  write_channel_intel(ch1, i);
}

void wrapper() {
  readWriteABHelper(a[1], a[0]);
}

__kernel void kernel1(__global int *data) {
  data[0] = readAHelper(a[0]);
}

__kernel void kernel2(__global int *data) {
  wrapper();
}

// expected-error@-27 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-5 {{usage of channel 'a[0]' for reading found in the kernel 'kernel2'. See call stack below:}}
// expected-note@-5 {{channel 'a[0]' is used for reading here}}
// expected-note@-14 {{channel 'a[0]' is used for reading here}}
// expected-note@-20 {{channel 'a[0]' is used for reading here}}
// expected-note@-13 {{previous usage of channel 'a[0]' for reading found in the kernel 'kernel1'. See call stack below:}}
// expected-note@-13 {{channel 'a[0]' is used for reading here}}
// expected-note@-27 {{channel 'a[0]' is used for reading here}}

#elif USE_CASE == 3

channel int a[2];

int readAHelper(channel int ch);
void readWriteABHelper(channel int ch1, channel int ch2);
void wrapper();

__kernel void kernel1(__global int *data) {
  data[0] = readAHelper(a[0]);
}

__kernel void kernel2(__global int *data) {
  wrapper();
}

int readAHelper(channel int ch) {
  return read_channel_intel(ch);
}

void readWriteABHelper(channel int ch1, channel int ch2) {
  int i = read_channel_intel(ch2);
  write_channel_intel(ch1, i);
}

void wrapper() {
  readWriteABHelper(a[1], a[0]);
}

// expected-error@-27 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-18 {{usage of channel 'a[0]' for reading found in the kernel 'kernel2'. See call stack below:}}
// expected-note@-18 {{channel 'a[0]' is used for reading here}}
// expected-note@-6 {{channel 'a[0]' is used for reading here}}
// expected-note@-12 {{channel 'a[0]' is used for reading here}}
// expected-note@-26 {{previous usage of channel 'a[0]' for reading found in the kernel 'kernel1'. See call stack below:}}
// expected-note@-26 {{channel 'a[0]' is used for reading here}}
// expected-note@-19 {{channel 'a[0]' is used for reading here}}

#elif USE_CASE == 4

channel int a[2];

int readAHelper(channel int ch);
void readWriteABHelper(channel int ch1, channel int ch2);
void wrapper();

__kernel void kernel1(__global int *data) {
  data[0] = readAHelper(a[0]);
}

void readWriteABHelper(channel int ch1, channel int ch2) {
  int i = read_channel_intel(ch2);
  write_channel_intel(ch1, i);
}

void wrapper() {
  readWriteABHelper(a[1], a[0]);
}

__kernel void kernel2(__global int *data) {
  wrapper();
}

int readAHelper(channel int ch) {
  return read_channel_intel(ch);
}

// expected-error@-27 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-22 {{usage of channel 'a[0]' for reading found in the kernel 'kernel1'. See call stack below:}}
// expected-note@-22 {{channel 'a[0]' is used for reading here}}
// expected-note@-6 {{channel 'a[0]' is used for reading here}}
// expected-note@-12 {{previous usage of channel 'a[0]' for reading found in the kernel 'kernel2'. See call stack below:}}
// expected-note@-12 {{channel 'a[0]' is used for reading here}}
// expected-note@-17 {{channel 'a[0]' is used for reading here}}
// expected-note@-23 {{channel 'a[0]' is used for reading here}}

#elif USE_CASE == 5

channel int a, b;

int readHelper(channel int ch);
void readWriteHelper(channel int ch1, channel int ch2);
void wrapper();

__kernel void kernel1(__global int *data) {
  data[0] = readHelper(a);
  data[0] = readHelper(b);
}

void readWriteHelper(channel int ch1, channel int ch2) {
  int i = read_channel_intel(ch2);
  write_channel_intel(ch1, i);
}

void wrapper() {
  readWriteHelper(a, b);
}

__kernel void kernel2(__global int *data) {
  wrapper();
}

int readHelper(channel int ch) {
  return read_channel_intel(ch);
}

// expected-error@-28 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-23 {{usage of channel 'b' for reading found in the kernel 'kernel1'. See call stack below:}}
// expected-note@-22 {{channel 'b' is used for reading here}}
// expected-note@-6 {{channel 'b' is used for reading here}}
// expected-note@-12 {{previous usage of channel 'b' for reading found in the kernel 'kernel2'. See call stack below:}}
// expected-note@-12 {{channel 'b' is used for reading here}}
// expected-note@-17 {{channel 'b' is used for reading here}}
// expected-note@-23 {{channel 'b' is used for reading here}}

#elif USE_CASE == 6

channel int a[2];

int readAHelper(channel int ch);
void readWriteABHelper(channel int ch1, channel int ch2);
void wrapper();

__kernel void kernel1(__global int *data) {
  data[0] = readAHelper(a[0]);
}

__kernel void kernel2(__global int *data) {
  wrapper();
}

int readAHelper(channel int ch) {
  return read_channel_intel(ch);
}

void wrapper() {
  readWriteABHelper(a[1], a[0]);
}

void readWriteABHelper(channel int ch1, channel int ch2) {
  int i = read_channel_intel(ch2);
  write_channel_intel(ch1, i);
}

// expected-error@-27 {{usage of the same channel for reading from different kernels is prohibited}}
// expected-note@-18 {{usage of channel 'a[0]' for reading found in the kernel 'kernel2'. See call stack below:}}
// expected-note@-18 {{channel 'a[0]' is used for reading here}}
// expected-note@-11 {{channel 'a[0]' is used for reading here}}
// expected-note@-8 {{channel 'a[0]' is used for reading here}}
// expected-note@-26 {{previous usage of channel 'a[0]' for reading found in the kernel 'kernel1'. See call stack below:}}
// expected-note@-26 {{channel 'a[0]' is used for reading here}}
// expected-note@-19 {{channel 'a[0]' is used for reading here}}

#elif USE_CASE == 7

channel int a, b, c, d, e, f, g;

void helper1(channel int ch1, channel int ch2, channel int ch3, channel int ch4) {
  write_channel_intel(ch3, 10);
}

void helper2(channel int ch1, channel int ch2, channel int ch3, channel int ch4) {
  write_channel_intel(ch4, 10);
}

__kernel void kernel1() {
  helper1(a, b, c, d);
}

__kernel void kernel2() {
  helper2(e, f, g, c);
}

// expected-error@-18 {{usage of the same channel for writing from different kernels is prohibited}}
// expected-note@-5 {{usage of channel 'c' for writing found in the kernel 'kernel2'. See call stack below:}}
// expected-note@-5 {{channel 'c' is used for writing here}}
// expected-note@-14 {{channel 'c' is used for writing here}}
// expected-note@-12 {{previous usage of channel 'c' for writing found in the kernel 'kernel1'. See call stack below:}}
// expected-note@-12 {{channel 'c' is used for writing here}}
// expected-note@-21 {{channel 'c' is used for writing here}}

#elif USE_CASE == 8

// expected-no-diagnostics

int helper(channel int ch); // defined in another compilation unit, which will
// be linked

channel int a;

__kernel void kernel1(__global int* data) {
  *data = helper(a);
}

__kernel void kernel2(__global int* data) {
  *data = helper(a);
}

#elif USE_CASE == 9

// expected-no-diagnostics

int helper(channel int ch); // defined in another compilation unit, which will
// be linked

int helper2(channel int ch) {
  return helper(ch);
}

channel int a;

__kernel void kernel1(__global int* data) {
  *data = helper2(a);
}

__kernel void kernel2(__global int* data) {
  *data = helper2(a);
}

#endif
