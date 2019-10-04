// REQUIRES: x86-registered-target
// RUN: %clangxx -c %s -o %t_fat.o
// RUN: %clangxx %t_fat.o -o %t.exe
// RUN: clang-offload-bundler -type=o -targets=host-x86_64-unknown-linux-gnu,openmp-x86_64-pc-linux-gnu -outputs=%t_host.o,%t_device.o -inputs=%t_fat.o -unbundle
// RUN: %t.exe %t_device.o | FileCheck %s
// CHECK:11

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define BUNDLE_SECTION_PREFIX "__CLANG_OFFLOAD_BUNDLE__"

#define TARGET0 "host-x86_64-unknown-linux-gnu"
#define TARGET1 "openmp-x86_64-pc-linux-gnu"

// Populate section with special names recognized by the bundler;
// this emulates fat object with one host and one device section.
char str0[] __attribute__((section(BUNDLE_SECTION_PREFIX TARGET0))) = { 0 };
char str1[] __attribute__((section(BUNDLE_SECTION_PREFIX TARGET1))) = { "11\n" };

// main is invoked with the bundler output file as argument -
// read this file and print their contents to stdout.
int main(int argc, char **argv) {
  string DeviceObj(argv[1]);
  string Line;
  ifstream F(DeviceObj);

  if (F.is_open()) {
    while (getline(F, Line)) {
      cout << Line;
    }
    F.close();
  }
  else {
    cout << "Unable to open file " << DeviceObj;
    return 1;
  }

  return 0;
}

