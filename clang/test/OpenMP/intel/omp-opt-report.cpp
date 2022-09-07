// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fopenmp-version=50 -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 \
// RUN:  -emit-llvm-bc %s -o %t-host.bc -opt-record-file %t-host.yaml
// RUN: FileCheck -check-prefix=HOST --input-file %t-host.yaml %s
//
// RUN: %clang_cc1 -triple spir64 -fopenmp -fopenmp-version=50 \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o %t-ir.txt \
// RUN:  -opt-record-file %t-target.yaml
// RUN: FileCheck -check-prefix=TARG --input-file %t-target.yaml %s

int y = 5;
void foo() {
  int i = 0;
  int x;
#pragma omp parallel
  {
    //HOST: !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 28, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic update
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic
    ++i;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 39, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic update
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic update
    ++i;

//HOST: --- !Passed
//HOST: Pass:{{.*}}openmp
//HOST: Name:{{.*}}Region
//HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
//HOST: Line: 50, Column: 1 }
//HOST: Function:{{.*}}foo
//HOST: Construct:{{.*}}atomic update
//HOST: String:{{.*}}construct handled by clang
#pragma omp atomic update seq_cst
    ++i;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 61, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic update
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic seq_cst update
    ++i;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 72, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic update
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic seq_cst
    ++i;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 83, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic capture
    i = ++x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 94, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic capture seq_cst
    i = ++x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 105, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic read
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic read
    i = x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 116, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic read
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic read seq_cst
    i = x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 127, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic write
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic write
    i = ++x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 138, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic write
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic write seq_cst
    i = x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 149, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic capture
    i = x++;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 160, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic read
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic read
    i = ++x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 171, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic write
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic write seq_cst
    i = 1;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 182, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic read
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic read seq_cst
    i = ++x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 193, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic seq_cst, capture
    i = ++x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 204, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic seq_cst, capture
    i = ++x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 215, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic relaxed capture
    i = ++x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 226, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic capture release
    {
      i = x;
      x++;
    }

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 240, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic capture acquire
    i = x++;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 251, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic acquire capture
    i = x++;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 262, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic capture acq_rel
    {
      i = x;
      x++;
    }

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 276, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic acq_rel capture
    {
      i = x;
      x++;
    }

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 290, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic capture relaxed
    i = ++x;

    //HOST: --- !Passed
    //HOST: Pass:{{.*}}openmp
    //HOST: Name:{{.*}}Region
    //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //HOST: Line: 301, Column: 1 }
    //HOST: Function:{{.*}}foo
    //HOST: Construct:{{.*}}atomic capture
    //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic release capture
    {
      i = x;
      x++;
    }
  }
}

void baz() {
  int j = 0;
  int x = 10;

  //HOST: --- !Passed
  //HOST: Pass:{{.*}}openmp
  //HOST: Name:{{.*}}Region
  //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
  //HOST: Line: 321, Column: 1 }
  //HOST: Function{{.*}}baz
  //HOST: Construct:{{.*}}atomic update
  //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic
  ++j;

  //HOST: --- !Passed
  //HOST: Pass:{{.*}}openmp
  //HOST: Name:{{.*}}Region
  //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
  //HOST: Line: 332, Column: 1 }
  //HOST: Function{{.*}}baz
  //HOST: Construct:{{.*}}atomic update
  //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic update
  ++j;

  //HOST: --- !Passed
  //HOST: Pass:{{.*}}openmp
  //HOST: Name:{{.*}}Region
  //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
  //HOST: Line: 343, Column: 1 }
  //HOST: Function{{.*}}baz
  //HOST: Construct:{{.*}}atomic
  //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic capture
  x = ++j;

  //HOST: --- !Passed
  //HOST: Pass:{{.*}}openmp
  //HOST: Name:{{.*}}Region
  //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
  //HOST: Line: 354, Column: 1 }
  //HOST: Function{{.*}}baz
  //HOST: Construct:{{.*}}atomic read
  //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic read
  j = ++x;

  //HOST: --- !Passed
  //HOST: Pass:{{.*}}openmp
  //HOST: Name:{{.*}}Region
  //HOST: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
  //HOST: Line: 365, Column: 1 }
  //HOST: Function{{.*}}baz
  //HOST: Construct:{{.*}}atomic write
  //HOST: String:{{.*}}construct handled by clang
#pragma omp atomic write
  j = ++x;
}

void bar() {
#pragma omp target parallel
  {

    //TARG: --- !Missed
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //TARG: Line: 381, Column: 13 }
    //TARG: Function:{{.*}}bar
    //TARG: Construct:{{.*}}task
    //TARG: String:{{.*}}unsupported construct ignored by clang
#pragma omp task
    baz();
    int k = 0;

    //TARG: --- !Passed
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //TARG: Line: 393, Column: 1 }
    //TARG: Function:{{.*}}bar
    //TARG: Construct:{{.*}}atomic update
    //TARG: String:{{.*}}construct handled by clang
#pragma omp atomic
    k++;
  }
}

void bat() {

#pragma omp target map(tofrom \
                       : y)
  {

    //TARG: --- !Missed
    //TARG: Pass:{{.*}}openmp
    //TARG: Name:{{.*}}Region
    //TARG: DebugLoc: { File: '{{.*}}omp-opt-report.cpp',
    //TARG: Line: 412, Column: 38 }
    //TARG: Function:{{.*}}bat
    //TARG: Construct:{{.*}}flush
    //TARG: String:{{.*}}unsupported construct ignored by clang
                         #pragma omp flush
  }
}

int main() {
  foo();
  bar();
  bat();
  return 0;
}
