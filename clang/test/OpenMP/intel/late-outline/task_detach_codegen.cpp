// INTEL_COLLAB
//
// RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-late-outline \
// RUN: -triple x86_64-unknown-linux-gnu -fopenmp-version=50 %s | FileCheck %s

typedef unsigned long omp_event_handle_t;

// CHECK-LABEL: @_Z3foov(
void foo() {
  // CHECK: [[EVT:%evt]] = alloca i64, align 8
  omp_event_handle_t evt;
  
  // CHECK-NEXT: [[TMP0:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.NUM_THREADS"(i32 2), "QUAL.OMP.PRIVATE:TYPED"(ptr [[EVT]], i64 0, i32 1) ]
  // CHECK-NEXT: [[TMP1:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.DETACH:TYPED"(ptr [[EVT]], i64 0, i32 1) ]
  #pragma omp parallel num_threads(2) private(evt)
  {
    #pragma omp task detach(evt) 
     {}
  }
  // CHECK-NEXT: call void @llvm.directive.region.exit(token [[TMP1]]) [ "DIR.OMP.END.TASK"() ]
  // CHECK-NEXT: call void @llvm.directive.region.exit(token [[TMP0]]) [ "DIR.OMP.END.PARALLEL"() ]
  
  // CHECK-NEXT: [[TMP2:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.DETACH:TYPED"(ptr [[EVT]], i64 0, i32 1) ]
  // CHECK-NEXT: [[TMP3:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKGROUP"() ]
  // CHECK-NEXT: [[TMP4:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"() ]
  #pragma omp task detach(evt)
  {
    #pragma omp taskgroup
    {
      #pragma omp task
       {}
    }
  }
  // CHECK-NEXT: call void @llvm.directive.region.exit(token [[TMP4]]) [ "DIR.OMP.END.TASK"() ]
  // CHECK-NEXT: call void @llvm.directive.region.exit(token [[TMP3]]) [ "DIR.OMP.END.TASKGROUP"() ]
  // CHECK-NEXT: call void @llvm.directive.region.exit(token [[TMP2]]) [ "DIR.OMP.END.TASK"() ]
}
// end INTEL_COLLAB
