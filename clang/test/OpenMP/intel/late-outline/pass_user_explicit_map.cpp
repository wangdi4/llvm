// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
//
// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN: -discard-value-names  -triple x86_64-unknown-linux-gnu %s |\
// RUN:  FileCheck %s -check-prefix CHECK-DISNAME
#pragma omp declare target
int * arr;
struct st {
  int zoo;
};
st *sta;
#pragma omp end declare target
//CHECK-LABEL: foo
void foo(int i) {
  arr[i] = i;
}
//CHECK-LABEL: xoo
void xoo(int i) {
  sta[i].zoo = i;
}
//CHECK-LABEL: bar
int bar(void)
{
  int len;
//CHECK: [[L0:%[0-9]+]] = load ptr, ptr @arr, align 8
//CHECK: [[AR:%arrayidx]] = getelementptr inbounds i32,
//CHECK: [[L3:%[0-9]+]] = mul nuw i64
//CHECK: [[T1:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
//CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @arr, ptr [[AR]], i64 [[L3]], i64 19
  #pragma omp target map(tofrom: arr[:len])
  for( int i = 0; i < 10; i++)
    foo(i);
//CHECK: region.exit{{.*}}DIR.OMP.END.TARGET
//CHECK: [[L8:%[0-9]+]] = load ptr, ptr @sta
//CHECK: [[AR1:%arrayidx1]] =  getelementptr inbounds %struct.st
//CHECK: [[L11:%[0-9]+]] = mul nuw i64
//CHECK: [[T1:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
//CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr @sta, ptr [[AR1]], i64 [[L11]], i64 19
  #pragma omp target map(tofrom: sta[:len])
  for( int i = 0; i < 10; i++)
    xoo(i);
//CHECK: region.exit{{.*}}DIR.OMP.END.TARGET
  return 0;
}

void specified_alignment()
{
  __attribute__((aligned(64))) int (*ptr)[10];
//CHECK-DISNAME: [[L1:%[0-9]+]] = alloca ptr, align 64
// Do not generate temp map
//CHECK-DISNAME-NOT: alloca ptr, align 64
//CHECK-DISNAME-NEXT: [[T1:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
//CHECK-DISNAME-SAME: "QUAL.OMP.IS_DEVICE_PTR:PTR_TO_PTR"(ptr [[L1]])
  #pragma omp target is_device_ptr(ptr)
  {
    ptr[0][0] = 41;
  }
}

// end INTEL_COLLAB
