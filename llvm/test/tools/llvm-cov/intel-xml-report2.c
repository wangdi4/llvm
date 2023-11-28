// Test XML output report format for a case where a macro definition is used,
// which expands out to multiple regions during the code coverage generation.
// The report format reports the block location of the macro use. When the
// frequency values are reported, the macro definition location is given.

// RUN: llvm-cov report --format=xml %S/Inputs/intel-xml-report2.covmapping --instr-profile %S/Inputs/intel-xml-report2.profdata --show-regions-in-xml | FileCheck %s

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
// CHECK:          <INSTANCE id="1" freq="1" macro_file="{{.*}}intel-xml-report2.c" macro_line="{{.*}}" macro_col="3" end_macro_line="{{.*}}" end_macro_col="4">
// CHECK:          </INSTANCE>
// CHECK:          <INSTANCE id="2" freq="1" macro_file="{{.*}}intel-xml-report2.c" macro_line="{{.*}}" macro_col="7" end_macro_line="{{.*}}" end_macro_col="14">
// CHECK:          </INSTANCE>
// CHECK:          <INSTANCE id="3" freq="0" macro_file="{{.*}}intel-xml-report2.c" macro_line="{{.*}}" macro_col="{{.*}}" end_macro_line="{{.*}}" end_macro_col="4">
// CHECK:          </INSTANCE>
// CHECK:        </BLOCK>
// CHECK:        <BLOCK line="{{.*}}" col="25" end_line="{{.*}}" end_col="2">
// CHECK:          <INSTANCE id="1" freq="1">
// CHECK:          </INSTANCE>
// CHECK:        </BLOCK>
// CHECK:        <BLOCK line="{{.*}}" col="3" end_line="{{.*}}" end_col="9">
// CHECK:          <INSTANCE id="1" freq="1" macro_file="{{.*}}intel-xml-report2.c" macro_line="{{.*}}" macro_col="3" end_macro_line="{{.*}}" end_macro_col="4">
// CHECK:          </INSTANCE>
// CHECK:          <INSTANCE id="2" freq="1" macro_file="{{.*}}intel-xml-report2.c" macro_line="{{.*}}" macro_col="7" end_macro_line="{{.*}}" end_macro_col="14">
// CHECK:          </INSTANCE>
// CHECK:          <INSTANCE id="3" freq="0" macro_file="{{.*}}intel-xml-report2.c" macro_line="{{.*}}" macro_col="16" end_macro_line="{{.*}}" end_macro_col="4">
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
// icx -fcoverage-mapping -fprofile-instr-generate -mllvm -enable-name-compression=false intel-xml-report2.c -o intel-xml-report2
// llvm-cov convert-for-testing intel-xml-report2 -o intel-xml-report2.covmapping
// ./intel-xml-report2
// llvm-profdata merge -o intel-xml-report2.profdata *.profraw

extern int printf(const char*, ...);
extern void exit(int);

#define assert(expr) \
  if (!(expr)) { \
    printf("Fatail error\n"); \
    exit(-1); \
  }

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
