//RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-unknown-linux-gnu | FileCheck %s
//RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple x86_64-pc-windows-msvc | FileCheck %s
//RUN: %clang_cc1 -emit-llvm -o - %s -std=c++14 -fopenmp -fintel-compatibility -fintel-openmp-region -triple i386-pc-windows-msvc | FileCheck %s

struct NP {
  NP();
  NP(const NP&);
  NP& operator=(const NP&);
  ~NP();
};

struct A {
  int data_of_A;
  int other_part_of_A;
  NP non_pod_of_A;
  int func_of_A();
};

//CHECK: define {{.*}}func_of_A
int A::func_of_A() {
//CHECK: [[THISADDR:%this.*]] = alloca %struct.A*,
//CHECK: [[THIS1:%this.*]] = load %struct.A*, %struct.A** [[THISADDR]],
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: [[NP:%non_pod_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 2
//CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
//CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[DA]])
//CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD"(%struct.NP* [[NP]],
//CHECK: store i32 13, i32* [[DA]],
//CHECK: region.exit{{.*}}[ "DIR.OMP.END.PARALLEL"() ]
  #pragma omp parallel private(data_of_A,non_pod_of_A)
  {
    data_of_A = 13;
  }
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: [[NP:%non_pod_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 2
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: [[NP:%non_pod_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 2
//CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
//CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[DA]])
//CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%struct.NP* [[NP]],
//CHECK: store i32 14, i32* [[DA]],
//CHECK: region.exit{{.*}}[ "DIR.OMP.END.PARALLEL"() ]
  #pragma omp parallel firstprivate(data_of_A,non_pod_of_A)
  {
    data_of_A = 14;
  }
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: region.entry() [ "DIR.OMP.PARALLEL"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(i32* [[DA]])
//CHECK: store i32 16, i32* [[DA]],
//CHECK: region.exit{{.*}}[ "DIR.OMP.END.PARALLEL"() ]
  #pragma omp parallel reduction(+:data_of_A)
  {
    data_of_A = 16;
  }

//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: [[NP:%non_pod_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 2
//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.PRIVATE"(i32* [[DA]])
//CHECK-SAME: "QUAL.OMP.PRIVATE:NONPOD"(%struct.NP* [[NP]],
//CHECK: store i32 23, i32* [[DA]],
//CHECK: region.exit{{.*}}[ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for private(data_of_A,non_pod_of_A)
  for (int i=0; i < 16; ++i) {
    if (i==10) data_of_A = 23;
  }
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: [[NP:%non_pod_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 2
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: [[NP:%non_pod_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 2
//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(i32* [[DA]])
//CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE:NONPOD"(%struct.NP* [[NP]],
//CHECK: store i32 24, i32* [[DA]],
//CHECK: region.exit{{.*}}[ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for firstprivate(data_of_A,non_pod_of_A)
  for (int i=0; i < 16; ++i) {
    if (i==10) data_of_A = 24;
  }
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: [[NP:%non_pod_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 2
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: [[NP:%non_pod_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 2
//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.LASTPRIVATE"(i32* [[DA]])
//CHECK-SAME: "QUAL.OMP.LASTPRIVATE:NONPOD"(%struct.NP* [[NP]],
//CHECK: store i32 25, i32* [[DA]],
//CHECK: region.exit{{.*}}[ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for lastprivate(data_of_A,non_pod_of_A)
  for (int i=0; i < 16; ++i) {
    if (i==10) data_of_A = 25;
  }
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: [[DA:%data_of_A.*]] = getelementptr {{.*}}[[THIS1]], i32 0, i32 0
//CHECK: region.entry() [ "DIR.OMP.PARALLEL.LOOP"()
//CHECK-SAME: "QUAL.OMP.REDUCTION.ADD"(i32* [[DA]])
//CHECK: store i32 26, i32* [[DA]],
//CHECK: region.exit{{.*}}[ "DIR.OMP.END.PARALLEL.LOOP"() ]
  #pragma omp parallel for reduction(+:data_of_A)
  for (int i=0; i < 16; ++i) {
    if (i==10) data_of_A = 26;
  }
  return 0;
}
