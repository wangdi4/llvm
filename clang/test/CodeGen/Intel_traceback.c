// Check module flags are correctly generated for -traceback

// RUN: %clang_cc1 -S -emit-llvm -traceback %s -o - | FileCheck %s --check-prefix=TRACEBACK

// RUN: %clang -S -emit-llvm -traceback -target x86_64 %s -o %t1
// RUN: FileCheck < %t1 %s --check-prefix=TRACEBACK
// RUN: FileCheck < %t1 %s --check-prefix=DEBUGINFO

// TRACEBACK: !"TraceBack", i32 1
// DEBUGINFO: !"Debug Info Version", i32 3

int main(void) {
  return 0;
}
