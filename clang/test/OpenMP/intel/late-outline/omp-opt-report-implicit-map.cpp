// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fopenmp-version=50 -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc -opt-record-file %t-host.yaml
//
//
// RUN: %clang_cc1 -triple spir64 -fopenmp -fopenmp-version=50 \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o %t-ir.txt \
// RUN:  -opt-record-file %t-target.yaml
// RUN: FileCheck -check-prefix=TARG --input-file %t-target.yaml %s


extern "C" int printf(const char*,...);
struct S1 {
    int i;
    float f[50];
};
void xoo() {
  double B[10];
  int x;
  S1 s;
  auto fn = [&x]() { return x; };
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 47, Column: 3
    //TARG: Function:{{.*}}xoo
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' "B" has an implicit clause: "map(tofrom : B)" because "B" is a non-scalar variable referenced within the construct at line:[49:11]'
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 47, Column: 3
    //TARG: Function:{{.*}}xoo
    //TARG: Args:
    //TARG: ' "fn" has an implicit clause: "map(to : fn)" because "fn" is a non-scalar variable referenced within the construct at line:[50:11]'
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 47, Column: 3
    //TARG: Function:{{.*}}xoo
    //TARG: Args:
    //TARG: ' "x"(captured by lambda) has an implicit clause: "map(tofrom : x)" because "x" is captured in a lambda mapped on the construct, and is referenced at line:[24:15]'
  #pragma omp target
  {
    (void)B;
    (void)fn();
    (void)s.f;
  }
}

class A {
  public:
  int nlocal;
  int dt;
  void zoo(int *a) {

    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 73, Column: 5
    //TARG: Function:{{.*}}zoo
    //TARG: Args:
    //TARG: ' "nlocal" has an implicit clause: "map(tofrom : nlocal)" because field "nlocal" is a non-scalar variable referenced within the construct at line:[74:25]
    //TARG: Args:
    //TARG:  ' "dt" has an implicit clause: "map(tofrom : dt)" because field "dt"  is a non-scalar variable referenced within the construct at line:[75:7]'
    //TARG: Args:
    //TARG: ' "a" has an implicit clause: "map(tofrom : a[:0])" because "a" is a pointer variable referenced within the construct at line:[75:21]'

    #pragma omp target
    for (int i = 0; i < nlocal; i++)
      dt = dt + 1, *a = 10;
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 83, Column: 5
    //TARG: Function:{{.*}}zoo
    //TARG: Args:
    //TARG: ' "dt" has an implicit clause: "map(tofrom : dt)" because field "dt" is a non-scalar variable referenced within the construct at line:[84:7]'
    #pragma omp target
      dt = this->dt + 1;
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 93, Column: 5
    //TARG: Function:{{.*}}zoo
    //TARG: Args:
    //TARG: {{.*}}String:
    //TARG: ' "nlocal" has an implicit clause: "map(tofrom : nlocal)" because field "nlocal" is a non-scalar variable referenced within the construct at line:[94:27]'
    #pragma omp target map(a)
      for (int i = 0; i < nlocal; i++)
         *a = 10;
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 103, Column: 5
    //TARG: Function:{{.*}}zoo
    //TARG: Args:
    //TARG: ' "this" has an implicit clause: "map(tofrom : this[:1])" because "this" this keyword is referenced within the construct at line:[104:22]'
    #pragma omp target
       printf("%p\n",this);
    int m = 0;
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 113, Column: 5
    //TARG: Function:{{.*}}zoo
    //TARG: Args:
    //TARG: ' "m" has an implicit clause: "firstprivate(m)" because "m" is a scalar variable referenced within the construct at line:[114:8]'
    #pragma omp target
       m++;
  }
  A() {
    this->dt = 0; this->nlocal = 0;
    auto lambda = [&]( ){++this->dt;};
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 128, Column: 5
    //TARG: Function:{{.*}}_ZN1AC2Ev
    //TARG: Args:
    //TARG: ' "this" has an implicit clause: "map(tofrom : this[:1])" because "this" this keyword is referenced within the construct at line:[118:28]'
    //TARG: Args:
    //TARG: ' "lambda" has an implicit clause: "map(tofrom : lambda)" because "lambda" is a non-scalar variable referenced within the construct at line:[129:5]'
    #pragma omp target
    lambda();
  }
};

int main() {
  A obj;
  int *a;
  obj.zoo(a);
  xoo();
  return 0;
}
