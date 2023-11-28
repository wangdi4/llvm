// Test XML output report format for a case where a macro definition uses
// another macro definition, and is located in another source file, requiring
// multiple iterations over the expansion region table.

// RUN: llvm-cov report --format=xml %S/Inputs/intel-xml-report3.covmapping --instr-profile %S/Inputs/intel-xml-report3.profdata --show-regions-in-xml | FileCheck %s

// Note: Regular expressions are used for the line number checks, instead
// of the absolute line number reported in the report to facilate being
// able to recompile this test to produce a new data file when changing
// this comment header or run lines.

// CHECK: <FUNCTION name="main" freq="1">
// CHECK:      <BLOCKS total="9" covered="7" coverage="77.777778%">
// CHECK:        <BLOCK line="{{.*}}" col="34" end_line="{{.*}}" end_col="2">
// CHECK:          <INSTANCE id="1" freq="1">
// CHECK:          </INSTANCE>
// CHECK:        </BLOCK>
// CHECK:        <BLOCK line="{{.*}}" col="3" end_line="{{.*}}" end_col="9">
// CHECK:          <INSTANCE id="1" freq="1" macro_file="{{.*}}intel-xml-report3.c" macro_line="{{.*}}" macro_col="22" end_macro_line="{{.*}}" end_macro_col="44">
// CHECK:          </INSTANCE>
// CHECK:          <INSTANCE id="2" freq="1" macro_file="{{.*}}intel-xml-report3.c" macro_line="{{.*}}" macro_col="26" end_macro_line="{{.*}}" end_macro_col="33">
// CHECK:          </INSTANCE>
// CHECK:          <INSTANCE id="3" freq="0" macro_file="{{.*}}intel-xml-report3.h" macro_line="{{.*}}" macro_col="19" end_macro_line="{{.*}}" end_macro_col="58">
// CHECK:          </INSTANCE>
// CHECK:        </BLOCK>
// CHECK:        <BLOCK line="{{.*}}" col="25" end_line="{{.*}}" end_col="2">
// CHECK:          <INSTANCE id="1" freq="1">
// CHECK:          </INSTANCE>
// CHECK:        </BLOCK>
// CHECK:        <BLOCK line="{{.*}}" col="3" end_line="{{.*}}" end_col="9">
// CHECK:          <INSTANCE id="1" freq="1" macro_file="{{.*}}intel-xml-report3.c" macro_line="{{.*}}" macro_col="22" end_macro_line="{{.*}}" end_macro_col="44">
// CHECK:          </INSTANCE>
// CHECK:          <INSTANCE id="2" freq="1" macro_file="{{.*}}intel-xml-report3.c" macro_line="{{.*}}" macro_col="26" end_macro_line="{{.*}}" end_macro_col="33">
// CHECK:          </INSTANCE>
// CHECK:          <INSTANCE id="3" freq="0" macro_file="{{.*}}intel-xml-report3.h" macro_line="{{.*}}" macro_col="19" end_macro_line="{{.*}}" end_macro_col="58">
// CHECK:          </INSTANCE>
// CHECK:        </BLOCK>
// CHECK:        <BLOCK line="{{.*}}" col="22" end_line="{{.*}}" end_col="14">
// CHECK:          <INSTANCE id="1" freq="1">
// CHECK:          </INSTANCE>
// CHECK:        </BLOCK>
// CHECK:      </BLOCKS>
// CHECK:    </FUNCTION>

// Data generation done with the following commands to get files in Inputs
// directory:
// icx -fcoverage-mapping -fprofile-instr-generate -mllvm -enable-name-compression=false intel-xml-report3.c -o intel-xml-report3
// llvm-cov convert-for-testing intel-xml-report3 -o intel-xml-report3.covmapping
// ./intel-xml-report3
// llvm-profdata merge -o intel-xml-report3.profdata *.profraw

// Begin contents of "intel-xml-report3.h"
// #ifndef INTEL_XML_REPORT3_H_INCLUDED
// #define INTEL_XML_REPORT3_H_INCLUDED
//
// #define TERMINATE { printf("Fatail error\n"); exit(-1); }
//
// #endif // INTEL_XML_REPORT3_H_INCLUDED
// End contents of "intel-xml-report3.h"

#include "intel-xml-report3.h"

extern int printf(const char*, ...);
extern void exit(int);

#define assert(expr) if (!(expr)) TERMINATE

int gVar = 0;

int foo(int count) {
  if (count) {
    return ++gVar;
  }

  return 0;
}

int main(int argc, char *argv[]) {
  // Test with macro used multiple times.
  assert(foo(argc) == 1);
  gVar++;
  assert(foo(0) == 0);
  gVar++;
  return gVar;
}
