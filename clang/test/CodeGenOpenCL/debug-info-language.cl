// RUN: %clang %s -o - -emit-llvm -g -S | FileCheck %s

// Validate the debug information describes *.cl files as DW_LANG_OpenCL.

// CHECK: !llvm.dbg.cu = !{[[CU:![0-9]+]]}
// CHECK: [[CU]] ={{.*}} !DICompileUnit(language: DW_LANG_OpenCL,{{.*}})

