// INTEL_CUSTOMIZATION
// Temporarily remove "-Wl,--gc-sections" from compiler command lines because
// this is causing errors with the version of ld.gold in the binutils we use.
// When our binutils is upgraded to at least version 2.37, this option can
// be added back.
// RUN: %clang_profgen -fdata-sections -ffunction-sections -fuse-ld=gold -o %t -O3 %s
// end INTEL_CUSTOMIZATION
// RUN: env LLVM_PROFILE_FILE=%t.profraw %run %t
// RUN: llvm-profdata merge -o %t.profdata %t.profraw
// RUN: %clang_profuse=%t.profdata -o - -S -emit-llvm %s | FileCheck %s

int begin(int i) {
  // CHECK: br i1 %{{.*}}, label %{{.*}}, label %{{.*}}, !prof ![[PD1:[0-9]+]]
  if (i)
    return 0;
  return 1;
}

int end(int i) {
  // CHECK: br i1 %{{.*}}, label %{{.*}}, label %{{.*}}, !prof ![[PD2:[0-9]+]]
  if (i)
    return 0;
  return 1;
}

int main(int argc, const char *argv[]) {
  begin(0);
  end(1);

  // CHECK: br i1 %{{.*}}, label %{{.*}}, label %{{.*}}, !prof ![[PD2:[0-9]+]]
  if (argc)
    return 0;
  return 1;
}

// CHECK: ![[PD1]] = !{!"branch_weights", i32 1, i32 2}
// CHECK: ![[PD2]] = !{!"branch_weights", i32 2, i32 1}
