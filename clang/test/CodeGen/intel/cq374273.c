// RUN: %clang_cc1 -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

int main(void) {
  int i = 0, x = 0;
  #pragma unroll = 2
  for (i = 0; i < 10; ++i)
    ++x;

  return 0;
}

// CHECK: !llvm.loop !{{.+}}
// CHECK: !{{.+}} = !{!"llvm.loop.unroll.count", i32 2}
