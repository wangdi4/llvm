// Test option that filters results based on filenames.

// Test filter files via a regex.
// RUN: llvm-cov report --ignore-filename-regex="(intel-xml-report1a.cpp|intel-xml-report1c.cpp)" --format=xml %S/Inputs/intel-xml-report1.covmapping --instr-profile %S/Inputs/intel-xml-report1.profdata | FileCheck %s

// CHECK: <PROJECT>
// CHECK:   <MODULE name="{{.*}}intel-xml-report1b.cpp">
// CHECK:     <FUNCTION name="bar()" freq="1">
// CHECK:       <BLOCKS total="1" covered="1" coverage="100.000000%">
// CHECK:       </BLOCKS>
// CHECK:     </FUNCTION>
// CHECK:     <FUNCTION name="func()" freq="0">
// CHECK:       <BLOCKS total="1" covered="0" coverage="0.000000%">
// CHECK:       </BLOCKS>
// CHECK:     </FUNCTION>
// CHECK:   </MODULE>
// CHECK: </PROJECT>

// Data generation done with the following commands to get files in Inputs
// directory:
// icpx -fcoverage-mapping -fprofile-instr-generate -mllvm -enable-name-compression=false intel-xml-report1a.cpp intel-xml-report1b.cpp intel-xml-report1c.cpp -o intel-xml-report1
// llvm-cov convert-for-testing intel-xml-report1 -o intel-xml-report1.covmapping
// ./intel-xml-report1
// llvm-profdata merge -o intel-xml-report1.profdata *.profraw

//----------------------------------
// Source for intel-xml-report1b.cpp
//----------------------------------
// extern int gVar;
//
// int bar() {
//     return ++gVar;
// }
//
// void func() {
//     gVar = 10;
// }

//----------------------------------
// Source for intel-xml-report1c.cpp
//----------------------------------
// extern int gVar;
//
// int foo(int count) {
//   if (count) {
//     return ++gVar;
//   }
//
//   return --gVar;
// }

// This file is intel-xml-report1a.cpp
int gVar = 0;

extern int bar();
extern void func();
extern int foo(int count);

int main(int argc, char *argv[]) {
  foo(argc);
  bar();
  return gVar;
}
