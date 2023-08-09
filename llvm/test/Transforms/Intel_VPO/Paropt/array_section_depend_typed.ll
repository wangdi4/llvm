; RUN: opt -bugpoint-enable-legacy-pm -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck --check-prefix=CHECK %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck --check-prefix=CHECK %s

; Test src:
;
; int main() {
;   int a[10];
;   int n = 3;
; #pragma omp taskwait depend(in : a [n:n])
; }

; CHECK: [[START:%[0-9A-Za-z._]+]] = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 %{{.*}}
; CHECK: [[SIZE:%[0-9A-Za-z._]+]] = sub nuw i64 %{{.*}}, %{{.*}}
; CHECK: [[STARTCAST:%[0-9A-Za-z._]+]] = ptrtoint ptr [[START]] to i64
; CHECK: [[DEPBASEPTR:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.kmp_depend_info, ptr %{{.*}}, i32 0, i32 0
; CHECK: store i64 [[STARTCAST]], ptr [[DEPBASEPTR]]
; CHECK: [[DEPNUMBYTES:%[0-9A-Za-z._]+]] = getelementptr inbounds %struct.kmp_depend_info, ptr %{{.*}}, i32 0, i32 1
; CHECK: store i64 [[SIZE]], ptr [[DEPNUMBYTES]]
; CHECK:  call void @__kmpc_omp_wait_deps(ptr @{{.*}}, i32 %{{.*}}, i32 1, ptr %{{.*}}, i32 0, ptr null)
; CHECK-NOT: call void @__kmpc_omp_taskwait

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.kmp_depend_info = type { i64, i64, i8 }

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %a = alloca [10 x i32], align 16
  %n = alloca i32, align 4
  %.dep.arr.addr = alloca [1 x %struct.kmp_depend_info], align 8
  %dep.counter.addr = alloca i64, align 8
  store i32 3, ptr %n, align 4
  %0 = getelementptr inbounds [1 x %struct.kmp_depend_info], ptr %.dep.arr.addr, i64 0, i64 0
  %1 = load i32, ptr %n, align 4
  %2 = sext i32 %1 to i64
  %arrayidx = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 %2
  %3 = load i32, ptr %n, align 4
  %4 = sext i32 %3 to i64
  %5 = load i32, ptr %n, align 4
  %6 = sext i32 %5 to i64
  %lb_add_len = add nsw i64 %4, %6
  %idx_sub_1 = sub nsw i64 %lb_add_len, 1
  %arrayidx1 = getelementptr inbounds [10 x i32], ptr %a, i64 0, i64 %idx_sub_1
  %7 = getelementptr i32, ptr %arrayidx1, i32 1
  %8 = ptrtoint ptr %arrayidx to i64
  %9 = ptrtoint ptr %7 to i64
  %10 = sub nuw i64 %9, %8
  %11 = ptrtoint ptr %arrayidx to i64
  %12 = getelementptr %struct.kmp_depend_info, ptr %0, i64 0
  %13 = getelementptr inbounds %struct.kmp_depend_info, ptr %12, i32 0, i32 0
  store i64 %11, ptr %13, align 8
  %14 = getelementptr inbounds %struct.kmp_depend_info, ptr %12, i32 0, i32 1
  store i64 %10, ptr %14, align 8
  %15 = getelementptr inbounds %struct.kmp_depend_info, ptr %12, i32 0, i32 2
  store i8 1, ptr %15, align 8
  store i64 1, ptr %dep.counter.addr, align 8
  %16 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASKWAIT"(),
    "QUAL.OMP.DEPARRAY"(i32 1, ptr %0) ]
  fence acq_rel
  call void @llvm.directive.region.exit(token %16) [ "DIR.OMP.END.TASKWAIT"() ]
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
