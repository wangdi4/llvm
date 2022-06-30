; RUN: opt -enable-new-pm=0 -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=CHECK %s

; Test src:
;
; int main() {
;   int a[10];
; #pragma omp taskwait depend(in : a)
; }

; CHECK: [[DEPNUMBYTES:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.kmp_depend_info, ptr %{{.*}}, i32 0, i32 1
; CHECK: store i64 40, ptr [[DEPNUMBYTES]]
; CHECK:  call void @__kmpc_omp_wait_deps(ptr @{{.*}}, i32 %{{.*}}, i32 1, ptr %{{.*}}, i32 0, ptr null)
; CHECK:  call void @__kmpc_omp_taskwait(ptr @{{.*}}, i32 %{{.*}})

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kmp_depend_info = type { i64, i64, i8 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %a = alloca [10 x i32], align 16
  %.dep.arr.addr = alloca [1 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  %0 = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %1 = ptrtoint ptr %a to i64
  %2 = getelementptr %struct.kmp_depend_info, ptr %0, i64 0
  %3 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 0
  store i64 %1, ptr %3, align 8
  %4 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 1
  store i64 40, ptr %4, align 8
  %5 = getelementptr inbounds %struct.kmp_depend_info, ptr %2, i32 0, i32 2
  store i8 1, ptr %5, align 8
  store i64 1, ptr %dep.counter.addr, align 8
  %6 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(),
    "QUAL.OMP.DEPARRAY"(i32 1, ptr %0) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %6) [ "DIR.OMP.END.TASKWAIT"() ]
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "approx-func-fp-math"="true" "frame-pointer"="all" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 51}
!2 = !{i32 7, !"uwtable", i32 2}
!3 = !{i32 7, !"frame-pointer", i32 2}
