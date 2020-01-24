// RUN: %clang_cc1 -triple spir64-unknown-unknown-sycldevice -disable-llvm-passes -fsycl-is-device -emit-llvm %s -o - | FileCheck %s

<<<<<<< HEAD
// INTEL_CUSTOMIZATION
// CHECK: br label %for.cond, !llvm.loop ![[MD_X:[0-9]+]]
// CHECK: br label %for.cond, !llvm.loop ![[MD_Y:[0-9]+]]
// end INTEL_CUSTOMIZATION
// CHECK: br label %for.cond, !llvm.loop ![[MD_A:[0-9]+]]
// CHECK: br label %for.cond, !llvm.loop ![[MD_B:[0-9]+]]
// CHECK: br label %for.cond, !llvm.loop ![[MD_C:[0-9]+]]
// CHECK: br label %for.cond2, !llvm.loop ![[MD_D:[0-9]+]]
// CHECK: br label %for.cond,  !llvm.loop ![[MD_E:[0-9]+]]
// CHECK: br label %for.cond2, !llvm.loop ![[MD_F:[0-9]+]]

// INTEL_CUSTOMIZATION
// CHECK: ![[MD_X]] = distinct !{![[MD_X]], ![[MD_ivdep_X:[0-9]+]]}
// CHECK-NEXT: ![[MD_ivdep_X]] = !{!"llvm.loop.ivdep.enable"}
void bar() {
  int a[10];
  [[intelfpga::ivdep]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

// CHECK: ![[MD_Y]] = distinct !{![[MD_Y]], ![[MD_ivdep_Y:[0-9]+]]}
// CHECK-NEXT: ![[MD_ivdep_Y]] = !{!"llvm.loop.ivdep.safelen", i32 2}
void car() {
  int a[10];
  [[intelfpga::ivdep(2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}
// end INTEL_CUSTOMIZATION

// CHECK: ![[MD_A]] = distinct !{![[MD_A]], ![[MD_ii:[0-9]+]]}
// CHECK-NEXT: ![[MD_ii]] = !{!"llvm.loop.ii.count", i32 2}
void goo() {
  int a[10];
  [[intelfpga::ii(2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

// CHECK: ![[MD_B]] = distinct !{![[MD_B]], ![[MD_max_concurrency:[0-9]+]]}
// CHECK-NEXT: ![[MD_max_concurrency]] = !{!"llvm.loop.max_concurrency.count", i32 2}
void zoo() {
  int a[10];
  [[intelfpga::max_concurrency(2)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

// CHECK: ![[MD_C]] = distinct !{![[MD_C]], ![[MD_ii_2:[0-9]+]]}
// CHECK-NEXT: ![[MD_ii_2]] = !{!"llvm.loop.ii.count", i32 4}
=======
// CHECK: br label %for.cond,  !llvm.loop ![[MD_II:[0-9]+]]
// CHECK: br label %for.cond2, !llvm.loop ![[MD_II_2:[0-9]+]]
// CHECK: br label %for.cond,  !llvm.loop ![[MD_MC:[0-9]+]]
// CHECK: br label %for.cond2, !llvm.loop ![[MD_MC_2:[0-9]+]]

>>>>>>> a91b0f5061247abb7e4b90eccda208de29f7000d
template <int A>
void ii() {
  int a[10];
  // CHECK: ![[MD_II]] = distinct !{![[MD_II]], ![[MD_ii_count:[0-9]+]]}
  // CHECK-NEXT: ![[MD_ii_count]] = !{!"llvm.loop.ii.count", i32 4}
  [[intelfpga::ii(A)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // CHECK: ![[MD_II_2]] = distinct !{![[MD_II_2]], ![[MD_ii_count_2:[0-9]+]]}
  // CHECK-NEXT: ![[MD_ii_count_2]] = !{!"llvm.loop.ii.count", i32 8}
  [[intelfpga::ii(8)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

template <int A>
void max_concurrency() {
  int a[10];
  // CHECK: ![[MD_MC]] = distinct !{![[MD_MC]], ![[MD_max_concurrency:[0-9]+]]}
  // CHECK-NEXT: ![[MD_max_concurrency]] = !{!"llvm.loop.max_concurrency.count", i32 0}
  [[intelfpga::max_concurrency(A)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
  // CHECK: ![[MD_MC_2]] = distinct !{![[MD_MC_2]], ![[MD_max_concurrency_2:[0-9]+]]}
  // CHECK-NEXT: ![[MD_max_concurrency_2]] = !{!"llvm.loop.max_concurrency.count", i32 4}
  [[intelfpga::max_concurrency(4)]]
  for (int i = 0; i != 10; ++i)
    a[i] = 0;
}

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel_single_task(Func kernelFunc) {
  kernelFunc();
}

int main() {
  kernel_single_task<class kernel_function>([]() {
<<<<<<< HEAD
// INTEL_CUSTOMIZATION
    bar();
    car();
// end INTEL_CUSTOMIZATION
    goo();
    zoo();
    boo<4>();
    foo<0>();
=======
    ii<4>();
    max_concurrency<0>();
>>>>>>> a91b0f5061247abb7e4b90eccda208de29f7000d
  });
  return 0;
}
