// Test generating report into XML file format for function and
// block coverage.
//
// Test function coverage
// RUN: llvm-cov report --format=xml %S/Inputs/intel-xml-report.covmapping --instr-profile %S/Inputs/intel-xml-report.profdata | FileCheck %s

// Test with option to show details within the function.
// RUN: llvm-cov report --format=xml %S/Inputs/intel-xml-report.covmapping --instr-profile %S/Inputs/intel-xml-report.profdata --show-regions-in-xml | FileCheck -check-prefixes CHECK,BLOCK %s

// Data generation done with the following commands to get files in Inputs
// directory:
// icpx -fcoverage-mapping -fprofile-instr-generate -mllvm -enable-name-compression=false intel-xml-report.cpp -o intel-xml-report
// llvm-cov convert-for-testing intel-xml-report -o intel-xml-report.covmapping
// ./intel-xml-report
// llvm-profdata merge -o intel-xml-report.profdata *.profraw

// Note: Regular expressions are used for the line number checks, instead
// of the absolute line number reported in the report to facilate being
// able to recompile this test to produce a new data file when changing
// this comment header or run lines.

// CHECK: <PROJECT>
// CHECK:   <MODULE name="{{.*}}intel-xml-report.cpp">
// CHECK:     <FUNCTION name="foo(int)" freq="1">
// CHECK:       <BLOCKS total="4" covered="3" coverage="75.000000%">
// BLOCK:        <BLOCK line="{{.*}}" col="20" end_line="{{.*}}" end_col="2">
// BLOCK:          <INSTANCE id="1" freq="1">
// BLOCK:          </INSTANCE>
// BLOCK:        </BLOCK>
// BLOCK:        <BLOCK line="{{.*}}" col="7" end_line="{{.*}}" end_col="12">
// BLOCK:          <INSTANCE id="1" freq="1">
// BLOCK:          </INSTANCE>
// BLOCK:        </BLOCK>
// BLOCK:        <BLOCK line="{{.*}}" col="14" end_line="{{.*}}" end_col="4">
// BLOCK:          <INSTANCE id="1" freq="1">
// BLOCK:          </INSTANCE>
// BLOCK:        </BLOCK>
// BLOCK:        <BLOCK line="{{.*}}" col="3" end_line="{{.*}}" end_col="16">
// BLOCK:          <INSTANCE id="1" freq="0">
// BLOCK:          </INSTANCE>
// BLOCK:        </BLOCK>
// CHECK:       </BLOCKS>
// CHECK:     </FUNCTION>
// CHECK:     <FUNCTION name="bar()" freq="1">
// CHECK:       <BLOCKS total="1" covered="1" coverage="100.000000%">
// BLOCK:        <BLOCK line="{{.*}}" col="11" end_line="{{.*}}" end_col="2">
// BLOCK:          <INSTANCE id="1" freq="1">
// BLOCK:          </INSTANCE>
// BLOCK:        </BLOCK>
// CHECK:       </BLOCKS>
// CHECK:     </FUNCTION>
// CHECK:     <FUNCTION name="func()" freq="0">
// CHECK:       <BLOCKS total="1" covered="0" coverage="0.000000%">
// BLOCK:        <BLOCK line="{{.*}}" col="13" end_line="{{.*}}" end_col="2">
// BLOCK:          <INSTANCE id="1" freq="0">
// BLOCK:          </INSTANCE>
// BLOCK:        </BLOCK>
// CHECK:       </BLOCKS>
// CHECK:     </FUNCTION>
// CHECK:     <FUNCTION name="main" freq="1">
// CHECK:       <BLOCKS total="1" covered="1" coverage="100.000000%">
// BLOCK:        <BLOCK line="{{.*}}" col="34" end_line="{{.*}}" end_col="2">
// BLOCK:          <INSTANCE id="1" freq="1">
// BLOCK:          </INSTANCE>
// BLOCK:        </BLOCK>
// CHECK:       </BLOCKS>
// CHECK:     </FUNCTION>
// CHECK:   </MODULE>
// CHECK: </PROJECT>

int gVar = 0;

int foo(int count) {
  if (count) {
    return ++gVar;
  }

  return --gVar;
}

int bar() {
    return ++gVar;
}

void func() {
    gVar = 10;
}

int main(int argc, char *argv[]) {
  foo(argc);
  bar();
  return gVar;
}
