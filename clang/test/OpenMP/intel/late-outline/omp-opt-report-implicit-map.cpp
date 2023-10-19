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
//
//
// RUN: %clang_cc1 -triple spir64 -fopenmp -fopenmp-version=50 \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o %t-ir.txt \
// RUN:  -mllvm -intel-opt-report=high -mllvm -intel-opt-report-file=stdout \
// RUN:  -mllvm -intel-opt-report-emitter=ir -O1 -mllvm -paropt=63 \
// RUN:  -emit-llvm-bc -disable-lifetime-markers \
// RUN:  -debug-info-kind=line-tables-only \
// RUN:   | FileCheck %s --strict-whitespace --check-prefixes=OPTREPORT
//
//
// RUN: %clang_cc1 -triple spir64 -fopenmp -fopenmp-version=50 \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o %t-ir.txt \
// RUN:  -mllvm -intel-opt-report=medium -mllvm -intel-opt-report-file=stdout \
// RUN:  -mllvm -intel-opt-report-emitter=ir -O1 -mllvm -paropt=63 \
// RUN:  -emit-llvm-bc -disable-lifetime-markers \
// RUN:  -debug-info-kind=line-tables-only \
// RUN:   | FileCheck %s --strict-whitespace --check-prefixes=OPTREPORT_MEDIUM

// These are high-verbosity remarks, so they should not show up at medium
// verbosity.
//OPTREPORT_MEDIUM-NOT: has an implicit clause

extern "C" int printf(const char*,...);
struct S1 {
    int i;
    float f[50];
};

void xoo() {
  double B[10];
  int x, j;
  S1 s;
  auto fn = [&x]() { return x; };
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 137, Column: 3
    //TARG: Function:{{.*}}xoo
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}B
    //TARG: '" has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'tofrom : B'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}it is a non-scalar variable referenced
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}139
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}11
    //TARG: ']'

    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 137, Column: 3
    //TARG: Function:{{.*}}xoo
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}fn
    //TARG: '" has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'to : fn'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}it is a non-scalar variable referenced
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}140
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}11
    //TARG: ']'

    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 137, Column: 3
    //TARG: Function:{{.*}}xoo
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}x
    //TARG: '" (captured by lambda) has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'tofrom : x'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}a lambda referenced inside the construct captures it
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}50
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}15
    //TARG: ']'

    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 137, Column: 3
    //TARG: Function:{{.*}}xoo
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}j
    //TARG: '" has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'tofrom : j'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}it is a non-scalar variable referenced
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}140
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}17
    //TARG: ']'

    //OPTREPORT: Global optimization report for : __omp_offloading_{{.*}}__Z3xoov_l137

    //OPTREPORT: OMP TARGET BEGIN at {{.*}}omp-opt-report-implicit-map.cpp (137, 3)
    //OPTREPORT:     remark #30012: "B" has an implicit clause "map(tofrom : B)" because it is a non-scalar variable referenced at line:[139:11]
    //OPTREPORT:     remark #30012: "fn" has an implicit clause "map(to : fn)" because it is a non-scalar variable referenced at line:[140:11]
    //OPTREPORT:     remark #30012: "x" (captured by lambda) has an implicit clause "map(tofrom : x)" because a lambda referenced inside the construct captures it at line:[50:15]
    //OPTREPORT:     remark #30012: "j" has an implicit clause "map(tofrom : j)" because it is a non-scalar variable referenced at line:[140:17]
    //OPTREPORT:     remark #30012: "s" has an implicit clause "map(tofrom : s)" because it is a non-scalar variable referenced at line:[141:11]
    //OPTREPORT: OMP TARGET END

  #pragma omp target reduction(+:j)
  {
    (void)B;
    (void)fn(); j = 1;
    (void)s.f;
  }
}

class A {
  public:
  int nlocal;
  int dt;

