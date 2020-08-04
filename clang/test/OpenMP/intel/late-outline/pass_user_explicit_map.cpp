// INTEL_COLLAB
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s
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
//CHECK: [[L0:%[0-9]+]] = load i32*, i32** @arr, align 8
//CHECK: [[AR:%arrayidx]] = getelementptr inbounds i32,
//CHECK: [[L3:%[0-9]+]] = mul nuw i64
//CHECK: [[T1:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
//CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(i32** @arr, i32* [[AR]], i64 [[L3]], i64 51)
  #pragma omp target map(tofrom: arr[:len])
  for( int i = 0; i < 10; i++)
    foo(i);
//CHECK: region.exit{{.*}}DIR.OMP.END.TARGET
//CHECK: [[L8:%[0-9]+]] = load %struct.st*, %struct.st** @sta
//CHECK: [[AR1:%arrayidx1]] =  getelementptr inbounds %struct.st
//CHECK: [[L11:%[0-9]+]] = mul nuw i64
//CHECK: [[T1:%[0-9]+]] = {{.*}}region.entry{{.*}}DIR.OMP.TARGET
//CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(%struct.st** @sta, %struct.st* [[AR1]], i64 [[L11]], i64 51)
  #pragma omp target map(tofrom: sta[:len])
  for( int i = 0; i < 10; i++)
    xoo(i);
//CHECK: region.exit{{.*}}DIR.OMP.END.TARGET
  return 0;
}
// end INTEL_COLLAB
