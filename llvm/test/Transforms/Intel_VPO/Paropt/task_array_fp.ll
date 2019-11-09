; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S < %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S < %s  | FileCheck %s
;
; Test src:
;
; #define N 1000000
; void bar() {
;     int  B[N];
; #pragma omp task firstprivate(B) // OK with private(B) or shared(B)
;     {
;         B[44] = 123;
;     }
; }

; Make sure that memcpy is used to copy the array into the task thunk.
; In the outlined task, we should not copy the whole array, but use the
; data directly from the thunk.

; Caller side
; CHECK: [[BPRIV:%[^ ]+]] = getelementptr {{.*}}struct.kmp_privates.t*{{.*}}i32 0, i32 0
; CHECK: [[BPRIVCAST:%.+]] = bitcast [1000000 x i32]* [[BPRIV]] to i8*
; CHECK: [[BCAST:%.+]] = bitcast {{.*}} %B to i8*
; CHECK: call void @llvm.memcpy{{.*}}align 4 [[BPRIVCAST]], i8* align 4 [[BCAST]], i64 4000000

; Task side
; CHECK: define{{.*}}TASK
; CHECK-NOT: fpriv
; CHECK-NOT: call{{.*}}memcpy

; ModuleID = 'intel-task-array.c'
source_filename = "intel-task-array.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @bar() #0 {
entry:
  %B = alloca [1000000 x i32], align 16
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.FIRSTPRIVATE"([1000000 x i32]* %B) ]
  %arrayidx = getelementptr inbounds [1000000 x i32], [1000000 x i32]* %B, i64 0, i64 44
  store i32 123, i32* %arrayidx, align 16
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 8.0.0"}