  A() {
    this->dt = 0; this->nlocal = 0;
    auto lambda = [&]( ){++this->dt;};

    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 180, Column: 5
    //TARG: Function:{{.*}}_ZN1AC2Ev
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}lambda
    //TARG: '" has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'tofrom : lambda'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}it is a non-scalar variable referenced
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}181
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}5
    //TARG: ']'

    //OPTREPORT: Global optimization report for : __omp_offloading_{{.*}}__ZN1AC1Ev_l180

    //OPTREPORT: OMP TARGET BEGIN at {{.*}}omp-opt-report-implicit-map.cpp (180, 5)
    //OPTREPORT:     remark #30012: "lambda" has an implicit clause "map(to : lambda)" because it is a non-scalar variable referenced at line:[181:5]
    //OPTREPORT:     remark #30012: "lambda" has an implicit clause "map(tofrom : lambda)" because it is a non-scalar variable referenced at line:[181:5]
    //OPTREPORT: OMP TARGET END

    #pragma omp target
    lambda();
  }

  void zoo(int *a) {

    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 241, Column: 5
    //TARG: Function:{{.*}}zoo
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}nlocal
    //TARG: '" has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'tofrom : nlocal'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}it is a non-scalar field referenced
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}242
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}25
    //TARG: ']'

    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}dt
    //TARG: '" has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'tofrom : dt'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}it is a non-scalar field referenced
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}243
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}7
    //TARG: ']'

    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}a
    //TARG: '" has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'tofrom : a[:0]'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}it is a pointer variable referenced
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}243
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}21
    //TARG: ']'

    //OPTREPORT: Global optimization report for : __omp_offloading_{{.*}}__ZN1A3zooEPi_l241

    //OPTREPORT: OMP TARGET BEGIN at {{.*}}omp-opt-report-implicit-map.cpp (241, 5)
    //OPTREPORT:     remark #30012: "nlocal" has an implicit clause "map(tofrom : nlocal)" because it is a non-scalar field referenced at line:[242:25]
    //OPTREPORT:     remark #30012: "dt" has an implicit clause "map(tofrom : dt)" because it is a non-scalar field referenced at line:[243:7]
    //OPTREPORT:     remark #30012: "a" has an implicit clause "map(tofrom : a[:0])" because it is a pointer variable referenced at line:[243:21]
    //OPTREPORT: OMP TARGET END

    #pragma omp target
    for (int i = 0; i < nlocal; i++)
      dt = dt + 1, *a = 10;

    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 270, Column: 5
    //TARG: Function:{{.*}}zoo
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}dt
    //TARG: '" has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'tofrom : dt'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}it is a non-scalar field referenced
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}271
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}7
    //TARG: ']'

    //OPTREPORT: Global optimization report for : __omp_offloading_{{.*}}__ZN1A3zooEPi_l270

    //OPTREPORT: OMP TARGET BEGIN at {{.*}}omp-opt-report-implicit-map.cpp (270, 5)
    //OPTREPORT:     remark #30012: "dt" has an implicit clause "map(tofrom : dt)" because it is a non-scalar field referenced at line:[271:7]
    //OPTREPORT: OMP TARGET END

    #pragma omp target
      dt = this->dt + 1;

    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 298, Column: 5
    //TARG: Function:{{.*}}zoo
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}nlocal
    //TARG: '" has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'tofrom : nlocal'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}it is a non-scalar field referenced
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}299
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}27
    //TARG: ']'

    //OPTREPORT: Global optimization report for : __omp_offloading_{{.*}}__ZN1A3zooEPi_l298

    //OPTREPORT: OMP TARGET BEGIN at {{.*}}omp-opt-report-implicit-map.cpp (298, 5)
    //OPTREPORT:     remark #30012: "nlocal" has an implicit clause "map(tofrom : nlocal)" because it is a non-scalar field referenced at line:[299:27]
    //OPTREPORT: OMP TARGET END

    #pragma omp target map(a)
      for (int i = 0; i < nlocal; i++)
         *a = 10;

    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 327, Column: 5
    //TARG: Function:{{.*}}zoo
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}this
    //TARG: '" has an implicit clause "map('
    //TARG: {{.*}}MapClause:{{.*}}'tofrom : this[:1]'
    //TARG: ')" because '
    //TARG: {{.*}}Reason:{{.*}}'"this" is referenced'
    //TARG: ' at line:['
    //TARG: {{.*}}Line:{{.*}}328
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}22
    //TARG: ']'

    //OPTREPORT: Global optimization report for : __omp_offloading_{{.*}}__ZN1A3zooEPi_l327

    //OPTREPORT: OMP TARGET BEGIN at {{.*}}omp-opt-report-implicit-map.cpp (327, 5)
    //OPTREPORT:     remark #30012: "this" has an implicit clause "map(tofrom : this[:1])" because "this" is referenced at line:[328:22]
    //OPTREPORT: OMP TARGET END

    #pragma omp target
       printf("%p\n",this);
    int m = 0;

    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report-implicit-map.cpp',
    //TARG: Line: 354, Column: 5
    //TARG: Function:{{.*}}zoo
    //TARG: Args:
    //TARG: {{.*}}Construct:{{.*}}target
    //TARG: ' construct: "'
    //TARG: {{.*}}Name:{{.*}}m
    //TARG: '" has an implicit clause "firstprivate('
    //TARG: {{.*}}FirstPrivateClause:{{.*}}m
    //TARG: ')" because it is a scalar variable referenced at line:['
    //TARG: {{.*}}Line:{{.*}}355
    //TARG: ':'
    //TARG: {{.*}}Column:{{.*}}8
    //TARG: ']'

    //OPTREPORT: Global optimization report for : __omp_offloading_{{.*}}__ZN1A3zooEPi_l354

    //OPTREPORT: OMP TARGET BEGIN at {{.*}}omp-opt-report-implicit-map.cpp (354, 5)
    //OPTREPORT:     remark #30013: "m" has an implicit clause "firstprivate(m)" because it is a scalar variable referenced at line:[355:8]
    //OPTREPORT: OMP TARGET END

    #pragma omp target
       m++;
  }
};

int main() {
  A obj;
  int *a;
  obj.zoo(a);
  xoo();
  return 0;
}
